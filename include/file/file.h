/*
 * File: file.h
 * Description: File-related functions.
 *
 * Copyright (C) 2025 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_DISK_H
#define KURO_DISK_H

#include <efi.h>
#include <protocol/efi-fp.h>

EFI_FILE_PROTOCOL get_volume_handle(EFI_HANDLE img_handle);
void open(EFI_FILE_PROTOCOL* volume, CHAR16* path, EFI_FILE_PROTOCOL* file);
void read(EFI_FILE_PROTOCOL* file, UINTN* size, void* buffer);
void close(EFI_FILE_PROTOCOL* file);
UINT64 get_file_size(EFI_FILE_PROTOCOL* file);

#endif  //KURO_DISK_H
