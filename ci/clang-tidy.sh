#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
echo "Running clang-tidy checks..."
echo "================================"
echo "Bootloader:"
echo "================================"
./bootloader/automated/check.sh
echo "================================"
echo "Installer:"
echo "================================"
./installer/automated/check.sh
echo "================================"
echo "Signer:"
echo "================================"
./signer/automated/check.sh