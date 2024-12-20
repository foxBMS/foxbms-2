# foxBMS 2

foxBMS is a free, open and flexible development environment to design battery
management systems.
It is the first modular open source BMS development platform.

## Overview

foxBMS is an universal hardware and software platform providing a fully open
source BMS development platform.
foxBMS aims to control modern and complex electrical energy storage systems of
any size.
foxBMS is successfully used for electrical energy storage consisting of

- Lithium-Ion and Solid State Batteries
- Lithium-Sulfur Batteries
- Sodium-Ion Batteries
- Lithium-Ion Capacitors (LIC)
- Electric Double-Layer Capacitors (EDLC, supercapacitors or ultracaps)
- Redox-Flow Batteries (RFB) (e.g., Vanadium Redox-Flow), and
- Fuel Cells (FC)

or in an hybrid combination of these and in various domains.

## Documentation

A current build of the documentation of this project can be found
here:

- [latest documentation build (of the most recent release)](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/docs/html/latest/)
- [list of all available documentation builds](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/docs/html/)

## Changelog

The project changelog is found in
[docs/general/changelog.rst](./docs/general/changelog.rst).

## Installation Instructions

See [INSTALL.md](./INSTALL.md) for installation instructions.

## Repository Structure

The repository is structured as follows:

| Directory Name            |  Content Description                                                                                                            |
| ------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| `.`                       | The repository root contains the license information, installation instructions, a link to the changelog, the main build script |
| [`cli`](./cli)            | CLI tool to interact with the repository                                                                                        |
| [`conf`](./conf)          | Contains all high level configurations                                                                                          |
| [`docs`](./docs)          | Documentation source files                                                                                                      |
| [`hardware`](./hardware)  | Hardware schematic and layout information                                                                                       |
| [`src`](./src)            | Parent directory for all source files for the BMS embedded software                                                             |
| [`tests`](./tests)        | Tests for embedded sources, the tool chain, and scripts                                                                         |
| [`tools`](./tools)        | Tools needed to build foxBMS binaries and additional tools to work with foxBMS                                                  |

These directories each have a `README.md` that explain the content of the
directory.

## License

The license information of the project is found in [LICENSE.md](./LICENSE.md).

Basically, the software is covered by the
[**BSD 3-Clause License (SPDX short identifier: BSD-3-Clause)**](https://opensource.org/licenses/BSD-3-Clause)
and the hardware and documentation by the
[**Creative Commons Attribution 4.0 International License (SPDX short identifier: CC-BY-4.0)**](https://creativecommons.org/licenses/by/4.0/legalcode).

## Open Source Hardware Certification

foxBMS 2 has been certified as open source hardware by the
Open Source Hardware Association under the OSHWA UID
[DE000128](https://certification.oshwa.org/de000128.html).

## Acknowledgment

For funding acknowledgements and instructions on how to acknowledge foxBMS 2
please see [foxbms.org/acknowledgements](https://foxbms.org/acknowledgements/).
