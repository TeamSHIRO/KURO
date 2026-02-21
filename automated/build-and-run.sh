#!/bin/bash
#
# Copyright (C) 2026 Ellicode
# Copyright (C) 2026 TheMonHub
# SPDX-License-Identifier: Apache-2.0
#
# "Build and run" is a script that builds the project and then opens the resulting EFI application in QEMU.
# It assumes that the project is set up to build an EFI application and that QEMU is installed on the system.

ALLOCATED_MEMORY=512M
EXTRA_QEMU_ARGS="" # You can add extra arguments for QEMU here if needed
LOCAL_OVMF_CODE_PATH="ignore-automated/ovmf/OVMF_CODE.fd"
REMOTE_OVMF_CODE_PATH="/usr/share/edk2/x64/OVMF_CODE.4m.fd"
BOOT_DIRECTORY="ignore-automated/esp/EFI/BOOT"
BUILD_FILE_NAME="KUROX64"

RED='\033[0;31m' # Red color for error messages
YELLOW='\033[1;33m' # Yellow color for warnings
NC='\033[0m' # No Color (reset)

echo "➔ Starting build process..."

cd "$(dirname "$0")"/.. || error_exit "${RED}⚠ Error: Failed to change directory${NC}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
mkdir -p ignore-automated

echo "➔ Build successful. Preparing boot directory..."

if [ -d "$BOOT_DIRECTORY" ]; then
    rm "$BOOT_DIRECTORY/BOOTX64.EFI" 2>/dev/null || true # Remove existing BOOTX64.EFI if it exists, ignore error if it doesn't
    cp build/$BUILD_FILE_NAME.EFI $BOOT_DIRECTORY/BOOTX64.EFI
else
    echo -e "${YELLOW}⚠ Warning: Boot directory not found. Creating required directories...${NC}"
    mkdir -p $BOOT_DIRECTORY
    cp build/$BUILD_FILE_NAME.EFI $BOOT_DIRECTORY/BOOTX64.EFI
fi

echo "➔ Build completed. Attempting to launch QEMU..."

if [ -f "$LOCAL_OVMF_CODE_PATH" ]; then
    echo "➔ Launching QEMU with local OVMF..."
else
    echo -e "${RED}⚠ Error: OVMF_CODE.fd not found in the '.ovmf' directory${NC}"
    read -p "Do you wish to move automatically $REMOTE_OVMF_CODE_PATH to the '.ovmf' directory? (y/n): " yn
    case $yn in
        [Yy]* ) mkdir -p ignore-automated/ovmf && cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH; echo "➔ Moved.";;
        * ) echo "Cancelled. Run 'cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH' manually if needed."; exit;;
    esac
fi

qemu-system-x86_64 \
    -m $ALLOCATED_MEMORY \
    -drive if=pflash,format=raw,readonly=on,file=$LOCAL_OVMF_CODE_PATH \
    -drive if=ide,format=raw,file=fat:rw:ignore-automated/esp \
    -net none \
    $EXTRA_QEMU_ARGS