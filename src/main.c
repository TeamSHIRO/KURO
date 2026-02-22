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

#include "config.h"
#include "cout.h"
#include "file.h"
#include "logger.h"
#include "string.h"

EFI_HANDLE g_image_handle;
EFI_SYSTEM_TABLE* g_system_table;

EFI_FILE_PROTOCOL* g_file_prot = NULL;

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  g_image_handle = image_handle_p;
  g_system_table = system_table_p;

  const EFI_STATUS init_status = init();
  if (init_status != EFI_SUCCESS) {
    fini(init_status);
    return init_status;
  }

  const EFI_STATUS bootloader_status = main();

  fini(bootloader_status);
  return bootloader_status;
}

EFI_STATUS init() {
  const EFI_LOADED_IMAGE_PROTOCOL* loaded_image_handle =
      get_loaded_image_protocol(g_image_handle);
  if (loaded_image_handle == NULL) {
    ERROR_PRINT(L"Failed to get loaded image protocol.\n\r");
    return EFI_LOAD_ERROR;
  }

  g_file_prot = get_volume_handle(loaded_image_handle);
  if (g_file_prot == NULL) {
    ERROR_PRINT(L"Failed to get volume handle.\n\r");
    g_system_table->BootServices->CloseProtocol(g_image_handle, &g_lip_guid,
                                                g_image_handle, NULL);
    return EFI_LOAD_ERROR;
  }
  g_system_table->BootServices->CloseProtocol(g_image_handle, &g_lip_guid,
                                              g_image_handle, NULL);

  EFI_FILE_PROTOCOL* kuro_dir;
  const EFI_STATUS kuro_dir_status = mkdir(&kuro_dir, L"\\kuro");
  EFI_FILE_PROTOCOL* config_dir;
  EFI_STATUS config_dir_status =
      mkdir_if_not_exists(&config_dir, L"\\kuro\\config");

  if (kuro_dir_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to create or open KURO directory.\n\r");
    return kuro_dir_status;
  }

  if (config_dir_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to create or open config directory.\n\r");
    return config_dir_status;
  }

  const EFI_STATUS config_status = init_config(g_file_prot, 0);
  if (config_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to initialize config.\n\r");
    return config_status;
  }

  const EFI_STATUS logger_status = init_logger(g_file_prot);
  if (logger_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to initialize logger.\n\r");
    return logger_status;
  }

  log("Disabling watchdog timer...\n");
  const EFI_STATUS disable_wd =
      g_system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0);

  if (disable_wd != EFI_SUCCESS) {
    log("Error: Failed to disable watchdog timer.\n");
    return disable_wd;
  }
  log("Watchdog timer disabled successfully.\n");
  return EFI_SUCCESS;
}

void fini(EFI_STATUS exit_status) {
  if (exit_status == EFI_SUCCESS) {
    log("KURO bootloader exited successfully.\n");
  } else {
    char exit_code[21];
    log("KURO bootloader exited with error: ");
    u64_to_str(exit_status, &exit_code[0]);
    log(&exit_code[0]);
    log("\n");
  }

  if (g_file_prot != NULL) {
    g_file_prot->Close(g_file_prot);
  }

  log("Closing config...\n");
  if (close_config() == EFI_SUCCESS) {
    log("Config closed successfully.\n");
  } else {
    log("Error: Failed to close config.\n");
  }
  log("Closing logger...\n");
  if (close_logger() != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to close logger.\n\r");
  }
}

EFI_STATUS main(void) {
  // Test config parsing by reading the kernel path from the config file and
  char kernel_path[256];
  get_config_key("kernel_path", kernel_path);
  CHAR16 kernel_path_wide[256];
  wchar(kernel_path, kernel_path_wide, sizeof(kernel_path_wide));

  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       L"Kernel path: ");
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       kernel_path_wide);

  g_system_table->BootServices->WaitForEvent(
      1, &g_system_table->ConIn->WaitForKey, NULL);

  return EFI_SUCCESS;
}