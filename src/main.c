#include <efi.h>
#include "boot.h"
#include "conf.h"

ErrorStatus main(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    system_table->ConOut->ClearScreen(system_table->ConOut);

    KuroConfigInternal config;
    ErrorStatus config_status = get_config(system_table, image_handle, &config);
    if (config_status.error_code != EFI_SUCCESS) {
        return config_status;
    }
    k_br(system_table, KURO_LOG_LEVEL_DEBUG);

    const EFI_STATUS DISABLE_WD = system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0);
    if (DISABLE_WD != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = DISABLE_WD,
            .status = WATCHDOG_DISABLE_FAILED
        };
    }

    return boot_elf(image_handle, system_table, &config);
}

EFI_STATUS efi_main(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    const ErrorStatus STATUS = main(image_handle, system_table);
    if (g_console_log_level >= KURO_LOG_LEVEL_DEBUG && g_console_log_level != KURO_LOG_LEVEL_ERROR) {
        system_table->ConOut->OutputString(system_table->ConOut, L"\n\r");
    }
    k_error(system_table, STATUS);
    k_br(system_table, KURO_LOG_LEVEL_ERROR);
    if (g_console_log_level >= KURO_LOG_LEVEL_ERROR) {
        system_table->ConOut->OutputString(system_table->ConOut, L"Press any key to continue...");
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, 0);
    }

    fini_log_file(system_table);

    return STATUS.error_code;
}
