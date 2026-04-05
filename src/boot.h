#ifndef BOOT_H
#define BOOT_H
#include "efi-st.h"
#include "efi.h"

EFI_STATUS boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table);

#endif // BOOT_H
