/*
* File: main.c
 * Description: Entry point.
 *
 * Copyright (C) 2025 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main.h"

#include <efi.h>

#include "file/file.h"

EFI_HANDLE image_handle;
EFI_SYSTEM_TABLE* system_table;

CHAR16* hello_str = L"Hello, World!\r\n";

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  image_handle = image_handle_p;
  system_table = system_table_p;

  system_table->ConOut->OutputString(system_table->ConOut,
                                     L"Hello, World!\r\n");

  EFI_FILE_PROTOCOL* volume = get_volume_handle(image_handle);

  EFI_FILE_PROTOCOL* file_protocol;
  volume->Open(volume, &file_protocol, L"something", EFI_FILE_MODE_READ, 0);

  UINTN file_size = (UINTN)get_file_size(file_protocol);

  void* input_buffer;
  if (system_table->BootServices->AllocatePool(EfiLoaderData, file_size,
                                          &input_buffer) != EFI_SUCCESS) {
    return EFI_OUT_OF_RESOURCES;
                                          }

  file_protocol->Read(file_protocol, &file_size, input_buffer);

  const UINTN char_count = file_size / sizeof(CHAR16);

  CHAR16* out;
  if (system_table->BootServices->AllocatePool(EfiLoaderData,
                                           (char_count + 3) * sizeof(CHAR16),
                                           (void**)&out) != EFI_SUCCESS) {
    return EFI_OUT_OF_RESOURCES;
                                           }

  const CHAR16* in = (CHAR16*)input_buffer;
  for (UINTN i = 0; i < char_count; ++i) {
    const CHAR16 c = in[i];
    out[i] = (c >> 8) | (c << 8);
  }

  out[char_count] = L'\r';
  out[char_count + 1] = L'\n';
  out[char_count + 2] = L'\0';

  system_table->ConOut->OutputString(system_table->ConOut, out);
  return (EFI_SUCCESS);
}