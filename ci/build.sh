#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
echo "Running build tests..."
mkdir -p build
cmake -S . -B build
cmake --build build