/*
 * File: main.c
 * Description: Entry point.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main.h"

#include <efi.h>

#include "file/file.h"

EFI_HANDLE g_image_handle;
EFI_SYSTEM_TABLE* g_system_table;

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  g_image_handle = image_handle_p;
  g_system_table = system_table_p;

  const EFI_LOADED_IMAGE_PROTOCOL* loaded_image_handle =
      get_loaded_image_protocol(image_handle_p);
  const EFI_FILE_PROTOCOL* file_protocol =
      get_volume_handle(loaded_image_handle);

  if (g_system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0) !=
      EFI_SUCCESS) {
    // error log here
  }
  return (EFI_SUCCESS);
}