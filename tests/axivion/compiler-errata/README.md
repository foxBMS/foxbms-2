# Axivion Compiler Errata Tests

This directory and its subdirectories contain Compiler Errata tests for
different compilers.

> **NOTE:** ``perform_tests.exe`` must be on PATH in the correct version.

- The test files of the compiler errata tests are in the directory
  ``./compiler-errata/``.
  **The test files are not part of the public foxBMS 2 release.**
  - ``ti-cgt-arm_20.2.6.lts``:
    - [ARM Code Generation Tools v20.2.6.LTS](https://software-dl.ti.com/codegen/esd/cgt_public_sw/TMS470/20.2.6.LTS/README.html)
    - test script: ``run_axivion_compiler_errata_ti-cgt-arm_20.2.6.lts_tests.bat``
    - Bauhaus version: X.Y.Z
- To ensure that the test suite is not changed inadvertently, the SHA-256 of
  the directory is checked in CI:
  ```console
  <repo-root>tools\utils\cmd\run-python-script.bat tools\utils\verify_checksums.py tests\axivion\compiler-errata\ti-cgt-arm_20.2.6.lts <known-hash> -vv
  ```
- If the compiler errata tests are changed/updated the following needs to be done:
  - The changes need to be applied in ``./compiler-errata/``
  - The known SHA-256 of the test files needs to be updated in the CI step.
  - the commit message **MUST** contain ``Axivion``.
    This ensures that the compiler errata tests are run.
- When the Axivion Bauhaus Suite is updated, the compiler-errata tests need
  also to be updated.
  The pinned Bauhaus version is found in
  ``<repository-root>/conf/env/paths_win32.txt``.
