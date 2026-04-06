#include "boot.h"

#include "efi.h"
#include "efi_helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"
#include "string.h"
#include "transfer_control.h"

// TODO: TheMonHub - Refactor this mess too!

static EFI_STATUS get_mem_size(const EFI_FILE_PROTOCOL *file, const Elf64_Ehdr *ehdr, UINTN *output) {
    const size_t phnum = ehdr->e_phnum;
    EFI_STATUS status = EFI_SUCCESS;
    *output = 0;
    size_t end_mem = 0;
    size_t start_mem = -1;
    for (size_t i = 0; i < phnum; i++) {
        Elf64_Phdr phdr;
        size_t phdr_size = sizeof(Elf64_Phdr);
        status = file->SetPosition((EFI_FILE_PROTOCOL*) file, ehdr->e_phoff + i * ehdr->e_phentsize);
        if (status != EFI_SUCCESS) {
            return status;
        }
        status = file->Read((EFI_FILE_PROTOCOL*) file, &phdr_size, &phdr);
        if (status != EFI_SUCCESS) {
            return status;
        }
        if (phdr.p_type != PT_LOAD) {
            continue;
        }
        if (start_mem > phdr.p_vaddr) {
            start_mem = phdr.p_vaddr;
        }
        if (end_mem < phdr.p_vaddr + phdr.p_memsz) {
            end_mem = phdr.p_vaddr + phdr.p_memsz;
        }
    }
    *output = end_mem - start_mem;
    return status;
}

// This function loads the executable into memory
// after calling, the `info` arg will point to KuroExecutableInfo. This is heap-allocated. Free on failure.
static EFI_STATUS load_exec(const char *base_addr, EFI_FILE_PROTOCOL *file, const EFI_SYSTEM_TABLE *system_table,
                            const Elf64_Ehdr *ehdr, KuroExecutableInfo *info) {
    const char *kernel_addr = (char*) base_addr + STACK_SIZE;
    const char *stack_addr = (char*) base_addr + STACK_SIZE - 16;

    CHAR16 str[21];
    to_hex((size_t) kernel_addr, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"Kernel memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"\r\n");
    to_hex((size_t) stack_addr, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"Stack memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"\r\n");

    Elf64_Phdr phdr;
    size_t phdr_size = sizeof(Elf64_Phdr);
    const size_t phdr_size_on_disk = ehdr->e_phentsize;
    EFI_STATUS status;

    size_t start_mem = -1;
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        phdr_size = sizeof(Elf64_Phdr);
        status = file->SetPosition((EFI_FILE_PROTOCOL*) file, ehdr->e_phoff + i * phdr_size_on_disk);
        if (status != EFI_SUCCESS) {
            goto error;
        }
        status = file->Read((EFI_FILE_PROTOCOL*) file, &phdr_size, &phdr);
        if (status != EFI_SUCCESS) {
            goto error;
        }
        if (phdr.p_type == PT_LOAD && start_mem > phdr.p_vaddr) {
            start_mem = phdr.p_vaddr;
        }
    }

    info->ke_identifier.k_magic0 = 0x7f;
    info->ke_identifier.k_magic1 = 'K';
    info->ke_identifier.k_magic2 = 'U';
    info->ke_identifier.k_magic3 = 'R';
    info->ke_identifier.k_magic4 = 'O';
    info->ke_identifier.k_version = 1;
    info->ke_identifier.k_reserved = 0;

    info->ke_entry_point = (size_t) kernel_addr + (ehdr->e_entry - start_mem);
    info->ke_stack_start = (size_t) stack_addr;
    info->ke_stack_end = (size_t) base_addr;
    info->ke_stack_size = STACK_SIZE - 16;

    info->ke_segment_count = ehdr->e_phnum;

    status = system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(KuroSegmentInfo) * info->ke_segment_count, (void**) &info->ke_segments);
    if (status != EFI_SUCCESS) {
        return status;
    }

    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        phdr_size = sizeof(Elf64_Phdr);
        status = file->SetPosition((EFI_FILE_PROTOCOL*) file, ehdr->e_phoff + i * phdr_size_on_disk);
        if (status != EFI_SUCCESS) {
            goto error;
        }
        status = file->Read((EFI_FILE_PROTOCOL*) file, &phdr_size, &phdr);
        if (status != EFI_SUCCESS) {
            goto error;
        }

        KuroSegmentInfo* seg = &info->ke_segments[i];
        seg->ks_flags = phdr.p_flags;
        seg->ks_address = (uint64_t) kernel_addr + (phdr.p_vaddr - start_mem);
        seg->ks_size = phdr.p_memsz;
        seg->ks_align = phdr.p_align;

        if (phdr.p_type != PT_LOAD) {
            continue;
        }
        const char* load_addr = (const char*) seg->ks_address;
        const size_t load_size = phdr.p_memsz;
        const char* file_addr;
        const size_t file_size = phdr.p_filesz;
        system_table->BootServices->AllocatePool(EfiLoaderData, file_size, (void**) &file_addr); // FIXME: TheMonHub - Reductant allocation
        status = file->SetPosition((EFI_FILE_PROTOCOL*) file, phdr.p_offset);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool((void*)file_addr);
            goto error;
        }
        status = file->Read((EFI_FILE_PROTOCOL*) file,(UINTN*) &file_size, (void*) file_addr);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool((void*)file_addr);
            goto error;
        }
        system_table->BootServices->CopyMem((void*)load_addr, (void*)file_addr, file_size);
        system_table->BootServices->SetMem((void*)(load_addr + file_size), load_size - file_size, 0);
        system_table->BootServices->FreePool((void*)file_addr);
    }

    return EFI_SUCCESS;

    error:
    system_table->BootServices->FreePool(info->ke_segments);
    return status;
}

