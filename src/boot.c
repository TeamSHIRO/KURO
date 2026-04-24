#include "boot.h"

#include "efi.h"
#include "efi_helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"
#include "status.h"
#include "string.h"
#include "transfer_control.h"

const char *g_boot_id = "KURO";

static void set_ident(KuroIdentifier *ident) {
    ident->k_magic0 = 0x7f;
    ident->k_magic1 = 'K';
    ident->k_magic2 = 'U';
    ident->k_magic3 = 'R';
    ident->k_magic4 = 'O';
    ident->k_version = 1;
    ident->k_reserved = 0;
}

// This function loads the executable into memory
// after calling, the `info` arg will point to KuroExecutableInfo. The segment field is heap-allocated, managed by the
// callee and automatically free on failure.
static ErrorStatus load_exec(const char *base_addr, EFI_FILE_PROTOCOL *file, const EFI_SYSTEM_TABLE *system_table,
                             const Elf64_Ehdr *ehdr, KuroExecutableInfo *info, const size_t ALLOC_SIZE,
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
    system_table->BootServices->SetMem((void *) base_addr, ALLOC_SIZE, 0);

    Elf64_Phdr phdr;
    size_t phdr_size = sizeof(Elf64_Phdr);
    const size_t PHDR_SIZE_ON_DISK = ehdr->e_phentsize;

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
        status = file->SetPosition((EFI_FILE_PROTOCOL *) file, ehdr->e_phoff + i * PHDR_SIZE_ON_DISK);
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
        const size_t FILE_SIZE = phdr.p_filesz;

        status = file->SetPosition((EFI_FILE_PROTOCOL *) file, phdr.p_offset);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(info->ke_segments);
            ErrorStatus error = {.error_code = status, .status = Elf_FailedSetPos};
            k_error((EFI_SYSTEM_TABLE *) system_table, error);
            return error;
        }
        status = file->Read((EFI_FILE_PROTOCOL *) file, (UINTN *) &FILE_SIZE, (void *) load_addr);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(info->ke_segments);
            ErrorStatus error = {.error_code = status, .status = Elf_Unreadable};
            k_error((EFI_SYSTEM_TABLE *) system_table, error);
            return error;
        }
    }

    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = Success,
    };
}

ErrorStatus boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS VOLUME_OPEN_STATUS = volume_open(image_handle, system_table, &file_protocol);
    if (VOLUME_OPEN_STATUS != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = VOLUME_OPEN_STATUS, .status = System_CannotOpenVolume};
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
        ErrorStatus error = {
            .error_code = EFI_ERR(EFI_LOAD_ERROR),
            .status = Elf_InvalidProgramHeader
        };
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }

    k_success(system_table, (CHAR16 *) L"This ELF file contains a valid program header!\r\n");

    status = check_for_rel_section(&ehdr, system_table, file);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {
            .error_code = EFI_ERR(EFI_LOAD_ERROR),
            .status = Elf_ContainsRelocation
        };
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

    const size_t TOTAL_PAGE_NEEDED = (mem_size + 0xfff) / PAGE_SIZE;

    EFI_PHYSICAL_ADDRESS addr = HIGH_ADDR;

    status = system_table->BootServices->AllocatePages(AllocateMaxAddress, EfiLoaderData,
                                                       TOTAL_PAGE_NEEDED + STACK_SIZE_PAGE, &addr);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        ErrorStatus error = {
            .error_code = status,
            .status = System_AllocationFailed
        };
        k_error((EFI_SYSTEM_TABLE *) system_table, error);
        return error;
    }
    CHAR16 str[HEX_BUFFER_SIZE];
    to_hex(addr, str);

    k_debug(system_table, (CHAR16 *) L"Allocated memory at: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\n\r");

    KuroExecutableInfo executable_info;
    ErrorStatus err_status;
    err_status = load_exec((char *) addr, file, system_table, &ehdr, &executable_info,
                           (TOTAL_PAGE_NEEDED + STACK_SIZE_PAGE) * PAGE_SIZE, mem_start);
    if (err_status.status != Success) {
        file_protocol->Close(file_protocol);
        return err_status;
    }

    to_hex(executable_info.ke_entry_point, str);

    k_debug(system_table, (CHAR16 *) L"Jumping to: ");
    system_table->ConOut->OutputString(system_table->ConOut, str);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\n\r");

    file_protocol->Close(file_protocol);
    transfer_control(&executable_info, NULL, NULL, NULL, (char *) g_boot_id, executable_info.ke_stack_start,
                     executable_info.ke_entry_point);
}
