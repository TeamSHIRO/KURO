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
        LANGUAGES C
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

set(CMAKE_C_COMPILE_OBJECT
        "<CMAKE_C_COMPILER> <FLAGS> <DEFINES> -target x86_64-pc-win32-coff <INCLUDES> -c -o <OBJECT> <SOURCE>"
)

set(CMAKE_C_FLAGS "-ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -fshort-wchar")
set(CMAKE_C_LINK_EXECUTABLE "lld -flavor link -subsystem:efi_application -entry:efi_main -nodefaultlib <OBJECTS> <LINK_LIBRARIES> /out:<TARGET>.EFI")
# =========================================
# End of EFI toolchain setup
# =========================================

set(KURO_BOOTLOADER_SOURCES
    src/main.c
)

add_executable(KUROX64
        ${KURO_BOOTLOADER_SOURCES}
)
target_include_directories(KUROX64 PRIVATE _deps/efi)