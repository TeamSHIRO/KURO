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
static EFI_MEMORY_DESCRIPTOR* g_efi_memory_map = NULL;
static UINTN g_efi_memory_map_capacity = 0;
static struct kuro_memory_descriptor* g_kuro_memory_map = NULL;
static UINTN g_kuro_memory_map_capacity = 0;

static void free_boot_memory_map_buffers() {
  if (g_efi_memory_map != NULL) {
    g_system_table->BootServices->FreePool(g_efi_memory_map);
    g_efi_memory_map = NULL;
    g_efi_memory_map_capacity = 0;
  }

  if (g_kuro_memory_map != NULL) {
    g_system_table->BootServices->FreePool(g_kuro_memory_map);
    g_kuro_memory_map = NULL;
    g_kuro_memory_map_capacity = 0;
  }
}

static EFI_STATUS ensure_boot_memory_map_buffers(UINTN required_map_size,
                                                 UINTN descriptor_size) {
  EFI_STATUS status;
  UINTN descriptor_count;
  UINTN kuro_map_size;

  if (descriptor_size == 0) {
    return EFI_LOAD_ERROR;
  }

  if (required_map_size > g_efi_memory_map_capacity) {
    if (g_efi_memory_map != NULL) {
      g_system_table->BootServices->FreePool(g_efi_memory_map);
      g_efi_memory_map = NULL;
      g_efi_memory_map_capacity = 0;
    }

    status = g_system_table->BootServices->AllocatePool(
        EfiLoaderData, required_map_size, (void**)&g_efi_memory_map);
    if (status != EFI_SUCCESS) {
      return status;
    }

    g_efi_memory_map_capacity = required_map_size;
  }

  descriptor_count = required_map_size / descriptor_size;
  if ((required_map_size % descriptor_size) != 0) {
    ++descriptor_count;
  }

  if (descriptor_count > ((UINTN)-1) / sizeof(struct kuro_memory_descriptor)) {
    return EFI_OUT_OF_RESOURCES;
  }

  kuro_map_size = descriptor_count * sizeof(struct kuro_memory_descriptor);
  if (kuro_map_size > g_kuro_memory_map_capacity) {
    if (g_kuro_memory_map != NULL) {
      g_system_table->BootServices->FreePool(g_kuro_memory_map);
      g_kuro_memory_map = NULL;
      g_kuro_memory_map_capacity = 0;
    }

    status = g_system_table->BootServices->AllocatePool(
        EfiLoaderData, kuro_map_size, (void**)&g_kuro_memory_map);
    if (status != EFI_SUCCESS) {
      return status;
    }

    g_kuro_memory_map_capacity = kuro_map_size;
  }

  return EFI_SUCCESS;
}

static EFI_STATUS store_boot_memory_map(UINTN map_size, UINTN descriptor_size) {
  UINTN descriptor_count;
  unsigned char* current;

  if (descriptor_size == 0 || map_size < descriptor_size) {
    return EFI_LOAD_ERROR;
  }

  descriptor_count = map_size / descriptor_size;
  current = (unsigned char*)g_efi_memory_map;

  for (UINTN i = 0; i < descriptor_count; ++i) {
    const EFI_MEMORY_DESCRIPTOR* source =
        (const EFI_MEMORY_DESCRIPTOR*)(current + (i * descriptor_size));
    struct kuro_memory_descriptor* dest = &g_kuro_memory_map[i];

    dest->type = source->Type;
    dest->reserved = 0;
    dest->physical_start = source->PhysicalStart;
    dest->virtual_start = source->VirtualStart;
    dest->number_of_pages = source->NumberOfPages;
    dest->attribute = source->Attribute;
  }

  g_boot_info.memory_map.descriptors = (UINT64)(UINTN)g_kuro_memory_map;
  g_boot_info.memory_map.size =
      (UINT64)(descriptor_count * sizeof(struct kuro_memory_descriptor));
  g_boot_info.memory_map.descriptor_size =
      sizeof(struct kuro_memory_descriptor);
  g_boot_info.memory_map.descriptor_count = (UINT32)descriptor_count;

  return EFI_SUCCESS;
}

