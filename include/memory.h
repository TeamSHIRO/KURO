/*
 * File: memory.h
 * Description: Memory helpers.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_BOOTLOADER_MEMORY_H
#define KURO_BOOTLOADER_MEMORY_H

#include <protocol/efi-fp.h>
#include <stddef.h>

EFI_STATUS malloc(UINT64 size, void** buffer);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);

#endif  // KURO_MEMORY_H