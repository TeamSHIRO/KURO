/*
 * File: string.h
 * Description: String helpers.
 *
 * Copyright (C) 2026 Ellicode
 * Copyright (C) 2026 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_STRING_H
#define KURO_STRING_H

#include <efi.h>
#include <stddef.h>

char* strtok(char* str, const char* delim);
char* strchr(const char* str, int c);
void strcat(char* dest, const char* src);
int strcmp(const char* str1, const char* str2);
void strcpy(char* dest, const char* src);
size_t strlen(const char* str);
void wchar(const char* src, CHAR16* dest, size_t max_len);
void u64_to_str(uint64_t value, char* buffer);
void u16_to_str(uint16_t value, char* buffer);

#endif  // KURO_STRING_H
