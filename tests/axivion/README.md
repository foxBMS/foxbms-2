# Axivion Bauhaus Suite Configuration for foxBMS 2

> **NOTE:** For running local Axivion builds see
  [scripts/README.md](scripts/README.md)

## Directory Description

- Shared configurations are in this directory.
- The actual configuration base for each project are subdirectories in the
  `targets` directory.
  The projects are (details of each directory are described below):
  - `app` and
  - `bootloader`.
- This directory contains the common Axivion setup for the `TI ARM CGT`
  compiler for TI TMS570LC4357.
  The Axivion setup is done in the following files:
  - `axivion_preinc.h` and `compiler_config.json`
    describe compiler built-ins for the TI ARM CGT compiler for TMS570LC4357.
  - `compiler_config.json`, `rule_config_c.json`,
    `rule_config_c.py`, `rule_config_include_guard.py`,
    `rule_config_metric_lines.json`, `rule_config_names.json`, and
    `rule_config_names.py` provide project specific settings.
- The script `axivion_self_tests.py` and its accompanying configuration script
  `axivion_self_tests.json` implement self tests of Axivion and the project
  specific configuration.
- The subdirectory `violations` contains a list of rules that shall be
  followed, i.e., it is not possible to introduce source code, that creates
  an Axivion warning/error for that type (`forbidden-rule-violations.txt`).
  A script is included to verify that such source code can not be introduced to
  the default branch (`check_violations.py`).
- The subdirectory `scripts` contains a set of helper scripts to
  interact with Axivion or the output of an Axivion build.
  See [scripts/README.md](scripts/README.md) for a detailed description.

### `app`-Project

The `app`-project configuration files are in [targets/app](targets/app).

- Project configuration:
  - `axivion_config.json` (default configuration name when the environment
    variable `BAUHAUS_CONFIG` is used)
  - `ci_config.json`
- Software architecture description of foxBMS 2:
  - `architecture.gxl`,
  - `architecture_config.json`,
  - `mapping.gxl`, and
  - `architecture_hierarchy_belongs_to_layout.gvl`.

### `bootloader`-Project

The `app`-project configuration files are in
[targets/bootloader](targets/bootloader).

- Project configuration:
  - `axivion_config.json` (default configuration name when the environment
    variable `BAUHAUS_CONFIG` is used)
  - `ci_config.json`

## Self Tests

There are four self tests implemented:

- project specific rule extensions to Axivion (`addon`).
  The accompanying tests are in (`addon-test`),
- tests that the project specific configuration for naming conventions is
  correct (`config-test`),
- Axivion Tool Qualification Kit (`qualification-test`), and
- TI ARM CGT 20.2.6.LTS compiler errata tests (`compiler-errata`).

Available tests can be run using the script `axivion_self_tests.py`.

## `compiler_config.json`

The compiler configuration file is created by running `ticgtsetup`.
This example shows the command line for TI ARM CGT 20.2.6.

```pwsh
ticgtsetup --cc "C:\ti\ccs1200\ccs\tools\compiler\ti-cgt-arm_20.2.6.LTS\bin\armcl.EXE --silicon_version=7R5 --code_state=32 --float_support=VFPv3D16 -g --diag_wrap=off --display_error_number --enum_type=packed --abi=eabi --c11 -Ooff --issue_remarks --strict_ansi" --cxx "C:\ti\ccs1200\ccs\tools\compiler\ti-cgt-arm_20.2.6.LTS\bin\armcl.EXE --silicon_version=7R5 --code_state=32 --float_support=VFPv3D16 -g --diag_wrap=off --display_error_number --enum_type=packed --abi=eabi --c11 -Ooff --issue_remarks --strict_ansi" --config tests\axivion\compiler_config.json
```

With the appropriate Axivion version in PATH and the appropriate compiler
version specified in the arguments.

## `axivion_preinc.h`

Most of the declaration are from the TI ARM CGT compiler manual.
The information is extended with the information from the Axivion pre-include
file.

- SPNU151V-January1998-RevisedFebruary2020: 6.9 Built-In Functions
- SPNU151V-January1998-RevisedFebruary2020: 5.14 ARM Instruction Intrinsics
  Table 5-3. ARM Intrinsic Support by Target (entries for ARM V7R (Cortex-R4))
- SPNU151V-January1998-RevisedFebruary2020: 5.14 ARM Instruction Intrinsics
  Table 5-4. ARM Compiler Intrinsics

## MISRA C:2012 Hints

| Strictness | Meaning                     | Deviations                                     | foxBMS 2 specifics              |
|------------|-----------------------------|------------------------------------------------|---------------------------------|
| mandatory  | Rule **SHALL** be followed  | No deviations.                                 | -                               |
| required   | Rule **SHOULD** be followed | Deviations need to be justified and documented | -                               |
| advisory   | Rule **SHOULD** be followed | Deviations need to be documented               | Rule also needs to be justified |
