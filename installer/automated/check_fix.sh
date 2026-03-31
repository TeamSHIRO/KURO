#!/bin/bash
set -e

cd "$(dirname "$0")"/.. || error_exit "Failed to change directory"
mkdir -p ignore_clang_tidy
cmake -S . -B ignore_clang_tidy -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build ignore_clang_tidy
run-clang-tidy '^(?!.*_deps/)' -p ignore_clang_tidy -quiet -fix