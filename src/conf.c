#include "conf.h"

#include "string.h"

#include "efi-st.h"
#include "file.h"
#include "protocol/efi-lip.h"
#include "protocol/efi-dptt.h"
#include "helper.h"
#include "elf.h"
#include "status.h"
#include "kuro_const.h"

const EFI_GUID DPTT_GUID = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
const EFI_GUID GLOBAL_VARIABLE_GUID = EFI_GLOBAL_VARIABLE;

typedef enum {
    IS_PATH_FILE,
    IS_CMD_ARG
} IsPath;

static size_t len_n_verify(const char *str, _Bool *is_valid, IsPath is_path) {
    size_t i = 0;
    *is_valid = 1;

    if (is_path == IS_PATH_FILE) {
        if (str[0] != '\\') {
            *is_valid = 0;
            return 0;
        }
    }
    while (str[i] != '\0') {
        // Not ASCII
        if (is_path == IS_PATH_FILE && str[i] > 127 && *is_valid == 1) {
            *is_valid = 0;
        }
        i++;
    }

    if (is_path == IS_PATH_FILE && str[i - 1] == '\\') {
        *is_valid = 0;
    }
    if (is_path == IS_PATH_FILE && i < 2 ) {
        *is_valid = 0;
    }
    return i;
}

static ErrorStatus get_bootloader_file(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **exec_file, CHAR16 **path_text) {
    EFI_FILE_PROTOCOL *file_protocol;
    EFI_STATUS status = cached_volume_open(image_handle, system_table, &file_protocol);
    ErrorStatus error = SUCCESS;

    _Bool is_lip_opened = 0;
    _Bool is_path_text = 0;

    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_OPEN_VOLUME);
        goto cleanup;
    }

    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

    status = system_table->BootServices->OpenProtocol(image_handle, (EFI_GUID *) &LIP_GUID,
        (void **) &loaded_image,image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    is_lip_opened = 1;

    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_OPEN_PROTOCOL);
        goto cleanup;
    }

    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *device_path_to_text;

    status = system_table->BootServices->LocateProtocol((EFI_GUID *) &DPTT_GUID, NULL, (VOID**)&device_path_to_text);

    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_OPEN_PROTOCOL);
        goto cleanup;
    }

    *path_text = device_path_to_text->ConvertDevicePathToText(
        loaded_image->FilePath,
        1,
        1
    );
    if (path_text == NULL) {
        error = ERROR(EFI_ERR(EFI_OUT_OF_RESOURCES), SYSTEM_OUT_OF_MEMORY);
        goto cleanup;
    }
    is_path_text = 1;

    system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);

    status = file_protocol->Open(file_protocol, exec_file, *path_text, EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, CONFIG_LOAD_FAILED);
    }

    cleanup:

    if (error.status == K_SUCCESS) {
        return error;
    }

    if (is_path_text == 1) {
        system_table->BootServices->FreePool(path_text);
    }
    if (is_lip_opened == 1) {
        system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);
    }
    return error;
}

static void setup_config(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, const KuroConfig *kuro_config, CHAR16 *path_text, KuroConfigInternal *config) {
    if (kuro_config != NULL) {
        g_console_log_level = kuro_config->console_log_level;
        g_file_log_level = kuro_config->log_level;
        if (kuro_config->console_log_level > KURO_LOG_LEVEL_DEBUG) {
            g_console_log_level = KURO_LOG_LEVEL_DEBUG;
        }
        if (kuro_config->log_level > KURO_LOG_LEVEL_DEBUG) {
            g_file_log_level = KURO_LOG_LEVEL_DEBUG;
        }
    }

    k_info(system_table, L"KURO bootloader v" PROJECT_VERSION "\r\n");
    k_info(system_table, L"Copyright (c) KURO Contributors\r\n");
    k_info(system_table, L"Licensed under the Apache License, Version 2.0\r\n");
    k_br(system_table, KURO_LOG_LEVEL_INFO);
    if (path_text != NULL) {
        k_debug_str(system_table, L"Bootloader path", path_text);
        system_table->BootServices->FreePool(path_text);
    }

    const ErrorStatus LOG_STATUS = init_log_file(image_handle, system_table);
    if (LOG_STATUS.status != K_SUCCESS) {
        k_error(system_table, LOG_STATUS);
        k_warning(system_table, L"There will be no file logging!\r\n");
    }
    k_br(system_table, KURO_LOG_LEVEL_INFO);

    k_success(system_table, L"Config found!\r\n");
    k_info(system_table, L"Reading config...\r\n");

    uint8_t secure_boot;
    UINTN secure_boot_size = sizeof(secure_boot);
    EFI_STATUS status = system_table->RuntimeServices->GetVariable(L"SecureBoot", (EFI_GUID *) &GLOBAL_VARIABLE_GUID, NULL, &secure_boot_size, &secure_boot);
    if (status != EFI_SUCCESS) {
        k_error(system_table, ERROR(status, SYSTEM_CANNOT_GET_VARIABLE));
        secure_boot = 0;
        k_warning(system_table, L"Failed to get secure boot variable, assuming it's disabled\r\n");
    }

    k_debug_num(system_table, L"Secure boot variable", secure_boot);

    if (secure_boot == 1) {
        k_debug(system_table, L"Secure boot is enabled!\r\n");
        k_debug(system_table, L"Forcing secure mode...\r\n");
        config->secure_mode = 1;
    } else {
        k_debug(system_table, L"Secure boot is disabled!\r\n");
        if (kuro_config != NULL) {
            config->secure_mode = kuro_config->secure_mode;
        }
    }

    if (config->secure_mode == 0) {
        k_warning(system_table, L"Secure mode is disabled!\r\n");
    }

    if (kuro_config != NULL) {
        config->aslr_enabled = kuro_config->aslr_enabled;
        if (config->secure_mode == 1) {
            memcpy(config->public_key, kuro_config->public_key, 32);
        }
    }
}