EFI_STATUS init_elf(EFI_FILE_PROTOCOL* volume_handle, CHAR16* kernel_path) {
  const EFI_STATUS open_status = volume_handle->Open(
      volume_handle, &elf_file, kernel_path, EFI_FILE_MODE_READ, 0);
  if (open_status != EFI_SUCCESS) {
    WARNING_PRINT((CHAR16*)L"Failed to open ELF file. Trying fallback...\n\r");
    const EFI_STATUS fallback_status = volume_handle->Open(
        volume_handle, &elf_file, FALLBACK_KERNEL_PATH, EFI_FILE_MODE_READ, 0);
    if (fallback_status != EFI_SUCCESS) {
      ERROR_PRINT((CHAR16*)L"Failed to open fallback ELF file.\n\r");
      return fallback_status;
    }

    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

EFI_STATUS efi_read_fixed(struct EFI_FILE_PROTOCOL* file, UINT64 offset,
                          size_t size, void* dst) {
  EFI_STATUS status;
  unsigned char* buf = dst;
  size_t read = 0;

  status = file->SetPosition(file, offset);

  if (status != EFI_SUCCESS) {
    ERROR_PRINT((CHAR16*)L"Failed to set file position.\n\r");
    return status;
  }

  while (read < size) {
    UINT64 remaining64 = size - read;
    UINTN chunk;

    /* Clamp to UINTN_MAX in case remaining64 exceeds what Read can take */
    if (remaining64 > (UINT64)(~(UINTN)0)) {
      chunk = (~(UINTN)0);
    } else {
      chunk = (UINTN)remaining64;
    }

    status = file->Read(file, &chunk, (void*)(buf + read));

    if (status != EFI_SUCCESS) {
      ERROR_PRINT((CHAR16*)L"Failed to read file.\n\r");
      return status;
    }

    if (chunk == 0) {
      ERROR_PRINT((CHAR16*)L"Unexpected end of file.\n\r");
      return EFI_END_OF_FILE;
    }

    read += (size_t)chunk;
  }

  return EFI_SUCCESS;
}

EFI_STATUS load_elf(struct elf_app* elf) {
  if (elf->image_end <= elf->image_begin) {
    ERROR_PRINT((CHAR16*)L"Invalid or corrupted ELF image!\n\r");
    return EFI_LOAD_ERROR;
  }

  if (elf->page_size == 0) {
    ERROR_PRINT((CHAR16*)L"Invalid ELF page size!\n\r");
    return EFI_LOAD_ERROR;
  }

  const UINT64 linked_begin = elf->image_begin;
  const UINT64 page_mask = (UINT64)(elf->page_size - 1);
  /* Check for overflow in elf->image_end + page_mask */
  if (elf->image_end > (~(UINT64)0 - page_mask)) {
    ERROR_PRINT((CHAR16*)L"ELF image range overflows address space!\n\r");
    return EFI_LOAD_ERROR;
  }
  const UINT64 aligned_begin = linked_begin & ~page_mask;
  const UINT64 rounded_end = elf->image_end + page_mask;
  const UINT64 aligned_end = rounded_end & ~page_mask;
  /* Validate the aligned range */
  if (aligned_end <= aligned_begin || aligned_end < elf->image_end) {
    ERROR_PRINT((CHAR16*)L"Aligned ELF image range is invalid!\n\r");
    return EFI_LOAD_ERROR;
  }
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
      ERROR_PRINT((CHAR16*)L"Failed to allocate pages for kernel\n\r");
      return status;
    }
  }

  // Save some bookkeeping information for cleanup in case of errors
  /* Validate that the ELF entry point lies within the loaded image range
   * before computing image_entry. This ensures that the eventual jump
   * target is within [image_begin, image_end) and within the allocated
   * [aligned_begin, aligned_end) region.
   */
  UINT64 e_entry = (UINT64)elf->header.e_entry;
  if (e_entry < elf->image_begin || e_entry >= elf->image_end) {
    ERROR_PRINT(
        (CHAR16*)L"Malformed ELF: entry point outside image range.\n\r");
    elf->system->BootServices->FreePages(alloc_base, pages);
    elf->image_pages = 0;
    elf->image_addr = 0;
    elf->image_entry = 0;
    return EFI_LOAD_ERROR;
  }
  /* At this point we know:
   *   aligned_begin <= image_begin <= e_entry < image_end <= aligned_end
   * so (e_entry - aligned_begin) is within [0, image_size).
   */
  UINT64 entry_offset = e_entry - aligned_begin;
  // Save some bookkeeping information for cleanup in case of errors
  elf->image_pages = pages;
  elf->image_addr = alloc_base;
  elf->image_entry = alloc_base + entry_offset;

  memset((void*)alloc_base, 0, image_size);

  for (size_t i = 0; i < elf->header.e_phnum; ++i) {
    const struct elf64_phdr* phdr = &elf->program_headers[i];
    UINT64 phdr_addr;

    if (phdr->p_type != PT_LOAD) {
      continue;
    }

    /* Validate segment bounds before loading:
     * - p_filesz <= p_memsz
     * - p_vaddr >= image_begin
     * - (p_vaddr - aligned_begin) + p_filesz <= image_size (with overflow
     * check)
     */
    UINT64 p_vaddr = (UINT64)phdr->p_vaddr;
    UINT64 p_filesz = (UINT64)phdr->p_filesz;
    UINT64 p_memsz = (UINT64)phdr->p_memsz;

    if (p_filesz > p_memsz) {
      ERROR_PRINT((CHAR16*)L"Malformed ELF segment: p_filesz > p_memsz.\n\r");
      elf->system->BootServices->FreePages(elf->image_addr, elf->image_pages);
      elf->image_pages = 0;
      elf->image_addr = 0;
      elf->image_entry = 0;
      return EFI_LOAD_ERROR;
    }

    if (p_vaddr < elf->image_begin) {
      ERROR_PRINT(
          (CHAR16*)L"Malformed ELF segment: p_vaddr before image_begin.\n\r");
      elf->system->BootServices->FreePages(elf->image_addr, elf->image_pages);
      elf->image_pages = 0;
      elf->image_addr = 0;
      elf->image_entry = 0;
      return EFI_LOAD_ERROR;
    }

    /* At this point p_vaddr >= image_begin >= aligned_begin, so subtraction
     * is safe. */
    UINT64 segment_offset = p_vaddr - aligned_begin;

    if (segment_offset > image_size) {
      ERROR_PRINT(
          (CHAR16*)L"Malformed ELF segment: offset beyond image size.\n\r");
      elf->system->BootServices->FreePages(elf->image_addr, elf->image_pages);
      elf->image_pages = 0;
      elf->image_addr = 0;
      elf->image_entry = 0;
      return EFI_LOAD_ERROR;
    }

    if (p_filesz > image_size - segment_offset) {
      ERROR_PRINT(
          (CHAR16*)L"Malformed ELF segment: segment exceeds image size.\n\r");
      elf->system->BootServices->FreePages(elf->image_addr, elf->image_pages);
      elf->image_pages = 0;
      elf->image_addr = 0;
      elf->image_entry = 0;
      return EFI_LOAD_ERROR;
    }

    phdr_addr = elf->image_addr + segment_offset;
    status =
        efi_read_fixed(elf->kernel, phdr->p_offset, p_filesz, (void*)phdr_addr);
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

void gop_highest_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop, UINTN* mode_buffer) {
  UINTN max_width = 0;
  UINTN max_height = 0;

  for (UINTN i = 0; i < gop->Mode->MaxMode; ++i) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    UINTN info_size;
    if (gop->QueryMode(gop, i, &info_size, &info) == EFI_SUCCESS) {
      if ((UINTN)info->HorizontalResolution * (UINTN)info->VerticalResolution >
          max_width * max_height) {
        max_width = info->HorizontalResolution;
        max_height = info->VerticalResolution;
        *mode_buffer = i;
      }
      g_system_table->BootServices->FreePool(info);
    }
  }
}

