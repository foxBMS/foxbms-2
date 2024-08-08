# Axivion Compiler Errata Tests

This directory and its subdirectories contain Compiler Errata Tests (CET) for
different compilers.

> **NOTE:** `perform_tests.exe` must be on PATH in the correct version.

- This test can be run using the test script:
  `<repository-root>/tests/axivion/axivion_self_tests.py`
- The test files of the CET are in the directory
  `<repository-root>/tests/axivion/compiler-errata`.
  **These test files are not part of the public foxBMS 2 release.**
  - For ARM Code Generation Tools v20.2.6.LTS:
    - Tests: `<repository-root>/tests/axivion/compiler-errata/ti-cgt-arm_20.2.6.lts`:
    - Compiler errata documentation: [ARM Code Generation Tools v20.2.6.LTS](https://software-dl.ti.com/codegen/esd/cgt_public_sw/TMS470/20.2.6.LTS/README.html)
- To ensure that the test suite is not changed inadvertently, the SHA-256 of
  the directory is checked in CI:

  ```pwsh
  PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 run-script tools\utils\verify_checksums.py tests\axivion\compiler-errata\ti-cgt-arm_20.2.6.lts <known-hash> -vv
  ```

- If the CET are changed/updated the following needs to be done:
  - The changes need to be applied in
    `<repository-root>/tests/axivion/compiler-errata/`.
  - The known SHA-256 of the test files needs to be updated in the CI step.
- When the Axivion Bauhaus Suite is updated, the CET also needs to be updated.
  The pinned Bauhaus version is found in
  `<repository-root>/conf/env/paths_win32.txt`.
