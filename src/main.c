/*
 * File: main.c
 * Description: Entry point.
 *
 * Copyright (C) 2025 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#include <efi.h>

EFI_HANDLE image_handle;
EFI_SYSTEM_TABLE* system_table;

CHAR16* hello_str = L"Hello, World!\r\n";

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  image_handle = image_handle_p;
  system_table = system_table_p;

  system_table->ConOut->OutputString(system_table->ConOut, hello_str);

  return (EFI_SUCCESS);
}