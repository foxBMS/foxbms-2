# Unit Tests

## "Standard" Unit Test Build

The *standard* way of building and running the unit tests is to run
``waf build_unit_test`` in the root of the repository.

This runs the unit test suite using Ceedling.

The overview of the results of the unit tests (if they are successfully run) is
then found at
``<repository-root>/build/unit_test/artifacts/gcov/GcovCoverageResults.html``

## Axivion Analysis of the Unit Test Implementation

The unit test implementation, i.e., the files in this directory
(``<repository-root>/tests/unit``), also needs to be analyzed by Axivion as it
is done for the actual software (``<repository-root>/src``).

### Workflow

To achieve this, this directory holds a configuration to build the unit tests
directly, without using Ceedling, but instead using ``gcc`` and ``cafeCC``
directly.
**However, it is required to first run the Ceedling based unit test, as this
creates the mocks and runners for the test files.**

The workflow is therefore as follows (assuming the user name is ``vulpes`` and
the repository is cloned into ``C:\Users\vulpes\Documents\foxbms-2``):

```cmd
C:\Users\vulpes>cd "C:\Users\vulpes\Documents\foxbms-2"
C:\Users\vulpes\Documents\foxbms-2>waf build_unit_test
@REM the mocks and unit tests are now created and the test suite runs
C:\Users\vulpes\Documents\foxbms-2>cd "tests\unit"
@REM configure the project
C:\Users\vulpes\Documents\foxbms-2\tests\unit>run_ut_gcc_build.bat configure
@REM use 'build_host' to create a host build of the unit tests; this is the
@REM same as ceedling does; this just exists to ensure, that the test suite
@REM builds and works as expected
C:\Users\vulpes\Documents\foxbms-2\tests\unit>run_ut_gcc_build.bat build_host
@REM Run an Axivion build (argument: 'build_axivion'), to pass the results to
@REM Axivion Suite
C:\Users\vulpes\Documents\foxbms-2\tests\unit>run_ut_gcc_build.bat build_axivion
```

The analysis of the test source is configured in
``<repository-root>/tests/unit/axivion``.

### Adding an Additional Test to the Analysis

- Analyze what ceedling does, when it creates a test suite for a specific test,
  e.g., ``src/app/driver/crc/crc.c`` by running:
  ```cmd
  C:\Users\vulpes\Documents\foxbms-2>cd "build\unit_test"
  C:\Users\vulpes\Documents\foxbms-2>..\..\conf\unit\ceedling.cmd test:test_crc.c
  ```
- Ceedling will then tell what it actually does:
  ```
  Test 'test_crc.c'
  -----------------
  Generating include list for fassert.h...
  Creating mock for fassert...
  Generating runner for test_crc.c...
  Compiling test_crc_runner.c...
  Compiling test_crc.c...
  Compiling Mockfassert.c...
  Compiling crc.c...
  Linking test_crc.out...
  Running test_crc.out...
  ```
- It can be seen, that the test of the ``crc`` module requires generating some
  mocks and compiling these and other files, linking all that together and
  finally running the created test binary.
