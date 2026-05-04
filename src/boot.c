#include "boot.h"

#include "conf.h"
#include "string.h"
#include "efi.h"
#include "helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"
#include "status.h"
#include "verify.h"

static ErrorStatus exit_boot(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, MemoryMap *memory_map) {
    k_info(system_table, L"Exiting UEFI Boot Services...\r\n");
    fini_log_file(system_table);
    // TODO(mono): Actually do that
    return SUCCESS;
}

typedef struct {
    CHAR16 *str;
    size_t exec_len;
    size_t mod_len;
    size_t cmd_len;
} ConfPath;

static ErrorStatus conf_to_wchar(const EFI_SYSTEM_TABLE *system_table, const KuroConfigInternal *config, ConfPath *path_out) {
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
        return ERROR(error, SYSTEM_OUT_OF_MEMORY);
    }
    to_wchar(config->exec_path, exec_path, total_len);
        k_info_str(system_table, L"       Booting", exec_path);
    if (config->module_path != NULL) {
        to_wchar(config->module_path, exec_path + exec_path_len, total_len);
        k_info_str(system_table, L"   with module", exec_path + exec_path_len);
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

    return SUCCESS;
}

static ErrorStatus load_file(const EFI_SYSTEM_TABLE *system_table, void* file, size_t file_size, EFI_FILE_PROTOCOL *file_prot) {
    file = NULL;
    EFI_STATUS status = system_table->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, TO_PAGES(file_size), (EFI_PHYSICAL_ADDRESS*) &file);
    ErrorStatus error = SUCCESS;
    if (status != EFI_SUCCESS) {
        file_prot->Close(file);
        error = ERROR(status, SYSTEM_OUT_OF_MEMORY);
        goto cleanup;
    }
    status = file_prot->Read(file_prot, &file_size, file);
    if (status != EFI_SUCCESS) {
        file_prot->Close(file_prot);
        error = ERROR(status, LOAD_FAILED);
    }

    cleanup:
    if (file_prot != NULL) {
        file_prot->Close(file_prot);
    }
    if (file != NULL && error.status != K_SUCCESS) {
        system_table->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) file, TO_PAGES(file_size));
    }
    return error;
}

ErrorStatus boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, const KuroConfigInternal *config) {
    ErrorStatus error;

    EFI_FILE_PROTOCOL *file_protocol = NULL;

    EFI_FILE_PROTOCOL *exec_file = NULL;
    EFI_FILE_PROTOCOL *mod_file = NULL;

    void* exec = NULL;
    void* mod = NULL;

    UINTN exec_file_size = 0;
    UINTN mod_file_size = 0;

    ConfPath paths = {
        .str = NULL,
        .exec_len = 0,
        .mod_len = 0,
        .cmd_len = 0
    };

    const EFI_STATUS VOLUME_OPEN_STATUS = cached_volume_open(image_handle, system_table, &file_protocol);
    if (VOLUME_OPEN_STATUS != EFI_SUCCESS) {
        error = ERROR(VOLUME_OPEN_STATUS, SYSTEM_CANNOT_OPEN_VOLUME);
        goto cleanup;
    }

    error = conf_to_wchar(system_table, config, &paths);
    if (error.status != K_SUCCESS) {
        goto cleanup;
    }

    k_info(system_table, L"Loading the executable...\r\n");
    EFI_STATUS status = file_protocol->Open(file_protocol, &exec_file, paths.str, EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        error = ERROR(VOLUME_OPEN_STATUS, SYSTEM_CANNOT_GET_FILE);
        goto cleanup;
    }

    status = get_file_size(system_table, exec_file, &exec_file_size);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_READ_FILESIZE);
        goto cleanup;
    }
    k_debug_num(system_table, L"Executable size", exec_file_size);

    // ELF cannot realistically go under 128 if included KuroFooter.
    if (exec_file_size < MINIMUM_ELF_SIZE) {
        error = ERROR(EFI_SUCCESS, ELF_INVALID_FILE_SIZE);
        goto cleanup;
    }

    status = exec_file->SetPosition(exec_file, 0);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, LOAD_FAILED);
        goto cleanup;
    }

    error = load_file(system_table, exec, exec_file_size, exec_file);
    if (error.status != K_SUCCESS) {
        goto cleanup;
    }

    k_success(system_table, L"Loaded the executable!\r\n");

    if (config->secure_mode == 1) {
        k_info(system_table, L"Verifying the executable signature...\r\n");
        const KuroStatus SIGN_STATUS = verify_footer(exec, exec_file_size, config->public_key);
        if (SIGN_STATUS != K_SUCCESS) {
            error = ERROR(EFI_SUCCESS, SIGN_STATUS);
            goto cleanup;
        }
        k_success(system_table, L"The executable signature is valid!\r\n");
    }

    k_br(system_table, KURO_LOG_LEVEL_INFO);

    if (paths.mod_len == 0) {
        k_info(system_table, L"No module specified, skipping...\r\n");
        goto done_module;
    }

    k_info(system_table, L"Loading the module...\r\n");

    status = file_protocol->Open(file_protocol, &mod_file, paths.str + paths.exec_len, EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_GET_FILE);
        goto done_module;
    }

    status = get_file_size(system_table, mod_file, &mod_file_size);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_READ_FILESIZE);
        goto done_module;
    }
    k_debug_num(system_table, L"Module size", mod_file_size);
    status = mod_file->SetPosition(mod_file, 0);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, LOAD_FAILED);
        goto done_module;
    }

    error = load_file(system_table, mod, mod_file_size, mod_file);
    if (error.status != K_SUCCESS) {
        goto done_module;
    }

    k_success(system_table, L"Loaded the module!\r\n");

    if (config->secure_mode == 1) {
        k_info(system_table, L"Verifying the module signature...\r\n");
        const KuroStatus SIGN_STATUS = verify_footer(mod, mod_file_size, config->public_key);
        if (SIGN_STATUS != K_SUCCESS) {
            error = ERROR(EFI_SUCCESS, SIGN_STATUS);
            goto done_module;
        }
        k_success(system_table, L"The module signature is valid!\r\n");
    }

    done_module:
    if (error.status != K_SUCCESS) {
        k_error(system_table, error);
        k_warning(system_table, L"Failed to load the module, skipping...\r\n");
    }

    k_br(system_table, KURO_LOG_LEVEL_INFO);

    k_info(system_table, L"Parsing executable...\r\n");

    // ELF parser getting called here

    k_br(system_table, KURO_LOG_LEVEL_DEBUG);

    // TODO(mono): Get framebuffer

    MemoryMap memory_map;
    error = exit_boot(system_table, image_handle, &memory_map);
    if (error.status != K_SUCCESS) {
        goto cleanup;
    }

    // Load the executable and module at the desired place and then relocate the executable

    cleanup:

    if (error.status != K_SUCCESS) {
        if (file_protocol != NULL) {
            file_protocol->Close(file_protocol);
        }
        if (config->exec_path != NULL && config->free == 1) {
            system_table->BootServices->FreePool(config->exec_path);
        }
        if (paths.str != NULL) {
            system_table->BootServices->FreePool(paths.str);
        }
        if (exec != NULL) {
            system_table->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) exec, TO_PAGES(exec_file_size));
        }
        if (mod != NULL) {
            system_table->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) mod, TO_PAGES(mod_file_size));
        }
        if (exec_file != NULL) {
            exec_file->Close(exec_file);
        }
        if (mod_file != NULL) {
            mod_file->Close(mod_file);
        }
    }
    return error;
}
