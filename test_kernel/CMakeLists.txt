cmake_minimum_required(VERSION 3.20)
project(kernel C)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_C_COMPILER_FORCED TRUE)

add_compile_definitions(EFI_PLATFORM=1)

add_executable(kernel main.c)

target_compile_options(kernel PRIVATE
        -fPIE
        -ffreestanding
        -fshort-wchar
)

target_link_options(kernel PRIVATE
        -nostdlib
        -pie
)

target_include_directories(kernel PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../_deps/efi
)