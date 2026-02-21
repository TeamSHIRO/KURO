/*
 * File: main.h
 * Description: Common global variable.
 *
 * Copyright (C) 2025 TheMonHub
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_MAIN_H
#define KURO_MAIN_H

#include <efi.h>
#include <protocol/efi-fp.h>

extern EFI_HANDLE g_image_handle;
extern EFI_SYSTEM_TABLE* g_system_table;

extern EFI_FILE_PROTOCOL* g_file_prot;

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p);
EFI_STATUS init(void);
void fini(EFI_STATUS exit_status);
EFI_STATUS main(void);

#endif  // KURO_MAIN_H