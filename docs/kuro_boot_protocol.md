<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_banner_dark.png">
  <img src="res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO Boot Protocol

***Revision `1` Errata `A`***

**2026-04-13**

## Table of Contents

1. [Introduction](#1-introduction)
    1. [Target Audience](#11-target-audience)
    2. [Conventions](#12-conventions)
        1. [Typography](#121-typography)
        2. [Revisions](#122-revisions)
2. [Executable Structure](#2-executable-structure)
3. [KURO Footer](#3-kuro-footer)
4. [KURO Identifier](#4-kuro-identifier)
5. [Calling Convention and Registers](#5-calling-convention-and-registers)
    1. [X86-64 Registers](#51-x86-64-registers)
    2. [ARM64 Registers](#52-arm64-registers)
6. [KURO Boot Information](#6-kuro-boot-information)
7. [KURO Framebuffer](#7-kuro-framebuffer)
    1. [KURO Pixel Format](#71-kuro-pixel-format)
    2. [KURO Pixel Information](#72-kuro-pixel-information)
8. [KURO Executable Information](#8-kuro-executable-information)
    1. [KuroSegmentInfo](#81-kurosegmentinfo)
9. [Memory Layout](#9-memory-layout)
     1. [Lower Half](#91-lower-half)
     2. [Higher Half](#92-higher-half)
         1. [x86-64](#921-x86-64)
         2. [ARM64](#922-arm64)
9. [Appendix A: Bootloader Identifier String](#appendix-a-bootloader-identifier-string)
10. [Appendix B: Legacy Boot Protocols](#appendix-b-legacy-boot-protocols)
11. [Appendix C: Changes](#appendix-c-changes)
12. [Contact](#contact)
13. [Copyright](#copyright)
14. [References](#references)

## 1. Introduction

This document describes the protocols used by the KURO bootloader to load executables.

This document is intended to provide a reference for developers who want to create executables that can be loaded by the
KURO bootloader or to create their own bootloader that follows the KURO boot protocol.

## 1.1 Target Audience

Kernel developers, operating system developers, and anyone inbetween.

## 1.2 Conventions

These are the conventions used throughout this document.

### 1.2.1 Typography

Reference[^1]: Used to indicate a reference to an external source of information. You can find all the references
[here](#references).

      The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL
      NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and
      "OPTIONAL" in this document are to be interpreted as described in
      RFC 2119.

### 1.2.2 Revisions

Updates to this document are considered either revisions or errata as described below:
- A new revision is made when significant changes are made to the document. Which changes the behavior of the
  bootloader, executable, or the booting process such as adding a new feature or fixing a bug.
- Errata version is made when a typo is found in the document or a minor change is made to the document that does not
  affect the behavior of the bootloader, executable, or the booting process such as fixing a typo or updating a link.

> [!CAUTION]
> Not all commits of the document are considered final. Please refer to the git tag for each final version of the
> document.

## 2. Executable Structure

A valid KURO executable must be a Position-independent executable (PIE).

The following diagram shows a simple structure of a KURO executable:

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_exec_file_dark.png">
  <img src="res/kuro_exec_file.png" alt="Example of a valid KURO executable file structure">
</picture>

The executable must be a position-independent executable (PIE) with ELF header field `e_type` set to `ET_DYN`.

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
> the `kuro-sign` tool.

#### k_signature

The signature is an Ed25519[^4] signature of the executable to verify its authenticity. The signature is calculated over
the entire executable file, excluding the KURO footer itself.

#### k_identifier

As described in [section 4.1](#4-kuro-identifier).

## 4. KURO Identifier

KURO identifier is a fixed-size structure that contains the magic number and version information used to identify the
executable as a KURO executable and to verify that the argument passed to the executable is valid. The KURO identifier
is located at the beginning of the KURO footer and KURO executable information and contains the following fields:

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

The first five bytes of the KURO identifier are used to identify the executable as a KURO executable.
The magic number is `0x7F` followed by `KURO` in ASCII, which is `0x4B 0x55 0x52 0x4F` in hexadecimal.

The bootloader must verify that the magic number in the KURO identifier matches the expected value before loading the
executable. If the magic number does not match, the bootloader must reject the executable and not load it.

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

If the version does not match the expected version, the bootloader must reject the executable and not load it.

Any other undefined version number is considered reserved for future use.

| Version | Description                        |
|---------|------------------------------------|
| `0`     | Invalid Version                    |
| `1`     | KURO Boot Protocol `Legacy 1.0`    |
| `2`     | KURO Boot Protocol `Legacy 2.0`    |
| `3`     | KURO Boot Protocol `1.0` (current) |

Information regarding the legacy boot protocols can be found in [appendix B](#appendix-b-legacy-boot-protocols).

## 5. Calling Convention and Registers

As before the bootloader transfers control to the entry point of the executable, the bootloader must prepare the
boot information structure and pass it to the executable following the ABI/Calling
Convention in the register for each architecture.

### 5.1 X86-64 Registers

Following the System V AMD64 ABI[^5].

`rsi` - Pointer to the KURO boot information.

### 5.2 ARM64 Registers

Following the Procedure Call standard for the Arm 64-bit Architecture[^6].

`x0` - Pointer to the KURO boot information.

## 6. KURO Boot Information

The boot information structure is a data structure that contains information that the executable must use. The
structure contains the following fields:

```c++
typedef struct {
    KuroIdentifier kb_identifier;
    char *kb_boot_id;
    char *kb_initimage;
    uint64_t kb_initimage_size;
    KuroFramebuffer *kb_framebuffer;
    KuroExecutableInfo *kb_executable_info;
} KuroBootInfo;
```

#### kb_identifier

As described in [section 4](#4-kuro-identifier).

#### kb_boot_id

Points to a null-terminated string that contains the boot identifier of the bootloader.
See [appendix A](#appendix-a-bootloader-identifier-string) for the list of boot identifiers.

#### kb_initimage

Points to a file in the memory that contains the information such as:

- Drivers
- Init program
- Filesystem table

More information about the initimage can be found in the related specification[^7].

#### kb_initimage_size

Specifies the size of the initimage in bytes inside the memory.

#### kb_framebuffer

Pointer to the framebuffer structure.
As described in [section 7](#7-kuro-framebuffer).

This field can be null if no framebuffer is available.

#### kb_executable_info

Pointer to the executable information structure.
As described in [section 8](#8-kuro-executable-information).

## 7. KURO Framebuffer

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

Specifies the base address of the framebuffer.

#### kf_size

Specifies the size of the framebuffer in bytes.

#### kf_width

Specifies the horizontal resolution of the framebuffer.

#### kf_height

Specifies the vertical resolution of the framebuffer.

#### kf_pixels_per_scanline

Specifies the number of pixels per scanline.

#### kf_pixel_format

As described in [section 7.1](#71-kuro-pixel-format).

#### kf_pixel_info

As described in [section 7.2](#72-kuro-pixel-information).

### 7.1 KURO Pixel Format

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
> Please refer to the UEFI specification[^3] for more information about `EFI_GRAPHICS_PIXEL_FORMAT`

### 7.2 KURO Pixel Information

```c++
typedef struct {
    uint32_t kp_red_mask;
    uint32_t kp_green_mask;
    uint32_t kp_blue_mask;
    uint32_t kp_reserved_mask;
} KuroPixelInfo;
```

> [!NOTE]
> Please refer to the UEFI specification[^3] for more information about `EFI_PIXEL_BITMASK`

## 8. KURO Executable Information

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

This field contains the address to the entry point of the executable.

#### ke_segment_count

This field specifies the number of entries in the `ke_segments` array.

#### ke_segments

Points to an array of `KuroSegmentInfo` structures as defined in [section 8.1](#81-kurosegmentinfo).

#### ke_stack_start

Specifies the top of the stack. The bootloader must set the stack pointer
to the top of the stack before transferring control to the entry point of the executable.

#### ke_stack_size

Specifies the size of the stack in bytes. The stack size is implementation-defined.
`ke_stack_end - ke_stack_start` must be equal to `ke_stack_size`.

### 8.1 KuroSegmentInfo

`KuroSegmentInfo` is a structure that describes a segment of the executable that has been loaded into memory. It
contains information about each segment, such as its address, size, permission, and alignment.

Each `KuroSegmentInfo` structure is defined as follows:

```c++
typedef struct {
    uint32_t ks_flags;
    char pad[4]; // padding to align the structure to 8 bytes
    uint64_t ks_paddress;
    uint64_t ks_vaddress;
    uint64_t ks_size;
    uint64_t ks_align;
} KuroSegmentInfo;
```

> [!TIP]
> This structure depends heavily on the ELF program header structure, so it is recommended to read the ELF
> specification[^2] to understand how the ELF program header structure is used.

#### ks_flags

32-bit unsigned integer that specifies the permissions of the segment. The permissions are defined as follows:

| Name        | Value        | Meaning     |
|-------------|--------------|-------------|
| PF_X        | `0x1`        | Execute     |
| PF_W        | `0x2`        | Write       |
| PF_R        | `0x4`        | Read        |
| PF_MASKOS   | `0x0FF00000` | Unspecified |
| PF_MASKPROC | `0xF0000000` | Unspecified |

More information about the flags can be found in the ELF specification[^2].

#### ks_paddress

64-bit unsigned integer that specifies the address of the segment in physical memory.

#### ks_vaddress

64-bit unsigned integer that specifies the address of the segment in virtual memory.

#### ks_size

64-bit unsigned integer that specifies the size of the segment in memory. This value is the size of the segment in
memory in bytes.

#### ks_align

64-bit unsigned integer that specifies the alignment of the segment in memory. This value is the same as the `p_align`
field in the ELF program header[^2] for the segment.

## 9. Memory Layout

The bootloader must arrange the memory layout as shown down below.

The following diagram shows the memory layout:

PLACEHOLDER

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/PLACEHOLDER_DARK.png">
  <img src="res/PLACEHOLDER.png" alt="Memory layout">
</picture>

As shown in the diagram, the memory layout is split into two regions:
- Lower half
- Higher half

### 9.1 Lower Half

This region is identity mapped to the same address in the physical memory.

`virtual_address = physical_address`

### 9.2 Higher Half

This region is identity mapped with offset in the physical memory.

`virtual_address = physical_address + offset`

The offset is specific to each architecture as described in the following sections.

#### 9.2.1 x86-64

The offset is `0xFFFF800000000000`.

#### 9.2.2 ARM64

The offset is `0xFFFF000000000000`.

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

Due to the lack of real-world usage and impracticality in the first two versions of the KURO boot protocol, they are considered legacy and
should never be used. Those versions are now considered legacy and the versioning is reset to `1`.

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
    - Big changes to the boot protocol, please refer to git history for details.

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

[^1]: **This is an example of a reference.**

[^2]: **ELF Executable and Linkable Format, Xinuos** – https://gabi.xinuos.com/elf/

[^3]: **Unified Extensible Firmware Interface Specification, Version 2.11** – https://uefi.org/specs/UEFI/2.11/

[^4]: **Ed25519, Wikipedia** – https://en.wikipedia.org/wiki/EdDSA#Ed25519

[^5]: **System V ABI for the X86-64 Architecture, GitLab** – https://gitlab.com/x86-psABIs/x86-64-ABI

[^6]: **Procedure Call Standard for the Arm 64-bit Architecture, GitHub** – https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst

[^7]: **The KURO Initimage, GitHub** – https://github.com/TeamSHIRO/KURO/blob/main/docs/kuro_initimage.md