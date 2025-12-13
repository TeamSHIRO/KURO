/*
 * File: main.c
 * Description: Entry point.
 *
 * Copyright (C) 2025 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main.h"

#include "output/cout.h"
#include <efi.h>

EFI_HANDLE g_image_handle;
EFI_SYSTEM_TABLE *g_system_table;

CHAR16 *hello_str = L"Hello, World!\r\n";

EFI_STATUS efi_main(EFI_HANDLE image_handle_p, EFI_SYSTEM_TABLE *system_table_p)
{
    g_image_handle = image_handle_p;
    g_system_table = system_table_p;

    DEBUG_PRINT(L"Hello, World!\r\n\0");

    if (g_system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0) != EFI_SUCCESS)
    {
        ERROR_PRINT(L"Failed to disable watchdog timer.\r\n\0");
    }
    return (EFI_SUCCESS);
}