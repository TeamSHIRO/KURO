#!/bin/bash
#
# Copyright (C) 2025 TheMonHub
# SPDX-License-Identifier: Apache-2.0
#

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
shopt -s globstar
find . -type f \( -name "*.h" -o -name "*.c" \) -not -path "*/_deps/*" -not -path "*/CMakeFiles/*" -exec clang-format -i -style=file --verbose {} +