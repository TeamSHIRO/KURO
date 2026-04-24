<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/res/kuro_banner_dark.png">
  <img src="docs/res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO: An UEFI bootloader

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-3.0-0baaaa.svg)](CODE_OF_CONDUCT.md)
[![GitHub license](https://img.shields.io/github/license/TeamSHIRO/KURO.svg)](LICENSE)
[![Build](https://github.com/teamSHIRO/KURO/actions/workflows/build.yml/badge.svg)](https://github.com/TeamSHIRO/KURO/actions)

A minimal secure UEFI bootloader to load executables in the ELF format using KURO booting convention. It is notably used to load the SHIRO kernel.

## Features

- Loads ELF executables using the KURO boot protocol 
- Modern and secure design
- UEFI specification compliant
- Monolithic design for security, simplicity, and performance

## For Developers

Please visit the [**KURO boot protocol**](https://github.com/TeamSHIRO/kuro-protocol) for details on how to develop an
application that uses KURO boot protocol.

## Building

Please read the [**building**](docs/building.md) guide for details on how to build this project.

## Contributing

Please read the [**CONTRIBUTING**](CONTRIBUTING.md) guide for details on how to contribute to this project.

## License

<a href="https://www.apache.org/">
    <img src="https://www.apache.org/logos/originals/foundation.svg" alt="The Apache Software Foundation" align="right" height="128">
</a>

**KURO** is licensed under **Apache License 2.0**.

The full text of the license can be obtained at
http://www.apache.org/licenses/LICENSE-2.0
or in the [**LICENSE**](LICENSE) file included in this repository.

**NOTICE** file included in this repository can be found [**here**](NOTICE).
