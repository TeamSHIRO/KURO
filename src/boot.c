#include "boot.h"

#include "aslr.h"
#include "conf.h"
#include "string.h"
#include "kuro.h"
#include "efi.h"
#include "efi_helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"
#include "status.h"

void set_ident(KuroIdentifier *ident) {
    static const char G_MAGIC[] = KURO_MAGIC;
    memcpy(ident->k_magic, G_MAGIC, KURO_MAGIC_LEN);
    ident->k_version = KURO_VERSION_1;
    ident->k_reserved[0] = 0;
    ident->k_reserved[1] = 0;
}

static EFI_STATUS exit_boot(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, MemoryMap *memory_map) {
    k_info(system_table, L"Exiting UEFI Boot Services...\r\n");
    fini_log_file(system_table);
    // TODO: mono - Actually do that
    return EFI_SUCCESS;
}

ErrorStatus boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, const KuroConfigInternal *config) {
    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS VOLUME_OPEN_STATUS = cached_volume_open(image_handle, system_table, &file_protocol);
    if (VOLUME_OPEN_STATUS != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = VOLUME_OPEN_STATUS, .status = SYSTEM_CANNOT_OPEN_VOLUME};
        return error;
    }

    EFI_FILE_PROTOCOL *file;

    CHAR16 *exec_path = NULL;
    size_t exec_path_len = strlen(config->exec_path);
    EFI_STATUS status = system_table->BootServices->AllocatePool(EfiLoaderData, (exec_path_len + 1) * sizeof(CHAR16), (void **) &exec_path);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_OUT_OF_MEMORY
        };
    }
    to_wchar(config->exec_path, exec_path, exec_path_len + 1);

    CHAR16 *mod_path = NULL;
    size_t mod_path_len;
    if (config->module_path != NULL) {
        mod_path_len = strlen(config->module_path);
        status = system_table->BootServices->AllocatePool(EfiLoaderData, (mod_path_len + 1) * sizeof(CHAR16), (void **) &mod_path);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(exec_path);
            file_protocol->Close(file_protocol);
            return (ErrorStatus) {
                .error_code = status,
                .status = SYSTEM_OUT_OF_MEMORY
            };
        }
        to_wchar(config->module_path, mod_path, mod_path_len + 1);
    }

    char *cmd_arg = NULL;
    size_t cmd_len;
    if (config->cmd_arg != NULL) {
        cmd_len = strlen(config->cmd_arg);
        status = system_table->BootServices->AllocatePool(EfiLoaderData, (cmd_len + 1) * sizeof(char), (void **) &cmd_arg);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(exec_path);
            system_table->BootServices->FreePool(mod_path);
            file_protocol->Close(file_protocol);
            return (ErrorStatus) {
                .error_code = status,
                .status = SYSTEM_OUT_OF_MEMORY
            };
        }
        strncpy(cmd_arg, config->cmd_arg, cmd_len + 1);
    }
    if (config->free == 1) {
        system_table->BootServices->FreePool(config->exec_path);
    }

    k_br(system_table, KURO_LOG_LEVEL_INFO);
    k_info_str(system_table, L"Executable path", exec_path);

    if (mod_path != NULL) {
        k_info_str(system_table, L"Module path", mod_path);
    } else {
        k_info(system_table, L"No module provided\r\n");
    }

    if (cmd_arg != NULL) {
        CHAR16 *w_cmd_arg = NULL;
        status = system_table->BootServices->AllocatePool(EfiLoaderData, (cmd_len + 1) * sizeof(CHAR16), (void **) &w_cmd_arg);
        if (status != EFI_SUCCESS) {
            system_table->BootServices->FreePool(exec_path);
            system_table->BootServices->FreePool(mod_path);
            system_table->BootServices->FreePool(cmd_arg);
            file_protocol->Close(file_protocol);
            return (ErrorStatus) {
                .error_code = status,
                .status = SYSTEM_OUT_OF_MEMORY
            };
        }
        to_wchar(cmd_arg, w_cmd_arg, cmd_len + 1);
        k_info_str(system_table, L"Command argument", w_cmd_arg);
        system_table->BootServices->FreePool(w_cmd_arg);
    } else {
        k_info(system_table, L"No command argument provided\r\n");
    }

    k_br(system_table, KURO_LOG_LEVEL_DEBUG);

    k_info(system_table, L"Reading and validating the executable...\r\n");

    status = file_protocol->Open(file_protocol, &file, exec_path, EFI_FILE_MODE_READ, 0);
    file_protocol->Close(file_protocol);
    system_table->BootServices->FreePool(exec_path);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = ELF_FILE_NOT_FOUND};
        return error;
    }

    UINTN file_size;
    status = get_file_size(system_table, file, &file_size);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = SYSTEM_CANNOT_READ_FILESIZE};
        file->Close(file);
        return error;
    }

    // ELF cannot realistically go under 128 if included KuroFooter. Could be more accurate but eh
    if (file_size < MINIMUM_ELF_SIZE) {
        file->Close(file);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = ELF_INVALID_FILE_SIZE};
        return error;
    }

    // TODO: mono - Verify KuroFooter

    Elf64_Ehdr ehdr;
    UINTN ehdr_size = sizeof(ehdr);

    status = file->Read(file, &ehdr_size, &ehdr);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = ELF_UNREADABLE};
        file->Close(file);
        return error;
    }

    status = is_valid_elf_header(&ehdr, file);
    if (status == CHECK_FAILED) {
        file->Close(file);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = ELF_INVALID_HEADER};
        return error;
    }

    size_t mem_size;
    size_t mem_start;

    status = get_elf_info(file, &ehdr, &mem_size, &mem_start);
    if (status != EFI_SUCCESS) {
        file->Close(file);
        ErrorStatus error = {
            .error_code = EFI_ERR(EFI_LOAD_ERROR),
            .status = ELF_INVALID_PROGRAM_HEADER,
        };
        return error;
    }

    if (mem_size == 0) {
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = ELF_INVALID_MEMORY_SIZE};
        return error;
    }

    k_success(system_table, L"This is a valid ELF file!\r\n");
    k_br(system_table, KURO_LOG_LEVEL_INFO);
    k_info(system_table, L"Starting boot procedure...\r\n");

    const size_t PROGRAM_SIZE_PAGE = (mem_size + 0xfff) / PAGE_SIZE;

    // bi as in boot info not bisexual *please laugh*
    const size_t PAGES_REQUIRED_NO_BI = PROGRAM_SIZE_PAGE + STACK_SIZE_PAGE;

    k_debug(system_table, L"Base page requirements:\r\n");
    k_debug_num(system_table, L"  Program", PROGRAM_SIZE_PAGE);
    k_debug_num(system_table, L"  Stack  ", STACK_SIZE_PAGE);
    k_debug_num(system_table, L"  Total  ", PAGES_REQUIRED_NO_BI);

    k_br(system_table, KURO_LOG_LEVEL_DEBUG);

    // TODO: mono - Get framebuffer and module

    MemoryMap memory_map;
    status = exit_boot(system_table, image_handle, &memory_map);
    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_EXIT_BOOT_SERVICES_FAILED
        };
    }

    EFI_PHYSICAL_ADDRESS addr;
    // TODO: mono - Boot info setup call here
    // status = alloc_prog(TOTAL_PAGE_NEEDED, config->aslr_enabled, &addr);
    // if (status != EFI_SUCCESS) {
    //     goto panic;
    // }

    // panic:
    while (1) {
        __asm__ volatile ("hlt");
    }
}
