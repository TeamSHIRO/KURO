<div style="height: 175px; overflow: hidden;">
  <img src="https://raw.githubusercontent.com/TeamSHIRO/.github/refs/heads/main/profile/kuro.png" 
       alt="KURO banner" 
       title="KURO banner"
       style="width: 100%; height: 100%; object-fit: cover;">
</div>

# KURO: An UEFI bootloader
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-3.0-0baaaa.svg)](CODE_OF_CONDUCT.md)
[![GitHub license](https://img.shields.io/github/license/TeamSHIRO/KURO.svg)](LICENSE)

A minimal secure UEFI bootloader to load executables in the ELF format using KURO booting convention. 

## Building

### Prerequisites

- A Clang compiler
- A LLD linker
- CMake 3.26 or Newer
- A Git Client
- Internet Connection (for fetching dependencies duh)

### Building in a nutshell

To build the project, follow these steps:
1. Clone the repository:
   ```bash
   git clone https://github.com/TeamSHIRO/KURO.git
   ```
2. CD to repository directory:
   ```bash
   cd KURO
   ```
3. Run CMake to configure the project. You may additionally add build options here (`-D`). You can find available options inside [**CMakeLists.txt**](CMakeLists.txt).
    ```bash
    cmake -S . -B build --toolchain toolchain/efi.cmake
    ```
   Change the `--toolchain` argument to where the **[efi.cmake](toolchain/efi.cmake)** toolchain file is located.
   tip: You can change `build` to your preferred build directory.
4. Build the project with CMake
    ```bash
    cmake --build build
    ```
   Pro tip: Replace `build` with your build directory.


## Contributing
Please read the [**CONTRIBUTING**](CONTRIBUTING.md) guide for details on how to contribute to this project.

## License
<a href="https://www.apache.org/">
    <img src="https://www.apache.org/logos/originals/foundation.svg" alt="The Apache Software Foundation" style="float: right; width: 300px;">
</a>

**KURO** is licensed under **Apache License 2.0**.

The full text of the **Apache License 2.0** can be obtained at: <http://www.apache.org/licenses/LICENSE-2.0>
or in the [**LICENSE**](LICENSE) file included in this repository.

**NOTICE** file included in this repository can be found [**here**](NOTICE).