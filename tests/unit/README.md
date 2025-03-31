# Unit Tests

## "Standard" Unit Test Build

The *standard* way of building and running the unit tests is to run
``waf build_app_host_unit_test`` in the root of the repository.

This runs the unit test suite using ``Ceedling``.

The overview of the results of the unit tests (if they are successfully run) is
then found at
``<repository-root>/build/app_host_unit_test/artifacts/gcov/GcovCoverageResults.html``

## Axivion Analysis of the Unit Test Implementation

The unit test implementation, i.e., the files in this directory
(``<repository-root>/tests/unit``), also needs to be analyzed by Axivion as it
is done for the actual software (``<repository-root>/src``).

### Workflow

To achieve this, this directory holds a configuration to build the unit tests
directly, without using ``Ceedling``, but instead using ``gcc`` and ``cafeCC``
directly.
**However, it is required to first run the ``Ceedling`` based unit test, as this
creates the mocks and runners for the test files.**

The workflow is therefore as follows (assuming the username is ``vulpes`` and
the repository is cloned into ``C:\Users\vulpes\Documents\foxbms-2``) and
the cwd is also ``C:\Users\vulpes\Documents\foxbms-2``.

```pwsh
# build the unit tests (as mocks etc. are required)
.\fox.ps1 waf build_app_host_unit_test
# set the environment variable 'BAUHAUS_CONFIG'
$env:BAUHAUS_CONFIG="C:\Users\vulpes\Documents\foxbms-2\tests\unit\axivion"
# configure the project
.\fox.ps1 waf --cwd tests\unit configure
# use 'build_host' to create a host build of the unit tests; this is the
# same as ceedling does; this just exists to ensure, that the test suite
# builds and works as expected
.\fox.ps1 waf --cwd tests\unit build_host
# Run an Axivion build (argument: 'build_axivion'), to pass the results to
# Axivion Suite
.\fox.ps1 waf --cwd tests\unit build_axivion
```

The analysis of the test source is configured in
``<repository-root>/tests/unit/axivion``.

### Adding an Additional Test to the Analysis

- Analyze what ceedling does, when it creates a test suite for a specific test,
  e.g., ``src/app/driver/crc/crc.c`` by running:

  ```pwsh
  .\fox.ps1 ceedling test:test_crc.c
  ```

- ``Ceedling`` will then tell what it actually does:

  ```cmd
  .\fox.ps1 ceedling test:test_crc.c

  Loaded project configuration at default location using ./project.yml

  Ceedling set up completed in 17.22 seconds

  Preparing Build Paths...

  Extracting Build Directive Macros
  ---------------------------------
  Parsing test_crc.c...

  Ingesting Test Configurations
  -----------------------------
  Collecting search paths, flags, and defines for test_crc.c...

  Collecting Testing Context
  --------------------------
  Extracting #include statements via preprocessor from test_crc.c...
  Processing #include statements for test_crc.c...

  Determining Files to be Generated...

  Preprocessing for Mocks
  -----------------------
  Preprocessing test_crc::fassert.h...
  Extracting #include statements via preprocessor from test_crc::fassert.h...

  Mocking
  -------
  Generating mock for test_crc::fassert.h...

  Preprocessing for Test Runners
  ------------------------------
  Preprocessing test_crc.c...
  Loading #include statement listing file for test_crc.c...

  Test Runners
  ------------
  Generating runner for test_crc.c...

  Determining Artifacts to Be Built...

  Building Objects
  ----------------
  Compiling test_crc.c...
  Compiling test_crc::crc.c...
  Compiling test_crc::unity.c...
  Compiling test_crc::cmock.c...
  Compiling test_crc::CException.c...
  Compiling test_crc::test_crc_runner.c...
  Compiling test_crc::Mockfassert.c...

  Building Test Executables
  -------------------------
  Linking test_crc.out...

  Executing
  ---------
  Running test_crc.out...

  Running Raw Tests Output Report
  -------------------------------
  Tests produced no extra console output.

  --------------------
  OVERALL TEST SUMMARY
  --------------------
  TESTED:  1
  PASSED:  1
  FAILED:  0
  IGNORED: 0

  Running Test Suite Reports
  --------------------------
  Generating artifact ./artifacts/test/junit_tests_report.xml...
  Generating artifact ./artifacts/test/cppunit_tests_report.xml...

  Ceedling operations completed in 5.25 seconds
  ```

- It can be seen, that the test of the ``crc`` module requires generating some
  mocks and compiling these and other files, linking all that together and
  finally running the created test binary.
- In order to add this ``Ceedling``-based test to this build, the following
  needs to be done in ``build.json``.
  The list of ``includes`` can be derived from the test file itself (see the
  ``TEST_INCLUDE_PATH`` macro) and the paths configured in
  ``conf/unit/app_project_win32.yml`` or ``conf/unit/app_project_posix.yml``.
  The list of ``sources`` from the list of included mocks, plus the unit under
  test itself plus all files that are added via ``TEST_SOURCE_FILE`` in the
  test file.
  Specific ``defines`` can be added per test.
  For the file ``src/app/driver/crc/crc.c`` the setup looks then like this:

  ```json
  "src/app/driver/crc/crc.c": [
      {
          "include": [
              "build/app_host_unit_test/include",
              "build/app_host_unit_test/test/mocks/test_crc",
              "src/app/driver/crc",
              "src/app/application/config",
              "src/app/driver/mcu",
              "src/app/engine/config",
              "src/app/engine/database",
              "src/app/main/include",
              "src/app/task/os",
              "src/os/freertos/include",
              "src/os/freertos/portable/ccs/arm_cortex-r5"
          ],
          "sources": [
              "build/app_host_unit_test/test/mocks/test_crc/Mockfassert.c",
              "src/app/driver/crc/crc.c",
              "tests/unit/app/driver/crc/test_crc.c",
              "build/app_host_unit_test/test/runners/test_crc_runner.c"
          ]
      }
  ]
  ```

  Note: This is a list of tests, i.e., there can be multiple tests per
  implementation file, see for example for ``src/app/driver/can/can.c``:

  - ``tests/unit/app/driver/can/test_can.c``
  - ``tests/unit/app/driver/can/test_can_1.c``
  - ``tests/unit/app/driver/can/test_can_2.c``

## Configuration

Most of the Axivion configuration is the same for the embedded source and the
unit tests (compare ``test/axivion/axivion_config.json`` and
``tests/unit/axivion/axivion_config.json``).
The main difference is that unit test functions need to start with ``test`` as
the ceedling requires this (and accordingly to our guidelines would functions
would need to start with ``TEST``).
