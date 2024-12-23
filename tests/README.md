# `tests` Directory Description

| Directory Name   | Long Name                   | Content Description                                                      |
| ---------------- | --------------------------- | ------------------------------------------------------------------------ |
| `axivion`        | Axivion                     | Configuration files to describe the software architecture of foxBMS. Additional static program analysis configurations are stored here (e.g., MISRA-C) |
| `can`            | CAN                         | Heuristic tests for the correct implementation of DBC file in the code   |
| `ccs`            | Code Composer Studio        | Compiler setup for different CCS versions to compile foxBMS with         |
| `cli`            | Command line interface      | Tests for the CLI tool, to interact with the repository                  |
| `c-std`          | C Standard                  | Test setup for foxBMS's C standard detection                             |
| `dbc`            | CAN Database                | Check the validity of the DBC/SYM files                                  |
| `hil`            | Hardware-in-the-Loop        | Tests and setup of the Hardware-in-the-Loop test (**not published**)     |
| `os-information` | OS information              | Information to search the sources for OS relevant includes, names etc.   |
| `unit`           | Embedded code unit tests    | Unit test code for the embedded code                                     |
| `unit-hw`        | Embedded code unit tests    | Unit test code for the embedded code that need to run on the target      |
| `variants`       | Variants                    | foxBMS build configurations (for testing in e.g., CI)                    |
