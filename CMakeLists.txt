#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Build options:
# efi-clang - path to EFI-clang repository. if empty, it will be fetched automatically. Useful for offline builds.
#

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO
        DESCRIPTION "A minimal monolithic UEFI bootloader"
        HOMEPAGE_URL https://oshub.org/projects/kuro
)

set(KURO_BUILD_DIRECTLY OFF CACHE BOOL "Build KURO bootloader directly as an executable instead of a library")
add_subdirectory(bootloader)
add_subdirectory(installer)