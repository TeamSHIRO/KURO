set(efi-clang "" CACHE PATH "Path to EFI-clang repository. if empty, it will be fetched automatically.")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_C_COMPILER_FORCED TRUE)

set(CMAKE_ASM_NASM_COMPILE_OBJECT
        "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -f win64 <INCLUDES> -o <OBJECT> <SOURCE>"
)
set(CMAKE_C_COMPILE_OBJECT
        "<CMAKE_C_COMPILER> <FLAGS> -target x86_64-pc-win32-coff <INCLUDES> -c -o <OBJECT> <SOURCE>"
)

set(CMAKE_C_FLAGS "-ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -fshort-wchar")
set(CMAKE_C_LINK_EXECUTABLE "lld -flavor link -subsystem:efi_application -entry:efi_main -nodefaultlib <OBJECTS> <LINK_LIBRARIES> /out:<TARGET>")

include(FetchContent)

if ("${efi-clang}" STREQUAL "")
    FetchContent_Declare(
            efi-clang-fetch
            GIT_REPOSITORY https://github.com/yoppeh/efi.git
    )

    FetchContent_MakeAvailable(efi-clang-fetch)
    message(STATUS "Fetched EFI-clang to ${efi-clang-fetch_SOURCE_DIR}")
    set(efi-clang_SOURCE_DIR "${efi-clang-fetch_SOURCE_DIR}")
else ()
    set(efi-clang_SOURCE_DIR "${efi-clang}")
endif ()
message(STATUS "Using EFI-clang from ${efi-clang_SOURCE_DIR}")

include_directories(${efi-clang_SOURCE_DIR})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_EXECUTABLE_SUFFIX ".EFI")