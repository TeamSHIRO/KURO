/*
 * File: config.c
 * Description: Configuration helpers.
 *
 * Copyright (C) 2026 Ellicode
 * Copyright (C) 2026 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <protocol/efi-fp.h>

#include "../include/file.h"
#include "../include/memory.h"
#include "../include/output/cout.h"
#include "../include/string.h"
#include "main.h"

EFI_FILE_PROTOCOL* config_dir;

EFI_STATUS read_config(char* buffer, UINT64 buffer_size) {
  if (!config_dir) {
    return EFI_NOT_READY;
  }

  UINT64 file_size = get_writable_file_size(config_dir);

  if (file_size > buffer_size) {
    return EFI_BUFFER_TOO_SMALL;
  }

  const EFI_STATUS read_status =
      config_dir->Read(config_dir, &file_size, buffer);

  return read_status;
}

EFI_STATUS get_config_key(const char* key, char* value) {
  char buffer[1024];
  const EFI_STATUS read_status = read_config(buffer, sizeof(buffer));

  if (read_status != EFI_SUCCESS) {
    return read_status;
  }

  const char* line = strtok(buffer, "\n");
  while (line != NULL) {
    // Skip lines that start with a hashtag (comments)
    if (line[0] == '#') {
      line = strtok(NULL, "\n");
      continue;
    }

    char* equal_sign = strchr(line, '=');
    if (equal_sign != NULL) {
      *equal_sign = '\0';  // Split the line into key and value
      const char* current_key = line;
      const char* current_value = equal_sign + 1;

      if (strcmp(current_key, key) == 0) {
        strcpy(value, current_value);
        return EFI_SUCCESS;
      }
    }
    line = strtok(NULL, "\n");
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS write_config(const char* buffer) {
  UINT64 buffer_size = strlen(buffer);

  if (!config_dir) {
    return EFI_NOT_READY;
  }

  const EFI_STATUS write_status =
      config_dir->Write(config_dir, &buffer_size, (void*)buffer);

  return write_status;
}

EFI_STATUS init_config(const EFI_FILE_PROTOCOL* volume_handle,
                       BOOLEAN break_on_error) {
  DEBUG_PRINT("Initializing configuration.\n\r");
  const EFI_STATUS dir_status = volume_handle->Open(
      (EFI_FILE_PROTOCOL*)volume_handle, &config_dir, L".\\kuro.conf",
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

  if (dir_status != EFI_SUCCESS) {
    return dir_status;
  }

  DEBUG_PRINT("Config file opened successfully.\n\r");
  // Check if a file is empty (newly created)
  const UINT64 file_size = get_writable_file_size(config_dir);

  if (file_size == 0 && !break_on_error) {
    // Write default config to a newly created file
    const char default_config[] =
        "# Default config file for KURO\n\r"
        "kernel_path=\\kuro.elf\n\r";

    const EFI_STATUS write_status = write_config(default_config);
    DEBUG_PRINT("Default config written successfully.\n\r");

    if (write_status != EFI_SUCCESS) {
      config_dir->Close(config_dir);
      config_dir = NULL;
      return write_status;
    }

    // Reset file position to beginning
    config_dir->SetPosition(config_dir, 0);
  }

  return EFI_SUCCESS;
}

EFI_STATUS close_config() {
  if (!config_dir) {
    return EFI_NOT_READY;
  }

  const EFI_STATUS close_status = config_dir->Close(config_dir);

  return close_status;
}