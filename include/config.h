/*
 * File: config.h
 * Description: Configuration helpers.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_BOOTLOADER_CONFIG_H
#define KURO_BOOTLOADER_CONFIG_H
#include <protocol/efi-fp.h>

extern EFI_FILE_PROTOCOL* config_dir;

EFI_STATUS init_config(const EFI_FILE_PROTOCOL* volume_handle);
EFI_STATUS read_config(char* buffer, UINT64 buffer_size);
EFI_STATUS get_config_key(const char* key, char* value);
EFI_STATUS write_config(const char* buffer);
EFI_STATUS close_config();

#endif  // KURO_CONFIG_H