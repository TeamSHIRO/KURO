/*
 * File: load.h
 * Description: ELF-related helpers.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KURO_ELF_H
#define KURO_ELF_H

#include <efi.h>
#include <protocol/efi-fp.h>
#include <stdint.h>

struct elf64_ehdr {
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
};

struct elf64_phdr {
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
};

struct elf_app {
  struct EFI_SYSTEM_TABLE* system;
  struct EFI_FILE_PROTOCOL* kernel;
  struct elf64_ehdr header;
  struct elf64_phdr* program_headers;
  uint64_t image_begin;
  uint64_t image_end;
  uint64_t page_size;

  // Only populated when image is loaded into memory
  uint64_t image_pages;
  uint64_t image_addr;
  uint64_t image_entry;
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

struct kuro_boot_info {
  struct kuro_framebuffer framebuffer;
};

#define PT_LOAD 1

extern EFI_FILE_PROTOCOL* elf_file;

EFI_STATUS init_elf(EFI_FILE_PROTOCOL* volume_handle, CHAR16* kernel_path);
EFI_STATUS efi_read_fixed(struct EFI_FILE_PROTOCOL* file, UINT64 offset,
                          size_t size, void* dst);
EFI_STATUS load_elf(struct elf_app* app);
EFI_STATUS prepare_kernel_boot_info(struct elf_app* app);
EFI_STATUS boot_elf(struct elf_app* app);

EFI_STATUS exit_boot_services();

#endif