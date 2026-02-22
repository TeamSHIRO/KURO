/*
 * File: file.h
 * Description: File-related functions.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_DISK_H
#define KURO_DISK_H

#include <efi.h>
#include <protocol/efi-fp.h>
#include <protocol/efi-lip.h>

extern EFI_GUID g_lip_guid;
extern EFI_GUID g_sfsp_guid;

EFI_LOADED_IMAGE_PROTOCOL* get_loaded_image_protocol(EFI_HANDLE image_handle);
EFI_FILE_PROTOCOL* get_volume_handle(
    const EFI_LOADED_IMAGE_PROTOCOL* loaded_image_protocol);
UINT64 get_writable_file_size(EFI_FILE_PROTOCOL* file);
static void write_string(EFI_FILE_PROTOCOL* file, const CHAR16* str);
EFI_STATUS mkdir_if_not_exists(EFI_FILE_PROTOCOL** dir, CHAR16* dir_name);

#endif  // KURO_DISK_H
