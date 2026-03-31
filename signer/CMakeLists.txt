#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Sign the KURO UEFI bootloader and enroll the signing key in the system firmware
#
# Build options:
# Name - Description

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO_SIGNER
        VERSION 0.1.0
        DESCRIPTION "A signer for the KURO UEFI bootloader"
        HOMEPAGE_URL "https://oshub.org/projects/kuro"
        LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

set(KURO_SIGNER_SOURCES
        src/main.c
)

add_executable(kuro-sign
        ${KURO_SIGNER_SOURCES}
)