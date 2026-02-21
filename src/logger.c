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
#include "main.h"
#include "string.h"

EFI_FILE_PROTOCOL* log_dir = NULL;
EFI_FILE_PROTOCOL* log_file = NULL;

EFI_STATUS init_logger(const EFI_FILE_PROTOCOL* volume_handle) {
  char logger_path[256];
  get_config_key("logger_path", logger_path);
  CHAR16 logger_path_wide[256];
  wchar(logger_path, logger_path_wide, sizeof(logger_path));
  const EFI_STATUS dir_status = volume_handle->Open(
      (EFI_FILE_PROTOCOL*)volume_handle, &log_dir, logger_path_wide,
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
      EFI_FILE_DIRECTORY);

  if (dir_status != EFI_SUCCESS) {
    log_dir = NULL;
    return dir_status;
  }

  EFI_TIME time;
  g_system_table->RuntimeServices->GetTime(&time, NULL);

  const CHAR16 year = time.Year;
  const CHAR16 month = time.Month;
  const CHAR16 day = time.Day;
  const CHAR16 hour = time.Hour;
  const CHAR16 minute = time.Minute;
  const CHAR16 second = time.Second;

  char year_str[6];
  char month_str[6];
  char day_str[6];
  char hour_str[6];
  char minute_str[6];
  char second_str[6];

  u16_to_str(year, year_str);
  u16_to_str(month, month_str);
  u16_to_str(day, day_str);
  u16_to_str(hour, hour_str);
  u16_to_str(minute, minute_str);
  u16_to_str(second, second_str);

  char log_file_name[34] = "log";
  strcat(log_file_name, year_str);
  strcat(log_file_name, month_str);
  strcat(log_file_name, day_str);
  strcat(log_file_name, hour_str);
  strcat(log_file_name, minute_str);
  strcat(log_file_name, second_str);

  CHAR16 log_file_name_wide[34];

  wchar(log_file_name, log_file_name_wide, sizeof(log_file_name));

  const EFI_STATUS file_status = log_dir->Open(
      log_dir, &log_file, log_file_name_wide,
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  if (file_status != EFI_SUCCESS) {
    log_file = NULL;
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
    g_system_table->ConOut->OutputString(g_system_table->ConOut, L"[KURO] Error: Failed to write to log file.\n\r");
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
