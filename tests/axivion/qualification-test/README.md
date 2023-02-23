# Axivion Tool Qualification Kit

This directory and its subdirectories contain the Axivion Tool Qualification
Kit (TQK).

> **NOTE:** ``perform_tests.exe`` must be on PATH in the correct version.

- The test files of the TQK are in the directory ``./qualification-kit/``.
  **The test files are not part of the public foxBMS 2 release.**
    - test script: ``run_axivion_qualification_kit_tests.bat``
- To ensure that the test suite is not changed inadvertently, the SHA-256 of
  the directory is checked in CI:
  ```console
  <repo-root>\tools\utils\cmd\run-python-script.bat tools\utils\verify_checksums.py tests\axivion\qualification-test\qualification-kit <known-hash> -vv
  ```
- If the TQK is changed/updated the following needs to be done:
  - The changes need to be applied in ``./qualification-kit/``
  - The known SHA-256 of the test files needs to be updated in the CI step.
  - the commit message **MUST** contain ``Axivion``.
    This ensures that the TQK is run.
- When the Axivion Bauhaus Suite is updated, the TQK needs also to be updated.
  The pinned Bauhaus version is found in
  ``<repository-root>/conf/env/paths_win32.txt``.
