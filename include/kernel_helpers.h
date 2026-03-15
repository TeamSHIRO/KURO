/*
 * File: kernel_helpers.h
 * Description: Bootloader-side helper callbacks for loaded kernels.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_KERNEL_HELPERS_H
#define KURO_KERNEL_HELPERS_H

#include <efi.h>

typedef void(__attribute__((sysv_abi)) * kuro_kernel_print_fn)(const char*);

void __attribute__((sysv_abi)) kuro_kernel_print(const char* msg);

#endif  // KURO_KERNEL_HELPERS_H
