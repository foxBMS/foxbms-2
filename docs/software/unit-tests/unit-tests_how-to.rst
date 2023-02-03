.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_USE_UNIT_TESTS:

How to use unit tests
=====================

Verify that the unit testing framework is work is working as expected:

.. code-block:: console

    waf build_unit_test
    waf build_unit_test --coverage

Typical usage and more information on the unit tests can be found in
:ref:`Unit tests <UNIT_TESTS>`.

Guidelines for the Unit Test Skeleton
=====================================

In this example a driver that resides in `src/app/driver/abc/abc.c` and
`src/app/driver/abc/abc.h` is added and therefore the accompanying unit tests
need to be added also.
The module `abc.c` implements the public function
`extern uint8_t ABC_DoThis(void)` and the static function
`static uint8_t ABC_DoSomethingElse(uint8_t someArgument)`.
Therefore, there are now two functions that need to be united tested.

At first the accompanying unit test file needs to be created in
`tests/unit/app/driver/abc/test_abc.c` (notice the prefix `test`) based on the
template in `conf/tpl/test_c.c`.
The helper script `tools/utils/generate_missing_test_files.py` can be used to
automatically create the file.

Public/Extern Function Tests
----------------------------

#. Add a function `uint8_t testABC_DoThis()` in the appropriate section in the
   test file `tests/unit/app/driver/abc/test_abc.c`.
   This function implements the tests for `ABC_DoThis()`.
   The prefix `test` (**no** trailing underscore) is required for ceedling to
   detect the function as a *test*-function.
#. Write the test code.

Static Function Tests
---------------------

#. Static functions are not seen by other translation units, therefore the
   static functions must be made callable from other modules by creating a
   wrapper.
   For this example the wrapper will be
   `extern uint8_t TEST_ABC_DoSomethingElse(uint8_t someArgument)` (notice
   the `TEST_` prefix followed by the name of the static function).
#. The declaration of the wrapper needs to be placed in the appropriate section
   in the header of the driver (`src/app/driver/abc/abc.h`).
#. The definition of the wrapper needs to be placed in the appropriate section
   in the source of the driver (`src/app/driver/abc/abc.c`).
   The only thing this wrapper needs to do, is to verbatim pass all arguments
   to the original function (i.e., the static function) and return its result.
   The definition of the wrapper looks therefore like this

   .. code-block:: c
       :linenos:

       extern uint8_t testABC_DoSomethingElse(uint8_t someArgument) {
           return ABC_DoSomethingElse(someArgument);
       }

#. Add a function `void testABC_DoSomethingElse(void)` in the appropriate
   section in the test file `tests/unit/app/driver/abc/test_abc.c`.
   This function implements the tests for `ABC_DoSomethingElse()`.
   The prefix `test` (**no** trailing underscore) is required for ceedling to
   detect the function as a *test*-function.
   Note: The `TEST_` prefix of the *externalization* wrapper is removed and not
   part of the test function name.
#. Write test code.

.. note::

   `void`-returning functions that are externalized shall use
   `return <FunctionCall>` nevertheless, as this makes the intention of the
   wrapper even more clear.

Result
======

.. literalinclude:: ./abc.h
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Header of the `abc`-driver (`src/app/driver/abc/abc.h`)

The wrapper function `TEST_ABC_DoSomethingElse` needs to be put inside the
`UNITY_UNIT_TEST` guard, so that it is not build during target builds.

.. literalinclude:: ./abc.c
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Implementation of the `abc`-driver (`src/app/driver/abc/abc.c`)

.. literalinclude:: ./test_abc.c
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Implementation of the `abc`-driver test
             (`tests/unit/app/driver/abc/test_abc.c`)

How to exclude files from unit tests
====================================

Normally, all files should be covered by a (at least empty) unit test.
If a certain file is not meant to be covered by unit tests, it has to be
excluded in several locations in order to suppress checking mechanisms in the
toolchain.

The configuration of ceedling is stored in a file called ``project.yml``.
In this file the files that will not receive any coverage must be added to
``:uncovered_ignore_list:``.
Otherwise, ceedling will report uncovered files.

Additionally, the main wscript contains a mechanism that checks that every
file has a corresponding test file in the proper location.
Untested files have to be added to ``excl`` in ``check_test_files(ctx)``.

Using ceedling directly
=======================

.. warning::
    Ceedling (the unit test runner that is used by this project) is wrapped by
    the waf-toolchain.
    The toolchain makes sure that the path handed to ceedling contains the
    necessary applications and files, so without the wrapper extensive
    knowledge of the project structure is helpful.

    That being said, it is normally not necessary to directly execute ceedling
    as the wrapper handles this.

The waf-script of this project executes always the complete unit test-suite.
This can take some time.
When developing or debugging a unit test, it might be beneficial to have finer
control over which test is executed.

For this and especially for debugging the unit test integration it is possible
to directly access ceedling.
It is therefore necessary for the shell from which ceedling is run that all
dependencies are available.
Especially, it is necessary that it is possible to find ceedling from the
current ``PATH`` of the shell in which this is attempted.
This will not be covered in this manual.

Once such a shell is available it also has to be made sure that all files that
are generated or adapted on-the-fly are available in the ``build``-directory.
This can be ensured by executing the waf-task ``build_unit_test`` just until
ceedling is run by it.
After this step (it does not matter whether the task completes or is imported
after it has generated the dependencies) the following example can be executed.

.. code-block:: console
    :emphasize-lines: 3, 7, 11

    # go to the build directory of ceedling, assuming the current directory is
    # the project root (IMPORTANT, otherwise it will not work correctly)
    cd build/unit_test/

    # interact directly with ceedling (refer to ceedling manual for details)
    # this line executes test_plausibility.c with coverage
    ceedling gcov:test_plausibility.c

    # should the current shell not be able to directly follow the ceedling.cmd
    # file, it might be necessary to access the vendored ceedling directly:
    ruby ../../tools/vendor/ceedling/bin/ceedling gcov:test_plausibility.c
