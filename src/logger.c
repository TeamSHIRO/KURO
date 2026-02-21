/*
 * File: logger.c
 * Description: Logging helpers.
 *
 * Copyright (C) 2026 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger.h"

#include <protocol/efi-fp.h>

#include "config.h"
#include "cout.h"
#include "main.h"
#include "string.h"

EFI_FILE_PROTOCOL* log_dir = NULL;
EFI_FILE_PROTOCOL* log_file = NULL;

EFI_STATUS init_logger(const EFI_FILE_PROTOCOL* volume_handle) {
  char logger_path[256];
  get_config_key("logger_path", logger_path);
  CHAR16 logger_path_wide[256];
  wchar(logger_path, logger_path_wide, sizeof(logger_path_wide));
  const EFI_STATUS dir_status = volume_handle->Open(
      (EFI_FILE_PROTOCOL*)volume_handle, &log_dir, logger_path_wide,
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
      EFI_FILE_DIRECTORY);

  if (dir_status != EFI_SUCCESS) {
    log_dir = NULL;
    return dir_status;
  }

  const EFI_STATUS exist =
      log_dir->Open(log_dir, &log_file, L".\\log", EFI_FILE_MODE_READ, 0);
  if (exist == EFI_SUCCESS) {
    log_file->Delete(log_file);
  } else if (exist != 0x800000000000000E) {
    ERROR_PRINT(L"Failed to check log file existence.\n\r");
    return exist;
  }

  const EFI_STATUS file_status = log_dir->Open(
      log_dir, &log_file, L".\\log",
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  if (file_status != EFI_SUCCESS) {
    log_file = NULL;
    ERROR_PRINT(L"Failed to open log file.\n\r");
    log_dir->Close(log_dir);
    log_dir = NULL;
    return file_status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS log(const char* buffer) {
  UINT64 buffer_size = strlen(buffer);

  if (!log_file) {
    return EFI_NOT_READY;
  }

  const EFI_STATUS write_status =
      log_file->Write(log_file, &buffer_size, (void*)buffer);
  if (write_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to write to log file.\n\r");
  }

  return write_status;
}

EFI_STATUS close_logger() {
  if (!log_dir) {
    return EFI_NOT_READY;
  }

  const EFI_STATUS dir_close_status = log_dir->Close(log_dir);
  EFI_STATUS file_close_status = EFI_NOT_READY;
  if (log_file) {
    file_close_status = log_file->Close(log_file);
  }

  if (dir_close_status != EFI_SUCCESS) {
    return dir_close_status;
  }
  if (file_close_status != EFI_SUCCESS) {
    return file_close_status;
  }
  return EFI_SUCCESS;
}