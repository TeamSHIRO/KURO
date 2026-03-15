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
#include <protocol/efi-gop.h>
#include <protocol/efi-lip.h>
#include <protocol/efi-sfsp.h>
#include <stddef.h>

#include "cout.h"
#include "file.h"
#include "logger.h"
#include "main.h"
#include "memory.h"
#include "string.h"

EFI_FILE_PROTOCOL* elf_file = NULL;
static struct kuro_boot_info g_boot_info;

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
    UINT64 remaining64 = size - read;
    UINTN chunk;

    /* Clamp to UINTN_MAX in case remaining64 exceeds what Read can take */
    if (remaining64 > (UINT64)(~(UINTN)0)) {
      chunk = (UINTN)(~(UINTN)0);
    } else {
      chunk = (UINTN)remaining64;
    }

    status = file->Read(file, &chunk, (void*)(buf + read));
    if (status != EFI_SUCCESS) return status;
    if (chunk == 0) return EFI_END_OF_FILE;

    read += (size_t)chunk;
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

    /* Validate segment bounds before loading:
     * - p_filesz <= p_memsz
     * - p_vaddr >= image_begin
     * - (p_vaddr - aligned_begin) + p_filesz <= image_size (with overflow check)
     */
    UINT64 p_vaddr = (UINT64)phdr->p_vaddr;
    UINT64 p_filesz = (UINT64)phdr->p_filesz;
    UINT64 p_memsz = (UINT64)phdr->p_memsz;

    if (p_filesz > p_memsz) {
      ERROR_PRINT(L"Malformed ELF segment: p_filesz > p_memsz.\n\r");
      return EFI_LOAD_ERROR;
    }

    if (p_vaddr < elf->image_begin) {
      ERROR_PRINT(L"Malformed ELF segment: p_vaddr before image_begin.\n\r");
      return EFI_LOAD_ERROR;
    }

    /* At this point p_vaddr >= image_begin >= aligned_begin, so subtraction is safe. */
    UINT64 segment_offset = p_vaddr - aligned_begin;

    if (segment_offset > image_size) {
      ERROR_PRINT(L"Malformed ELF segment: offset beyond image size.\n\r");
      return EFI_LOAD_ERROR;
    }

    if (p_filesz > image_size - segment_offset) {
      ERROR_PRINT(L"Malformed ELF segment: segment exceeds image size.\n\r");
      return EFI_LOAD_ERROR;
    }

    phdr_addr = elf->image_addr + segment_offset;
    status = efi_read_fixed(elf->kernel, phdr->p_offset, p_filesz,
                            (void*)phdr_addr);
    if (status != EFI_SUCCESS) {
      elf->system->BootServices->FreePages(elf->image_addr, elf->image_pages);
      elf->image_pages = 0;
      elf->image_addr = 0;
      elf->image_entry = 0;
      return status;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS prepare_kernel_boot_info(struct elf_app* elf) {
  EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
  EFI_STATUS status;

  memset(&g_boot_info, 0, sizeof(g_boot_info));

  status =
      elf->system->BootServices->LocateProtocol(&gop_guid, NULL, (void**)&gop);
  if (status != EFI_SUCCESS || gop == NULL || gop->Mode == NULL ||
      gop->Mode->Info == NULL) {
    ERROR_PRINT(L"Failed to locate GOP protocol.\n\r");
    return (status != EFI_SUCCESS) ? status : EFI_LOAD_ERROR;
  }

  g_boot_info.framebuffer.base = (UINT64)gop->Mode->FrameBufferBase;
  g_boot_info.framebuffer.size = (UINT64)gop->Mode->FrameBufferSize;
  g_boot_info.framebuffer.width = gop->Mode->Info->HorizontalResolution;
  g_boot_info.framebuffer.height = gop->Mode->Info->VerticalResolution;
  g_boot_info.framebuffer.pixels_per_scanline =
      gop->Mode->Info->PixelsPerScanLine;
  g_boot_info.framebuffer.pixel_format = (UINT32)gop->Mode->Info->PixelFormat;
  g_boot_info.framebuffer.red_mask = gop->Mode->Info->PixelInformation.RedMask;
  g_boot_info.framebuffer.green_mask =
      gop->Mode->Info->PixelInformation.GreenMask;
  g_boot_info.framebuffer.blue_mask =
      gop->Mode->Info->PixelInformation.BlueMask;
  g_boot_info.framebuffer.reserved_mask =
      gop->Mode->Info->PixelInformation.ReservedMask;

  return EFI_SUCCESS;
}

EFI_STATUS boot_elf(struct elf_app* elf) {
  int(__attribute__((sysv_abi)) * entry)(const struct kuro_boot_info*);
  int ret = 0;

  entry = (int(__attribute__((sysv_abi))*)(
      const struct kuro_boot_info*))elf->image_entry;
  ret = (*entry)(&g_boot_info);

  if (ret != 42) {
    ERROR_PRINT(L"ELF image returned unexpected value.\n\r");
    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS exit_boot_services() {
  EFI_STATUS status;
  UINTN map_size = 0, map_key, descriptor_size;
  UINT32 descriptor_version;
  EFI_MEMORY_DESCRIPTOR* memory_map = NULL;

  // Get the required memory map size
  status = g_system_table->BootServices->GetMemoryMap(
      &map_size, memory_map, &map_key, &descriptor_size, &descriptor_version);
  if (status != (EFI_ERR_MASK | EFI_BUFFER_TOO_SMALL)) {
    return status;
  }

  // Allocate enough pool for the memory map (with extra space)
  map_size += 2 * descriptor_size;
  status = g_system_table->BootServices->AllocatePool(EfiLoaderData, map_size,
                                                      (void**)&memory_map);
  if (status != EFI_SUCCESS) {
    return status;
  }

  // Get the actual memory map
  status = g_system_table->BootServices->GetMemoryMap(
      &map_size, memory_map, &map_key, &descriptor_size, &descriptor_version);
  if (status != EFI_SUCCESS) {
    g_system_table->BootServices->FreePool(memory_map);
    return status;
  }

  // Exit boot services
  status =
      g_system_table->BootServices->ExitBootServices(g_system_table, map_key);

  return status;
}
