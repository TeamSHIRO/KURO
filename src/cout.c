/*
 * File: cout.c
 * Description: Console output related helpers.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cout.h"

#include "logger.h"
#include "main.h"

void DEBUG_PRINT(const CHAR16* str) {
#ifdef NDEBUG
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)KURO_PREFIX);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTBLUE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)DEBUG_LABEL);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)LABEL_SEPARATOR);
  g_system_table->ConOut->OutputString(g_system_table->ConOut, (CHAR16*)str);
  log((char*)str);
#endif
}

void ERROR_PRINT(const CHAR16* str) {
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)KURO_PREFIX);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTRED);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)ERROR_LABEL);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)LABEL_SEPARATOR);
  g_system_table->ConOut->OutputString(g_system_table->ConOut, (CHAR16*)str);

  log((char*)str);
}

void SUCCESS_PRINT(const CHAR16* str) {
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)KURO_PREFIX);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTGREEN);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)SUCCESS_LABEL);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)LABEL_SEPARATOR);
  g_system_table->ConOut->OutputString(g_system_table->ConOut, (CHAR16*)str);

  log((char*)str);
}

void WARNING_PRINT(const CHAR16* str) {
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)KURO_PREFIX);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_YELLOW);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)WARNING_LABEL);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)LABEL_SEPARATOR);
  g_system_table->ConOut->OutputString(g_system_table->ConOut, (CHAR16*)str);

  log((char*)str);
}

void INFO_PRINT(const CHAR16* str) {
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)KURO_PREFIX);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_LIGHTBLUE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)INFO_LABEL);
  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       (CHAR16*)LABEL_SEPARATOR);
  g_system_table->ConOut->OutputString(g_system_table->ConOut, (CHAR16*)str);

  log((char*)str);
}
