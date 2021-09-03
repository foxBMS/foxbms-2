# Axivion Bauhaus Suite Configuration for foxBMS 2

## Directory Description


- ``.axivion.preinc``, ``predefined_macros.txt`` and ``predefined_sizes.txt``
  describe compiler built-ins for the TI ARM CGT compiler for TMS570LC4357.
- ``architecture.gxl``, ``architecture.json`` and ``mapping.gxl`` describe the
  software architecture of foxBMS 2.
- ``axivion_config.json``, ``cafeCC.config``, ``ci_config.json``,
  ``rule_config_c.json``, ``rule_config_c.py`` provide project specific
  settings.
- ``start_analysis.bat``, ``start_local_analysis.bat``,
  ``start_local_analysis.sh``, ``start_local_dashserver.bat`` are helper
  scripts to run axivion. Files here might need to be updated to your local
  Axivion setup.
- ``wrapper_make_race_pdfs.bat`` wrapper script to simplify creating the
  report.

```
(repository-root)
└── tests
    └── axivion
        ├── .axivion.preinc
        ├── .clang-format
        ├── .gitignore
        ├── architecture.gxl
        ├── architecture.json
        ├── axivion_config.json
        ├── cafeCC.config
        ├── ci_config.json
        ├── mapping.gxl
        ├── predefined_macros.txt
        ├── predefined_sizes.txt
        ├── README.md
        ├── rule_config_c.json
        ├── rule_config_c.py
        ├── start_analysis.bat
        ├── start_local_analysis.bat
        ├── start_local_analysis.sh
        ├── start_local_dashserver.bat
        └── wrapper_make_race_pdfs.bat
```
