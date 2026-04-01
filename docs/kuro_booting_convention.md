<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_banner_dark.png">
  <img src="res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO Booting Convention

***Draft `1.0` | There be dragons!***

**Last Updated: 2026-04-02**

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
      6. [Bootloader](#136-bootloader)
          1. [KURO Bootloader](#1361-kuro-bootloader)
      7. [Executable](#137-executable)
      8. [Pseudocode](#138-pseudocode)
      9. [Revisions](#139-revisions)
2. [Overview](#2-overview)
   1. [Executable Structure](#21-executable-structure)
   2. [KURO Footer](#22-kuro-footer)
   3. [Booting Process](#23-booting-process)
   4. [Interfaces Provided to the Loaded Executable](#24-interfaces-provided-to-the-loaded-executable)
   5. [Security Considerations](#25-security-considerations)
   6. [Limitations](#26-limitations)
3. [Copyright](#copyright)
4. [References](#references)

## 1. Introduction

This document describes the conventions used by the KURO bootloader to load executables.

This document is intended to provide a clear and concise specification for developers who want to create bootloaders
that follow these conventions. It is also intended to provide a reference for developers who want to create executables
that can be loaded by the KURO bootloader.

## 1.1 Target Audience

Kernel developers, operating system developers, and anyone interested in understanding how the KURO bootloader loads
executables.

This document is not intended to be a step-by-step guide for creating bootloaders or executables. It is intended to be a
reference for developers who want to create bootloaders or executables that follow the KURO booting convention. It is
also intended to be a reference for developers who want to understand how the KURO bootloader loads executables.

We will not be holding your hand on the technical details of how to create a bootloader or an executable. We will be
providing the necessary information and conventions to allow you to create your own bootloader or executable that
follows the KURO booting convention. We will be providing examples and pseudocode to illustrate the concepts and
conventions, but we will not be providing a complete implementation of a bootloader or an executable. We will be
providing the necessary information and conventions to allow you to create your own bootloader or executable that
follows the KURO booting convention.

## 1.2 Scope

This document covers the following topics and concepts related to the KURO booting convention:

- Expected format of the executable file
- Booting process
- Interfaces provided to the loaded executable
- Security features provided by the bootloader

## 1.3 Conventions

These are the conventions used throughout this document. They are intended to provide clarity and consistency in the
presentation of information.

### 1.3.1 Typography

- **Bold**: Used to highlight important terms and concepts.
- _Italic_: Used to emphasize specific words or phrases.
- **Bold Italic**: Used to highlight highly important terms and concepts.
- ~~Strikethrough~~: Used to indicate deprecated or outdated information.
- <ins>Underline</ins>: Added to emphasize new or updated information.
- `Code`: Used to represent code snippets, commands, or technical terms.
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
> This is a warning. It is used to highlight information that could potentially cause minor harm or damage if not followed. It
> is intended to alert the reader to potential risks and to encourage them to take appropriate precautions.

> [!CAUTION]
> This is a caution. It is used to highlight information that could potentially cause harm or inconvenience if not
> followed. It is intended to alert the reader to potential risks and to encourage them to take appropriate precautions.

### 1.3.2 KURO Compliance

Any bootloader that is compliant with this specification is considered KURO compliant. This means that it adheres to the
conventions and requirements outlined in this document, allowing it to load executables that follow the KURO booting
convention.onventions and requirements for creating bootloaders and e

The term "KURO compliant" is used to describe bootloaders that meet these standards, ensuring compatibility with
executables designed for the KURO booting convention. It is important to note that while the KURO bootloader itself
is designed to be compliant with this specification, other bootloaders may also be compliant as long as they adhere to
the outlined conventions and requirements. This allows for flexibility in the implementation of bootloaders while
maintaining a consistent and standardized approach to loading executables that follow the KURO booting convention.

Any executable that is compliant with this specification is considered KURO compliant. This means that it adheres to the
conventions and requirements outlined in this document, allowing it to be loaded by KURO compliant bootloaders.

### 1.3.3 Data Structure

The supported architecture is x86\_64, little-endian byte order, which means that the most significant byte is stored
at the lowest address as an example of this; `0x12345678` will be stored as `0x78563412`. Noting that `12 34 56 78` is now
`78 56 34 12` in little-endian byte order.

Hexadecimal values are written in uppercase, with a leading `0x`.

All data structures, even those that are saved in a file, are represented in little-endian byte order. Providing ease
for the bootloader or any other software to read and write data without having to worry about byte order or swapping
bytes to match the expected format of the hardware architecture.

### 1.3.4 Executable and Linkable Format

Executable and Linkable Format[^2] – standard file format for executables, object code, shared libraries, and core dumps.
Notably, ELF executables are used by Linux, Unix systems, and other operating systems.

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

### 1.3.6 Bootloader

A small program that prepares the environment for the operating system or other software to run and, then loads an operating
system or other executable into memory and starts its execution. In this document, the term "bootloader" refers to a
bootloader that is KURO compliant.

KURO compliant bootloaders are designed to load ELF executables that follow the KURO booting convention, which specifies
the digital signature of the executable to verify its authenticity.

#### 1.3.6.1 KURO Bootloader
KURO bootloader is a small EFI application designed to load and execute an executable that follows the KURO
booting convention. It is designed to be as minimal as possible and self-contained, with a focus on security and
simplicity. The KURO bootloader is designed to be compliant with the UEFI specification and to provide a secure and
reliable booting process for executables that follow the KURO booting convention. It is designed to be minimal enough to
boot an executable while leaving the developer with almost full control over the boot process.

> [!NOTE]
> KURO bootloader is the official reference implementation of the KURO booting convention.

### 1.3.7 Executable

A file that contains a program that can be executed by the operating system or other software. In this document, the
term "executable" refers to an executable file in the ELF format that follows the KURO booting convention and can be loaded by a KURO
compliant bootloader.

For more information on ELF executables, please refer to the ELF Executable and Linkable Format specification.

### 1.3.8 Pseudocode

The pseudocode provided in this document is intended to provide a clear and concise representation of the concepts and
conventions outlined in this document.

It is not intended to be a complete implementation of a bootloader or an executable, but rather to illustrate the
concepts and conventions in a way that is easy to understand. The pseudocode is written in a C-like syntax, but it is
not intended to be compiled or run as-is. It is meant to be a reference for developers who want to create their own
bootloader or executable that follows the KURO booting convention.

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

### 1.3.9 Revisions

Updates to this document are considered either revisions or errata as described below:
- A new revision is made when significant changes are made to the document. Which changes the behavior of the bootloader,
  executable, or the booting process such as adding a new feature or fixing a bug.
- Errata version is made when a typo is found in the document or a minor change is made to the document that does not affect
  the behavior of the bootloader, executable, or the booting process such as fixing a typo or updating a link.

> [!IMPORTANT]
> Draft documents are not intended to be final and should not be considered a final version of the document. They are
> not versioned for each change until they are finalized and ready for release.

## 2. Overview

KURO is a minimal secure bootloader designed for x86\_64 UEFI systems. The bootloader is written in C and uses the UEFI
specification to communicate with the firmware.

The bootloader is designed to be as small as possible and to be self-contained. It loads ELF executables that follow the
KURO booting convention, which specifies how the executable should be structured and how the bootloader should load it
into memory.

The bootloader is designed to be secure, with a focus on minimizing the attack surface and preventing common
vulnerabilities.

The bootloader is designed to be minimal enough to boot an executable while leaving the developer with almost full
control over the boot process.

### 2.1 Executable Structure

The executable is expected to be in a standard ELF file with the following characteristics:
- Position-independent executable (PIE)
- x86\_64 architecture
- At the end of the file contains a KURO footer, which is used to identify the executable as a KURO executable.
  More information regarding the KURO footer can be found at PLACEHOLDER.

### 2.2 KURO Footer

The KURO footer is a fixed-size data structure located at the end of the executable file. It contains metadata about the
executable for the bootloader to use during the loading process.

It is expected that the bootloader will validate the KURO footer before loading the executable. The KURO footer contains the following fields:

- **Magic number**: A unique identifier to recognize the executable as a KURO executable.
- **Version**: The version of the KURO booting convention used by the executable.
- **Signature**: An Ed25519 signature of the executable to verify its authenticity.

### 2.3 Booting Process

Kuro bootloader performs the following steps to load an executable:

1. Read the ELF header and check if the executable is a valid ELF file.
2. Read the KURO footer and check if the executable is a valid KURO executable.
3. Verify the signature in the KURO footer to ensure the authenticity of the executable.
4. Load the executable into memory according to the ELF program headers.
5. Transfer control to the entry point of the executable.

### 2.4 Interfaces Provided to the Loaded Executable

The bootloader provides the following interfaces to the loaded executable:
- Executable information: Provides information about the loaded executable, such as its entry point, segment information, and more.
- System table[^3]: The UEFI system table, which provides access to various UEFI services and information.

### 2.5 Security Considerations

The bootloader is designed with security in mind, and the following security features are considered for the bootloader:

- **Secure boot**: an UEFI feature that verifies the authenticity of the bootloader before loading it.
- **Code integrity**: the bootloader verifies the integrity of the executable using the signature in the KURO footer.

### 2.6 Limitations

The bootloader is designed to be as minimal as possible, which imposes the following limitations:

- **Kernel heavy initialization**: The bootloader is designed to be minimal and provides few services or features. This
  means that the kernel or other executable loaded by the bootloader will need to perform more initialization and setup
  than it would if it were loaded by a more feature-rich bootloader.
- **No support for dynamic loading**: This is a common limitation of bootloaders, as they are designed to load a single executable and transfer control to it. This means that the kernel or other
  executable loaded by the bootloader will need to implement its own dynamic loading mechanism if it wants to load
  additional executables or libraries at runtime.
- **No support for multiple architectures**: The bootloader is not designed to support multiple architectures. This means
  that the bootloader will only be able to load executables that are designed for the x86\_64 architecture.

## Copyright

**Copyright © 2026 TheMonHub**

**Licensed under the Apache License, Version 2.0**

```text

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```

## References

The following publications and sources of information may be useful for understanding this document or are referred to
in this document:

[^1]: **This is an example of a reference.**

[^2]: **ELF Executable and Linkable Format, By Xinuos** – https://gabi.xinuos.com/elf/

[^3]: **Unified Extensible Firmware Interface Specification, Version 2.11 (UEFI 2.11)** – https://uefi.org/specs/UEFI/2.11/
