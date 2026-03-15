#!/bin/bash
#
# Copyright (C) 2026 Ellicode
#
# SPDX-License-Identifier: Apache-2.0
#
# "Build and test kernel" is a script that builds the kernel project and then runs it in QEMU for testing purposes.

cd test-kernel

cmake -S . -B build
cmake --build build

cp isofiles/boot/kernel.bin kernel.bin

grub-mkrescue -o mykernel.iso isofiles

qemu-system-x86_64 -m 512M -boot order=d -drive file=mykernel.iso,format=raw,media=cdrom