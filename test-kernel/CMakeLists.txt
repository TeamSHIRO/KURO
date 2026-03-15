#
# Example CMakeLists.txt for building a kernel module in ELF format. 
#

# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 17)

#=====================================================

project(kernel
        VERSION 0.1.0
        DESCRIPTION "A kernel module for testing purposes"
        LANGUAGES C
)


set(KERNEL_SOURCES
        src/kernel.c
)

add_executable(kernel ${KERNEL_SOURCES})

target_compile_options(kernel PRIVATE
        -m64
        -ffreestanding
        -fpie
        -fno-stack-protector
        -mno-red-zone
        -Wall
        -Wextra
)

target_link_options(kernel PRIVATE
        -m64
        -pie
        -nostdlib
        -nostartfiles
        -nodefaultlibs
        "-Wl,-T,${CMAKE_SOURCE_DIR}/linker.ld"
        -Wl,--build-id=none
        -Wl,-z,max-page-size=0x1000
)

set_target_properties(kernel PROPERTIES
        OUTPUT_NAME "kernel.bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/isofiles/boot"
)
