#include "boot.h"

#include "conf.h"
#include "string.h"
#include "efi.h"
#include "efi_helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"
#include "status.h"
#include "verify.h"

static ErrorStatus exit_boot(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, MemoryMap *memory_map) {
    k_info(system_table, L"Exiting UEFI Boot Services...\r\n");
    fini_log_file(system_table);
    // TODO: mono - Actually do that
    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
}

typedef struct {
    CHAR16 *str;
    size_t exec_len;
    size_t mod_len;
    size_t cmd_len;
} ConfPath;

// Remember to free ConfPath.str!
static ErrorStatus conf_to_wchar(const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL *file_protocol, const KuroConfigInternal *config, ConfPath *path_out) {
    CHAR16 *exec_path = NULL;
    size_t exec_path_len = strlen(config->exec_path) + 1;
    size_t mod_path_len = 0;
    if (config->module_path != NULL) {
        mod_path_len = strlen(config->module_path) + 1;
    }
    size_t cmd_arg_len = 0;
    if (config->cmd_arg != NULL) {
        cmd_arg_len = strlen(config->cmd_arg) + 1;
    }
    size_t total_len = exec_path_len + mod_path_len + cmd_arg_len;
    EFI_STATUS error = system_table->BootServices->AllocatePool(EfiLoaderData, total_len * sizeof(CHAR16), (void **) &exec_path);
    if (error != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = error,
            .status = SYSTEM_OUT_OF_MEMORY
        };
    }
    to_wchar(config->exec_path, exec_path, total_len);
        k_info_str(system_table, L"      Booting", exec_path);
    if (config->module_path != NULL) {
        to_wchar(config->module_path, exec_path + exec_path_len, total_len);
        k_info_str(system_table, L"  with module", exec_path + exec_path_len);
    }
    if (config->cmd_arg != NULL) {
        to_wchar(config->cmd_arg, exec_path + exec_path_len + mod_path_len, total_len);
        k_info_str(system_table, L"with arguments", exec_path + exec_path_len + mod_path_len);
    }
    k_br(system_table, KURO_LOG_LEVEL_INFO);

    path_out->str = exec_path;
    path_out->exec_len = exec_path_len;
    path_out->mod_len = mod_path_len;
    path_out->cmd_len = cmd_arg_len;

    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
}

static ErrorStatus load_file(const EFI_SYSTEM_TABLE *system_table, void* file, size_t file_size, EFI_FILE_PROTOCOL *file_prot) {
    EFI_STATUS status = system_table->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, TO_PAGES(file_size), (EFI_PHYSICAL_ADDRESS*) &file);
    if (status != EFI_SUCCESS) {
        file_prot->Close(file);
        ErrorStatus error = {.error_code = status, .status = SYSTEM_OUT_OF_MEMORY};
        return error;
    }
    status = file_prot->Read(file_prot, &file_size, file);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) file, TO_PAGES(file_size));
        file_prot->Close(file_prot);
        ErrorStatus error = {.error_code = status, .status = LOAD_FAILED};
        return error;
    }
    file_prot->Close(file_prot);
    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
}

