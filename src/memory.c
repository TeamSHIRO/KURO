/*
 * File: memory.c
 * Description: Memory helpers.
 *
 * Copyright (C) 2026 Ellicode
 * Copyright (C) 2026 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "main.h"

EFI_STATUS malloc(UINT64 size, void** buffer) {
  return g_system_table->BootServices->AllocatePool(EfiLoaderData, size,
                                                    buffer);
}

void* memcpy(void* dest, const void* src, size_t n) {
  unsigned char* d = dest;
  const unsigned char* s = src;

  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }

  return dest;
}

void* memset(void* s, int c, size_t n) {
  unsigned char* p = s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (unsigned char)c;
  }

  return s;
}