#include "elf.h"

// Encouraged to read ELF specifications
// Specifically https://gabi.xinuos.com/elf.pdf

static _Bool is_section_num_valid(const Elf64_Ehdr *file, const size_t FILE_SIZE) {
    if (file->e_shnum != 0) {
        return 1;
    }
    size_t sh_end = 0;
    if (__builtin_add_overflow(file->e_shoff, file->e_shentsize, &sh_end) == 1) {
        return 0;
    }
    if (sh_end > FILE_SIZE) {
        return 0;
    }
    Elf64_Shdr *shdr = (Elf64_Shdr*) ((char *) file + file->e_shoff);

    if (shdr->sh_size < 0xff00) {
        return 0;
    }
    return 1;
}

static KuroStatus is_valid_elf_hdr(const Elf64_Ehdr *file, const size_t FILE_SIZE) {
    if (FILE_SIZE < sizeof(Elf64_Ehdr)) {
        return ELF_INVALID_FILE_SIZE;
    }
    if (file->e_ident[EI_MAG0] != ELFMAG0 ||
        file->e_ident[EI_MAG1] != ELFMAG1 ||
        file->e_ident[EI_MAG2] != ELFMAG2 ||
        file->e_ident[EI_MAG3] != ELFMAG3) {
        return ELF_INVALID_HEADER;
    }
    if (file->e_ident[EI_CLASS] != ELFCLASS64) {
        return ELF_NOT_64_BIT;
    }
    if (file->e_ident[EI_DATA] != ELFDATA2LSB) {
        return ELF_UNSUPPORTED_ENDIAN;
    }
    if (file->e_ident[EI_VERSION] != EV_CURRENT || file->e_version != file->e_ident[EI_VERSION]) {
        return ELF_UNSUPPORTED_VERSION;
    }
    if (file->e_ehsize != sizeof(Elf64_Ehdr)) {
        return ELF_INVALID_HEADER;
    }
    if (file->e_type != ET_DYN) {
        return ELF_NOT_DYN;
    }
    if (file->e_machine != EM_X86_64) {
        return ELF_UNSUPPORTED_ARCH;
    }
    if (file->e_entry == 0) {
        return ELF_INVALID_ENTRY;
    }
    if (file->e_phoff == 0 || file->e_phentsize != sizeof(Elf64_Phdr) || file->e_phnum == 0) {
        return ELF_INVALID_HEADER;
    }
    if (file->e_shoff != 0) {
        if (is_section_num_valid(file, FILE_SIZE) == 0 || file->e_shentsize != sizeof(Elf64_Shdr)) {
            return ELF_INVALID_HEADER;
        }
    }

    size_t ph_size;
    size_t ph_end;
    if (__builtin_mul_overflow(file->e_phnum, sizeof(Elf64_Phdr), &ph_size) == 1) {
        return ELF_INVALID_HEADER;
    }
    if (__builtin_add_overflow(file->e_phoff, ph_size, &ph_end) == 1) {
        return ELF_INVALID_HEADER;
    }
    if (ph_end > FILE_SIZE) {
        return ELF_INVALID_HEADER;
    }
    return SUCCESS;
}

