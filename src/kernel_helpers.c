/*
 * File: kernel_helpers.c
 * Description: Bootloader-side helper callbacks for loaded kernels.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kernel_helpers.h"

#include <efi.h>
#include <stddef.h>

#include "main.h"

void __attribute__((sysv_abi)) kuro_kernel_print(const char* msg) {
  if (msg == NULL) {
    return;
  }

  g_system_table->ConOut->SetAttribute(g_system_table->ConOut, EFI_WHITE);

  while (*msg != '\0') {
    if (*msg == '\n') {
      CHAR16 nl[] = {(CHAR16)'\r', (CHAR16)'\n', (CHAR16)'\0'};
      g_system_table->ConOut->OutputString(g_system_table->ConOut, nl);
      msg++;
      continue;
    }

    CHAR16 ch[2];
    ch[0] = (CHAR16)(unsigned char)(*msg);
    ch[1] = L'\0';
    g_system_table->ConOut->OutputString(g_system_table->ConOut, ch);
    msg++;
  }
}
