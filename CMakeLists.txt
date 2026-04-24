#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Generate a UEFI bootloader
#
# Build options:
# Name - Description

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO_BOOTLOADER
        VERSION 0.1.0
        DESCRIPTION "A minimal secure UEFI bootloader"
        HOMEPAGE_URL "https://oshub.org/projects/kuro"
        LANGUAGES C ASM_NASM
)

set(CMAKE_C_STANDARD 17)

# =========================================
# Setup the EFI toolchain
# =========================================
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_C_COMPILER_FORCED TRUE)

add_compile_definitions(EFI_PLATFORM=1)

set(CMAKE_ASM_NASM_COMPILE_OBJECT
        "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -f win64 <INCLUDES> -o <OBJECT> <SOURCE>"
)

set(CMAKE_C_COMPILE_OBJECT
        "<CMAKE_C_COMPILER> <FLAGS> <DEFINES> -target x86_64-pc-win32-coff <INCLUDES> -c -o <OBJECT> <SOURCE>"
)

set(CMAKE_C_FLAGS "-ffreestanding -nostdlib -fshort-wchar")
set(CMAKE_C_LINK_EXECUTABLE "lld -flavor link -subsystem:efi_application -entry:efi_main -nodefaultlib <OBJECTS> <LINK_LIBRARIES> /out:<TARGET>.EFI")
# =========================================
# End of EFI toolchain setup
# =========================================

set(KURO_BOOTLOADER_SOURCES
    src/main.c
        src/boot.c
        src/boot.h
        src/file.c
        src/file.h
        src/string.c
        src/string.h
        src/elf.h
        src/elf.c
        src/efi_helper.h
        src/transfer_control.h
        src/transfer_control.asm
        src/status.c
)

add_executable(KUROX64
        ${KURO_BOOTLOADER_SOURCES}
)
target_include_directories(KUROX64 SYSTEM PRIVATE _deps/efi)
target_compile_definitions(KUROX64 PRIVATE PROJECT_VERSION=L"${CMAKE_PROJECT_VERSION}\\r\\n")