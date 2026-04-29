#include "conf.h"

#include "string.h"

#include "efi-st.h"
#include "file.h"
#include "protocol/efi-lip.h"
#include "protocol/efi-dptt.h"
#include "efi_helper.h"
#include "elf.h"
#include "status.h"
#include "kuro_const.h"

const EFI_GUID DPTT_GUID = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
const EFI_GUID GLOBAL_VARIABLE_GUID = EFI_GLOBAL_VARIABLE;

typedef enum {
    IS_PATH_FILE,
    IS_CMD_ARG
} IsPath;

static size_t len_n_verify(const char *str, int *is_valid, IsPath is_path) {
    size_t i = 0;
    *is_valid = 1;

    if (is_path == IS_PATH_FILE) {
        if (str[0] != '\\') {
            *is_valid = 0;
            return 0;
        }
    }
    while (str[i] != '\0') {
        i++;
        // Not ASCII
        if (is_path == IS_PATH_FILE && str[i] > 127 && *is_valid == 1) {
            *is_valid = 0;
        }
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
    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_CANNOT_OPEN_VOLUME
        };
    }

    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

    status = system_table->BootServices->OpenProtocol(image_handle, (EFI_GUID *) &LIP_GUID,
        (void **) &loaded_image,image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_CANNOT_OPEN_PROTOCOL
        };
    }

    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *device_path_to_text;

    status = system_table->BootServices->LocateProtocol((EFI_GUID *) &DPTT_GUID, NULL, (VOID**)&device_path_to_text);

    if (status != EFI_SUCCESS) {
        system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_CANNOT_OPEN_PROTOCOL
        };
    }

    *path_text = device_path_to_text->ConvertDevicePathToText(
        loaded_image->FilePath,
        1,
        1
    );
    if (path_text == NULL) {
        system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_OUT_OF_RESOURCES), // Assuming it was because of the memory and not that the device path was null
            .status = SYSTEM_OUT_OF_MEMORY
        };
    }

    system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);

    status = file_protocol->Open(file_protocol, exec_file, *path_text, EFI_FILE_MODE_READ, 0);

    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(path_text);
        return (ErrorStatus) {
            .error_code = status,
            .status = CONFIG_LOAD_FAILED
        };
    }
    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
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
    if (LOG_STATUS.error_code != EFI_SUCCESS) {
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
        k_error(system_table, (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_CANNOT_GET_VARIABLE
        });
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

    k_br(system_table, KURO_LOG_LEVEL_WARNING);
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
    config->secure_mode = 0;
    config->aslr_enabled = 1;
    config->exec_path = "\\kernel";
    config->module_path = NULL;
    config->cmd_arg = NULL;

#ifdef KURO_NO_CONFIG
    config->free = 0;

    setup_config(system_table, image_handle, NULL, NULL, config);

    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
}
#else

    config->free = 1;

    EFI_FILE_PROTOCOL *exec_file = 0;
    CHAR16 *path_text = 0;

    ErrorStatus get_boot_status = get_bootloader_file(system_table, image_handle, &exec_file, &path_text);
    if (get_boot_status.error_code != EFI_SUCCESS) {
        return get_boot_status;
    }

    UINTN file_size = 0;
    EFI_STATUS status = get_file_size(system_table, exec_file, &file_size);
    if (status != EFI_SUCCESS) {
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_CANNOT_READ_FILESIZE
        };
    }

    UINTN config_size = sizeof(KuroConfig);
    KuroConfig kuro_config;

    // The bootloader can boot, and that already proves that it's big enough
    // No point for underflow check here
    status = exec_file->SetPosition(exec_file, file_size - sizeof(KuroConfig));

    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(path_text);
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = status,
            .status = CONFIG_LOAD_FAILED
        };
    }

    status = exec_file->Read(exec_file, &config_size, &kuro_config);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(path_text);
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = status,
            .status = CONFIG_LOAD_FAILED
        };
    }

    if (memcmp(kuro_config.identifier.k_magic, KURO_MAGIC_CONST, KURO_MAGIC_LEN) != 0 || kuro_config.identifier.k_version != KURO_CONFIG_VERSION_1) {
        system_table->BootServices->FreePool(path_text);
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_LOAD_ERROR),
            .status = CONFIG_LOAD_FAILED
        };
    }

    setup_config(system_table, image_handle, &kuro_config, path_text, config);

    config->exec_path = NULL;

    UINTN str_abs_offset;
    int underflow_check = __builtin_sub_overflow(file_size - config_size, kuro_config.str_offset, &str_abs_offset);
    if (underflow_check == 1) {
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
            .status = INVALID_STRING_CONFIG
        };
    }

    UINTN str_config_size = kuro_config.str_offset;

    // "\\a\0\0\0" is 5 bytes
    // Executable cannot be a directory and cannot be null
    if (str_config_size < 5) {
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_BUFFER_TOO_SMALL),
            .status = INVALID_STRING_CONFIG
        };
    }

    char *str_config;
    status = system_table->BootServices->AllocatePool(EfiLoaderData, str_config_size + 1, (void**)&str_config);
    if (status != EFI_SUCCESS) {
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = status,
            .status = SYSTEM_OUT_OF_MEMORY
        };
    }
    str_config[kuro_config.str_offset] = '\0';

    status = exec_file->SetPosition(exec_file, str_abs_offset);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(str_config);
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = status,
            .status = CONFIG_LOAD_FAILED
        };
    }

    status = exec_file->Read(exec_file, &str_config_size, str_config);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(str_config);
        exec_file->Close(exec_file);
        return (ErrorStatus) {
            .error_code = status,
            .status = CONFIG_LOAD_FAILED
        };
    }
    exec_file->Close(exec_file);

    int is_valid;
    size_t exec_len = len_n_verify(str_config, &is_valid, IS_PATH_FILE);
    size_t remaining_len = str_config_size - 2;
    if (exec_len + 1 > remaining_len || !is_valid) {
        system_table->BootServices->FreePool(str_config);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
            .status = INVALID_STRING_CONFIG
        };
    }

    char *module_off = str_config + exec_len + 1;
    size_t module_len = len_n_verify(module_off, &is_valid, IS_PATH_FILE);
    remaining_len -= exec_len;
    if (module_len + 1 > remaining_len || !is_valid) {
        system_table->BootServices->FreePool(str_config);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
            .status = INVALID_STRING_CONFIG
        };
    }

    char *cmd_off = module_off + module_len + 1;
    size_t cmd_len = len_n_verify(cmd_off, &is_valid, IS_PATH_FILE);
    remaining_len -= module_len;
    if (cmd_len + 1 > remaining_len || !is_valid) {
        system_table->BootServices->FreePool(str_config);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
            .status = INVALID_STRING_CONFIG
        };
    }

    if (exec_len >= 2) {
        if (str_config[0] != '\\') {
            system_table->BootServices->FreePool(str_config);
            return (ErrorStatus) {
                .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
                .status = INVALID_STRING_CONFIG
            };
        }
        config->exec_path = str_config;
    } else {
        system_table->BootServices->FreePool(str_config);
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
            .status = INVALID_STRING_CONFIG
        };
    }

    if (module_len != 0) {
        if (module_off[0] != '\\') {
            system_table->BootServices->FreePool(str_config);
            return (ErrorStatus) {
                .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
                .status = INVALID_STRING_CONFIG
            };
        }

        config->module_path = module_off;
    }
    if (cmd_len != 0) {
        config->cmd_arg = cmd_off;
    }

    k_success(system_table, L"Finished reading config!");
    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
}
#endif