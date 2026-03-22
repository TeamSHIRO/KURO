/*
 * File: cout.h
 * Description: Console output related helpers.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_COUT_H
#define KURO_COUT_H

#define KURO_PREFIX L"[ KURO ] "
#define LABEL_SEPARATOR L": "
#define INFO_LABEL L"INFO "
#define DEBUG_LABEL L"DEBUG"
#define ERROR_LABEL L"ERROR"
#define SUCCESS_LABEL L"OK   "
#define WARNING_LABEL L"WARN "

#include <__stddef_wchar_t.h>
#include <efi.h>

#include "main.h"

void DEBUG_PRINT(const CHAR16* str);
void ERROR_PRINT(const CHAR16* str);
void SUCCESS_PRINT(const CHAR16* str);
void WARNING_PRINT(const CHAR16* str);
void INFO_PRINT(const CHAR16* str);

#endif