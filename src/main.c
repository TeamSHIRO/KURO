#include <efi.h>
#include "boot.h"
#include "conf.h"

ErrorStatus main(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    system_table->ConOut->ClearScreen(system_table->ConOut);

    KuroConfigInternal config;
    get_config(system_table, image_handle, &config);

    const EFI_STATUS DISABLE_WD = system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0);
    if (DISABLE_WD != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = DISABLE_WD,
            .status = WATCHDOG_DISABLE_FAILED
        };
    }

    k_info(system_table, L"KURO bootloader v" PROJECT_VERSION "\r\n");

    return boot_elf(image_handle, system_table, &config);
}

EFI_STATUS efi_main(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    const ErrorStatus STATUS = main(image_handle, system_table);

    k_error(system_table, STATUS);

    return STATUS.error_code;
}
