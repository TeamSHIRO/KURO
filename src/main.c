/*
 * File: main.c
 * Description: Entry point.
 *
 * Copyright (C) 2025-2026 TheMonHub
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main.h"

#include <efi.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "cout.h"
#include "file.h"
#include "load.h"
#include "logger.h"
#include "string.h"

EFI_HANDLE g_image_handle;
EFI_SYSTEM_TABLE* g_system_table;

EFI_FILE_PROTOCOL* g_file_prot = NULL;

EFI_STATUS efi_main(EFI_HANDLE image_handle_p,
                    EFI_SYSTEM_TABLE* system_table_p) {
  g_image_handle = image_handle_p;
  g_system_table = system_table_p;

  const EFI_STATUS init_status = init();
  if (init_status != EFI_SUCCESS) {
    fini(init_status);
    return init_status;
  }

  const EFI_STATUS bootloader_status = main();

  if (bootloader_status != EFI_SUCCESS) {
    fini(bootloader_status);
  }
  return bootloader_status;
}

EFI_STATUS init() {
  const EFI_LOADED_IMAGE_PROTOCOL* loaded_image_handle =
      get_loaded_image_protocol(g_image_handle);
  if (loaded_image_handle == NULL) {
    ERROR_PRINT(L"Failed to get loaded image protocol.\n\r");
    return EFI_LOAD_ERROR;
  }

  g_file_prot = get_volume_handle(loaded_image_handle);
  if (g_file_prot == NULL) {
    ERROR_PRINT(L"Failed to get volume handle.\n\r");
    g_system_table->BootServices->CloseProtocol(g_image_handle, &g_lip_guid,
                                                g_image_handle, NULL);
    return EFI_LOAD_ERROR;
  }
  g_system_table->BootServices->CloseProtocol(g_image_handle, &g_lip_guid,
                                              g_image_handle, NULL);

  EFI_FILE_PROTOCOL* kuro_dir;
  const EFI_STATUS kuro_dir_status = mkdir(&kuro_dir, L"\\kuro");
  EFI_FILE_PROTOCOL* temp_config_dir;
  const EFI_STATUS config_dir_status =
      mkdir(&temp_config_dir, L"\\kuro\\config");

  if (kuro_dir_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to create or open KURO directory.\n\r");
    return kuro_dir_status;
  }

  if (config_dir_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to create or open config directory.\n\r");
    return config_dir_status;
  }

  const EFI_STATUS config_status = init_config(g_file_prot);
  if (config_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to initialize config.\n\r");
    return config_status;
  }

  const EFI_STATUS logger_status = init_logger(g_file_prot);
  if (logger_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to initialize logger.\n\r");
    return logger_status;
  }

  char kernel_path[256];
  get_config_key("kernel_path", kernel_path);
  CHAR16 kernel_path_wide[256];
  wchar(kernel_path, kernel_path_wide, sizeof(kernel_path_wide));
  const EFI_STATUS loader_status = init_elf(g_file_prot, kernel_path_wide);
  if (loader_status == EFI_NOT_FOUND) {
    ERROR_PRINT(L"Kernel not found.\n\r");
    return loader_status;
  } else if (loader_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to initialize ELF loader.\n\r");
    return loader_status;
  }
  SUCCESS_PRINT(L"Kernel found at ");
  g_system_table->ConOut->OutputString(g_system_table->ConOut,
                                       kernel_path_wide);
  g_system_table->ConOut->OutputString(g_system_table->ConOut, L"\n\r");

  log("Disabling watchdog timer...\n");
  const EFI_STATUS disable_wd =
      g_system_table->BootServices->SetWatchdogTimer(0, 0xFFFFF, 0, 0);

  if (disable_wd != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to disable watchdog timer.\n\r");
    return disable_wd;
  }
  log("Watchdog timer disabled successfully.\n");

  return EFI_SUCCESS;
}

void fini(EFI_STATUS exit_status) {
  if (exit_status == EFI_SUCCESS) {
    log("KURO bootloader exited successfully.\n");
  } else {
    char exit_code[21];
    log("KURO bootloader exited with error: ");
    u64_to_str(exit_status, &exit_code[0]);
    log(&exit_code[0]);
    log("\n");
  }

  if (g_file_prot != NULL) {
    g_file_prot->Close(g_file_prot);
  }

  if (elf_file != NULL) {
    elf_file->Close(elf_file);
    elf_file = NULL;
  }

  log("Closing config...\n");
  if (close_config() == EFI_SUCCESS) {
    log("Config closed successfully.\n");
  } else {
    log("Error: Failed to close config.\n");
  }
  log("Closing logger...\n");
  if (close_logger() != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to close logger.\n\r");
  }
}

EFI_STATUS main(void) {
  struct elf_app app = {
      .system = g_system_table,
      .kernel = elf_file,
      .program_headers = NULL,
      .image_begin = UINT64_MAX,
      .image_end = 0,
      .page_size = 4096,
      .image_pages = 0,
      .image_addr = 0,
      .image_entry = 0,
  };

  EFI_STATUS status =
      efi_read_fixed(app.kernel, 0, sizeof(struct elf64_ehdr), &app.header);
  if (status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to read ELF header.\n\r");
    return status;
  }

  const UINTN phdrs_size =
      (UINTN)app.header.e_phnum * sizeof(struct elf64_phdr);
  status = g_system_table->BootServices->AllocatePool(
      EfiLoaderData, phdrs_size, (void**)&app.program_headers);
  if (status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to allocate ELF program headers.\n\r");
    return status;
  }

  status = efi_read_fixed(app.kernel, app.header.e_phoff, phdrs_size,
                          app.program_headers);
  if (status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to read ELF program headers.\n\r");
    g_system_table->BootServices->FreePool(app.program_headers);
    app.program_headers = NULL;
    return status;
  }

  for (UINTN i = 0; i < app.header.e_phnum; ++i) {
    const struct elf64_phdr* phdr = &app.program_headers[i];

    if (phdr->p_type != PT_LOAD) {
      continue;
    }

    if (phdr->p_vaddr < app.image_begin) {
      app.image_begin = phdr->p_vaddr;
    }

    const UINT64 segment_end = phdr->p_vaddr + phdr->p_memsz;
    if (segment_end > app.image_end) {
      app.image_end = segment_end;
    }
  }

  if (app.image_begin == UINT64_MAX || app.image_end <= app.image_begin) {
    ERROR_PRINT(L"Invalid ELF loadable segments.\n\r");
    g_system_table->BootServices->FreePool(app.program_headers);
    app.program_headers = NULL;
    return EFI_LOAD_ERROR;
  }
  status = load_elf(&app);
  g_system_table->BootServices->FreePool(app.program_headers);
  app.program_headers = NULL;

  if (status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to load ELF image.\n\r");
    return status;
  }

  SUCCESS_PRINT(L"ELF image loaded successfully.\n\r");

  char clear_screen[16] = {0};
  get_config_key("clear_screen", clear_screen);

  if (strcmp(clear_screen, "true") == 0) {
    g_system_table->ConOut->ClearScreen(g_system_table->ConOut);
  }

  status = prepare_kernel_boot_info(&app);
  if (status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to prepare kernel boot info.\n\r");
    return status;
  }

  EFI_STATUS exit_status = exit_boot_services();
  if (exit_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to exit boot services.\n\r");
    return exit_status;
  }

  EFI_STATUS boot_status = boot_elf(&app);
  if (boot_status != EFI_SUCCESS) {
    ERROR_PRINT(L"Failed to boot ELF image.\n\r");
    return boot_status;
  }

  return EFI_SUCCESS;
}