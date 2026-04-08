#include "elf.h"

#include "boot.h"
#include "string.h"

#include "efi_helper.h"
#include "protocol/efi-fp.h"

static Elf64_Xword get_section_num(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
    if (header->e_shnum != 0) {
        return header->e_shnum;
    }
    Elf64_Shdr shdr;
    size_t shdr_size = sizeof(Elf64_Shdr);
    if (file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff) != EFI_SUCCESS) {
        return CHECK_FAILED;
    }
    if (file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &shdr) != EFI_SUCCESS) {
        return CHECK_FAILED;
    }
    if (shdr.sh_size < 0xff00) {
        return CHECK_FAILED;
    }
    return shdr.sh_size;
}

static Elf64_Word get_strtab_index(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
    Elf64_Word str_index = header->e_shstrndx;
    if (str_index == SHN_UNDEF) {
        return CHECK_FAILED;
    }

    if (str_index >= 0xffff) {
        Elf64_Shdr shdr;
        size_t shdr_size = sizeof(Elf64_Shdr);
        if (file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff) != EFI_SUCCESS) {
            return CHECK_FAILED;
        }
        if (file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &shdr) != EFI_SUCCESS) {
            return CHECK_FAILED;
        }
        str_index = shdr.sh_link;
    }

    if (str_index == 0) {
        return CHECK_FAILED;
    }

    return str_index;
}

static int is_section_dyn(const Elf64_Ehdr *header, Elf64_Word strtab_index, const EFI_FILE_PROTOCOL *file,
                          const EFI_SYSTEM_TABLE *system_table, Elf64_Word index) {
    if (index == 0) {
        return CHECK_FAILED;
    }
    Elf64_Shdr strtab_shdr;
    size_t shdr_size = sizeof(Elf64_Shdr);

    EFI_STATUS status =
            file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff + strtab_index * header->e_shentsize);
    if (status != EFI_SUCCESS) {
        return CHECK_FAILED;
    }

    status = file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &strtab_shdr);
    if (status != EFI_SUCCESS) {
        return CHECK_FAILED;
    }

    if (strtab_shdr.sh_size == 0) {
        return CHECK_FAILED;
    }

    char *strtab;
    UINTN strtab_size = strtab_shdr.sh_size;
    status = system_table->BootServices->AllocatePool(EfiLoaderData, strtab_size, (void **) &strtab);
    if (status != EFI_SUCCESS) {
        return CHECK_FAILED;
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
    // Maximum scan length is 16 characters.
    for (size_t i = 0; name[i] != '\0' && index + i + 3 < strtab_size && i < 12; i++) {
        if (name[i] != '.') {
            continue;
        }
        if ((name[i + 1] == 'p' && name[i + 2] == 'l' && name[i + 3] == 't') ||
            (name[i + 1] == 'g' && name[i + 2] == 'o' && name[i + 3] == 't') ||
            (name[i + 1] == 'd' && name[i + 2] == 'y' && name[i + 3] == 'n')) {
            system_table->ConOut->OutputString(system_table->ConOut,
                                               L"This ELF file contains PLT, GOT, or DYNAMIC section: ");
            CHAR16 str[32];
            to_wchar(name, str, 32);
            system_table->ConOut->OutputString(system_table->ConOut, str);
            system_table->ConOut->OutputString(system_table->ConOut, L"\r\nIgnoring...\r\n");
            goto pos;
        }
    }

error:
    system_table->BootServices->FreePool(strtab);
    return CHECK_FAILED;

pos:
    system_table->BootServices->FreePool(strtab);
    return CHECK_PASSED;
}

// You're welcome. I know this is very beautiful.
int is_valid_elf_header(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
    if (header->e_ident[0] == 0x7f && header->e_ident[1] == 'E' && header->e_ident[2] == 'L' &&
        header->e_ident[3] == 'F' && header->e_ident[4] == 2 && header->e_ident[5] == 1 && header->e_ident[6] == 1 &&

        header->e_type == ET_DYN && header->e_machine == EM_X86_64 && header->e_version == 1 && header->e_entry != 0 &&
        header->e_phoff != 0 && header->e_phentsize >= sizeof(Elf64_Phdr) && header->e_phnum != 0) {
        if (header->e_shoff != 0) {
            if (get_section_num(header, file) == 0 || header->e_shentsize < sizeof(Elf64_Shdr)) {
                return CHECK_FAILED;
            }
        }
        return CHECK_PASSED;
    }
    return CHECK_FAILED;
}

