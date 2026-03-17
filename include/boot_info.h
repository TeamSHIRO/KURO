/*
 * File: boot_info.h
 * Description: Shared boot protocol between KURO and loaded kernels.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_BOOT_INFO_H
#define KURO_BOOT_INFO_H

#include <stdint.h>

#define KURO_BOOT_INFO_MAGIC 0x4b55524full
#define KURO_BOOT_INFO_VERSION 1u

enum kuro_memory_type {
  KURO_MEMORY_RESERVED = 0,
  KURO_MEMORY_LOADER_CODE = 1,
  KURO_MEMORY_LOADER_DATA = 2,
  KURO_MEMORY_BOOT_SERVICES_CODE = 3,
  KURO_MEMORY_BOOT_SERVICES_DATA = 4,
  KURO_MEMORY_RUNTIME_SERVICES_CODE = 5,
  KURO_MEMORY_RUNTIME_SERVICES_DATA = 6,
  KURO_MEMORY_CONVENTIONAL = 7,
  KURO_MEMORY_UNUSABLE = 8,
  KURO_MEMORY_ACPI_RECLAIM = 9,
  KURO_MEMORY_ACPI_NVS = 10,
  KURO_MEMORY_MMIO = 11,
  KURO_MEMORY_MMIO_PORT_SPACE = 12,
  KURO_MEMORY_PAL_CODE = 13,
  KURO_MEMORY_PERSISTENT = 14,
};

struct kuro_framebuffer {
  uint64_t base;
  uint64_t size;
  uint32_t width;
  uint32_t height;
  uint32_t pixels_per_scanline;
  uint32_t pixel_format;
  uint32_t red_mask;
  uint32_t green_mask;
  uint32_t blue_mask;
  uint32_t reserved_mask;
};

struct kuro_memory_descriptor {
  uint32_t type;
  uint32_t reserved;
  uint64_t physical_start;
  uint64_t virtual_start;
  uint64_t number_of_pages;
  uint64_t attribute;
};

struct kuro_memory_map {
  uint64_t descriptors;
  uint64_t size;
  uint32_t descriptor_size;
  uint32_t descriptor_count;
};

struct kuro_boot_info {
  uint64_t magic;
  uint32_t version;
  uint32_t size;
  struct kuro_framebuffer framebuffer;
  struct kuro_memory_map memory_map;
};

#endif
