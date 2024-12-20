# Helper Scripts to Interact with Axivion Bauhaus Suite

## Directory Description

These are helper scripts for the interaction with the Axivion Bauhaus Suite
in the foxBMS 2 project
(The Axivion configuration is described in this [README.md](../README.md)).

Files here might need to be updated to the local Axivion setup.

- ``gravis_export_architecture_svg.py`` is a startup script for ``gravis`` to
  create a SVG of the architecture.
  The script needs to be invoked by as follows:

  ```pwsh
  .\fox.ps1 run-program gravis --script tests\axivion\scripts\gravis_export_architecture_svg.py
  ```

- ``ci_check_freshness_of_architecture_svg.py`` is used to check
  whether the git timestamps of the architecture documentation and the rendered
  SVG are aligned, i.e., one can not change the architecture without updating
  the architecture image in the documentation.
  The script needs to be invoked by as follows:

  ```pwsh
  .\fox.ps1 run-script tests\axivion\scripts\ci_check_freshness_of_architecture_svg.py
  ```

- ``scripts/wrapper_make_race_pdfs.bat`` wrapper script to simplify creating
  the race condition report.

## Local Build

1. Run once ``.\fox.ps1 axivion local-dashserver`` to start a local dashserver.
1. Then, run ``.\fox.ps1 axivion local-analysis`` to run a local analysis
   build.
