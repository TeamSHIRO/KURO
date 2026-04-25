<picture>
  <source media="(prefers-color-scheme: dark)" srcset="res/kuro_banner_dark.png">
  <img src="res/kuro_banner.png" alt="KURO banner">
</picture>

# KURO Boot Protocol
[![GitHub license](https://img.shields.io/github/license/TeamSHIRO/kuro-protocol.svg)](LICENSE)

This repository contains the KURO Boot Protocol specification and C headers that can be used to implement the protocol.

You can read the specification here:
[KURO Boot Protocol Specification](docs/protocol.md)

C headers are available in the [`include`](include) directory. Containing the following files:
- [kuro.h](include/kuro.h) – Containing the KURO Identifier and common definitions
- [kuro_boot.h](include/kuro_boot.h) – Containing the KURO Boot Information structure and the related definitions
- [kuro_footer.h](include/kuro_footer.h) – Containing the KURO Footer structure and the related definitions

## License

The headers are licensed under the Boost Software License, Version 1.0. See the [LICENSE](LICENSE) file or visit
https://www.boost.org/LICENSE_1_0.txt for more information.

The boot protocol documentation is licensed under the
[Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/).