EFI_STATUS boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS volume_open_status = volume_open(image_handle, system_table, &file_protocol);
    if (volume_open_status != EFI_SUCCESS) {
        return volume_open_status;
    }

    EFI_FILE_PROTOCOL *file;

    EFI_STATUS status = file_protocol->Open(file_protocol, &file, L"\\kernel", EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    Elf64_Ehdr ehdr;
    UINTN ehdr_size = sizeof(ehdr);

    status = file->Read(file, &ehdr_size, &ehdr);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    status = is_valid_elf_header(&ehdr, file);
    if (status == 0) {
        status = EFI_ERR(EFI_LOAD_ERROR);
        system_table->ConOut->OutputString(system_table->ConOut, L"This is NOT a valid ELF file!\r\n");
        goto error;
    }

    system_table->ConOut->OutputString(system_table->ConOut, L"This is a valid ELF file!\r\n");

    status = verify_phdr(&ehdr, file);
    if (status != EFI_SUCCESS) {
        status = EFI_ERR(EFI_LOAD_ERROR);
        system_table->ConOut->OutputString(system_table->ConOut, L"This ELF file contains invalid program header!\r\n");
        goto error;
    }

    system_table->ConOut->OutputString(system_table->ConOut, L"This ELF file contains valid program header!\r\n");

    status = check_for_rel_section(&ehdr, system_table, file);
    if (status != EFI_SUCCESS) {
        status = EFI_ERR(EFI_LOAD_ERROR);
        system_table->ConOut->OutputString(system_table->ConOut,
                                           L"This ELF file contains relocation section or has failed to check!\r\n");
        goto error;
    }

    system_table->ConOut->OutputString(system_table->ConOut,
                                       L"This ELF file does not contain any relocation section!\r\n");
    system_table->ConOut->OutputString(system_table->ConOut, L"Booting...\r\n");

    size_t mem_size;
    status = get_mem_size(file, &ehdr, &mem_size);
    if (status != EFI_SUCCESS) {
        goto error;
    }
    if (mem_size == 0) {
        status = EFI_ERR(EFI_LOAD_ERROR);
        goto error;
    }

    const size_t total_page_needed = (mem_size + 0xfff) / 0x1000;

    EFI_PHYSICAL_ADDRESS addr = 0xffffffffffffffff;

    status = system_table->BootServices->AllocatePages(
        AllocateMaxAddress,
        EfiLoaderData,
        total_page_needed + STACK_SIZE_PAGE,
        &addr
    );
    if (status != EFI_SUCCESS) {
        goto error;
    }
    CHAR16 str[21];
    to_hex(addr, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"Allocated memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"\r\n");

    KuroExecutableInfo *executable_info;
    status = system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(KuroExecutableInfo), (void**) &executable_info);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    status = load_exec((char*) addr, file, system_table, &ehdr, executable_info);
    if (status != EFI_SUCCESS) {
        goto error2;
    }

    status = system_table->BootServices->AllocatePool(EfiLoaderData, 5, (void**) &transfer_boot_id_addr);
    if (status != EFI_SUCCESS) {
        goto error2;
    }

    to_hex(executable_info->ke_entry_point, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"Jumping to: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, L"\r\n");

    transfer_exec_info = executable_info;
    transfer_image_handle = image_handle;
    transfer_system_table = (EFI_SYSTEM_TABLE*) system_table;
    transfer_boot_id_addr[0] = 'K';
    transfer_boot_id_addr[1] = 'U';
    transfer_boot_id_addr[2] = 'R';
    transfer_boot_id_addr[3] = 'O';
    transfer_boot_id_addr[4] = '\0';
    transfer_stack_start = executable_info->ke_stack_start;
    transfer_entry_point = executable_info->ke_entry_point;

    transfer_control();

    error2:
    system_table->BootServices->FreePool(executable_info);
error:
    file_protocol->Close(file_protocol);
    return status;
}
