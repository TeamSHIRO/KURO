/*
 * File: string.h
 * Description: String helpers.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_STRING_H
#define KURO_STRING_H

#include <efi.h>
#include <stddef.h>

char* strtok(char* str, const char* delim);
char* strchr(const char* str, int c);
int strcmp(const char* str1, const char* str2);
void strcpy(char* dest, const char* src);
size_t strlen(const char* str);
void wchar(const char* src, CHAR16* dest, size_t max_len);

#endif  // KURO_STRING_H