ErrorStatus boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, const KuroConfigInternal *config) {
    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS VOLUME_OPEN_STATUS = cached_volume_open(image_handle, system_table, &file_protocol);
    if (VOLUME_OPEN_STATUS != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = VOLUME_OPEN_STATUS, .status = SYSTEM_CANNOT_OPEN_VOLUME};
        return error;
    }

    EFI_FILE_PROTOCOL *exec_file;
    EFI_FILE_PROTOCOL *mod_file;

    ConfPath paths;

    ErrorStatus error_status = conf_to_wchar(system_table, file_protocol, config, &paths);
    if (error_status.error_code != EFI_SUCCESS) {
        return error_status;
    }

    k_info(system_table, L"Loading the executable...\r\n");
    EFI_STATUS status = file_protocol->Open(file_protocol, &exec_file, paths.str, EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        if (status == EFI_ERR(EFI_NOT_FOUND)) {
            ErrorStatus error = {.error_code = EFI_ERR(EFI_NOT_FOUND), .status = ELF_FILE_NOT_FOUND};
            file_protocol->Close(file_protocol);
            return error;
        }
        ErrorStatus error = {.error_code = status, .status = SYSTEM_CANNOT_GET_FILE};
        file_protocol->Close(file_protocol);
        return error;
    }

    UINTN exec_file_size;
    status = get_file_size(system_table, exec_file, &exec_file_size);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = SYSTEM_CANNOT_READ_FILESIZE};
        file_protocol->Close(file_protocol);
        exec_file->Close(exec_file);
        return error;
    }
    k_debug_hex(system_table, L"Executable size", exec_file_size);

    // ELF cannot realistically go under 128 if included KuroFooter. Could be more accurate but eh
    if (exec_file_size < MINIMUM_ELF_SIZE) {
        file_protocol->Close(file_protocol);
        exec_file->Close(exec_file);
        ErrorStatus error = {.error_code = EFI_ERR(EFI_LOAD_ERROR), .status = ELF_INVALID_FILE_SIZE};
        return error;
    }

    status = exec_file->SetPosition(exec_file, 0);
    if (status != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        exec_file->Close(exec_file);
        ErrorStatus error = {.error_code = status, .status = LOAD_FAILED};
        return error;
    }

    void* exec = 0;
    error_status = load_file(system_table, exec, exec_file_size, exec_file);
    if (error_status.error_code != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        return error_status;
    }

    k_success(system_table, L"Loaded the executable!\r\n");

    if (config->secure_mode == 1) {
        k_info(system_table, L"Verifying the executable signature...\r\n");
        const ErrorStatus SIGN_STATUS = verify_footer(exec, exec_file_size, config->public_key);
        if (SIGN_STATUS.error_code != EFI_SUCCESS) {
            exec_file->Close(exec_file);
            file_protocol->Close(file_protocol);
            return SIGN_STATUS;
        }
        k_success(system_table, L"The executable signature is valid!\r\n");
    }

    if (paths.mod_len == 0) {
        file_protocol->Close(file_protocol);
        goto skip_module;
    }

    k_br(system_table, KURO_LOG_LEVEL_INFO);
    k_info(system_table, L"Loading the module...\r\n");

    status = exec_file->Open(exec_file, &mod_file, paths.str + paths.exec_len, EFI_FILE_MODE_READ, 0);
    file_protocol->Close(file_protocol);
    if (status != EFI_SUCCESS) {
        if (status == EFI_ERR(EFI_NOT_FOUND)) {
            ErrorStatus error = {.error_code = EFI_ERR(EFI_NOT_FOUND), .status = ELF_FILE_NOT_FOUND};
            return error;
        }
        ErrorStatus error = {.error_code = status, .status = SYSTEM_CANNOT_GET_FILE};
        return error;
    }

    UINTN mod_file_size;
    status = get_file_size(system_table, mod_file, &mod_file_size);
    if (status != EFI_SUCCESS) {
        ErrorStatus error = {.error_code = status, .status = SYSTEM_CANNOT_READ_FILESIZE};
        mod_file->Close(mod_file);
        return error;
    }
    k_debug_hex(system_table, L"Module size", mod_file_size);
    status = mod_file->SetPosition(mod_file, 0);
    if (status != EFI_SUCCESS) {
        mod_file->Close(mod_file);
        ErrorStatus error = {.error_code = status, .status = LOAD_FAILED};
        return error;
    }

    void* mod = 0;
    error_status = load_file(system_table, mod, mod_file_size, mod_file);
    if (error_status.error_code != EFI_SUCCESS) {
        file_protocol->Close(file_protocol);
        return error_status;
    }

    k_success(system_table, L"Loaded the module!\r\n");

    if (config->secure_mode == 1) {
        k_info(system_table, L"Verifying the module signature...\r\n");
        const ErrorStatus SIGN_STATUS = verify_footer(mod, mod_file_size, config->public_key);
        if (SIGN_STATUS.error_code != EFI_SUCCESS) {
            mod_file->Close(mod_file);
            return SIGN_STATUS;
        }
        k_success(system_table, L"The module signature is valid!\r\n");
    }

    skip_module:

    system_table->BootServices->FreePool(paths.str);

    k_br(system_table, KURO_LOG_LEVEL_INFO);
    k_info(system_table, L"Starting boot procedure...\r\n");


    // bi as in boot info not bisexual *please laugh*
    // const size_t PAGES_REQUIRED_NO_BI = PROGRAM_SIZE_PAGE + STACK_SIZE_PAGE;

    // k_debug(system_table, L"Base page requirements:\r\n");
    // k_debug_num(system_table, L"  Program", PROGRAM_SIZE_PAGE);
    // k_debug_num(system_table, L"  Stack  ", STACK_SIZE_PAGE);
    // k_debug_num(system_table, L"  Total  ", PAGES_REQUIRED_NO_BI);

    k_br(system_table, KURO_LOG_LEVEL_DEBUG);

    // TODO: mono - Get framebuffer

    MemoryMap memory_map;
    const ErrorStatus EXIT_STATUS = exit_boot(system_table, image_handle, &memory_map);
    if (EXIT_STATUS.error_code != EFI_SUCCESS) {
        return EXIT_STATUS;
    }



    // panic:
    while (1) {
        __asm__ volatile ("hlt");
    }
}
