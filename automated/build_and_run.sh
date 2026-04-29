#!/bin/bash
# "Build and run" is a script that builds the project and then opens the resulting EFI application in QEMU.
# It assumes that the project is set up to build an EFI application and that QEMU is installed on the system.

set -e

error_exit() {
    echo -e "$1"
    exit 1
}

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

ALLOCATED_MEMORY=1G
EXTRA_QEMU_ARGS="" # You can add extra arguments for QEMU here if needed
OMVF_SUBDIR="ovmf"
OVMF_SUFFIX=".secboot"
OVMF_PREFIX="OVMF_"
LOCAL_OVMF_CODE_PATH="ignore_automated/ovmf/${OVMF_PREFIX}CODE${OVMF_SUFFIX}.fd"
REMOTE_OVMF_CODE_PATH="/usr/share/edk2/${OMVF_SUBDIR}/${OVMF_PREFIX}CODE${OVMF_SUFFIX}.fd"
LOCAL_OVMF_VAR_PATH="ignore_automated/ovmf/${OVMF_PREFIX}VARS${OVMF_SUFFIX}.fd"
REMOTE_OVMF_VAR_PATH="/usr/share/edk2/${OMVF_SUBDIR}/${OVMF_PREFIX}VARS${OVMF_SUFFIX}.fd"
BOOT_DIRECTORY="ignore_automated/esp/EFI/BOOT"
BUILD_FILE_NAME="KUROX64"

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

cd "$PROJECT_ROOT" || error_exit "${B_RED} ERR! ${A_RESET} Failed to change directory"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DKURO_NO_CONFIG=ON
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

cd "$PROJECT_ROOT" || error_exit "${B_RED} ERR! ${A_RESET} Failed to change directory"

mkdir -p ignore_automated/ovmf

echo -e "${B_BLUE} INFO ${A_RESET} Build completed. Attempting to launch QEMU..."

if [ ! -f "$LOCAL_OVMF_CODE_PATH" ] && [ -f "$REMOTE_OVMF_VAR_PATH" ]; then
    echo -e "${B_YELLOW} WARN ${A_RESET} ${OVMF_PREFIX}CODE${OVMF_SUFFIX}.fd not found in the 'ovmf' directory"
    read -p "       Do you wish to move automatically $REMOTE_OVMF_CODE_PATH to the 'ovmf' directory? (y/n): " yn
    case $yn in
        [Yy]* ) cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH; echo -e "${B_GREEN}  OK  ${A_RESET} Moved.";;
        * ) echo -e "${B_YELLOW} WARN ${A_RESET} Cancelled. Run 'cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH' manually if needed."; exit 1;;
    esac
elif [ ! -f "$LOCAL_OVMF_CODE_PATH" ] && [ ! -f "$REMOTE_OVMF_VAR_PATH" ]; then
  echo -e "${B_YELLOW} WARN ${A_RESET} ${OVMF_PREFIX}CODE${OVMF_SUFFIX}.fd not found in the 'ovmf' directory"
  echo -e "${B_YELLOW} WARN ${A_RESET} Cannot find ${OVMF_PREFIX}CODE${OVMF_SUFFIX}.fd automatically, please find and copy it to $LOCAL_OVMF_CODE_PATH manually."; exit 1
fi

if [ -f "$LOCAL_OVMF_VAR_PATH" ]; then
    echo -e "${B_BLUE} INFO ${A_RESET} Launching QEMU with local OVMF..."
elif [ -f "$REMOTE_OVMF_VAR_PATH" ]; then
    echo -e "${B_YELLOW} WARN ${A_RESET} ${OVMF_PREFIX}VARS${OVMF_SUFFIX}.fd not found in the 'ovmf' directory"
    read -p "       Do you wish to move automatically $REMOTE_OVMF_VAR_PATH to the 'ovmf' directory? (y/n): " yn
    case $yn in
        [Yy]* ) cp $REMOTE_OVMF_VAR_PATH $LOCAL_OVMF_VAR_PATH; echo -e "${B_GREEN}  OK  ${A_RESET} Moved.";;
        * ) echo -e "${B_YELLOW} WARN ${A_RESET} Cancelled. Run 'cp $REMOTE_OVMF_VAR_PATH $LOCAL_OVMF_VAR_PATH' manually if needed."; exit 1;;
    esac
    read -s -n 1 -p "If no bootable option or device was found, please go to the EFI Firmware Setup and disable the secure boot. Press any key to continue."
elif [ ! -f "$REMOTE_OVMF_VAR_PATH" ]; then
  echo -e "${B_YELLOW} WARN ${A_RESET} ${OVMF_PREFIX}VARS${OVMF_SUFFIX}.fd not found in the 'ovmf' directory"
  echo -e "${B_YELLOW} WARN ${A_RESET} Cannot find ${OVMF_PREFIX}VARS${OVMF_SUFFIX}.fd automatically, please find and copy it to $LOCAL_OVMF_VAR_PATH manually."; exit 1
fi


qemu-system-x86_64 \
    -m $ALLOCATED_MEMORY \
    -drive if=pflash,format=raw,readonly=on,file=$LOCAL_OVMF_CODE_PATH \
    -drive if=pflash,format=raw,file=$LOCAL_OVMF_VAR_PATH \
    -drive if=ide,format=raw,file=fat:rw:ignore_automated/esp \
    -cpu host \
    -machine q35 \
    -accel kvm \
    -net none \
    -serial stdio \
    $EXTRA_QEMU_ARGS