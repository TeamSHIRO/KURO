/*
 * File: logger.c
 * Description: Logging helpers.
 *
 * Copyright (C) 2026 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../include/logger.h"

#include <protocol/efi-fp.h>

// TODO: Implement a proper logger that writes to a file.

EFI_FILE_PROTOCOL* log_dir;

EFI_STATUS init_logger(const EFI_FILE_PROTOCOL* volume_handle) {
  const EFI_STATUS dir_status = volume_handle->Open(
      (EFI_FILE_PROTOCOL*)volume_handle, &log_dir, L".\\logs",
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
      EFI_FILE_DIRECTORY);

  if (dir_status != EFI_SUCCESS) {
    return dir_status;
  }

  return EFI_SUCCESS;
}