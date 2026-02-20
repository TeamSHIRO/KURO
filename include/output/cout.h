/*
 * File: cout.h
 * Description: Console output related macro.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_COUT_H
#define KURO_COUT_H

#include <__stddef_wchar_t.h>
#include <efi.h>

#include "../main.h"

// TODO: Remove this thang outta here boi

#ifdef NDEBUG
#define DEBUG_PRINT(str)
#else
#define DEBUG_PRINT(str)                                                   \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE); \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,             \
                                       L"[KURO|DEBUG] " str)
#endif

#define PRINT(str)                                                             \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTGRAY); \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,                 \
                                       L"[KURO|INFO] " str)

#define WARN_PRINT(str)                                                     \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_YELLOW); \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,              \
                                       L"[KURO|WARN] " str)

#define ERROR_PRINT(str)                                                 \
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_RED); \
  g_system_table->ConOut->OutputString(g_system_table->ConOut,           \
                                       L"[KURO|ERROR] " str)

#endif  // KURO_COUT_H
