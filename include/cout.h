/*
 * File: cout.h
 * Description: Console output related macro.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_COUT_H
#define KURO_COUT_H

#define KURO_PREFIX L"[ KURO ] "
#define LABEL_SEPARATOR L": "
#define INFO_LABEL L"INFO "
#define ERROR_LABEL L"ERROR"
#define SUCCESS_LABEL L"OK   "

#include <__stddef_wchar_t.h>
#include <efi.h>

#include "main.h"

#ifdef NDEBUG
#define DEBUG_PRINT(str)
#else
#define DEBUG_PRINT(str)                                                       \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);     \
  g_system_table->ConOut->OutputString(g_system_table->ConOut, KURO_PREFIX);   \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTBLUE); \
  g_system_table->ConOut->OutputString(g_system_table->ConOut, INFO_LABEL);    \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);     \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,                 \
                                       LABEL_SEPARATOR str);
#endif

#define ERROR_PRINT(str)                                                      \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);    \
  g_system_table->ConOut->OutputString(g_system_table->ConOut, KURO_PREFIX);  \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTRED); \
  g_system_table->ConOut->OutputString(g_system_table->ConOut, ERROR_LABEL);  \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);    \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,                \
                                       LABEL_SEPARATOR str);

#define SUCCESS_PRINT(str)                                                     \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);     \
  g_system_table->ConOut->OutputString(g_system_table->ConOut, KURO_PREFIX);   \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut,                 \
                                       EFI_LIGHTGREEN);                        \
  g_system_table->ConOut->OutputString(g_system_table->ConOut, SUCCESS_LABEL); \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);     \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,                 \
                                       LABEL_SEPARATOR str);

#endif  // KURO_COUT_H
