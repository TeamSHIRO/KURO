#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
echo "Running clang-tidy checks..."
./bootloader/automated/check.sh