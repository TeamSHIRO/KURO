#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Generate a UEFI bootloader
#
# Build options:
# Name - Description
# KURO_NO_CONFIG - Disable configuration support (only for testing)

# Default Config when KURO_NO_CONFIG is ON:
# KuroConfig default_config = {
#     .secure_mode = 0,
#     .aslr_enabled = 1,
#     // .public_key is set to all zeros.
#     .log_level = 4,
#     .console_log_level = 4,
#     // .str_offset is not defined
#     // Default is:
#     // Executable path: \kernel
#     // No module
#     // Command-line arguments is empty
#     .identifier = // identification stuff yk the deal
# };

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

option(KURO_NO_CONFIG "Disable configuration support (only for testing)" OFF)

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
        src/status.c
        src/status.h
        include/kuro_conf.h
        src/aslr.c
        src/aslr.h
        src/conf.c
        src/conf.h
        src/efi_dptt.h
        src/kuro_const.h
        src/kuro_const.c
        src/boot_info.c
        src/boot_info.h
)

add_executable(KUROX64
        ${KURO_BOOTLOADER_SOURCES}
)

target_include_directories(KUROX64 SYSTEM PRIVATE _deps/efi _deps/kuro/include include)
target_compile_definitions(KUROX64 PRIVATE PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
if (KURO_NO_CONFIG)
    target_compile_definitions(KUROX64 PRIVATE KURO_NO_CONFIG=1)
endif()