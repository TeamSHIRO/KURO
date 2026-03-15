/*
 * File: kernel.c
 * Description: Minimal monolithic 64-bit kernel entry for KURO.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Define the function pointer type for the print function
typedef void (*kuro_kernel_print_fn)(const char*);

int _start(kuro_kernel_print_fn print) {
  // Use the provided print function to output a successful message
  print("Hello from Kernel!\n");
  return 42;
}