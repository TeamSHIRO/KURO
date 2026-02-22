#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Generate an incomplete library for the installer subproject.
#
# Build options:
# efi-clang - path to EFI-clang repository. if empty, it will be fetched automatically. Useful for offline builds.
#

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO
        VERSION 0.1.0
        DESCRIPTION "A minimal monolithic UEFI bootloader"
        HOMEPAGE_URL "https://oshub.org/projects/kuro"
        LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

include(toolchain/efi.cmake)

# =========================================

set(KURO_SOURCES
        src/main.c
        src/file.c
        src/logger.c
        src/config.c
        src/memory.c
        src/string.c
)

add_executable(KUROX64
        ${KURO_SOURCES}
)
target_include_directories(KUROX64 PRIVATE
        include
)