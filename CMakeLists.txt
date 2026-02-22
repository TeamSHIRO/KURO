#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Generate an incomplete library for the installer subproject.
#
# Build options:
# efi-clang - path to EFI-clang repository. if empty, it will be fetched automatically. Useful for offline builds.
# KURO_DEFAULT_CONFIG - path to default configuration file. If empty, the default configuration will be used
#

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO
        VERSION 0.1.0
        DESCRIPTION "A minimal monolithic UEFI bootloader"
        HOMEPAGE_URL "https://oshub.org/projects/kuro"
        LANGUAGES C
)

set(KURO_DEFAULT_CONFIG "" CACHE STRING "Path to default configuration file. If empty, the default configuration will be used")
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

if (NOT "${KURO_DEFAULT_CONFIG}" STREQUAL "")
    file(READ "${KURO_DEFAULT_CONFIG}" KURO_DEFAULT_CONFIG_SERIALIZED)

    string(REPLACE "\\" "\\\\" KURO_DEFAULT_CONFIG_SERIALIZED "${KURO_DEFAULT_CONFIG_SERIALIZED}")

    string(REPLACE "\"" "\\\"" KURO_DEFAULT_CONFIG_SERIALIZED "${KURO_DEFAULT_CONFIG_SERIALIZED}")

    string(REPLACE "\n" "\\n" KURO_DEFAULT_CONFIG_SERIALIZED "${KURO_DEFAULT_CONFIG_SERIALIZED}")

    target_compile_definitions(KUROX64 PRIVATE
            KURO_DEFAULT_CONFIG="${KURO_DEFAULT_CONFIG_SERIALIZED}"
    )
endif()