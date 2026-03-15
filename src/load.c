/*
 * File: load.c
 * Description: ELF loading functions.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "load.h"

#include <efi.h>
#include <protocol/efi-lip.h>
#include <protocol/efi-sfsp.h>
#include <stddef.h>

#include "cout.h"
#include "file.h"
#include "kernel_helpers.h"
#include "main.h"
#include "memory.h"
#include "string.h"

EFI_FILE_PROTOCOL* elf_file = NULL;

EFI_STATUS init_elf(EFI_FILE_PROTOCOL* volume_handle, CHAR16* kernel_path) {
  const EFI_STATUS open_status = volume_handle->Open(
      volume_handle, &elf_file, kernel_path, EFI_FILE_MODE_READ, 0);
  return open_status;
}

EFI_STATUS efi_read_fixed(struct EFI_FILE_PROTOCOL* file, UINT64 offset,
                          size_t size, void* dst) {
  EFI_STATUS status;
  unsigned char* buf = dst;
  size_t read = 0;

  status = file->SetPosition(file, offset);
  if (status != EFI_SUCCESS) return status;

  while (read < size) {
    UINT64 remains = size - read;

    status = file->Read(file, &remains, (void*)(buf + read));
    if (status != EFI_SUCCESS) return status;
    if (remains == 0) return EFI_END_OF_FILE;

    read += remains;
  }

  return EFI_SUCCESS;
}

EFI_STATUS load_elf(struct elf_app* elf) {
  if (elf->image_end <= elf->image_begin) {
    return EFI_LOAD_ERROR;
  }

  const UINT64 linked_begin = elf->image_begin;
  const UINT64 aligned_begin = linked_begin & ~(UINT64)(elf->page_size - 1);
  const UINT64 aligned_end =
      (elf->image_end + elf->page_size - 1) & ~(UINT64)(elf->page_size - 1);
  const UINT64 image_size = aligned_end - aligned_begin;
  const UINT64 pages = image_size / elf->page_size;

  UINT64 alloc_base = aligned_begin;
  EFI_STATUS status;

  status = elf->system->BootServices->AllocatePages(
      AllocateAddress, EfiLoaderData, pages, &alloc_base);
  if (status != EFI_SUCCESS) {
    // Fallback: load at any available address and rebase entry/segment
    // pointers.
    alloc_base = 0;
    status = elf->system->BootServices->AllocatePages(
        AllocateAnyPages, EfiLoaderData, pages, &alloc_base);
    if (status != EFI_SUCCESS) {
      ERROR_PRINT(L"Failed to allocate pages for kernel\n\r");
      return status;
    }
  }

  // Save some bookkeeping information for cleanup in case of errors
  elf->image_pages = pages;
  elf->image_addr = alloc_base;
  elf->image_entry = alloc_base + (elf->header.e_entry - aligned_begin);

  memset((void*)alloc_base, 0, image_size);

  for (size_t i = 0; i < elf->header.e_phnum; ++i) {
    const struct elf64_phdr* phdr = &elf->program_headers[i];
    UINT64 phdr_addr;

    if (phdr->p_type != PT_LOAD) continue;

    phdr_addr = elf->image_addr + (phdr->p_vaddr - aligned_begin);
    status = efi_read_fixed(elf->kernel, phdr->p_offset, phdr->p_filesz,
                            (void*)phdr_addr);
    if (status != EFI_SUCCESS) return status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS boot_elf(struct elf_app* elf) {
  int(__attribute__((sysv_abi)) * entry)(kuro_kernel_print_fn);
  int ret = 0;

  entry =
      (int(__attribute__((sysv_abi))*)(kuro_kernel_print_fn))elf->image_entry;
  ret = (*entry)(kuro_kernel_print);

  if (ret != 42) {
    ERROR_PRINT(L"ELF image returned unexpected value.\n\r");
    return EFI_LOAD_ERROR;
  }

  elf->kernel->Close(elf->kernel);
  return EFI_SUCCESS;
}
