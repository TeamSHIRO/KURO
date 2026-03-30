[◀ Documentation Home](README.md)

# Getting Started

This guide walks you through installing prerequisites, building KURO, and running it — from a fresh clone to a booting kernel.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Cloning the Repository](#cloning-the-repository)
- [Building KURO](#building-kuro)
    - [Standard Build](#standard-build)
    - [Build Options](#build-options)
    - [Automated Scripts](#automated-scripts)
- [Running in QEMU](#running-in-qemu)
    - [OVMF Setup](#ovmf-setup)
    - [ESP Layout](#esp-layout)
    - [Launching QEMU](#launching-qemu)
- [Installing on Real Hardware](#installing-on-real-hardware)
- [Next Steps](#next-steps)

---

## Prerequisites

Ensure the following tools are installed and available in your `PATH`:

| Tool                 | Purpose                           | Minimum Version |
| -------------------- | --------------------------------- | --------------- |
| `clang`              | C compiler (freestanding x86-64)  | Any recent      |
| `lld`                | Linker (`-flavor link`)           | Any recent      |
| `cmake`              | Build system                      | 3.26            |
| `git`                | Cloning & fetching dependencies   | Any             |
| `qemu-system-x86_64` | Running KURO in a VM _(optional)_ | Any             |

Verify the required tools are available:

```bash
clang --version && lld --version && cmake --version
```

**Debian / Ubuntu**

```bash
sudo apt install clang lld cmake git qemu-system-x86_64
```

**Arch Linux**

```bash
sudo pacman -S clang lld cmake git qemu-system-x86_64
```

**Fedora**

```bash
sudo dnf install clang lld cmake git qemu-system-x86_64
```

---

## Cloning the Repository

```bash
git clone https://github.com/TeamSHIRO/KURO.git
cd KURO
```

The EFI headers ([efi-clang](https://github.com/yoppeh/efi)) are fetched automatically during the CMake configure step via `FetchContent`. An active internet connection is required unless you supply a local copy via `-Defi-clang=` (see [Build Options](#build-options)).

---

## Building KURO

### Standard Build

```bash
cmake -S . -B build
cmake --build build
```

The resulting EFI application is placed at `build/KUROX64.EFI`.

> [!TIP]
> Add `-DCMAKE_BUILD_TYPE=Debug` to include debug information.

### Build Options

| Option                | Type   | Default               | Description                                                                                                                          |
| --------------------- | ------ | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| `efi-clang`           | `PATH` | _(auto-fetched)_      | Path to a local copy of the [efi-clang](https://github.com/yoppeh/efi) headers. Useful for offline or reproducible builds.           |
| `KURO_DEFAULT_CONFIG` | `PATH` | _(built-in defaults)_ | Path to a `.conf` file to embed as the compiled-in default configuration. If omitted, KURO generates a default config on first boot. |

Example — offline build using a local efi-clang copy:

```bash
cmake -S . -B build -Defi-clang=/path/to/efi-clang
cmake --build build
```

Example — embedding a custom default configuration:

```bash
cmake -S . -B build -DKURO_DEFAULT_CONFIG=myconfig.conf
cmake --build build
```

### Automated Scripts

The `automated/` directory contains helper scripts for common tasks:

| Script                            | Description                                   |
| --------------------------------- | --------------------------------------------- |
| `automated/build.sh`              | Configures and builds KURO only               |
| `automated/build-and-run.sh`      | Builds KURO + test kernel, then launches QEMU |
| `automated/clean-build-folder.sh` | Removes the `build/` directory                |
| `automated/clang-format.sh`       | Checks source formatting with clang-format    |
| `automated/clang-format-fix.sh`   | Applies clang-format fixes in-place           |

---

## Running in QEMU

### OVMF Setup

KURO requires an OVMF firmware image to boot in QEMU. Place `OVMF_CODE.fd` at:

```
ignore-automated/ovmf/OVMF_CODE.fd
```

The firmware image can be found via your package manager:

| Distro          | Package     | Path                                  |
| --------------- | ----------- | ------------------------------------- |
| Debian / Ubuntu | `ovmf`      | `/usr/share/OVMF/OVMF_CODE.fd`        |
| Arch Linux      | `edk2-ovmf` | `/usr/share/edk2/x64/OVMF_CODE.4m.fd` |
| Fedora          | `edk2-ovmf` | `/usr/share/edk2/ovmf/OVMF_CODE.fd`   |

Copy it to the expected location:

```bash
mkdir -p ignore-automated/ovmf
cp /usr/share/edk2/x64/OVMF_CODE.4m.fd ignore-automated/ovmf/OVMF_CODE.fd
```

> [!TIP]
> Running `./automated/build-and-run.sh` will detect a missing OVMF image and offer to copy it for you automatically.

### ESP Layout

KURO reads files from an EFI System Partition directory at `ignore-automated/esp/`. The expected layout is:

```
ignore-automated/esp/
├── EFI/
│   └── BOOT/
│       └── BOOTX64.EFI       ← KURO binary (copied here by the build script)
├── shiro.kernel               ← Kernel ELF binary (default kernel_path)
└── kuro/
    ├── config/                ← KURO configuration file location
    └── logs/                  ← Logger output
```

### Launching QEMU

The easiest way is to run the all-in-one script, which builds everything and launches QEMU in one step:

```bash
./automated/build-and-run.sh
```

To launch QEMU manually after building and placing the EFI binary:

```bash
qemu-system-x86_64 \
    -m 256M \
    -drive if=pflash,format=raw,readonly=on,file=ignore-automated/ovmf/OVMF_CODE.fd \
    -drive if=ide,format=raw,file=fat:rw:ignore-automated/esp \
    -net none \
    -serial stdio
```

---

## Installing on Real Hardware

1. Format a USB drive or partition with a **FAT32** EFI System Partition.
2. Copy `build/KUROX64.EFI` to `EFI/BOOT/BOOTX64.EFI` on the partition.
3. Place your compiled kernel ELF at the path defined by `kernel_path` in the config (default: `\shiro.kernel`).
4. Boot from the drive — your UEFI firmware will discover `BOOTX64.EFI` automatically.

> [!WARNING]
> Secure Boot must be disabled (or your own signing keys enrolled) when booting KURO on real hardware, as the binary is not currently signed.
