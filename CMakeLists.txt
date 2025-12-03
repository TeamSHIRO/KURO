#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Build options:
# efi-clang - path to EFI-clang repository. if empty, it will be fetched automatically. Useful for offline builds.
#

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO
        VERSION 0.1.0
        DESCRIPTION "A minimal monolithic UEFI bootloader"
        HOMEPAGE_URL https://oshub.org/projects/kuro
        LANGUAGES C ASM_NASM
)

# Build options


# KURO Bootloader
add_executable(KUROX64
        src/main.c
        src/file/file.c
)
target_include_directories(KUROX64 PRIVATE include)