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