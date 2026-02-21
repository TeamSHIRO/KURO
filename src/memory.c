/*
 * File: memory.c
 * Description: Memory helpers.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <protocol/efi-fp.h>
#include <stddef.h>

#include "main.h"

EFI_STATUS malloc(UINT64 size, void** buffer) {
  return g_system_table->BootServices->AllocatePool(EfiLoaderData, size,
                                                    buffer);
}

void* memcpy(void* dest, const void* src, size_t n) {
  unsigned char* d = (unsigned char*)dest;
  const unsigned char* s = (const unsigned char*)src;

  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }

  return dest;
}

void* memset(void* s, int c, size_t n) {
  unsigned char* p = (unsigned char*)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (unsigned char)c;
  }

  return s;
}