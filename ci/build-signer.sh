#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"

echo "Running build tests..."
echo "================================"
echo "Signer:"
echo "================================"
mkdir -p build_signer
cmake -S signer -B build_signer
cmake --build build_signer