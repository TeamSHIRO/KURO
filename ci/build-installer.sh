#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"

echo "Running build tests..."
echo "================================"
echo "Installer:"
echo "================================"
mkdir -p build_installer
cmake -S installer -B build_installer
cmake --build build_installer