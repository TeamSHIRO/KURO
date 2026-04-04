<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_banner_dark.png">
  <img src="res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO Booting Convention

***Release `1.0` Errata `A`***

**2026-04-03**

## Table of Contents

1. [Introduction](#1-introduction)
   1. [Target Audience](#11-target-audience)
   2. [Scope](#12-scope)
   3. [Conventions](#13-conventions)
      1. [Typography](#131-typography)
      2. [KURO Compliance](#132-kuro-compliance)
      3. [Data Structure](#133-data-structure)
      4. [Executable and Linkable Format](#134-executable-and-linkable-format)
      5. [Unified Extensible Firmware Interface](#135-unified-extensible-firmware-interface)
      6. [System V AMD64 ABI Calling Convention](#136-system-v-amd64-abi-calling-convention)
      7. [Digital Signature and Ed25519](#137-digital-signature-and-ed25519)
      8. [Bootloader](#138-bootloader)
          1. [KURO Bootloader](#1381-kuro-bootloader)
      9. [Executable](#139-executable)
      10. [Pseudocode](#1310-pseudocode)
      11. [Revisions](#1311-revisions)
2. [Overview](#2-overview)
   1. [Executable Structure](#21-executable-structure)
   2. [KURO Footer](#22-kuro-footer)
   3. [Booting Process](#23-booting-process)
   4. [Arguments Provided to the Loaded Executable](#24-arguments-provided-to-the-loaded-executable)
   5. [Security Considerations](#25-security-considerations)
   6. [Limitations](#26-limitations)
3. [Executable Structure](#3-executable-structure)
4. [KURO Footer](#4-kuro-footer)
5. [Booting Process](#5-booting-process)
6. [Arguments Provided to the Loaded Executable](#6-arguments-provided-to-the-loaded-executable)
7. [KURO Executable Information](#7-kuro-executable-information)
    1. [KuroSegmentInfo](#71-kurosegmentinfo)
8. [Farewell](#farewell)
9. [Copyright](#copyright)
10. [References](#references)

## 1. Introduction

This document describes the conventions used by the KURO bootloader to load executables.

This document is intended to provide a reference for developers who want to create executables that can be loaded by the
KURO bootloader or to create their own bootloader that follows the KURO booting convention.

## 1.1 Target Audience

Kernel developers, operating system developers, and anyone inbetween.

We will not be holding your hand on the technical details of how to create a bootloader or an executable. We will be
providing the necessary information and conventions to allow you to create your own bootloader or executable that
follows the KURO booting convention.

## 1.2 Scope

This document covers the following topics and concepts related to the KURO booting convention:

- Expected format of the executable file.
- How the bootloader loads the executable.
- Interfaces provided to the loaded executable.
- And more.

## 1.3 Conventions

These are the conventions used throughout this document. They are intended to provide clarity and consistency in the
presentation of information.

### 1.3.1 Typography

- **Bold**: Used to highlight important terms and concepts.
- _Italic_: Used to emphasize specific words or phrases.
- **Bold Italic**: Used to highlight highly important terms and concepts.
- ~~Strikethrough~~: Used to indicate deprecated or outdated information.
- <ins>Underline</ins>: Added to emphasize new or updated information.
- `Code`: Used to represent code snippets, commands, technical terms, versioning numbers, or values.
- [Hyperlink](): Used to link to external resources or specific part of the document.
- Reference[^1]: Used to indicate a reference to an external source of information. You can find all the references
  [here](#references).

> [!NOTE]
> This is a note. It is used to provide additional information or clarification about a specific topic or concept. It is
> not intended to be a warning or an important notice, but rather to provide helpful information to the reader.

> [!TIP]
> This is a tip. It is used to provide helpful advice or best practices related to a specific topic or concept. It is
> not intended to be a warning or an important notice, but rather to provide useful information to the reader.

> [!IMPORTANT]
> This is an important notice. It is used to highlight critical information that the reader should pay attention to. It
> is not intended to be a warning, but rather to emphasize the importance of the information being presented.

> [!WARNING]
> This is a warning. It is used to highlight information that could potentially cause minor harm or damage if not
> followed. It is intended to alert the reader to potential risks and to encourage them to take appropriate precautions.

> [!CAUTION]
> This is a caution. It is used to highlight information that could potentially cause harm or inconvenience if not
> followed. It is intended to alert the reader to potential risks and to encourage them to take appropriate precautions.

| Term       | Definition                                                                                        |
|------------|---------------------------------------------------------------------------------------------------|
| **MUST**   | Indicates a requirement that must be met by the bootloader or executable. MANDATORY               |
| **SHOULD** | Indicates a recommendation that should be followed by the bootloader or executable. RECOMMENDED   |
| **MAY**    | Indicates an optional feature or behavior. It is not required, but it is allowed. AVAILABLE TO DO |

### 1.3.2 KURO Compliance

Any bootloader that is compliant with this specification is considered KURO compliant. This means that it adheres to the
conventions and requirements outlined in this document, allowing it to load executables that follow the KURO booting
convention and requirements for creating bootloaders and executables.

The term "KURO compliant" is used to describe bootloaders that meet these standards, ensuring compatibility with
executables designed for the KURO booting convention. It is important to note that while the KURO bootloader itself
is designed to be compliant with this specification, other bootloaders may also be compliant as long as they adhere to
the outlined conventions and requirements. This allows for flexibility in the implementation of bootloaders while
maintaining a consistent and standardized approach to loading executables that follow the KURO booting convention.

Any executable that is compliant with this specification is considered KURO compliant. This means that it adheres to the
conventions and requirements outlined in this document, allowing it to be loaded by KURO compliant bootloaders.

### 1.3.3 Data Structure

The supported architecture is x86\_64, little-endian byte order, which means that the most significant byte is stored
at the lowest address as an example of this; `0x12345678` will be stored as `0x78563412`. Noting that `12 34 56 78` is
now`78 56 34 12` in little-endian byte order.

Hexadecimal values are written in uppercase, with a leading `0x`.

All data structures, even those that are saved in a file, are represented in little-endian byte order. Providing ease
for the bootloader or any other software to read and write data without having to worry about byte order or swapping
bytes to match the expected format of the hardware architecture.

### 1.3.4 Executable and Linkable Format

Executable and Linkable Format[^2] – standard file format for executables, object code, shared libraries, and core
dumps. Notably, ELF executables are used by Linux, Unix systems, and other operating systems.

The KURO bootloader is designed to load ELF executables that follow the KURO booting convention, which specifies how the
executable should be structured and how the bootloader should load it into memory.

This document will not cover the details of the ELF executable format, but it is expected that the reader has a basic
understanding of the ELF format and how it works. For more information on the ELF format, please refer to the ELF
Executable and Linkable Format specification.

### 1.3.5 Unified Extensible Firmware Interface

Unified Extensible Firmware Interface[^3] – defines software interface between operating system and platform firmware.
UEFI is used notably by bootloaders as a modern alternative to BIOS to interact with the firmware and perform tasks such
as loading executables, accessing hardware, and providing services. Many other parts of the operating system use UEFI to
interact with the firmware.

Notably, UEFI has a secure boot feature that allows the firmware to verify the authenticity
of the bootloader before loading it, which can help prevent unauthorized software from running on the system.

This document will not cover the details of the UEFI specification, but it is expected that the reader has a basic
understanding of the UEFI specification and how it works. For more information on the UEFI specification, please refer
to the Unified Extensible Firmware Interface Specification.

### 1.3.6 System V AMD64 ABI Calling Convention

System V AMD64 ABI calling convention[^6] is a standard calling convention for the x86\_64 architecture. It defines how
the arguments are passed to and from functions in the x86\_64 architecture. It is used by the KURO bootloader to pass
arguments to the loaded executable, such as the executable information and the UEFI system table.

This document will not cover the details of the System V AMD64 ABI calling convention, but it is expected that the
reader has a basic understanding of the calling convention and how it works. For more information on the System V AMD64
ABI calling convention, please refer to the System V Application Binary Interface AMD64 Architecture Processor
Supplement.

### 1.3.7 Digital Signature and Ed25519

Digital signatures[^4] are a cryptographic mechanism used to verify the authenticity and integrity of data. They are
commonly used in software distribution to ensure that the software has not been tampered with and is from a trusted
source. In this document, digital signatures are used to verify the authenticity of the executable being loaded by the
bootloader.

Ed25519[^5] is a digital signature algorithm that uses elliptic curve cryptography to sign messages and verify their
integrity. It is designed to be fast, secure, and resistant to certain types of attacks. In this document, Ed25519 is
used as the digital signature algorithm for signing the executable and verifying its authenticity.

### 1.3.8 Bootloader

A small program that prepares the environment for the operating system or other software to run and, then loads an
operating system or other executable into memory and starts its execution. In this document, the term "bootloader"
refers to a bootloader that is KURO compliant.

KURO compliant bootloaders are designed to load ELF executables that follow the KURO booting convention, which specifies
the digital signature of the executable to verify its authenticity.

#### 1.3.8.1 KURO Bootloader
KURO bootloader is a small EFI application designed to load and execute an executable that follows the KURO
booting convention. It is designed to be as minimal as possible and self-contained, with a focus on security and
simplicity. The KURO bootloader is designed to be compliant with the UEFI specification and to provide a secure and
reliable booting process for executables that follow the KURO booting convention. It is designed to be minimal enough to
boot an executable while leaving the developer with almost full control over the boot process.

> [!NOTE]
> KURO bootloader is the official reference implementation of the KURO booting convention.

> [!IMPORTANT]
> KURO can mean different things in different contexts. The bootloader and the booting convention.

### 1.3.9 Executable

A file that contains a program that can be executed by the operating system or other software. In this document, the
term "executable" refers to an executable file in the ELF format that follows the KURO booting convention and can be
loaded by a KURO compliant bootloader.

For more information on ELF executables, please refer to the ELF Executable and Linkable Format specification.

### 1.3.10 Pseudocode

The pseudocode provided in this document is intended to provide a clear and concise representation of the concepts and
conventions outlined in this document.

It is not intended to be a complete implementation of a bootloader or an executable, but rather to illustrate the
concepts and conventions in a way that is easy to understand. The pseudocode is written in a C++ or C-like syntax, but
it is not intended to be compiled or run as-is. It is meant to be a reference for developers who want to create their
own bootloader or executable that follows the KURO booting convention.

```c++
// Example pseudocode
int main() {
    // Initialize the bootloader
    init_bootloader();
    // Load the executable
    load_executable();
    // Transfer control to the entry point of the executable
    execute_entry_point();
}
```

#### 1.3.10.1 Types

| Type       | Description                   |
|------------|-------------------------------|
| `uint8_t`  | 8-bit unsigned integer        |
| `uint16_t` | 16-bit unsigned integer       |
| `uint32_t` | 32-bit unsigned integer       |
| `uint64_t` | 64-bit unsigned integer       |
| `int8_t`   | 8-bit signed integer          |
| `int16_t`  | 16-bit signed integer         |
| `int32_t`  | 32-bit signed integer         |
| `int64_t`  | 64-bit signed integer         |
| `char`     | 8-bit character               |
| `bool`     | Boolean value (true or false) |
| `void`     | No return value               |

### 1.3.11 Revisions

Updates to this document are considered either revisions or errata as described below:
- A new revision is made when significant changes are made to the document. Which changes the behavior of the
  bootloader, executable, or the booting process such as adding a new feature or fixing a bug.
- Errata version is made when a typo is found in the document or a minor change is made to the document that does not
  affect the behavior of the bootloader, executable, or the booting process such as fixing a typo or updating a link.

> [!IMPORTANT]
> Draft documents are not intended to be final and must not be considered a final version of the document. They are
> not versioned for each change until they are finalized and ready for release.

## 2. Overview

KURO is a minimal secure booting convention designed for x86\_64 UEFI systems.

The bootloader loads ELF executables that follow the
KURO booting convention, which specifies how the executable should be structured and how the bootloader should load it
into memory.

### 2.1 Executable Structure

The executable is expected to be in a standard ELF file with the following characteristics:
- Position-independent executable (PIE)
- x86\_64 architecture
- At the end of the file contains a KURO footer, which is used to identify the executable as a KURO executable.
  More information regarding the KURO footer can be found at [section 4](#4-kuro-footer).

### 2.2 KURO Footer

The KURO footer is a fixed-size data structure located at the end of the executable file. It contains metadata about the
executable for the bootloader to use during the loading process.

It is expected that the bootloader will validate the KURO footer before loading the executable. The KURO footer contains
the following fields:

- **Magic number**: A unique identifier to recognize the executable as a KURO executable.
- **Version**: The version of the KURO booting convention used by the executable.
- **Signature**: An Ed25519[^5] signature of the executable to verify its authenticity.

### 2.3 Booting Process

The bootloader should perform the following steps to load an executable:

1. Read the ELF header and check if the executable is a valid ELF file.
2. Read the KURO footer and check if the executable is a valid KURO executable.
3. Verify the signature in the KURO footer to ensure the authenticity of the executable.
4. Load the executable into memory according to the ELF program headers.
5. Transfer control to the entry point of the executable.

### 2.4 Arguments Provided to the Loaded Executable

KURO booting convention uses the System V AMD64 ABI calling convention, which means that the first six integer or
pointer arguments are passed in the following registers: `RDI`, `RSI`, `RDX`, `RCX`, `R8`, and `R9`. Additional
arguments are passed on the stack.

The bootloader must provide the following arguments to the loaded executable:
- **Executable information**: Provides information about the loaded executable, such as its entry point, segment
  information, and more.
- **System table**: The UEFI[^3] system table, which provides access to various UEFI services and information.

### 2.5 Security Considerations

This booting convention is designed with security in mind, and the following security features are considered for the
bootloader:

- **Secure boot**: an UEFI[^3] feature that verifies the authenticity of the bootloader before loading it.
- **Code integrity**: the bootloader verifies the integrity of the executable using the signature in the KURO footer.
  When used alongside the secure boot feature, this creates a chain of trust.

### 2.6 Limitations

This booting convention is designed to be as minimal as possible, which imposes the following limitations:

- **Kernel heavy initialization**: The bootloader is designed to be minimal and provides few services or features. This
  means that the kernel or other executable loaded by the bootloader will need to perform more initialization and setup
  than it would if it were loaded by a more feature-rich bootloader.
- **No support for dynamic loading**: This is a common limitation of bootloaders, as they are designed to load a single
  executable and transfer control to it. This means that the kernel or other executable loaded by the bootloader will
  need to implement its own dynamic loading mechanism if it wants to load additional executables or libraries at
  runtime.
- **No support for multiple architectures**: The bootloader is not designed to support multiple architectures. This
  means that the bootloader will only be able to load executables that are designed for the x86\_64 architecture.

## 3. Executable Structure

A valid KURO executable is expected to be a Position-independent executable (PIE) with no relocations with no dynamic
linking.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_exec_file_dark.png">
  <img src="res/kuro_exec_file.png" alt="Example of a valid KURO executable file structure">
</picture>

The executable must be a valid ELF file with the following characteristics:

- It must be a position-independent executable (PIE) with ELF header field `e_type` set to `ET_DYN`.
- It must be for the x86\_64 architecture with ELF header field `e_machine` set to `EM_X86_64`.
- It must have no relocations in the ELF file (.rel and .rela sections).
- It must have a valid KURO footer at the end of the file.

The bootloader must verify the ELF header and KURO footer to ensure that the executable is a valid KURO executable.

Example of a valid ELF header in the KURO executable:

```c++
Elf64_Ehdr elf_header = {
    .e_ident = {
        0x7F, 'E', 'L', 'F',      // EI_MAG0 -> EI_MAG3
        2,                        // EI_CLASS
        1,                        // EI_DATA
        1,                        // EI_VERSION
        0,                        // EI_OSABI
        0,                        // EI_ABIVERSION
        0, 0, 0, 0, 0, 0, 0, 0,   // EI_PAD
        16,                       // EI_NIDENT
    },

    .e_type      = ET_DYN,        // Position-Independent Executable (PIE)
    .e_machine   = EM_X86_64,     // x86-64
    .e_version   = EV_CURRENT,    // 1

    .e_entry     = 0x1000,        // entry point
    .e_phoff     = 64,            // program headers offset
    .e_shoff     = 12944,         // section headers offset

    .e_flags     = 0,             // flags

    .e_ehsize    = 64,            // ELF header size
    .e_phentsize = 56,            // size of one program header
    .e_phnum     = 13,            // number of program headers

    .e_shentsize = 64,            // size of one section header
    .e_shnum     = 18,            // number of section headers
    .e_shstrndx  = 17             // section name string table index
};
```

```terminaloutput
[user@host ~]$ readelf -h kuro_executable
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x1000
  Start of program headers:          64 (bytes into file)
  Start of section headers:          12944 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         18
  Section header string table index: 17
```

> [!NOTE]
> These GCC/Clang compiler and linker flags may help you compile a valid KURO executable:
> 
> ```shell
> cc -fPIE -ffreestanding -o main.c.o -c main.c
> ld -nostdlib -pie -o main main.c.o
> ```
> 
> - `-fPIE` flag is used by compiler to create a position-independent executable (PIE).
> - `-ffreestanding` flag is used to create an executable that is not on a hosted environment.
> - `-pie` flag is used by linker to create a position-independent executable (PIE).
> - `-nostdlib` flag is used to prevent the linker from linking against the standard library, which includes the C 
>   standard library and startup code which introduces a dependency on the standard library and relocations.

> [!IMPORTANT]
> This will not include the KURO footer. You will need to add the KURO footer to the end of the file after compiling it.
> See [section 4](#4-kuro-footer) for more information.

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

The second byte of the KURO identifier is used to identify the version of the KURO booting convention used by the
executable. The version is a single byte that indicates the version of the KURO booting convention used by the
executable.

The bootloader must verify that the version in the KURO identifier matches the version of the KURO booting convention
used by the bootloader before loading the executable. If the version does not match, the bootloader must reject the
executable and not load it.

Any other undefined version number is considered reserved for future use.

| Version | Description                             |
|---------|-----------------------------------------|
| `0`     | Invalid version (never used)            |
| `1`     | KURO Booting Convention `1.0` (current) |

> [!CAUTION]
> Failure to reject the executable and load it might result in undefined behavior, as the structure of the KURO footer
> might be different from the expected structure.

## 5. Booting Process

The booting process for the bootloader may look like the following diagram:

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_booting_process_dark.png">
  <img src="res/kuro_booting_process.png" alt="Booting process diagram">
</picture>

At the beginning of the booting process, the bootloader should perform the initializations and setup necessary for the
booting process.

The bootloader then must read the ELF header and check if the executable is a valid ELF file that follows the
[section 3](#3-executable-structure) requirements. If the ELF header is not valid, the bootloader must reject the
executable and not load it.

After the ELF header is validated, the bootloader must read the KURO footer and check if the executable is a valid KURO
executable. If the KURO footer is not valid, the bootloader must reject the executable and not load it.

Then the bootloader must verify the signature in the KURO footer to ensure the authenticity of the executable. If the
signature is not valid, the bootloader must reject the executable and not load it.

Then the bootloader then may load the executable into memory according to the ELF program
headers, which specify the memory layout of the executable. The position of the executable in memory is
implementation-defined.

The bootloader must prepare the stack for the executable and set the stack pointer to the top of the stack. The stack
size is implementation-defined, but the start of the stack must be aligned to `8` bytes and must be located above the
executable memory.

The bootloader should then prepare the arguments to be passed to the executable.

After all the necessary setup is done, the bootloader then transfers control to the entry point of the executable, which
is specified in the ELF header.

Example of a KURO executable memory layout:

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_exec_mem_dark.png">
  <img src="res/kuro_exec_mem.png" alt="Example of a KURO executable memory layout">
</picture>


> [!NOTE]
> The order of operations is not strictly defined, but the bootloader must ensure that all the necessary steps are
> performed before transferring control to the entry point of the executable.

> [!IMPORTANT]
> - The bootloader must not perform any modification to the page tables.
> - The bootloader must not perform any modification to the CPU state, such as enabling or disabling interrupts,
>   changing the CPU mode, or changing the memory management unit (MMU) state. The bootloader must leave the CPU state
>   as it is.
> - The bootloader must disable the UEFI watchdog timers before loading the executable.
> - The bootloader must not exit the UEFI boot services. The executable is expected to exit the UEFI boot services
>   itself.
> - The bootloader must make sure that the public key used to verify the signature in the KURO footer is not tampered
>   with.

> [!WARNING]
> The executable should be aware not to trigger stack overflows as the stack is located above the executable memory
> which means that when stack overflow happens, it will overwrite the executable memory which might cause undefined
> behavior.

> [!CAUTION]
> Because of the UEFI boot services, the executable and the bootloader must allocate the memory through the UEFI
> interfaces. Otherwise, UEFI may use your memory and cause undefined behavior. As UEFI does not have any idea that the
> executable is using the memory, unless you are allocating the memory through the UEFI interfaces until you exit the
> UEFI boot services.

## 6. Arguments Provided to the Loaded Executable

As before the bootloader transfers control to the entry point of the executable, it must provide the following arguments
in their respective register as defined by the System V AMD64 ABI calling convention:

- `RDI`: The address of the executable information structure as defined in
  [section 7](#7-kuro-executable-information).
- `RSI`: The address of the UEFI system table as defined in UEFI specification[^3].

Pseudocode of the executable entry point:

```c++
_Noreturn void entry_point(KuroExecutableInfo* executable_info, EFI_SYSTEM_TABLE* system_table) {
    // The executable can now use the executable information and the UEFI system table to perform its tasks.
    // ...
}
```

> [!WARNING]
> The executable entry point should never return.

> [!NOTE]
> This can be interpreted as argument 1 and argument 2 in the System V AMD64 ABI calling convention, in their respective
> register.

## 7. KURO Executable Information

The executable information structure is a data structure that contains information about the loaded executable. It is
passed to the loaded executable in the `RDI` register as defined by the System V AMD64 ABI calling convention. The
structure contains the following fields:

```c++
typedef struct {
    KuroIdentifier ke_identifier;
    uint64_t ke_segment_count;
    KuroSegmentInfo* ke_segments;
    uint64_t ke_stack_start;
    uint64_t ke_stack_size;
} KuroExecutableInfo;
```

#### ke_identifier

As described in [section 4.1](#41-kuro-identifier), the `ke_identifier` field is a KURO identifier that contains the magic
number and version information used by the executable to identify that the argument passed to the executable is valid.

#### ke_segment_count

This field is a 64-bit unsigned integer that specifies the number of entries in the `ke_segments` array.

#### ke_segments

64-bit unsigned integer that points to an array of `KuroSegmentInfo` structures. Each `KuroSegmentInfo` structure
describes a segment of the executable that has been loaded into memory as defined in [section 7.1](#71-kurosegmentinfo).

#### ke_stack_start

64-bit unsigned integer that specifies the top/start address of the stack. The stack is located at the top of the
executable memory, and it grows downwards. The bootloader must set the stack pointer to the top of the stack before
transferring control to the entry point of the executable.

#### ke_stack_size

64-bit unsigned integer that specifies the size of the stack in bytes. The stack size is implementation-defined.

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

## Farewell

This document has covered the KURO booting convention, the necessary steps to make a bootloader or executable that uses
KURO booting convention. We hope that this document has helped you understand the KURO booting
convention and how it works.

Thank you for reading! If you have any questions or feedback, please feel free to reach out to us at
[mono@themonhub.net](mailto:mono@themonhub.net).

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
