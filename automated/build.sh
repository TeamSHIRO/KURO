#!/bin/bash
#
# Copyright (C) 2026 Ellicode
# Copyright (C) 2026 TheMonHub
#
# SPDX-License-Identifier: Apache-2.0
#
# This script is copied from build-and-run.sh but only builds the project without launching QEMU.
set -e

RED='\033[0;31m' # Red color for error messages
NC='\033[0m' # No Color (reset)

echo "➔ Starting build process..."

cd "$(dirname "$0")"/.. || error_exit "${RED}⚠ Error: Failed to change directory${NC}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

echo "➔ Build successful."