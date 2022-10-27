# Helper Scripts to Interact with Axivion Bauhaus Suite

## Directory Description

These are helper scripts for the interaction with the Axivion Bauhaus Suite
in the foxBMS 2 project
(The Axivion configuration is described in this [README.md](../README.md)).

- ``scripts/start_analysis.bat``, ``scripts/start_local_analysis.bat``,
``scripts/start_local_dashserver.bat`` are helper scripts to run Axivion.
Files here might need to be updated to the local Axivion setup.
- ``gravis_export_architecture_svg.py`` is a startup script for ``gravis`` to
  create a SVG of the architecture.
  The script needs to be invoked by as follows:
  ```
  gravis --script tests/axivion/scripts/gravis_export_architecture_svg.py
  ```
- ``scripts/ci_check_freshness_of_architecture_svg.py`` is used to check
  whether the git timestamps of the architecture documentation and the rendered
  SVG are aligned, i.e., one can not change the architecture without updating
  the architecture image in the documentation.
  The script needs to be invoked by as follows:
  ```
  tools/utils/cmd/run-python-script.bat tests/axivion/scripts ci_check_freshness_of_architecture_svg.py
  ```
- ``scripts/wrapper_make_race_pdfs.bat`` wrapper script to simplify creating
  the race condition report.

## Local Build

1. Run once ``tests/axivion/scripts/start_local_dashserver.bat`` to start a
   local dashserver.
1. Then, run ``tests/axivion/scripts/start_local_analysis.bat`` to run a local
   analysis build.
