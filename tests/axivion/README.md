# Axivion Bauhaus Suite Configuration for foxBMS 2

## Directory Description

- This directory contains the Axivion setup for the TI ARM CGT compiler for TI
  TMS570LC4357 and the project architecture.
  The Axivion setup is done in the following files:
  - ``.axivion.preinc`` and ``compiler_config.json``
    describe compiler built-ins for the TI ARM CGT compiler for TMS570LC4357.
  - ``architecture.gxl``, ``architecture_config.json`` and ``mapping.gxl``
    describe the software architecture of foxBMS 2.
  - ``axivion_config.json``, ``ci_config.json``, ``compiler_config.json``,
    ``rule_config_c.json``, ``rule_config_c.py`` provide project specific
    settings.
- The subdirectory ``forbidden-violations`` contains a list of rules that shall
  be followed, i.e., it is not possible to introduce source code, that creates
  an Axivion warning/error for that type (``forbidden-violations.txt``).
  A script is included to verify that such source code can not be introduced to
  the default branch (``check_forbidden_violations.py``).
- The subdirectory ``scripts`` contains a set of helper scripts to
  interact with Axivion or the output of an Axivion build.
  See [scripts/README.md](scripts/README.md) for a detailed description.

```
(repository-root)/tests/axivion
│   .axivion.preinc
│   .clang-format
│   .gitignore
│   architecture.gxl
│   architecture_config.json
│   architecture_hierarchy_belongs_to_layout.gvl
│   axivion_config.json
│   ci_config.json
│   compiler_config.json
│   mapping.gxl
│   README.md
│   rule_config_c.json
│   rule_config_c.py
│   rule_config_names.json
│   rule_config_names.py
│
├───forbidden-violations
│       check_forbidden_violations.py
│       forbidden-violations.txt
│
└───scripts
        ci_check_freshness_of_architecture_svg.py
        dep_analysis.py
        gravis_export_architecture_svg.py
        start_analysis.bat
        start_local_analysis.bat
        start_local_analysis.sh
        start_local_dashserver.bat
        wrapper_make_race_pdfs.bat
```

## MISRA C:2012 Hints

| Strictness | Meaning                     | Deviations                                     | foxBMS 2 specifics              |
|------------|-----------------------------|------------------------------------------------|---------------------------------|
| mandatory  | Rule **SHALL** be followed  | No deviations.                                 | -                               |
| required   | Rule **SHOULD** be followed | Deviations need to be justified and documented | -                               |
| advisory   | Rule **SHOULD** be followed | Deviations need to be documented               | Rule also needs to be justified |