- In order to add this Ceedling-based test to this build, the following needs
  to be done in ``build.test.json``:
  - a new list item must be created with this basic setup:
    ```json
    {
        "uut": {
            "source": "",
            "includes": [],
            "framework-includes": []
        },
        "test": {
            "includes": [],
            "framework-includes": []
        },
        "runner": {
            "includes": [],
            "framework-includes": [],
            "use": []
        }
    }
    ```

  - In this item the source of the new *unit-under-test* (key: ``uut``, sub key
    ``source``) must be added, e.g., for this exampling it then would be

    ``"source" : "src/app/driver/crc/crc.c"``.

  - Trying to compile this unit-under-test (``run_ut_gcc_build.bat -v -j 1``)
    will fail, as most likely includes are missing.
  - Therefore, the includes now need to added. The includes from the source
    tree (key: ``includes``) are added by the using the path relative to the
    repository root and the Ceedling and mock includes (key:
    ``framework-includes``) are added by using magic names.
    These magic names, and their respective values they are replaced with, are:
    - ``cexception`` for ``tools/vendor/ceedling/vendor/c_exception/lib``,
    - ``mocks`` for ``build/unit_test/test/mocks``,
    - ``cmock`` for ``tools/vendor/ceedling/vendor/cmock/src`` and
    - ``unity`` for ``tools/vendor/ceedling/vendor/unity/src``.

    Add ``includes`` and ``framework-includes`` as required, i.e., the
    unit-under-test needs to compile without any error.
    For this example, the ``uut`` key looks then like this:
    ```json
    "uut": {
        "source": "src/app/driver/crc/crc.c",
        "includes": [
            "src/app/main/include"
        ],
        "framework-includes": [
            "cexception"
        ]
    }
    ```
  - At next, the ``test`` key needs to defined.
    The name of the test file is derived from the unit-under-test, i.e., in
    example it is ``tests/unit/app/driver/crc/test_crc.c``.
    The build system therefore automatically derives the name of the test file
    and only the ``includes`` and ``framework`` includes needed to be added.
    This works the same as way as described above.
    For this example, the ``test`` key looks then like this:
    ```json
    "test": {
        "includes": [
            "src/app/driver/crc",
            "src/app/main/include",
            "src/os/freertos/include",
            "src/os/freertos/include",
            "src/os/freertos/portable/ccs/arm_cortex-r5"
        ],
        "framework-includes": [
            "cexception",
            "mocks",
            "unity"
        ]
    }
    ```
  - As last step, the test runner needs to added using the key ``runner``.
    The name of the test runner source is also derived automatically and is
    therefore omitted in the configuration file.
    Adding ``includes`` and ``framework-includes`` again works the same way.
    As creating the test runner requires a linking step, it is needed to add a
    list of required objects in order to link the binary (key: ``use``).
    The ``use`` key again accepts the magic strings. These are for the Ceedling
    objects:
    - ``cexception`` built from
      ``tools/vendor/ceedling/vendor/c_exception/lib/CException.c``,
    - ``cmock`` built from
      ``tools/vendor/ceedling/vendor/cmock/src/cmock.c`` and
    - ``unity`` built from
      ``tools/vendor/ceedling/vendor/unity/src/unity.c``.

    Furthermore, all other objects from other build steps for this binary
    that are required for linking are also defined by magic strings.
    The magic string for a object is the base name of the file that is compiled
    in all lowercase and without file extension.
    Based on the output from the original ceedling build it is seen, that
    ``Mockfassert.c`` needs to be compiled for that test, which means the
    object is required for linking.
    To add this object the entry in the ``use`` list would be ``mockfassert``.
    The unit-under-test as well as the test and the test runner are
    automatically added as required objects and are therefore omitted.
    For this example, the ``runner`` key looks then like this:
    ```json
    "runner": {
        "includes": [
            "src/app/main/include",
            "src/os/freertos/include",
            "src/os/freertos/include",
            "src/os/freertos/portable/ccs/arm_cortex-r5"
        ],
        "framework-includes": [
            "cexception",
            "cmock",
            "mocks",
            "unity"
        ],
        "use": [
            "cexception",
            "cmock",
            "mockfassert",
            "unity"
        ]
    }
    ```
  - For some tests, more than just the unit-under-test, test file and test
    runner are needed to be compiled and linked into the final test binary.
    For these cases, the ``needs`` key can used.
    This key contains a list of the additional sources.
    For each entry in ``needs``, the ``source``, the ``includes`` and the
    ``framework-includes`` can be added as needed in the same way as described above.
    The objects that are created do not need to be added to the list in the
    ``use`` key of the runner, as they are automatically added by the build
    system.


### Adding a New Mock to Build

If a new mock is created, it needs to be added in ``build.mocks.json``.
There it is only required to provide the path of the mock relative to
``build/unit_test`` using the key ``source`` and the includes relative to the
repository root using the key ``includes``.


# Configuration

Most of the Axivion configuration is the same for the embedded source and the
unit tests (compare ``test/axivion/axivion_config.json`` and
``tests/unit/axivion/axivion_config.json``).
The main difference is that unit test functions need to start with ``test`` as
the ceedling requires this (and accordingly to our guidelines would functions
would need to start with ``TEST``.)