static KuroStatus parse_dyn(const unsigned char *file, const Elf64_Phdr *phdr, size_t FILE_SIZE, ExecPointOfInterest *out) {
    size_t file_end_dyn;
    if (__builtin_add_overflow(phdr->p_offset, phdr->p_filesz, &file_end_dyn) == 1) {
        return ELF_INVALID_PROGRAM_HEADER;
    }
    if (file_end_dyn > FILE_SIZE) {
        return ELF_INVALID_PROGRAM_HEADER;
    }

    Elf64_Dyn *dyn = (Elf64_Dyn*) ((char *) file + phdr->p_offset);

    _Bool is_rela = 0;
    Elf64_Dyn *rela = NULL;
    Elf64_Dyn *relasz = NULL;
    Elf64_Dyn *relaent = NULL;

    _Bool is_rel = 0;
    Elf64_Dyn *rel = NULL;
    Elf64_Dyn *relent = NULL;
    Elf64_Dyn *relsz = NULL;

    _Bool is_relr = 0;
    Elf64_Dyn *relr = NULL;
    Elf64_Dyn *relrsz = NULL;
    Elf64_Dyn *relrnum = NULL;

    out->symtab = NULL;
    out->symtab_shndx = NULL;

    while (dyn->d_tag != DT_NULL) {
        switch (dyn->d_tag) {
            case DT_NULL:
                goto done;
            case DT_SYMTAB:
                if (out->symtab != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                out->symtab = dyn;
                break;
            case DT_RELA:
                if (rela != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                rela = dyn;
                is_rela = 1;
                break;
            case DT_RELASZ:
                if (relasz != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relasz = dyn;
                is_rela = 1;
                break;
            case DT_RELAENT:
                if (relaent != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relaent = dyn;
                is_rela = 1;
            case DT_REL:
                if (rel != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                rel = dyn;
                is_rel = 1;
                break;
            case DT_RELSZ:
                if (relsz != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relsz = dyn;
                is_rel = 1;
                break;
            case DT_RELENT:
                if (relent != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relent = dyn;
                is_rel = 1;
                break;
            case DT_SYMTAB_SHNDX:
                if (out->symtab_shndx != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                out->symtab_shndx = dyn;
                break;
            case DT_RELRSZ:
                if (relrsz != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relrsz = dyn;
                is_relr = 1;
                break;
            case DT_RELR:
                if (relr != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relr = dyn;
                is_relr = 1;
                break;
            case DT_RELRENT:
                if (relrnum != NULL) {
                    return ELF_INVALID_DYN_SECTION;
                }
                relrnum = dyn;
                is_relr = 1;
                break;
            default:
                break;
        }
        dyn++;
    }

    done:

    if (out->symtab == NULL) {
        return ELF_INVALID_DYN_SECTION;
    }
    if (is_rela == 1) {
        if (rela == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
        if (relasz == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
        if (relaent == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
    }
    if (is_rel == 1) {
        if (rel == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
        if (relsz == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
        if (relent == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
    }
    if (is_relr == 1) {
        if (relr == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
        if (relrsz == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
        if (relrnum == NULL) {
            return ELF_INVALID_DYN_SECTION;
        }
    }

    return SUCCESS;
}

KuroStatus parse_elf(const unsigned char *file, size_t FILE_SIZE, ExecPointOfInterest *out) {
    const KuroStatus IS_HDR_VALID = is_valid_elf_hdr((const Elf64_Ehdr*) file, FILE_SIZE);
    if (IS_HDR_VALID != SUCCESS) {
        return IS_HDR_VALID;
    }
    size_t mem_start = SIZE_MAX;
    size_t mem_end = 0;
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)file;
    _Bool saw_dyn = 0;

    out->phdr_load_start = 0;
    out->phdr_load_num = 0;
    out->is_dyn = 0;

    for (size_t i = 0; i < ehdr->e_phnum; ++i) {
        const Elf64_Phdr *phdr = (const Elf64_Phdr*) (file + ehdr->e_phoff + i * ehdr->e_phentsize);

        switch (phdr->p_type) {
            case PT_DYNAMIC:;
                if (out->is_dyn == 1) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                out->is_dyn = 1;
                const KuroStatus DYN_STATUS = parse_dyn(file, phdr, FILE_SIZE, out);
                if (DYN_STATUS != SUCCESS) {
                    return DYN_STATUS;
                }
                break;
            case PT_LOAD:
                if ((phdr->p_flags & (PF_W | PF_X)) == (PF_W | PF_X)) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                if (phdr->p_memsz == 0) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                if (phdr->p_filesz > phdr->p_memsz) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                if (phdr->p_align != 0 &&
                    phdr->p_vaddr % phdr->p_align != phdr->p_offset % phdr->p_align) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                if (phdr->p_align != 0x1000 && phdr->p_align != 0 && phdr->p_align != 1) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }

                if (mem_start > phdr->p_vaddr) {
                    mem_start = phdr->p_vaddr;
                }

                size_t file_end;
                if (__builtin_add_overflow(phdr->p_offset, phdr->p_filesz, &file_end) == 1) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                if (file_end > FILE_SIZE) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }

                size_t mem_size;
                if (__builtin_add_overflow(phdr->p_vaddr, phdr->p_memsz, &mem_size) == 1) {
                    return ELF_INVALID_PROGRAM_HEADER;
                }
                if (mem_end < mem_size) {
                    mem_end = mem_size;
                }
                if (out->phdr_load_start == 0) {
                    out->phdr_load_start = (Elf64_Phdr*) phdr;
                }
                out->phdr_load_num++;
                break;
            default:
                break;
        }
    }
    if (mem_start == SIZE_MAX || mem_end == 0) {
        return ELF_INVALID_PROGRAM_HEADER;
    }
    out->mem_start = mem_start;
    out->mem_end = mem_end;
    return SUCCESS;
}