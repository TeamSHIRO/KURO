#
# KURO - A minimal monolithic UEFI bootloader
# =========================================
# Install the KURO UEFI bootloader to the system
#
# Build options:
# Name - Description

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(KURO_INSTALLER
        VERSION 0.1.0
        DESCRIPTION "An installer for the KURO UEFI bootloader"
        HOMEPAGE_URL "https://oshub.org/projects/kuro"
        LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

set(KURO_INSTALLER_SOURCES
        src/main.c
)

add_executable(kuro-install
        ${KURO_INSTALLER_SOURCES}
)