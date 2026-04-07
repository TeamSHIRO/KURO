#include <efi.h>
#include "boot.h"
#include "string.h"

EFI_STATUS main(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    const EFI_STATUS disable_wd = system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0);
    if (disable_wd != EFI_SUCCESS) {
        return disable_wd;
    }

    return boot_elf(image_handle, system_table);
}

EFI_STATUS efi_main(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    const EFI_STATUS status = main(image_handle, system_table);

    CHAR16 status_str[19];
    to_hex(status, status_str);

    system_table->ConOut->OutputString(system_table->ConOut, L"\r\nBoot failed with status: ");
    system_table->ConOut->OutputString(system_table->ConOut, status_str);
    system_table->ConOut->OutputString(system_table->ConOut, L"\r\nPress any key to continue...\r\n");

    EFI_EVENT wait_for_key[1] = {system_table->ConIn->WaitForKey};
    system_table->BootServices->WaitForEvent(1, wait_for_key, NULL);

    return status;
}
