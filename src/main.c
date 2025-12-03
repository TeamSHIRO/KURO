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

EFI_HANDLE g_image_handle;
EFI_SYSTEM_TABLE* g_system_table;

CHAR16* hello_str = L"Hello, World!\r\n";

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  g_image_handle = image_handle_p;
  g_system_table = system_table_p;

  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                     L"Hello, World!\r\n");

  EFI_FILE_PROTOCOL* volume = get_volume_handle(g_system_table);

  EFI_FILE_PROTOCOL* file_protocol;
  volume->Open(volume, &file_protocol, L"something", EFI_FILE_MODE_READ, 0);

  UINTN file_size = (UINTN)get_file_size(file_protocol);

  void* input_buffer;
  g_system_table->BootServices->AllocatePool(EfiLoaderData, file_size,
                                           &input_buffer);

  file_protocol->Read(file_protocol, &file_size, input_buffer);

  const UINTN char_count = file_size / sizeof(CHAR16);

  CHAR16* out;
  g_system_table->BootServices->AllocatePool(EfiLoaderData,
                                           (char_count + 3) * sizeof(CHAR16),
                                           (void**)&out);

  const CHAR16* in = (CHAR16*)input_buffer;
  for (UINTN i = 0; i < char_count; ++i) {
    const CHAR16 c = in[i];
    out[i] = (c >> 8) | (c << 8);
  }

  out[char_count] = L'\r';
  out[char_count + 1] = L'\n';
  out[char_count + 2] = L'\0';

  g_system_table->ConOut->OutputString(g_system_table->ConOut, out);
  return (EFI_SUCCESS);
}