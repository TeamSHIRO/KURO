#include "elf.h"

#include <stddef.h>

#include "protocol/efi-fp.h"
#include "string.h"

Elf64_Xword get_section_num(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
    if (header->e_shnum != 0) {
        return header->e_shnum;
    }
    Elf64_Shdr shdr;
    size_t shdr_size = sizeof(Elf64_Shdr);
    if (file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff) != EFI_SUCCESS) {
        return 0;
    }
    if (file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &shdr) != EFI_SUCCESS) {
        return 0;
    }
    if (shdr.sh_size < 0xff00) {
        return 0;
    }
    return shdr.sh_size;
}

Elf64_Word get_strtab_index(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
    Elf64_Word str_index = header->e_shstrndx;
    if (str_index == SHN_UNDEF) {
        return 0;
    }

    if (str_index >= 0xffff) {
        Elf64_Shdr shdr;
        size_t shdr_size = sizeof(Elf64_Shdr);
        if (file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff) != EFI_SUCCESS) {
            return 0;
        }
        if (file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &shdr) != EFI_SUCCESS) {
            return 0;
        }
        str_index = shdr.sh_link;
    }

    if (str_index == 0) {
        return 0;
    }

    return str_index;
}

int str_contains_plt_got_dyn(const Elf64_Ehdr *header, Elf64_Word strtab_index, const EFI_FILE_PROTOCOL *file,
                             const EFI_SYSTEM_TABLE *system_table, Elf64_Word index) {
    if (file == 0 || system_table->BootServices == 0) {
        return 0;
    }

    Elf64_Shdr strtab_shdr;
    size_t shdr_size = sizeof(Elf64_Shdr);

    EFI_STATUS status =
            file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff + strtab_index * header->e_shentsize);
    if (status != EFI_SUCCESS) {
        return 0;
    }

    status = file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &strtab_shdr);
    if (status != EFI_SUCCESS) {
        return 0;
    }

    if (strtab_shdr.sh_size == 0) {
        return 0;
    }

    char *strtab;
    UINTN strtab_size = strtab_shdr.sh_size;
    status = system_table->BootServices->AllocatePool(EfiLoaderData, strtab_size, (void **) &strtab);
    if (status != EFI_SUCCESS) {
        return 0;
    }

    status = file->SetPosition((EFI_FILE_PROTOCOL *) file, strtab_shdr.sh_offset);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    strtab_size = strtab_shdr.sh_size;

    status = file->Read((EFI_FILE_PROTOCOL *) file, &strtab_size, strtab);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    if (index >= strtab_size) {
        goto error;
    }

    const char *name = strtab + index;
    for (size_t i = 0; name[i] != '\0' && index + i + 2 < strtab_size; i++) {
        if ((name[i] == 'p' && name[i + 1] == 'l' && name[i + 2] == 't') ||
            (name[i] == 'g' && name[i + 1] == 'o' && name[i + 2] == 't') ||
            (name[i] == 'd' && name[i + 1] == 'y' && name[i + 2] == 'n')) {
            system_table->ConOut->OutputString(system_table->ConOut,
                                               L"This ELF file contains PLT, GOT, or DYNAMIC section: ");
            CHAR16 str[32];
            wchar(name, str, 32);
            system_table->ConOut->OutputString(system_table->ConOut, str);
            system_table->ConOut->OutputString(system_table->ConOut, L"\r\nIgnoring...\r\n");
            goto pos;
        }
    }

error:
    system_table->BootServices->FreePool(strtab);
    return 0;

pos:
    system_table->BootServices->FreePool(strtab);
    return 1;
}

// You're welcome. I know this is very beautiful.
int is_valid_elf_header(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
    if (header->e_ident[0] == 0x7f && header->e_ident[1] == 'E' && header->e_ident[2] == 'L' &&
        header->e_ident[3] == 'F' && header->e_ident[4] == 2 && header->e_ident[5] == 1 && header->e_ident[6] == 1 &&

        header->e_type == ET_DYN && header->e_machine == 62 && // EM_X86_64
        header->e_version == 1 && header->e_entry != 0 && header->e_phoff != 0 &&
        header->e_phentsize >= sizeof(Elf64_Phdr) && header->e_phnum != 0) {
        if (header->e_shoff != 0) {
            if (get_section_num(header, file) == 0 || header->e_shentsize < sizeof(Elf64_Shdr)) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

int check_for_rel_section(const Elf64_Ehdr *header, const EFI_SYSTEM_TABLE *system_table,
                          const EFI_FILE_PROTOCOL *file) {
    const size_t section_num = get_section_num(header, file);
    if (section_num == 0) {
        system_table->ConOut->OutputString(system_table->ConOut,
                                           L"This ELF file does not contain any section! Skipping...\r\n");
        return 1;
    }
    const Elf64_Word strtab_index = get_strtab_index(header, file);
    if (strtab_index == 0) {
        system_table->ConOut->OutputString(system_table->ConOut,
                                           L"This ELF file does not contain any string table! Skipping...\r\n");
        return 1;
    }

    for (int i = 0; i < section_num; i++) {
        Elf64_Shdr shdr;
        size_t shdr_size = sizeof(Elf64_Shdr);
        if (file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff + i * header->e_shentsize) != EFI_SUCCESS) {
            return 0;
        }
        if (file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &shdr) != EFI_SUCCESS) {
            return 0;
        }
        if ((shdr.sh_type == SHT_REL || shdr.sh_type == SHT_RELA || shdr.sh_type == SHT_RELR) &&
            str_contains_plt_got_dyn(header, strtab_index, file, system_table, shdr.sh_name) == 0) {
            return 0;
        }
    }
    return 1;
}