EFI_STATUS validate_elf_headers(struct elf_app* elf) {
  /* Validate ELF header: magic, class, endianness, and program header entry
   * size. */

  if (elf->header.e_ident[0] != ELF_MAGIC || elf->header.e_ident[1] != 'E' ||
      elf->header.e_ident[2] != 'L' || elf->header.e_ident[3] != 'F') {
    ERROR_PRINT((CHAR16*)L"Invalid ELF magic.\n\r");
    return EFI_LOAD_ERROR;
  }

  /* Only support 64-bit little-endian ELF. */
  if (elf->header.e_ident[4] != 2 || /* ELFCLASS64 */
      elf->header.e_ident[5] != 1) { /* ELFDATA2LSB NOLINT */
    ERROR_PRINT((CHAR16*)L"Unsupported ELF class or endianness.\n\r");
    return EFI_LOAD_ERROR;
  }

  if (elf->header.e_phentsize != sizeof(struct elf64_phdr)) {
    ERROR_PRINT((CHAR16*)L"Unexpected ELF program header entry size.\n\r");
    return EFI_LOAD_ERROR;
  }

  if (elf->header.e_phnum == 0 || elf->header.e_phoff == 0) {
    ERROR_PRINT((CHAR16*)L"ELF file has no program headers.\n\r");
    return EFI_LOAD_ERROR;
  }

  /* Ensure e_phnum * sizeof(struct elf64_phdr) does not overflow UINTN. */
  const UINTN max_phnum = ((UINTN)-1) / sizeof(struct elf64_phdr);
  if ((UINTN)elf->header.e_phnum > max_phnum) {
    ERROR_PRINT((CHAR16*)L"Too many ELF program headers.\n\r");
    return EFI_LOAD_ERROR;
  }

  /* Validate machine, version, and type. */
  if (elf->header.e_machine != EM_X86_64) {
    ERROR_PRINT(
        (CHAR16*)L"Unsupported ELF machine type (expected x86_64).\n\r");
    return EFI_LOAD_ERROR;
  }
  /* e_version == 1: EV_CURRENT */
  if (elf->header.e_version != 1) {
    ERROR_PRINT((CHAR16*)L"Unsupported ELF version.\n\r");
    return EFI_LOAD_ERROR;
  }
  /* e_type: allow ET_EXEC (2) and ET_DYN (3) */
  if (elf->header.e_type != 2 && elf->header.e_type != 3) {
    ERROR_PRINT((CHAR16*)L"Unsupported ELF file type.\n\r");
    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS prepare_kernel_boot_info(struct elf_app* elf) {
  EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
  EFI_STATUS status;

  memset(&g_boot_info, 0, sizeof(g_boot_info));

  // Some basic verification information and shii.

  g_boot_info.magic = KURO_BOOT_INFO_MAGIC;
  g_boot_info.version = KURO_BOOT_INFO_VERSION;
  g_boot_info.size = sizeof(struct kuro_boot_info);

  // Find the GOP (Graphics Output Protocol)

  status =
      elf->system->BootServices->LocateProtocol(&gop_guid, NULL, (void**)&gop);
  if (status != EFI_SUCCESS || gop == NULL || gop->Mode == NULL ||
      gop->Mode->Info == NULL) {
    ERROR_PRINT((CHAR16*)L"Failed to locate GOP protocol.\n\r");
    return (status != EFI_SUCCESS) ? status : EFI_LOAD_ERROR;
  }

  // Finds the highest supported graphic mode and switches to it.

  UINTN graphic_mode = 0;
  gop_highest_mode(gop, &graphic_mode);
  status = gop->SetMode(gop, graphic_mode);
  if (status != EFI_SUCCESS) {
    ERROR_PRINT((CHAR16*)L"Failed to set GOP mode\n\r");
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

  if (ret != KURO_RETURN_MAGIC_NUMBER) {
    ERROR_PRINT((CHAR16*)L"ELF image returned unexpected value.\n\r");
    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS exit_boot_services() {
  EFI_STATUS status = EFI_LOAD_ERROR;
  UINTN map_size = 0;
  UINTN map_key = 0;
  UINTN descriptor_size = 0;
  UINT32 descriptor_version = 0;

  const UINTN max_attempts = 8;

  // The function has 8 attempts to get the memory map. If it still fails, the
  // bootloader will throw an error

  for (UINTN attempt = 0; attempt < max_attempts; ++attempt) {
    map_size = 0;
    status = g_system_table->BootServices->GetMemoryMap(
        &map_size, NULL, &map_key, &descriptor_size, &descriptor_version);
    if (status != (EFI_ERR_MASK | EFI_BUFFER_TOO_SMALL)) {  // Thanks, Mono!
      ERROR_PRINT((CHAR16*)L"Failed to get memory map.\n\r");
      break;
    }

    if (map_size > ~(UINTN)0 - (2 * descriptor_size)) {
      status = EFI_OUT_OF_RESOURCES;
      break;
    }

    map_size += 2 * descriptor_size;
    status = ensure_boot_memory_map_buffers(map_size, descriptor_size);
    if (status != EFI_SUCCESS) {
      ERROR_PRINT((CHAR16*)L"Failed to ensure boot memory map buffers.\n\r");
      break;
    }

    status = g_system_table->BootServices->GetMemoryMap(
        &map_size, g_efi_memory_map, &map_key, &descriptor_size,
        &descriptor_version);
    if (status == EFI_BUFFER_TOO_SMALL) {
      WARNING_PRINT((CHAR16*)L"Memory map buffer too small, retrying...\n\r");
      continue;
    }

    if (status != EFI_SUCCESS) {
      ERROR_PRINT((CHAR16*)L"Failed to get memory map.\n\r");
      break;
    }

    status = store_boot_memory_map(map_size, descriptor_size);
    if (status != EFI_SUCCESS) {
      ERROR_PRINT((CHAR16*)L"Failed to store boot memory map.\n\r");
      break;
    }

    status =
        g_system_table->BootServices->ExitBootServices(g_system_table, map_key);

    if (status == EFI_SUCCESS) {
      return EFI_SUCCESS;
    }

    if (status != EFI_INVALID_PARAMETER) {
      ERROR_PRINT((CHAR16*)L"Failed to exit boot services.\n\r");
      break;
    }
  }

  free_boot_memory_map_buffers();
  return status;
}
