#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
echo "Running clang-format checks..."
echo "================================"
echo "Installer:"
echo "================================"
./installer/automated/format.sh