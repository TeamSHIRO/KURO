<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_banner_dark.png">
  <img src="res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO Boot Protocol

***Release `2.0` Errata `-`***

**2026-04-05**

## Table of Contents

TO BE ADDED

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

> [!IMPORTANT]
> The versioning follows the [Semantic Versioning](https://semver.org/). The versioning scheme is `MAJOR.MINOR.PATCH`.
> However, this document will omit the `PATCH` version number.
>
> The `MAJOR` version is incremented when there are changes to the bootloader that are not backwards compatible.
> The `MINOR` version is incremented when there are changes to the bootloader that are backwards compatible but may
> introduce new features or changes in the behavior.
>
> Not all commits of the document are considered final. Please refer to the git tag for each final version of the
> document.

## 2. Executable Structure

A valid KURO executable is expected to be a Position-independent executable (PIE) with no relocations with no dynamic
linking.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_exec_file_dark.png">
  <img src="res/kuro_exec_file.png" alt="Example of a valid KURO executable file structure">
</picture>

The executable must be a valid ELF file with the following characteristics:

- It must be a position-independent executable (PIE) with ELF header field `e_type` set to `ET_DYN`.
- It must be for the x86\_64 architecture with ELF header field `e_machine` set to `EM_X86_64`.
- It must have no relocations in the ELF file (`SHT_REL`, `SHT_RELA`, or `SHT_RELR` sections) unless if
  that section is related to dynamic linker, in that case the bootloader should ignore it.
- It must have a valid KURO footer at the end of the file.

The bootloader must verify the ELF header and KURO footer to ensure that the executable is a valid KURO executable.

Example of a valid ELF header in the KURO executable:

## 4. KURO Footer

The KURO footer is a fixed-size data structure located at the end of the executable file. It contains metadata about the
executable for the bootloader to use during the loading process.

Starting from 72 bytes before the end of the executable, lies the KURO footer. The KURO footer contains the following
fields:

```c++
typedef struct {
    KuroIdentifier k_identifier;
    char k_signature[64];
} KuroFooter;
```

> [!TIP]
> You can create a KURO footer on an existing ELF executable that follows [section 3](#3-executable-structure) by using
> the `kuro-sign` tool.

#### k_identifier

Eight initial bytes of the KURO footer are used to identify the executable as a KURO executable. The identifier contains
a magic number and version information that are used to verify the authenticity of the executable. Complete descriptions
can be found in [section 4.1](#41-kuro-identifier).

#### k_signature

The signature is an Ed25519[^5] signature of the executable to verify its authenticity. The signature is calculated over
the entire executable file, excluding the KURO footer itself.

### 4.1 KURO Identifier

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
    uint16_t k_reserved;
} KuroIdentifier;
```

#### k_magic

The first five bytes of the KURO identifier are used to identify the executable as a KURO executable. The magic number
is a unique sequence of bytes that is used to identify the executable as a KURO executable. The magic number is `0x7F`
followed by `KURO` in ASCII, which is `0x4B 0x55 0x52 0x4F` in hexadecimal.

The bootloader must verify that the magic number in the KURO identifier matches the expected value before loading the
executable. If the magic number does not match, the bootloader must reject the executable and not load it.

| Byte     | Value  |
|----------|--------|
| k_magic0 | `0x7F` |
| k_magic1 | `0x4B` |
| k_magic2 | `0x55` |
| k_magic3 | `0x52` |
| k_magic4 | `0x4F` |

> [!CAUTION]
> Failure to reject the executable and load it might result in undefined behavior, as this indicates that the executable
> has no KURO footer.

#### k_version

The second byte of the KURO identifier is used to identify the version of the KURO boot protocol used by the
executable. The version is a single byte that indicates the version of the KURO boot protocol used by the
executable.

The bootloader must verify that the version in the KURO identifier matches the version of the KURO boot protocol
used by the bootloader before loading the executable. If the version does not match, the bootloader must reject the
executable and not load it.

Any other undefined version number is considered reserved for future use.

| Version | Description                        |
|---------|------------------------------------|
| `0`     | Invalid version (never used)       |
| `1`     | KURO Boot Protocol `1.0`           |
| `2`     | KURO Boot Protocol `2.0` (current) |

> [!CAUTION]
> Failure to reject the executable and load it might result in undefined behavior, as the structure of the KURO footer
> might be different from the expected structure.

## 6. Arguments Provided to the Loaded Executable

As before the bootloader transfers control to the entry point of the executable, it must provide the following arguments
in their respective register as defined by the System V AMD64 ABI calling convention:

- `RDI`: The address of the executable information structure as defined in
  [section 7](#7-kuro-executable-information).
- `RSI`: Contains the bootloader's image handle.
- `RDX`: Points to the UEFI system table as defined in UEFI specification[^3].
- `RCX`: Arbitrary data passed to the executable. This data is implementation-defined. If not being used, it can be set
  to null or `0`.
- `R8`: Pointer to bootloader identifier string. This data is implementation-defined but must be byte-sized
  null-terminated string. See the bootloader identifier string in [section 6.1](#61-bootloader-identifier-string).
- `R9` must be reserved for future use. Must be null or `0`.
- Stack pointer must be aligned to `16` bytes and the stack itself must be null-initialized.

Example of the arguments provided to the entry point of the executable:

```c++
_Noreturn void entry_point(KuroExecutableInfo* exec_info, EFI_HANDLE image_handle,
                           EFI_SYSTEM_TABLE* system_table, void* data, char* boot_id) {
    // Executable logic here
}
```

> [!CAUTION]
> These arguments must not be null or `0` unless otherwise specified.

> [!NOTE]
> This can be interpreted as argument 1, 2, and 3 in the System V AMD64 ABI calling convention, in their respective
> register.

## 7. KURO Executable Information

The executable information structure is a data structure that contains information about the loaded executable. It is
passed to the loaded executable in the `RDI` register as defined by the System V AMD64 ABI calling convention. The
structure contains the following fields:

```c++
typedef struct {
    KuroIdentifier ke_identifier;
    uint64_t ke_entry_point;
    uint64_t ke_segment_count;
    KuroSegmentInfo* ke_segments;
    uint64_t ke_stack_start;
    uint64_t ke_stack_end;
    uint64_t ke_stack_size;
} KuroExecutableInfo;
```

#### ke_identifier

As described in [section 4.1](#41-kuro-identifier), the `ke_identifier` field is a KURO identifier that contains the
magic number and version information used by the executable to identify that the argument passed to the executable is
valid.

#### ke_entry_point

This field contains the address to the entry point of the executable. The entry point is the first instruction that the
executable will execute.

#### ke_segment_count

This field is a 64-bit unsigned integer that specifies the number of entries in the `ke_segments` array.

#### ke_segments

64-bit unsigned integer that points to an array of `KuroSegmentInfo` structures. Each `KuroSegmentInfo` structure
describes a segment of the executable that has been loaded into memory as defined in [section 7.1](#71-kurosegmentinfo).

#### ke_stack_start

64-bit unsigned integer that specifies the starting/top address of the stack. The bootloader must set the stack pointer
to the top of the stack before transferring control to the entry point of the executable.

#### ke_stack_end

64-bit unsigned integer that specifies the ending/bottom address of the stack.

#### ke_stack_size

64-bit unsigned integer that specifies the size of the stack in bytes. The stack size is implementation-defined.
`ke_stack_end - ke_stack_start` must be equal to `ke_stack_size`.

### 7.1 KuroSegmentInfo

`KuroSegmentInfo` is a structure that describes a segment of the executable that has been loaded into memory. It
contains information about each segment, such as its address, size, permission, and alignment.

Each `KuroSegmentInfo` structure is defined as follows:

```c++
typedef struct {
    uint32_t ks_flags;
    char pad[4]; // padding to align the structure to 8 bytes
    uint64_t ks_address;
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

#### ks_address

64-bit unsigned integer that specifies the address of the segment in memory. This value is the value of where this
segment is loaded into memory.

#### ks_size

64-bit unsigned integer that specifies the size of the segment in memory. This value is the size of the segment in
memory in bytes.

#### ks_align

64-bit unsigned integer that specifies the alignment of the segment in memory. This value is the same as the `p_align`
field in the ELF program header[^2] for the segment.

#### Bootloader Implementation

The bootloader must prepare `KuroSegmentInfo` structures for each segment of the executable that has been loaded into
memory as follows:

1. Set the `ks_flags` field to the same value as the `p_flags` field in the ELF program header for the segment.
2. Set the `ks_address` field to the virtual address where the segment has been loaded into memory.
3. Set the `ks_size` field to the size of the segment in memory in bytes.
4. Set the `ks_align` field to the same value as the `p_align` field in the ELF program header for the segment.

## Appendix A: Bootloader Identifier String

This table lists the bootloader identifier strings that are currently known by the documents.

| Bootloader Identifier String | Description         |
|------------------------------|---------------------|
| `UNKNOWN`                    | Unknown Bootloader. |
| `KURO`                       | KURO Bootloader.    |

> [!NOTE]
> If you would like to add a new bootloader identifier string to this table, please [contact](#contact) the author of
> this document.

## Changes

- `1.0` - Initial release.
- `2.0`
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
    - The versioning should now be correct from now on.

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

[^4]: **Digital Signatures, Wikipedia** – https://en.wikipedia.org/wiki/Digital_signature

[^5]: **Ed25519, Wikipedia** – https://en.wikipedia.org/wiki/EdDSA#Ed25519

[^6]: **System V AMD64 ABI, GitLab** – https://gitlab.com/x86-psABIs/x86-64-ABI
