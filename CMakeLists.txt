#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Build options:
# efi-clang - path to EFI-clang repository. if empty, it will be fetched automatically.
#

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO
        VERSION 0.1.0
        DESCRIPTION "A minimal monolithic UEFI bootloader"
        HOMEPAGE_URL https://oshub.org/projects/kuro
        LANGUAGES C ASM_NASM
)

# Build options
set(efi-clang "" CACHE PATH "Path to EFI-clang repository. if empty, it will be fetched automatically.")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)

# Fetch Clang-EFI
if ("${efi-clang}" STREQUAL "")
    FetchContent_Declare(
            efi-clang
            GIT_REPOSITORY https://github.com/yoppeh/efi.git
    )

    FetchContent_MakeAvailable(efi-clang)
    message(STATUS "Fetched EFI-clang to ${efi-clang_SOURCE_DIR}")
else ()
    set(efi-clang_SOURCE_DIR "${efi-clang}")
endif ()
message(STATUS "Using EFI-clang from ${efi-clang_SOURCE_DIR}")

# Macro stuff
macro(set_efi_target target)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${target}.EFI"
        LINKER_LANGUAGE C
    )

    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:C>:-fshort-wchar>
        $<$<COMPILE_LANGUAGE:C>:-mno-red-zone>
        $<$<COMPILE_LANGUAGE:C>:-fno-stack-protector>
    )

    target_include_directories(${target} PRIVATE
        ${efi-clang_SOURCE_DIR}
    )
endmacro()

# KURO Bootloader
add_executable(kuro
    src/main.c
)
set_efi_target(kuro)
