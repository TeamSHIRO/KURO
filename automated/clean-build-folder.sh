#!/bin/bash
#
# Copyright (C) 2025-2026 TheMonHub
# Copyright (C) 2026 Ellicode
# SPDX-License-Identifier: Apache-2.0
#
# "Clean build folder" is a script that removes all files from the build directory, effectively cleaning it.
# It assumes that the build directory is named "build" and is located in the same directory

BUILD_DIR="build"

if [ -d "$BUILD_DIR" ]; then
    echo "➔ Cleaning build directory..."
    rm -rf "$BUILD_DIR"/*
    echo "➔ Build directory cleaned."
else
    echo "➔ Build directory not found. No action taken."
fi