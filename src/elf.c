#include "elf.h"

#include "boot.h"
#include "string.h"

#include "efi_helper.h"
#include "protocol/efi-fp.h"

// Encouraged to read ELF specifications
// Specifically https://gabi.xinuos.com/elf

// TODO: Add filesize check!!!

static Elf64_Xword get_section_num(const Elf64_Ehdr *header, const EFI_FILE_PROTOCOL *file) {
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

// TODO: Also get segment info and rename to get_elf_info
// This is a combination of the old function get_mem_size and get_start_mem and also verify phdr
// This is made to optimize the read calls
EFI_STATUS get_mem_info_and_verify(const EFI_FILE_PROTOCOL *file, const Elf64_Ehdr *ehdr, size_t *out_mem_size,
                                   size_t *out_start_mem) {
    const Elf64_Half PHNUM = ehdr->e_phnum;
    Elf64_Phdr phdr;
    size_t phdr_size = sizeof(Elf64_Phdr);
    size_t phdr_size_on_disk = ehdr->e_phentsize;
    size_t start_mem = -1;
    size_t end_mem = 0;
    for (size_t i = 0; i < PHNUM; i++) {
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

        if ((phdr.p_flags & (PF_W | PF_X)) == (PF_W | PF_X)) {
            return EFI_ERR(EFI_LOAD_ERROR);
        }

        if (phdr.p_filesz > phdr.p_memsz) {
            return EFI_ERR(EFI_LOAD_ERROR);
        }

        if (phdr.p_vaddr % phdr.p_align != phdr.p_offset % phdr.p_align) {
            return EFI_ERR(EFI_LOAD_ERROR);
        }

        if (phdr.p_align % PAGE_SIZE != 0 && phdr.p_vaddr != 0 && phdr.p_vaddr != 1) {
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