// This function automatically set the log level for the console print
// May output dangling pointer on failure
//
// Please free the exec_path using FreePool if you don't need it anymore, since it might be allocated using AllocatePool
// ONLY IF free == 1
ErrorStatus get_config(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, KuroConfigInternal *config) {
#ifdef KURO_NO_CONFIG

    config->secure_mode = 0;
    config->aslr_enabled = 1;
    config->exec_path = "\\kernel";
    config->module_path = "\\module";
    config->cmd_arg = "bootloader_test";
    config->free = 0;

    setup_config(system_table, image_handle, NULL, NULL, config);

    return SUCCESS;
}

#else

    config->free = 1;

    EFI_FILE_PROTOCOL *exec_file = NULL;
    CHAR16 *path_text = NULL;
    char *str_config = NULL;

    ErrorStatus error = SUCCESS;

    ErrorStatus get_boot_status = get_bootloader_file(system_table, image_handle, &exec_file, &path_text);
    if (get_boot_status.status != K_SUCCESS) {
        return get_boot_status;
    }

    UINTN file_size = 0;
    EFI_STATUS status = get_file_size(system_table, exec_file, &file_size);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_CANNOT_READ_FILESIZE);
        goto cleanup;
    }

    UINTN config_size = sizeof(KuroConfig);
    KuroConfig kuro_config;

    // The bootloader can boot, and that already proves that it's big enough to contain at least like 500~ bytes?
    // No point for underflow check here
    status = exec_file->SetPosition(exec_file, file_size - sizeof(KuroConfig));

    if (status != EFI_SUCCESS) {
        error = ERROR(status, CONFIG_LOAD_FAILED);
        goto cleanup;
    }

    status = exec_file->Read(exec_file, &config_size, &kuro_config);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, CONFIG_LOAD_FAILED);
        goto cleanup;
    }

    if (memcmp(kuro_config.identifier.k_magic, KURO_MAGIC_CONST, KURO_MAGIC_LEN) != 0 || kuro_config.identifier.k_version != KURO_CONFIG_VERSION_1) {
        error = ERROR(EFI_SUCCESS, CONFIG_LOAD_FAILED);
        goto cleanup;
    }

    setup_config(system_table, image_handle, &kuro_config, path_text, config);

    config->exec_path = NULL;

    UINTN str_abs_offset;
    if (__builtin_sub_overflow(file_size - config_size, kuro_config.str_offset, &str_abs_offset) == 1) {
        error = ERROR(EFI_SUCCESS, CONFIG_LOAD_FAILED);
        goto cleanup;
    }

    UINTN str_config_size = kuro_config.str_offset;

    // "\\a\0\0\0" is 5 bytes
    // Executable cannot be a directory and cannot be null
    if (str_config_size < 5) {
        error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
        goto cleanup;
    }

    status = system_table->BootServices->AllocatePool(EfiLoaderData, str_config_size + 1, (void**)&str_config);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, SYSTEM_OUT_OF_MEMORY);
        goto cleanup;
    }
    str_config[kuro_config.str_offset] = '\0';

    status = exec_file->SetPosition(exec_file, str_abs_offset);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, CONFIG_LOAD_FAILED);
        goto cleanup;
    }

    status = exec_file->Read(exec_file, &str_config_size, str_config);
    if (status != EFI_SUCCESS) {
        error = ERROR(status, CONFIG_LOAD_FAILED);
        goto cleanup;
    }
    exec_file->Close(exec_file);

    _Bool is_valid;
    size_t exec_len = len_n_verify(str_config, &is_valid, IS_PATH_FILE);
    size_t remaining_len = str_config_size - 2;
    if (exec_len + 1 > remaining_len || !is_valid) {
        error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
        goto cleanup;
    }

    char *module_off = str_config + exec_len + 1;
    size_t module_len = len_n_verify(module_off, &is_valid, IS_PATH_FILE);
    remaining_len -= exec_len;
    if (module_len + 1 > remaining_len || !is_valid) {
        error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
        goto cleanup;
    }

    char *cmd_off = module_off + module_len + 1;
    size_t cmd_len = len_n_verify(cmd_off, &is_valid, IS_CMD_ARG);
    remaining_len -= module_len;
    if (cmd_len + 1 > remaining_len || !is_valid) {
        error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
        goto cleanup;
    }

    if (exec_len >= 2) {
        if (str_config[0] != '\\') {
            error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
            goto cleanup;
        }
        config->exec_path = str_config;
    } else {
        error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
        goto cleanup;
    }

    if (module_len != 0) {
        if (module_off[0] != '\\') {
            error = ERROR(EFI_SUCCESS, INVALID_STRING_CONFIG);
            goto cleanup;
        }
        config->module_path = module_off;
    } else {
        config->module_path = NULL;
    }

    if (cmd_len != 0) {
        config->cmd_arg = cmd_off;
    } else {
        config->cmd_arg = NULL;
    }

    cleanup:

    if (exec_file != NULL) {
        exec_file->Close(exec_file);
    }
    if (path_text != NULL) {
        system_table->BootServices->FreePool(path_text);
    }
    if (str_config != NULL) {
        system_table->BootServices->FreePool(str_config);
    }

    if (error.status != K_SUCCESS) {
        return error;
    }

    k_success(system_table, L"Finished reading config!\r\n");
    return SUCCESS;
}
#endif