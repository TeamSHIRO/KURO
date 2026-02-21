/*
 * File: string.c
 * Description: String helpers.
 *
 * Copyright (C) 2026 Ellicode
 * Copyright (C) 2026 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "string.h"

#include <stddef.h>

static char* strtok_ptr = NULL;

char* strtok(char* str, const char* delim) {
  if (str != NULL) {
    strtok_ptr = str;
  }

  if (strtok_ptr == NULL) {
    return NULL;
  }

  // Skip leading delimiters
  while (*strtok_ptr != '\0') {
    const char* d = delim;
    int is_delim = 0;
    while (*d != '\0') {
      if (*strtok_ptr == *d) {
        is_delim = 1;
        break;
      }
      d++;
    }
    if (!is_delim) {
      break;
    }
    strtok_ptr++;
  }

  if (*strtok_ptr == '\0') {
    strtok_ptr = NULL;
    return NULL;
  }

  char* token = strtok_ptr;

  // Find the next delimiter
  while (*strtok_ptr != '\0') {
    const char* d = delim;
    while (*d != '\0') {
      if (*strtok_ptr == *d) {
        *strtok_ptr = '\0';
        strtok_ptr++;
        return token;
      }
      d++;
    }
    strtok_ptr++;
  }

  strtok_ptr = NULL;
  return token;
}

void strcat(char* dest, const char* src) {
  while (*dest != '\0') {
    dest++;
  }

  while (*src != '\0') {
    *dest = *src;
    dest++;
    src++;
  }

  *dest = '\0';
}

char* strchr(const char* str, int c) {
  while (*str != '\0') {
    if (*str == (char)c) {
      return (char*)str;
    }
    str++;
  }
  if (c == '\0') {
    return (char*)str;
  }
  return NULL;
}

int strcmp(const char* str1, const char* str2) {
  while (*str1 != '\0' && *str2 != '\0') {
    if (*str1 != *str2) {
      return (unsigned char)*str1 - (unsigned char)*str2;
    }
    str1++;
    str2++;
  }
  return (unsigned char)*str1 - (unsigned char)*str2;
}

void strcpy(char* dest, const char* src) {
  while (*src != '\0') {
    *dest = *src;
    dest++;
    src++;
  }
  *dest = '\0';
}

size_t strlen(const char* str) {
  size_t len = 0;
  while (*str != '\0') {
    len++;
    str++;
  }
  return len;
}

void wchar(const char* src, CHAR16* dest, size_t max_len) {
  size_t i = 0;
  while (src[i] != '\0' && i < max_len - 1) {
    dest[i] = (CHAR16)src[i];
    i++;
  }
  dest[i] = L'\0';
}

void u64_to_str(uint64_t value, char* buffer) {
  char temp[20];
  int i = 0;

  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }

  while (value > 0) {
    temp[i++] = (char)('0' + (uint8_t)(value % 10));
    value /= 10;
  }

  int j = 0;
  while (i > 0) {
    buffer[j++] = temp[--i];
  }

  buffer[j] = '\0';
}

void u16_to_str(uint16_t value, char* buffer) {
  char temp[5];
  int i = 0;

  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }

  while (value > 0) {
    temp[i++] = (char)('0' + (uint8_t)(value % 10));
    value /= 10;
  }

  int j = 0;
  while (i > 0) {
    buffer[j++] = temp[--i];
  }

  buffer[j] = '\0';
}