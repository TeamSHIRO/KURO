/*
 * File: file.c
 * Description: File-related functions.
 *
 * Copyright (C) 2025 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file/file.h"

#include <efi.h>
#include <protocol/efi-lip.h>
#include <protocol/efi-sfsp.h>
#include <stddef.h>

#include "main.h"

EFI_FILE_PROTOCOL* get_volume_handle(EFI_HANDLE img_handle) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image = NULL;
  EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* sfsp;
  EFI_GUID sfsp_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_FILE_PROTOCOL* volume_handle;

  g_system_table->BootServices->HandleProtocol(img_handle, &lip_guid,
                                             (void**)&loaded_image);
  g_system_table->BootServices->OpenProtocol(img_handle, &sfsp_guid, (void**)&sfsp, g_image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  g_system_table->BootServices->HandleProtocol(loaded_image->DeviceHandle,
                                             &sfsp_guid, (void**)&sfsp);
  sfsp->OpenVolume(sfsp, &volume_handle);

  return volume_handle;
}

UINT64 get_file_size(EFI_FILE_PROTOCOL* file) {
  // Nah, I'm going to use hack instead of the intended way of GetInfo() :3

  UINT64 size;
  file->SetPosition(file, 0xFFFFFFFFFFFFFFFF);
  file->GetPosition(file, &size);
  file->SetPosition(file, 0);
  return size;
}