#include "boot.h"

#include "efi.h"
#include "efi_helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"
#include "protocol/efi-gop.h"
#include "status.h"
#include "string.h"
#include "transfer_control.h"

const char *boot_id = "KURO";

static void set_ident(KuroIdentifier *ident) {
    ident->k_magic0 = 0x7f;
    ident->k_magic1 = 'K';
    ident->k_magic2 = 'U';
    ident->k_magic3 = 'R';
    ident->k_magic4 = 'O';
    ident->k_version = 1;
    ident->k_reserved = 0;
}

static void fetch_gop_highest_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINTN *mode_buffer,
                                   const EFI_SYSTEM_TABLE *system_table) {
    UINTN max_width = 0;
    UINTN max_height = 0;

    for (UINTN i = 0; i < gop->Mode->MaxMode; ++i) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
        UINTN info_size;
        if (gop->QueryMode(gop, i, &info_size, &info) == EFI_SUCCESS) {
            if ((UINTN) info->HorizontalResolution * (UINTN) info->VerticalResolution > max_width * max_height) {
                max_width = info->HorizontalResolution;
                max_height = info->VerticalResolution;
                *mode_buffer = i;
            }
            system_table->BootServices->FreePool(info);
        }
    }
}

// This function loads the executable into memory
// after calling, the `info` arg will point to KuroExecutableInfo. The segment field is heap-allocated, managed by the
// callee and automatically free on failure.
static ErrorStatus load_exec(const char *base_addr, EFI_FILE_PROTOCOL *file, const EFI_SYSTEM_TABLE *system_table,
                             const Elf64_Ehdr *ehdr, KuroExecutableInfo *info, const size_t alloc_size,
                             size_t start_mem) {
    const char *kernel_addr = (char *) base_addr + STACK_SIZE;

    // Minus 16 to prevent the same address of executable and stack conflicting with each other while keeping it 16
    // bytes aligned
    const char *stack_addr = (char *) base_addr + STACK_SIZE - 16;

    CHAR16 str[21];
    to_hex((size_t) kernel_addr, str);
    k_debug(system_table, (CHAR16 *) L"Kernel memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\r\n");
    to_hex((size_t) stack_addr, str);
    k_debug(system_table, (CHAR16 *) L"Stack memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\r\n");
    system_table->BootServices->SetMem((void *) base_addr, alloc_size, 0);

    Elf64_Phdr phdr;
    size_t phdr_size = sizeof(Elf64_Phdr);
    const size_t phdr_size_on_disk = ehdr->e_phentsize;

    set_ident(&info->ke_identifier);

    info->ke_entry_point = (size_t) kernel_addr + (ehdr->e_entry - start_mem);
    info->ke_stack_start = (size_t) stack_addr;
    info->ke_stack_end = (size_t) base_addr;
    info->ke_stack_size = STACK_SIZE - 16;

    info->ke_segment_count = ehdr->e_phnum;

    EFI_STATUS status = system_table->BootServices->AllocatePool(
            EfiLoaderData, sizeof(KuroSegmentInfo) * info->ke_segment_count, (void **) &info->ke_segments);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = System_AllocationFailed};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        phdr_size = sizeof(Elf64_Phdr);
        status = file->SetPosition((EFI_FILE_PROTOCOL *) file, ehdr->e_phoff + i * phdr_size_on_disk);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(info->ke_segments);
            ErrorStatus error = {.error_code = status, .status = Elf_FailedSetPos};
            k_error((EFI_SYSTEM_TABLE *) system_table, error);
            return error;
        }
        status = file->Read((EFI_FILE_PROTOCOL *) file, &phdr_size, &phdr);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(info->ke_segments);
            ErrorStatus error = {.error_code = status, .status = Elf_Unreadable};
            k_error((EFI_SYSTEM_TABLE *) system_table, error);
            return error;
        }

        KuroSegmentInfo *seg = &info->ke_segments[i];
        seg->ks_flags = phdr.p_flags;
        seg->ks_address = (uint64_t) kernel_addr + (phdr.p_vaddr - start_mem);
        seg->ks_size = phdr.p_memsz;
        seg->ks_align = phdr.p_align;

        if (phdr.p_type != PT_LOAD) {
            continue;
        }
        const char *load_addr = (const char *) seg->ks_address;
        const size_t file_size = phdr.p_filesz;

        status = file->SetPosition((EFI_FILE_PROTOCOL *) file, phdr.p_offset);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(info->ke_segments);
            ErrorStatus error = {.error_code = status, .status = Elf_FailedSetPos};
            k_error((EFI_SYSTEM_TABLE *) system_table, error);
            return error;
        }
        status = file->Read((EFI_FILE_PROTOCOL *) file, (UINTN *) &file_size, (void *) load_addr);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(info->ke_segments);
            ErrorStatus error = {.error_code = status, .status = Elf_Unreadable};
            k_error((EFI_SYSTEM_TABLE *) system_table, error);
            return error;
        }
    }

    return (ErrorStatus){.error_code = EFI_SUCCESS, .status = Success};
}

