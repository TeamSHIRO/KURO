/*
 * File: main.c
 * Description: Entry point.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main.h"

#include <efi.h>
#include <stddef.h>

#include "../include/config.h"
#include "../include/file.h"
#include "../include/logger.h"
#include "../include/string.h"

EFI_HANDLE g_image_handle;
EFI_SYSTEM_TABLE* g_system_table;

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  g_image_handle = image_handle_p;
  g_system_table = system_table_p;

  const EFI_LOADED_IMAGE_PROTOCOL* loaded_image_handle =
      get_loaded_image_protocol(image_handle_p);
  if (loaded_image_handle == NULL) {
    // error log here
    return EFI_LOAD_ERROR;
  }
  const EFI_FILE_PROTOCOL* file_protocol =
      get_volume_handle(loaded_image_handle);
  if (file_protocol == NULL) {
    // error log here
    return EFI_LOAD_ERROR;
  }

  const EFI_STATUS config_status =
      init_config((EFI_FILE_PROTOCOL*)file_protocol, 0);
  if (config_status != EFI_SUCCESS) {
    // error log here
    return config_status;
  }

  const EFI_STATUS logger_status =
      init_logger((EFI_FILE_PROTOCOL*)file_protocol);
  if (logger_status != EFI_SUCCESS) {
    // error log here
    return logger_status;
  }

  const EFI_STATUS disable_wd =
      g_system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0);

  if (disable_wd != EFI_SUCCESS) {
    // error log here
    return disable_wd;
  }

  // Test config parsing by reading the kernel path from the config file and
  char kernel_path[256];
  get_config_key("kernel_path", kernel_path);
  CHAR16 kernel_path_wide[256];
  wchar(kernel_path, kernel_path_wide, 256);

  system_table_p->ConOut->OutputString(system_table_p->ConOut,
                                       L"Kernel path: ");
  system_table_p->ConOut->OutputString(system_table_p->ConOut,
                                       kernel_path_wide);

  system_table_p->BootServices->WaitForEvent(1, &g_system_table->ConIn->WaitForKey, NULL);

  close_config();

  return EFI_SUCCESS;
}