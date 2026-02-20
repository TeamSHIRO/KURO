/*
 * File: file.c
 * Description: File-related functions.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../include/file.h"

#include <efi.h>
#include <protocol/efi-lip.h>
#include <protocol/efi-sfsp.h>
#include <stddef.h>

#include "main.h"

EFI_GUID g_lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID g_sfsp_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

EFI_LOADED_IMAGE_PROTOCOL* get_loaded_image_protocol(EFI_HANDLE image_handle) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;

  if (g_system_table->BootServices->OpenProtocol(
          image_handle, &g_lip_guid, (void**)&loaded_image, g_image_handle,
          NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL) != EFI_SUCCESS) {
    return NULL;
  }

  return loaded_image;
}

EFI_FILE_PROTOCOL* get_volume_handle(
    const EFI_LOADED_IMAGE_PROTOCOL* loaded_image_protocol) {
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* sfsp;

  EFI_FILE_PROTOCOL* volume_handle;

  if (g_system_table->BootServices->OpenProtocol(
          loaded_image_protocol->DeviceHandle, &g_sfsp_guid, (void**)&sfsp,
          g_image_handle, NULL,
          EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL) == EFI_SUCCESS) {
    if (sfsp->OpenVolume(sfsp, &volume_handle) != EFI_SUCCESS) {
      return NULL;
    }
  } else {
    return NULL;
  }

  return volume_handle;
}

UINT64 get_writable_file_size(EFI_FILE_PROTOCOL* file) {
  UINT64 size = 0;
  if (file->SetPosition(file, 0xFFFFFFFFFFFFFFFF) != EFI_SUCCESS) {
    return 0;
  }
  file->GetPosition(file, &size);
  file->SetPosition(file, 0);
  return size;
}

// Helper to write wide string to file
static void write_string(EFI_FILE_PROTOCOL* file, const CHAR16* str) {
  UINTN len = 0;
  while (str[len]) len++;
  UINTN size = len * sizeof(CHAR16);
  file->Write(file, &size, (void*)str);
}