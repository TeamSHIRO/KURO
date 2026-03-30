[◀ Documentation Home](README.md)

# Writing a Kernel for KURO

This guide explains how to write a 64-bit ELF kernel that is compatible with KURO's boot convention, from entry point to using the boot APIs.

## Table of Contents

- [Overview](#overview)
- [Entry Point](#entry-point)
- [Build Requirements](#build-requirements)
    - [Compiler Flags](#compiler-flags)
    - [Linker Script](#linker-script)
    - [CMake Example](#cmake-example)
- [The `boot_info.h` Header](#the-boot_infoh-header)
- [Using the Boot Info](#using-the-boot-info)
    - [Verifying Integrity](#verifying-integrity)
    - [Accessing the Framebuffer](#accessing-the-framebuffer)
    - [Parsing the Memory Map](#parsing-the-memory-map)
- [Returning from the Kernel](#returning-from-the-kernel)
- [Full Minimal Example](#full-minimal-example)

---

## Overview

KURO loads a **64-bit ELF** executable from the path defined by `kernel_path` in the config (default: `\shiro.kernel`). After mapping all `PT_LOAD` segments into memory and calling `ExitBootServices`, KURO jumps directly to the kernel's `_start` symbol with a single argument: a pointer to a `kuro_boot_info` structure.

```
KURO                                Your Kernel
  │                                     │
  ├─ Read kuro.conf ─────────────────── │
  ├─ Load ELF PT_LOAD segments ────────▶│
  ├─ Exit UEFI Boot Services ─────────  │
  └─ Jump to _start(boot_info) ────────▶ int _start(const struct kuro_boot_info*)
```

---

## Entry Point

Your kernel must export a C-linkage function named `_start` with this exact signature:

```c
int _start(const struct kuro_boot_info* boot_info);
```

KURO resolves the entry address from the ELF `e_entry` field, so your linker script **must** contain `ENTRY(_start)`.

> [!IMPORTANT]
> When `_start` is called, UEFI Boot Services are no longer available. Do not call any UEFI protocols or services from inside the kernel.

---

## Build Requirements

### Compiler Flags

Your kernel must be compiled as a **freestanding** 64-bit ELF. Do not link against libc, the Linux CRT, or any other host OS runtime.

```
-m64
-ffreestanding
-fpie
-fno-stack-protector
-mno-red-zone
-Wall
-Wextra
```

And the linker:

```
-m64
-pie
-nostdlib
-nostartfiles
-nodefaultlibs
-Wl,-T,linker.ld
-Wl,--build-id=none
-Wl,-z,max-page-size=0x1000
```

### Linker Script

A minimal linker script compatible with KURO:

```ld
ENTRY(_start)

SECTIONS {
    . = 0x100000;

    .text   : { *(.text*)   }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*)   }
    .bss    : {
        *(.bss*)
        *(COMMON)
    }
}
```

> [!NOTE]
> The base address `0x100000` (1 MiB) is a conventional starting point for kernels. KURO loads segments at whatever virtual address is specified in the ELF headers, so this is fully under your control.

### CMake Example

The `test-kernel/` directory in the KURO repository is a ready-to-use reference. Here is a minimal standalone `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_kernel LANGUAGES C)
set(CMAKE_C_STANDARD 17)

add_executable(my_kernel src/kernel.c)

target_compile_options(my_kernel PRIVATE
    -m64 -ffreestanding -fpie
    -fno-stack-protector -mno-red-zone
    -Wall -Wextra
)

target_include_directories(my_kernel PRIVATE
    include
    /path/to/KURO/include   # for boot_info.h
)

target_link_options(my_kernel PRIVATE
    -m64 -pie -nostdlib -nostartfiles -nodefaultlibs
    "-Wl,-T,${CMAKE_SOURCE_DIR}/linker.ld"
    -Wl,--build-id=none
    -Wl,-z,max-page-size=0x1000
)

set_target_properties(my_kernel PROPERTIES OUTPUT_NAME "kernel.bin")
```

---

## The `boot_info.h` Header

KURO ships a shared header at `include/boot_info.h` that defines all boot protocol structures. Copy it into your kernel's include path or reference it directly.

```c
#include "boot_info.h"
```

Key constants defined in the header:

| Constant                 | Value                          | Description                                      |
| ------------------------ | ------------------------------ | ------------------------------------------------ |
| `KURO_BOOT_INFO_MAGIC`   | `0x4B55524F` (`KURO` in ASCII) | Magic number to verify a valid boot info pointer |
| `KURO_BOOT_INFO_VERSION` | `1`                            | Current boot protocol version                    |

---

## Using the Boot Info

### Verifying Integrity

Always validate the `magic` and `version` fields before using any boot info data:

```c
int _start(const struct kuro_boot_info* boot_info) {
    if (boot_info->magic   != KURO_BOOT_INFO_MAGIC ||
        boot_info->version != KURO_BOOT_INFO_VERSION) {
        /* Boot info is corrupt or from an incompatible bootloader — halt */
        while (1) __asm__("hlt");
    }
    /* ... */
}
```

### Accessing the Framebuffer

```c
const struct kuro_framebuffer* fb = &boot_info->framebuffer;

/* Draw a single white pixel at (x=10, y=10) */
volatile uint32_t* pixels = (volatile uint32_t*)(uintptr_t)fb->base;
pixels[(uint64_t)10 * fb->pixels_per_scanline + 10] = 0xFFFFFFFF;
```

The pixel format is indicated by `fb->pixel_format`:

| Value | Constant             | Description                                                |
| ----- | -------------------- | ---------------------------------------------------------- |
| `0`   | `KURO_PIXEL_RGBR_8`  | 8-bit RGB, reserved byte                                   |
| `1`   | `KURO_PIXEL_BGRR_8`  | 8-bit BGR, reserved byte                                   |
| `2`   | `KURO_PIXEL_BITMASK` | Custom bitmask — use `red_mask`, `green_mask`, `blue_mask` |

See [`kuro_framebuffer`](KURO_FRAMEBUFFER.md) for the full structure reference and a pixel-encoding example.

### Parsing the Memory Map

```c
const struct kuro_memory_map* mm = &boot_info->memory_map;
const uint8_t* base = (const uint8_t*)(uintptr_t)mm->descriptors;

for (uint32_t i = 0; i < mm->descriptor_count; i++) {
    const struct kuro_memory_descriptor* desc =
        (const struct kuro_memory_descriptor*)
        (base + (size_t)i * mm->descriptor_size);

    if (desc->type == KURO_MEMORY_CONVENTIONAL) {
        uint64_t addr  = desc->physical_start;
        uint64_t bytes = desc->number_of_pages * 4096;
        /* This is freely usable RAM */
    }
}
```

> [!IMPORTANT]
> Always use `mm->descriptor_size` as the stride between descriptors rather than `sizeof(struct kuro_memory_descriptor)` — the size can vary between UEFI implementations.

See [`kuro_memory_map`](KURO_MEMORY_MAP.md) for the full structure reference and memory type table.

---

## Returning from the Kernel

Your `_start` function can return an `int`. KURO does not currently inspect the return value, but returning a recognizable constant (e.g. `42`) is useful for confirming successful execution during debugging.

If your kernel never returns, enter a halt loop:

```c
while (1) {
    __asm__("hlt");
}
```

---

## Full Minimal Example

```c
#include <stdint.h>
#include "boot_info.h"

int _start(const struct kuro_boot_info* boot_info) {

    /* 1. Verify boot info */
    if (boot_info->magic   != KURO_BOOT_INFO_MAGIC ||
        boot_info->version != KURO_BOOT_INFO_VERSION) {
        while (1) __asm__("hlt");
    }

    /* 2. Paint the entire screen a solid color */
    const struct kuro_framebuffer* fb = &boot_info->framebuffer;
    volatile uint32_t* pixels = (volatile uint32_t*)(uintptr_t)fb->base;

    for (uint32_t y = 0; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            pixels[(uint64_t)y * fb->pixels_per_scanline + x] = 0x002244FF;
        }
    }

    /* 3. Count usable RAM */
    const struct kuro_memory_map* mm = &boot_info->memory_map;
    const uint8_t* base = (const uint8_t*)(uintptr_t)mm->descriptors;
    uint64_t total = 0;

    for (uint32_t i = 0; i < mm->descriptor_count; i++) {
        const struct kuro_memory_descriptor* d =
            (const struct kuro_memory_descriptor*)
            (base + (size_t)i * mm->descriptor_size);
        if (d->type == KURO_MEMORY_CONVENTIONAL)
            total += d->number_of_pages * 4096;
    }

    /* 4. Halt */
    while (1) {
        __asm__("hlt");
    }

    return 42;
}
```

Compile this to `kernel.bin`, place it at `\shiro.kernel` on your ESP (or wherever `kernel_path` points in your config), and boot KURO.
