#!/bin/bash
# "Build and run" is a script that builds the project and then opens the resulting EFI application in QEMU.
# It assumes that the project is set up to build an EFI application and that QEMU is installed on the system.

set -e

ALLOCATED_MEMORY=256M
EXTRA_QEMU_ARGS="" # You can add extra arguments for QEMU here if needed
LOCAL_OVMF_CODE_PATH="ignore_automated/ovmf/OVMF_CODE.fd"
REMOTE_OVMF_CODE_PATH="/usr/share/edk2/x64/OVMF_CODE.4m.fd"
BOOT_DIRECTORY="ignore_automated/esp/EFI/BOOT"
BUILD_FILE_NAME="KUROX64"
KERNEL_SOURCE_DIR="test-kernel"
KERNEL_OUTPUT_FILE="test-kernel/build/kernel.bin"
KERNEL_ESP_PATH="ignore_automated/esp/shiro.kernel"


# Foreground colors
T_BLACK='\033[0;30m'
T_RED='\033[0;31m'
T_GREEN='\033[0;32m'
T_YELLOW='\033[0;33m'
T_BLUE='\033[0;34m'
T_MAGENTA='\033[0;35m'
T_CYAN='\033[0;36m'
T_WHITE='\033[0;37m'
T_DEFAULT='\033[0;39m'

# Background colors
B_BLACK='\033[0;40m'
B_RED='\033[0;41m'
B_GREEN='\033[0;42m'
B_YELLOW='\033[0;43m'
B_BLUE='\033[0;44m'
B_MAGENTA='\033[0;45m'
B_CYAN='\033[0;46m'
B_WHITE='\033[0;47m'
B_DEFAULT='\033[0;49m'

# Attributes
A_RESET='\033[0m'
A_BOLD='\033[1m'
A_DIM='\033[2m'
A_ITALIC='\033[3m'
A_UNDERLINE='\033[4m'

echo -e "${B_BLUE} INFO ${A_RESET} Starting build process..."

cd "$(dirname "$0")"/.. || error_exit "${B_RED} ERR! ${A_RESET} Failed to change directory"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
mkdir -p ignore_automated

echo -e "${B_GREEN}  OK  ${A_RESET} Build successful. Preparing boot directory..."

if [ -d "$BOOT_DIRECTORY" ]; then
    rm "$BOOT_DIRECTORY/BOOTX64.EFI" 2>/dev/null || true # Remove existing BOOTX64.EFI if it exists, ignore error if it doesn't
    cp build/$BUILD_FILE_NAME.EFI $BOOT_DIRECTORY/BOOTX64.EFI
else
    echo -e "${B_YELLOW} WARN ${A_RESET}: Boot directory not found. Creating required directories..."
    mkdir -p $BOOT_DIRECTORY
    cp build/$BUILD_FILE_NAME.EFI $BOOT_DIRECTORY/BOOTX64.EFI
fi



echo -e "${B_BLUE} INFO ${A_RESET} Build completed. Attempting to launch QEMU..."

if [ -f "$LOCAL_OVMF_CODE_PATH" ]; then
    echo -e "${B_BLUE} INFO ${A_RESET} Launching QEMU with local OVMF..."
else
    echo -e "${B_YELLOW} WARN ${A_RESET} OVMF_CODE.fd not found in the '.ovmf' directory"
    read -p "       Do you wish to move automatically $REMOTE_OVMF_CODE_PATH to the '.ovmf' directory? (y/n): " yn
    case $yn in
        [Yy]* ) mkdir -p ignore_automated/ovmf && cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH; echo -e "${B_GREEN}  OK  ${A_RESET} Moved.";;
        * ) echo -e "${B_YELLOW} WARN ${A_RESET} Cancelled. Run 'cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH' manually if needed."; exit;;
    esac
fi

qemu-system-x86_64 \
    -m $ALLOCATED_MEMORY \
    -drive if=pflash,format=raw,readonly=on,file=$LOCAL_OVMF_CODE_PATH \
    -drive if=ide,format=raw,file=fat:rw:ignore_automated/esp \
    -net none \
    -serial stdio \
    $EXTRA_QEMU_ARGS