#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"

echo "Running build tests..."
echo "================================"
echo "Bootloader:"
echo "================================"
mkdir -p build_bootloader
cmake -S bootloader -B build_bootloader
cmake --build build_bootloader
echo "================================"
echo "Installer:"
echo "================================"
mkdir -p build_installer
cmake -S installer -B build_installer
cmake --build build_installer
echo "================================"
echo "Signer:"
echo "================================"
mkdir -p build_signer
cmake -S signer -B build_signer
cmake --build build_signer