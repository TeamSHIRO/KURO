#
# KURO - A minimal monolithic UEFI bootloader
# Subproject - KURO_BOOTLOADER
# =========================================
# Generate an incomplete library for the installer subproject.
#
# Build options:
# efi-clang - path to EFI-clang repository. if empty, it will be fetched automatically. Useful for offline builds.
# KURO_BUILD_DIRECTLY - Build KURO bootloader directly as an executable instead of a library
# KURO_KERNEL_PATH - Path to the kernel that KURO is boot (needed only if KURO_BUILD_DIRECTLY is ON)
#

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO_BOOTLOADER
        VERSION 0.1.0
        DESCRIPTION "A minimal monolithic UEFI bootloader - Subproject"
        HOMEPAGE_URL https://oshub.org/projects/kuro
        LANGUAGES C
)

include(toolchain/efi.cmake)

set(KURO_BUILD_DIRECTLY ON CACHE BOOL "Build KURO bootloader directly as an executable instead of a library")

# In case build directly is on, this options are needed.
set(KURO_KERNEL_PATH "" CACHE STRING "Path to the KURO kernel build output directory")

# =========================================

set(KURO_BOOTLOADER_SOURCES
        src/main.c
        src/file/file.c
)

if (NOT KURO_BUILD_DIRECTLY)
    add_library(KUROX64 STATIC
            ${KURO_BOOTLOADER_SOURCES}
    )

    target_include_directories(KUROX64 PRIVATE
            include
    )
else ()
    add_executable(KUROX64
            ${KURO_BOOTLOADER_SOURCES}
    )
    target_include_directories(KUROX64 PRIVATE
            include
    )
endif ()