EFI_STATUS check_for_rel_section(const Elf64_Ehdr *header, const EFI_SYSTEM_TABLE *system_table,
                                 const EFI_FILE_PROTOCOL *file) {
    const size_t section_num = get_section_num(header, file);
    if (section_num == 0) {
        system_table->ConOut->OutputString(system_table->ConOut,
                                           L"This ELF file does not contain any section! Skipping...\r\n");
        return 1;
    }
    const Elf64_Word strtab_index = get_strtab_index(header, file);

    // Without string table, it's very unlikely for dynamic section to exist.
    if (strtab_index == 0) {
        system_table->ConOut->OutputString(system_table->ConOut,
                                           L"This ELF file does not contain any string table! Skipping...\r\n");
        return 1;
    }

    size_t shdr_size_on_disk = header->e_shentsize;

    for (int i = 0; i < section_num; i++) {
        Elf64_Shdr shdr;
        size_t shdr_size = sizeof(Elf64_Shdr);
        EFI_STATUS status = file->SetPosition((EFI_FILE_PROTOCOL *) file, header->e_shoff + i * shdr_size_on_disk);
        if (status != EFI_SUCCESS) {
            return status;
        }
        status = file->Read((EFI_FILE_PROTOCOL *) file, &shdr_size, &shdr);
        if (status != EFI_SUCCESS) {
            return status;
        }
        if (shdr.sh_type == SHT_REL || shdr.sh_type == SHT_RELA || shdr.sh_type == SHT_RELR) {
            // If there's string table then check if it contains dynamic linking keyword or not. If yes, ignore,
            // otherwise if not.
            if (strtab_index != 0) {
                if (is_section_dyn(header, strtab_index, file, system_table, shdr.sh_name) == CHECK_FAILED) {
                    return EFI_ERR(EFI_LOAD_ERROR);
                }
            } else {
                return EFI_ERR(EFI_LOAD_ERROR);
            }
        }
    }
    return EFI_SUCCESS;
}

// This is a combination of the old function get_mem_size and get_start_mem and also verify phdr
// This is made to optimize the read calls
EFI_STATUS get_mem_info_and_verify(const EFI_FILE_PROTOCOL *file, const Elf64_Ehdr *ehdr, size_t *out_mem_size,
                                   size_t *out_start_mem) {
    const Elf64_Half phnum = ehdr->e_phnum;
    Elf64_Phdr phdr;
    size_t phdr_size = sizeof(Elf64_Phdr);
    size_t phdr_size_on_disk = ehdr->e_phentsize;
    size_t start_mem = -1;
    size_t end_mem = 0;
    for (size_t i = 0; i < phnum; i++) {
        EFI_STATUS status = file->SetPosition((EFI_FILE_PROTOCOL *) file, ehdr->e_phoff + i * phdr_size_on_disk);
        if (status != EFI_SUCCESS) {
            return status;
        }
        phdr_size = sizeof(Elf64_Phdr);
        status = file->Read((EFI_FILE_PROTOCOL *) file, &phdr_size, &phdr);
        if (status != EFI_SUCCESS) {
            return status;
        }
        if (phdr.p_type != PT_LOAD) {
            continue;
        }

        if (phdr.p_filesz > phdr.p_memsz) {
            return EFI_ERR(EFI_LOAD_ERROR);
        }

        if (phdr.p_vaddr % phdr.p_align != phdr.p_offset % phdr.p_align) {
            return EFI_ERR(EFI_LOAD_ERROR);
        }

        // Currently, we only support p_align equal or lesser (power of two) than 0x1000, which is 4096 bytes page size
        if (phdr.p_align % PAGE_SIZE != 0) {
            return EFI_ERR(EFI_LOAD_ERROR);
        }

        if (start_mem > phdr.p_vaddr) {
            start_mem = phdr.p_vaddr;
        }

        if (end_mem < phdr.p_vaddr + phdr.p_memsz) {
            end_mem = phdr.p_vaddr + phdr.p_memsz;
        }
    }

    *out_start_mem = start_mem;
    *out_mem_size = end_mem - start_mem;
    return EFI_SUCCESS;
}
