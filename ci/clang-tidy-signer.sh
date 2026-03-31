#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
echo "Running clang-tidy checks..."
echo "================================"
echo "Signer:"
echo "================================"
./signer/automated/check.sh