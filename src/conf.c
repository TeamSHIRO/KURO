#include "conf.h"

#include "string.h"

#include "efi-st.h"
#include "file.h"
#include "protocol/efi-lip.h"
#include "efi_dptt.h"
#include "elf.h"
#include "status.h"
#include "kuro_const.h"

// This function automatically set the log level for the console print
// Please free the exec_path using FreePool if you don't need it anymore, since it might be allocated using AllocatePool
void get_config(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, KuroConfigInternal *config) {
    config->secure_mode = 0;
    config->aslr_enabled = 1;
    config->log_level = KURO_LOG_LEVEL_DEBUG;
    config->console_log_level = KURO_LOG_LEVEL_DEBUG;
    config->exec_path = "\\kernel";
    config->module_path = NULL;
    config->cmd_arg = NULL;

#ifdef KURO_NO_CONFIG
    return;
#endif

    EFI_FILE_PROTOCOL *file_protocol;
    if (cached_volume_open(image_handle, system_table, &file_protocol) != EFI_SUCCESS) {
        return;
    }

    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

    if (system_table->BootServices->OpenProtocol(image_handle, (EFI_GUID *) &LIP_GUID,
        (void **) &loaded_image,image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL) != EFI_SUCCESS) {
        return;
    }

    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *device_path_to_text;

    if (system_table->BootServices->LocateProtocol((EFI_GUID *) &DPTT_GUID, NULL, (VOID**)&device_path_to_text) != EFI_SUCCESS) {
        system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);
        return;
    }

    CHAR16 *path_text = device_path_to_text->ConvertDevicePathToText(
        loaded_image->FilePath,
        1,
        1
    );

    k_debug_str(system_table, L"Bootloader path", path_text);

    system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);

    EFI_FILE_PROTOCOL *exec_file;
    file_protocol->Open(file_protocol, &exec_file, path_text, EFI_FILE_MODE_READ, 0);

    UINTN file_size = 0;
    get_file_size(system_table, exec_file, &file_size);

    UINTN config_size = sizeof(KuroConfig);
    KuroConfig kuro_config;

    file_protocol->SetPosition(file_protocol, file_size - sizeof(KuroConfig));
    file_protocol->Read(file_protocol, &config_size, &kuro_config);

    if (memcmp(kuro_config.identifier.k_magic, KURO_MAGIC_CONST, KURO_MAGIC_LEN) != 0 || kuro_config.identifier.k_version != KURO_CONFIG_VERSION_1) {
        k_warning(system_table, L"Invalid config found, using default config instead\r\n");
        system_table->BootServices->FreePool(path_text);
        return;
    }

    KuroLogLevel log_level_con = kuro_config.console_log_level;
    KuroLogLevel log_level_file = kuro_config.log_level;

    if (log_level_con > KURO_LOG_LEVEL_DEBUG) {
        log_level_con = KURO_LOG_LEVEL_DEBUG;
    }
    if (log_level_file > KURO_LOG_LEVEL_DEBUG) {
        log_level_file = KURO_LOG_LEVEL_DEBUG;
    }

    kuro_config.console_log_level = log_level_con;
    kuro_config.log_level = log_level_file;
    g_console_log_level = log_level_con;

    config->secure_mode = kuro_config.secure_mode;
    // TODO: TheMonHub - CHECK FOR UEFI SECURE BOOT FIRST

    config->aslr_enabled = kuro_config.aslr_enabled;
    memcpy(config->public_key, kuro_config.public_key, 32);

    // String reading
    UINTN str_abs_offset;
    int underflow_check = __builtin_sub_overflow(file_size - config_size, kuro_config.str_offset, &str_abs_offset);
    if (underflow_check == 1) {
        system_table->BootServices->FreePool(path_text);
        return;
    }

    UINTN str_config_size = kuro_config.str_offset;

    char *str_config;
    system_table->BootServices->AllocatePool(EfiLoaderData, str_config_size + 1, (void**)&str_config);
    str_config[kuro_config.str_offset] = '\0';

    exec_file->SetPosition(exec_file, str_abs_offset);
    exec_file->Read(exec_file, &str_config_size, str_config);

    size_t exec_len = strlen(str_config);
    if (exec_len == str_config_size - 3) {
        goto error;
    }
    char *exec_off = str_config;

    size_t module_len = strlen(exec_off + 1);
    if (module_len == str_config_size - exec_len - 3) {
        goto error;
    }
    char *module_off = exec_off + 1 + exec_len;

    size_t cmd_len = strlen(module_off + 1);
    if (cmd_len == str_config_size - exec_len - module_len - 3) {
        goto error;
    }
    char *cmd_off = module_off + 1 + cmd_len;

    config->exec_path = exec_off;
    config->module_path = module_off;
    config->cmd_arg = cmd_off;

    error:
    k_debug_str(system_table, L"Bootloader path", path_text);
    system_table->BootServices->FreePool(path_text);
    exec_file->Close(exec_file);
}