// Fetches the current memory map to obtain a valid map key, then calls ExitBootServices.
// If the map key is stale (firmware updated the map between the two calls), the loop
// retries automatically until ExitBootServices succeeds.
// On success, boot services are no longer available and the map buffer is intentionally
// not freed (it cannot be freed after ExitBootServices).
static ErrorStatus exit_boot_services(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    EFI_BOOT_SERVICES *bs = system_table->BootServices;
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_version = 0;
    EFI_STATUS status;

    for (;;) {
        if (map != NULL) {
            bs->FreePool(map);
            map = NULL;
        }
        map_size = 0;

        // First call: retrieves required buffer size (returns EFI_BUFFER_TOO_SMALL).
        bs->GetMemoryMap(&map_size, NULL, &map_key, &desc_size, &desc_version);

        // Add slack: the AllocatePool call below may itself introduce new map entries.
        map_size += 2 * desc_size;

        status = bs->AllocatePool(EfiLoaderData, map_size, (void **) &map);
        if (status != EFI_SUCCESS) {
            return (ErrorStatus){.error_code = status, .status = System_AllocationFailed};
        }

        // Second call: populate the map and obtain the current map key.
        status = bs->GetMemoryMap(&map_size, map, &map_key, &desc_size, &desc_version);
        if (status == EFI_BUFFER_TOO_SMALL) {
            // Slack was insufficient — retry with a larger allocation.
            continue;
        }
        if (status != EFI_SUCCESS) {
            bs->FreePool(map);
            return (ErrorStatus){.error_code = status, .status = System_AllocationFailed};
        }

        // ExitBootServices returns EFI_INVALID_PARAMETER if the map key is already stale.
        // Boot services remain fully available in that case, so it is safe to loop.
        status = bs->ExitBootServices(image_handle, map_key);
        if (status != EFI_SUCCESS) {
            // ExitBootServices failed for some other reason.
            bs->FreePool(map);
            return (ErrorStatus){.error_code = status, .status = System_ExitBootServicesFailed};
        }

        return (ErrorStatus){.error_code = EFI_SUCCESS, .status = Success};
    }
}

ErrorStatus boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS volume_open_status = volume_open(image_handle, system_table, &file_protocol);
    if (volume_open_status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = volume_open_status, .status = System_CannotOpenVolume};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    EFI_FILE_PROTOCOL *file;

    // Hardcoded path, change once we have a configuration system going on.
    EFI_STATUS status = file_protocol->Open(file_protocol, &file, (CHAR16 *) L"\\kernel", EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = Elf_FileNotFound};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    Elf64_Ehdr ehdr;
    UINTN ehdr_size = sizeof(ehdr);

    status = file->Read(file, &ehdr_size, &ehdr);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = Elf_Unreadable};
        file_protocol->Close(file_protocol);
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    status = is_valid_elf_header(&ehdr, file);
    if (status == CHECK_FAILED) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = Elf_InvalidHeader};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    k_success(system_table, (CHAR16 *) L"This is a valid ELF file!\r\n");

    size_t mem_size;
    size_t mem_start;

    status = get_mem_info_and_verify(file, &ehdr, &mem_size, &mem_start);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = Elf_InvalidProgramHeader};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    k_success(system_table, (CHAR16 *) L"This ELF file contains a valid program header!\r\n");

    status = check_for_rel_section(&ehdr, system_table, file);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = Elf_ContainsRelocation};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    k_success(system_table, (CHAR16 *) L"This ELF file does not contain any relocation section!\r\n");
    k_info(system_table, (CHAR16 *) L"Booting...\r\n");

    if (mem_size == 0) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = Elf_InvalidMemorySize};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    const size_t total_page_needed = (mem_size + 0xfff) / PAGE_SIZE;

    EFI_PHYSICAL_ADDRESS addr = HIGH_ADDR;

    status = system_table->BootServices->AllocatePages(AllocateMaxAddress, EfiLoaderData,
                                                       total_page_needed + STACK_SIZE_PAGE, &addr);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {.error_code = status, .status = System_AllocationFailed};
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }
    CHAR16 str[HEX_BUFFER_SIZE];
    to_hex(addr, str);

    k_debug(system_table, (CHAR16 *) L"Allocated memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\n\r");

    KuroExecutableInfo executable_info;
    ErrorStatus errStatus;
    errStatus = load_exec((char *) addr, file, system_table, &ehdr, &executable_info,
                          (total_page_needed + STACK_SIZE_PAGE) * PAGE_SIZE, mem_start);
    if (errStatus.status != Success) {
        file_protocol->Close(file_protocol);
        return errStatus;
    }
    
    to_hex(executable_info.ke_entry_point, str);

    k_debug(system_table, (CHAR16 *) L"Jumping to: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\n\r");

    file_protocol->Close(file_protocol);

    errStatus = exit_boot_services(image_handle, system_table);
    if (errStatus.status != Success) {
        file_protocol->Close(file_protocol);
        return errStatus;
    }

    transfer_control(&executable_info, NULL, NULL, NULL, (char *) boot_id, executable_info.ke_stack_start,
                     executable_info.ke_entry_point);
}
