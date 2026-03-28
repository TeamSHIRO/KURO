[◀ Documentation Home](README.md)

# Quick Start Guide

Get KURO running in under 5 minutes.

## 1. Install prerequisites

```bash
# Debian / Ubuntu
sudo apt install clang lld cmake git qemu-system-x86_64 ovmf

# Arch Linux
sudo pacman -S clang lld cmake git qemu-system-x86_64 edk2-ovmf
```

## 2. Clone & build

```bash
git clone https://github.com/TeamSHIRO/KURO.git
cd KURO
```

## 3. Set up OVMF

```bash
mkdir -p ignore-automated/ovmf

# Debian / Ubuntu
cp /usr/share/OVMF/OVMF_CODE.fd ignore-automated/ovmf/OVMF_CODE.fd

# Arch Linux
cp /usr/share/edk2/x64/OVMF_CODE.4m.fd ignore-automated/ovmf/OVMF_CODE.fd
```

## 4. Build & run

```bash
./automated/build-and-run.sh
```

This builds KURO, builds the included test kernel, and launches QEMU. You should see the test kernel print boot information to the screen.

---

For a full walkthrough including real hardware installation, build options, and writing your own kernel, see the [Getting Started](GETTING_STARTED.md) guide.
