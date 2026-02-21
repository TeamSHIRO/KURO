/*
 * File: logger.c
 * Description: Logging helpers.
 *
 * Copyright (C) 2026 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_BOOTLOADER_LOGGER_H
#define KURO_BOOTLOADER_LOGGER_H
#include <protocol/efi-fp.h>

extern EFI_FILE_PROTOCOL* log_dir;

EFI_STATUS init_logger(const EFI_FILE_PROTOCOL* volume_handle);
EFI_STATUS log(const char* buffer);
EFI_STATUS close_logger(void);

#endif  // KURO_BOOTLOADER_LOGGER_H
