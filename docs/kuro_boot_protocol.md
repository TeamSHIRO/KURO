<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_banner_dark.png">
  <img src="res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO Boot Protocol

***Revision `1` Errata `A`***

**2026-04-13**

## Table of Contents

1. [Introduction](#1-introduction)
    1. [About KURO](#11-about-kuro)
    2. [Target Audience](#12-target-audience)
    3. [Conventions](#13-conventions)
        1. [Typography](#131-typography)
        2. [Revisions](#132-revisions)
        3. [Data Structures](#133-data-structures)
2. [Executable Structure](#2-executable-structure)
3. [KURO Footer](#3-kuro-footer)
4. [KURO Identifier](#4-kuro-identifier)
5. [Calling Convention and Registers](#5-calling-convention-and-registers)
    1. [x86-64 Registers](#51-x86-64-registers)
    2. [ARM64 Registers](#52-arm64-registers)
6. [KURO Boot Information](#6-kuro-boot-information)
7. [KURO Memory Map](#7-kuro-memory-map)
8. [KURO Module](#8-kuro-module)
9. [KURO Framebuffer](#9-kuro-framebuffer)
    1. [KURO Pixel Format](#91-kuro-pixel-format)
    2. [KURO Pixel Information](#92-kuro-pixel-information)
10. [KURO Executable Information](#10-kuro-executable-information)
    1. [KuroSegmentInfo](#101-kurosegmentinfo)
11. [Memory Layout](#11-memory-layout)
     1. [Lower Half](#111-lower-half)
     2. [Higher Half](#112-higher-half)
12. [Appendix A: Bootloader Identifier String](#appendix-a-bootloader-identifier-string)
13. [Appendix B: Legacy Boot Protocols](#appendix-b-legacy-boot-protocols)
14. [Appendix C: Changes](#appendix-c-changes)
15. [Contact](#contact)
16. [Copyright](#copyright)
17. [References](#references)

## 1. Introduction

This document describes the protocols used by the KURO bootloader to load executables.

This document is intended to provide a reference for developers who want to create executables that can be loaded by the
KURO bootloader or to create their own bootloader that follows the KURO boot protocol.

## 1.1 About KURO

KURO is both a bootloader and boot protocol. It is designed to be opinionated, secure, and minimalistic.

It supports UEFI `x86-64` and `ARM64` environments.

## 1.2 Target Audience

Kernel developers, operating system developers, and anyone inbetween who wants to develop a bootloader that is
compliant with the KURO boot protocol or kernel developers who want to create an executable that can be loaded by
the KURO bootloader.

## 1.3 Conventions

These are the conventions used throughout this document.

### 1.3.1 Typography

      The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL
      NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and
      "OPTIONAL" in this document are to be interpreted as described in
      RFC 2119.

### 1.3.2 Revisions

Updates to this document are considered either revisions or errata as described below:
- A new revision is made when significant changes are made to the document. Which changes the behavior of the
  bootloader, executable, or the booting process such as adding a new feature or fixing a bug.
- Errata version is made when a typo is found in the document or a minor change is made to the document that does not
  affect the behavior of the bootloader, executable, or the booting process such as fixing a typo or updating a link.

> [!CAUTION]
> Not all commits of the document are considered final. Please refer to the git tag for each final version of the
> document.

### 1.3.3 Data Structures

All data structures are presented in C syntax with the padding implicitly added to ensure that the structure alignment
follows the standard C alignment rules.

All the fields must not be null or 0 unless otherwise specified.

All the pointers are in the higher half of the address space.

## 2. Executable Structure

A valid KURO executable must be a Position-independent executable (PIE).

The following diagram shows an example structure of a KURO executable:

PLACEHOLDER – put the diagram here

![Executable structure diagram](res/kuro_execstruct.png)

- The executable must be a position-independent executable (PIE) with ELF header field `e_type` set to `ET_DYN`.
- The executable must contain a KURO footer at the end of the file.

The bootloader must verify the ELF header and KURO footer to ensure that the executable is a valid KURO executable.

## 3. KURO Footer

The KURO footer is a fixed-size data structure located at the end of the file. It contains the signature of
the file and the KURO identifier that is used to verify the authenticity of the file that is loaded by the bootloader.

Starting from 72 bytes before the end of the executable, lies the KURO footer. The KURO footer contains the following
fields:

```c++
typedef struct {
    char k_signature[64];
    KuroIdentifier k_identifier;
} KuroFooter;
```

> [!TIP]
> You can create a KURO footer on an existing ELF executable that follows [section 2](#2-executable-structure) by using
> the `kuro-sign`[^6] tool.

#### k_signature

The signature is an Ed25519[^3] signature of the executable to verify its authenticity. The signature is calculated over
the entire executable file, excluding the KURO footer itself.

The bootloader must verify the signature of the file before loading it fully except for when the UEFI secure boot is
disabled.

The bootloader must ensure that the public key and configuration are not tampered with.

#### k_identifier

As described in [section 4.1](#4-kuro-identifier).

## 4. KURO Identifier

KURO identifier is a fixed-size structure that contains the magic number and version information used to identify the
validity of a file and structure.

```c++
typedef struct {
    char k_magic0;
    char k_magic1;
    char k_magic2;
    char k_magic3;
    char k_magic4;
    uint8_t k_version;
    char k_reserved[2];
} KuroIdentifier;
```

#### k_magic

The magic number is `0x7F` followed by `KURO` in ASCII, which is `0x4B 0x55 0x52 0x4F` in hexadecimal.

If the magic number does not match the expected magic number, it must not be loaded.

| Byte     | Value  |
|----------|--------|
| k_magic0 | `0x7F` |
| k_magic1 | `0x4B` |
| k_magic2 | `0x55` |
| k_magic3 | `0x52` |
| k_magic4 | `0x4F` |

#### k_version

The sixth byte of the KURO identifier is used to identify the version of the KURO boot protocol used by the
executable.

If the version does not match the supported version, it must not be loaded.

Any other undefined version number is considered reserved for future use.

| Revision | Description                        |
|----------|------------------------------------|
| `0`      | Invalid Revision                   |
| `1`      | KURO Boot Protocol `Legacy 1.0`    |
| `2`      | KURO Boot Protocol `Legacy 2.0`    |
| `3`      | KURO Boot Protocol `1.0` (current) |

Information regarding the legacy boot protocols can be found in [appendix B](#appendix-b-legacy-boot-protocols).

## 5. Calling Convention and Registers

As before the bootloader transfers control to the entry point of the executable, the bootloader must prepare the
boot information structure and pass it to the executable following the ABI/calling convention in the register for each
architecture.

### 5.1 x86-64 Registers

Following the System V AMD64 ABI[^4].

`rdi` - Pointer to the KURO boot information.

### 5.2 ARM64 Registers

Following the Procedure Call standard for the Arm 64-bit Architecture[^5].

`x0` - Pointer to the KURO boot information.

## 6. KURO Boot Information

The boot information structure is a data structure that contains information that the executable must use. The
structure contains the following fields:

```c++
typedef struct {
    KuroIdentifier kb_identifier;
    char *kb_boot_id;
    char *kb_cmdline;
    EFI_SYSTEM_TABLE *kb_system_table;
    KuroMemoryMap *kb_memory_map;
    KuroModule *kb_module;
    KuroFramebuffer *kb_framebuffer;
    KuroExecutableInfo *kb_executable_info;
} KuroBootInfo;
```

#### kb_identifier

As described in [section 4](#4-kuro-identifier).

#### kb_boot_id

Points to a null-terminated string that contains the boot identifier of the bootloader.
See [appendix A](#appendix-a-bootloader-identifier-string) for the list of boot identifiers.

#### kb_cmdline

Points to a null-terminated string that contains the command line passed to the executable.
The command line is passed to the executable as-is and does not contain any modifications.

#### kb_system_table

Pointer to the EFI system table. Please refer to the UEFI specification[^2] for more information about the EFI system table.

#### kb_memory_map

Pointer to the memory map structure.
As described in [section 7](#7-kuro-memory-map).

#### kb_module

Pointer to the module structure.
As described in [section 8](#8-kuro-module).

This field can be null if no module is loaded.

#### kb_framebuffer

Pointer to the framebuffer structure.
As described in [section 9](#9-kuro-framebuffer).

This field can be null if no framebuffer is available.

#### kb_executable_info

Pointer to the executable information structure.
As described in [section 10](#10-kuro-executable-information).

## 7. KURO Memory Map

The memory map structure is a data structure that contains information about the memory map of the system. The
structure contains the following fields:

```c++
typedef struct {
    EFI_MEMORY_DESCRIPTOR *km_map;
    uint64_t km_map_size;
    uint64_t km_desc_size;
    uint32_t km_desc_version;
    uint64_t km_higher_half_base;
} KuroMemoryMap;
```

> [!TIP]
> This structure is heavily based on the UEFI specification[^2].

#### km_map

Point to the start of the memory map array as defined in the UEFI specification[^2].

#### km_map_size

Specifies the size of the memory map array in bytes.

#### km_desc_size

Specifies the size of each memory descriptor in bytes.

#### km_desc_version

Specifies the version of the memory descriptor structure as defined in the UEFI specification[^2].

#### km_higher_half_base

Specifies the base address of the higher half in the physical address.
See [section 11.2](#112-higher-half) for more information.

## 8. KURO Module

The module structure is a data structure that contains information about the module that is loaded into memory. The
structure contains the following fields:

```c++
typedef struct {
    void *km_module_base;
    uint64_t km_module_size;
} KuroModule;
```

#### km_module_base

Points to the base address of the module.

#### km_module_size

Specifies the size of the module in bytes.

### 8.1 Module Structure

A module is an arbitrary binary file loaded into memory by the bootloader.
The module must contain a KURO footer at the end of the file. There must be only one module loaded.

The bootloader must verify the KURO footer and then load the module into memory as-is but excluding the KURO footer.

The structure of a module can be found in the following diagram:

PLACEHOLDER – put the diagram here

![Module structure diagram](res/kuro_modstruct.png)

The KURO footer is described in [section 3](#3-kuro-footer).

## 9. KURO Framebuffer

The framebuffer structure is a data structure that contains information about the framebuffer. The structure contains
the following fields:

```c++
typedef struct {
    uint64_t kf_base;
    uint64_t kf_size;
    uint32_t kf_width;
    uint32_t kf_height;
    uint32_t kf_pixels_per_scanline;
    KuroPixelFormat kf_pixel_format;
    KuroPixelInfo kf_pixel_info;
} KuroFramebuffer;
```

#### kf_base

Specifies the base address of the framebuffer in the physical address.

#### kf_size

Specifies the size of the framebuffer in bytes.

#### kf_width

Specifies the horizontal resolution of the framebuffer.

#### kf_height

Specifies the vertical resolution of the framebuffer.

#### kf_pixels_per_scanline

Specifies the number of pixels per scanline.

#### kf_pixel_format

As described in [section 9.1](#91-kuro-pixel-format).

#### kf_pixel_info

As described in [section 9.2](#92-kuro-pixel-information).

### 9.1 KURO Pixel Format

```c++
typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} KuroPixelFormat;
```

> [!NOTE]
> Please refer to the UEFI specification[^2] for more information about `EFI_GRAPHICS_PIXEL_FORMAT`

### 9.2 KURO Pixel Information

```c++
typedef struct {
    uint32_t kp_red_mask;
    uint32_t kp_green_mask;
    uint32_t kp_blue_mask;
    uint32_t kp_reserved_mask;
} KuroPixelInfo;
```

> [!NOTE]
> Please refer to the UEFI specification[^2] for more information about `EFI_PIXEL_BITMASK`

## 10. KURO Executable Information

The executable information structure is a data structure that contains information about the loaded executable. The
structure contains the following fields:

```c++
typedef struct {
    uint64_t ke_entry_point;
    uint64_t ke_segment_count;
    KuroSegmentInfo* ke_segments;
    uint64_t ke_stack_start;
    uint64_t ke_stack_size;
} KuroExecutableInfo;
```

#### ke_entry_point

This field contains the address to the entry point of the executable in physical address.

#### ke_segment_count

This field specifies the number of entries in the `ke_segments` array.

#### ke_segments

Points to an array of `KuroSegmentInfo` structures as defined in [section 10.1](#101-kurosegmentinfo).

#### ke_stack_start

Specifies the top of the stack in physical address.

#### ke_stack_size

Specifies the size of the stack in bytes. The stack size is implementation-defined.

### 10.1 KuroSegmentInfo

`KuroSegmentInfo` is a structure that describes a segment of the executable that has been loaded into memory. It
contains information about each segment, such as its address, size, permission, and alignment.

Each `KuroSegmentInfo` structure is defined as follows:

```c++
typedef struct {
    uint32_t ks_flags;
    uint64_t ks_address;
    uint64_t ks_size;
    uint64_t ks_align;
} KuroSegmentInfo;
```

> [!TIP]
> This structure depends heavily on the ELF program header structure, so it is recommended to read the ELF
> specification[^1] to understand how the ELF program header structure is used.

#### ks_flags

32-bit unsigned integer that specifies the permissions of the segment. The permissions are defined as follows:

| Name        | Value        | Meaning     |
|-------------|--------------|-------------|
| PF_X        | `0x1`        | Execute     |
| PF_W        | `0x2`        | Write       |
| PF_R        | `0x4`        | Read        |
| PF_MASKOS   | `0x0FF00000` | Unspecified |
| PF_MASKPROC | `0xF0000000` | Unspecified |

More information about the flags can be found in the ELF specification[^1].

#### ks_address

64-bit unsigned integer that specifies the address of the segment in physical address.

#### ks_size

64-bit unsigned integer that specifies the size of the segment in memory. This value is the size of the segment in
memory in bytes.

#### ks_align

64-bit unsigned integer that specifies the alignment of the segment in memory. This value is the same as the `p_align`
field in the ELF program header[^1] for the segment.

## 11. Memory Layout

The bootloader must arrange the memory layout as shown down below.

The following diagram shows the memory layout:

PLACEHOLDER – put the diagram here

![Memory layout diagram](res/kuro_memlay.png)

The bootloader should configure the virtual address space to the maximum size supported by the hardware. When there is a
bigger available virtual address space, the bootloader should use the biggest one.

The bootloader must use the smallest page size supported by the hardware.

### 11.1 Lower Half

This region must not be mapped to any physical address when the control is transferred to the executable.

### 11.2 Higher Half

This region is mapped to the physical address with offset.

The offset is the base address of the higher half.

`Virtual Address = Physical Address + Higher Half Base`

## Appendix A: Bootloader Identifier String

This table lists the bootloader identifier strings that are currently known by the documents.

| Bootloader Identifier String | Description         |
|------------------------------|---------------------|
| `UNKNOWN`                    | Unknown Bootloader. |
| `KURO`                       | KURO Bootloader.    |

> [!NOTE]
> If you would like to add a new bootloader identifier string to this table, please [contact](#contact) the author of
> this document.

## Appendix B: Legacy Boot Protocols

Due to the lack of real-world usage and impracticality in the first two revisions of the KURO boot protocol, they are considered legacy and
should never be used. Those revisions are now considered legacy and the count is reset to `1`.

## Appendix C: Changes

- `Legacy 1.0` - Initial release.
- `Legacy 2.0`
    - Added support for passing arbitrary data to the executable. This allows the bootloader to pass data depending on
      the bootloader implementation.
    - Added explicit reservations for the other registers that are passed to the executable for future use.
    - Remove the requirement for signature verification however, it is encouraged to verify the signature anyway.
    - Clarified the requirements of having no relocations in the executable by explicitly stating the disallowed program
      segment types and exception to it.
    - Added caution about UEFI memory allocation.
    - Update `k_version` from `1` to `2`.
    - Changed stack location to be implementation-defined.
    - Added `ke_stack_end` field to the executable information structure.
    - Added Bootloader Identifier String to the arguments provided to the executable and the table containing the
      currently known bootloader identifier strings by the document.
    - Stack alignment is now `16` bytes.
    - Added an image handle to the arguments provided to the executable.
    - Added `ke_entry_point` field to the executable information structure.
    - Clarified the pointer and stack in the arguments provided to the executable.
    - Clarified versioning of this document.
    - Removed the farewell section from this document.
    - Added the contact section to this document.
    - Added the changes section to this document.
    - Renamed from KURO Booting Convention to KURO Boot Protocol.
- `1.0`
    - Fixed the stack alignment
    - Removed `ke_stack_end` field from the executable information structure.
    - Removed reductant sections from the document.
    - Bump the version to `3`.
    - Added support for ARM64.
    - Boot service is now being exited.
    - Added boot information structure.
    - Added framebuffer structure.
    - Added module structure.
    - Added memory map structure.
    - Defined memory layout.
    - Changed position of the KURO identifier.
    - Please refer to git history for details.

## Contact

In case of any questions or suggestions, please feel free to email
[mono@themonhub.net](mailto:mono@themonhub.net)

This document was written and maintained by [TheMonHub](https://github.com/TheMonHub).

You can get a copy of this document in here:
https://github.com/TeamSHIRO/KURO/blob/main/docs/kuro_boot_protocol.md.

## Copyright

Copyright 2026 TheMonHub

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## References

The following publications and sources of information may be useful for understanding this document or are referred to
in this document:

[^1]: **ELF Executable and Linkable Format, Xinuos** – https://gabi.xinuos.com/elf/

[^2]: **Unified Extensible Firmware Interface Specification, Version 2.11** – https://uefi.org/specs/UEFI/2.11/

[^3]: **Ed25519, Wikipedia** – https://en.wikipedia.org/wiki/EdDSA#Ed25519

[^4]: **System V ABI for the X86-64 Architecture, GitLab** – https://gitlab.com/x86-psABIs/x86-64-ABI

[^5]: **Procedure Call Standard for the Arm 64-bit Architecture, GitHub** – https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst

[^6]: **kuro-sign, GitHub** – https://github.com/TeamSHIRO/kuro-sign