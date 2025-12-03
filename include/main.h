/*
 * File: main.h
 * Description: Common global variable.
 *
 * Copyright (C) 2025 TheMonHub
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_MAIN_H
#define KURO_MAIN_H

#include <efi.h>

extern EFI_HANDLE g_image_handle;
extern EFI_SYSTEM_TABLE* g_system_table;

#endif  //KURO_MAIN_H