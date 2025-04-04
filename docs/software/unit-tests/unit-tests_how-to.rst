.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_USE_UNIT_TESTS:

How to use Unit Tests
=====================

Verify that the unit testing framework is working as expected:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 waf build_app_host_unit_test
         .\fox.ps1 waf build_app_host_unit_test --coverage

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh waf build_app_host_unit_test
         ./fox.sh waf build_app_host_unit_test --coverage

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh waf build_app_host_unit_test
         ./fox.sh waf build_app_host_unit_test --coverage

Typical usage and more information on the unit tests can be found in
:ref:`Unit tests <UNIT_TESTS>`.

Default Includes
----------------

The default include paths are defined in `app_project_win32.yml` and
`app_project_posix.yml` in the key ``:paths:`` |rarr| ``:include:``.

When a tests requires specific include-paths to be added the macro
``TEST_INCLUDE_PATH("../../path/to/directory")`` can be used.
The include needs to start with ``../../`` and then specifies the path to the
directory as seen from the repository root, e.g., to include the directory
``src/app/driver/config`` use
``TEST_INCLUDE_PATH("../../src/app/driver/config")``.
The path must be provided in POSIX form (i.e., forward slashes).

Additional Source files
-----------------------

If a test requires additional files to be compiled that cannot be derived from
the list of included files, then the macro ``TEST_SOURCE_FILE("file-name.c")``
can be used.

Guidelines for the Unit Test Skeleton
-------------------------------------

.. note::

   This example

In this example a driver that resides in ``src/app/driver/abc/abc.c`` and
``src/app/driver/abc/abc.h`` is added and therefore the accompanying unit tests
need to be added also.
The module ``abc.c`` implements the public function
``extern uint8_t ABC_DoThis(void)`` and the static function
``static uint8_t ABC_DoSomethingElse(uint8_t someArgument)``.
Therefore, there are now two functions that need to be united tested.

At first the accompanying unit test file needs to be created in
``tests/unit/app/driver/abc/test_abc.c`` (notice the prefix ``test``) based on
the template in ``conf/tpl/test_c.c``.
The helper script ``tools/utils/generate_missing_test_files.py`` can be used to
automatically create the file.

Public/Extern Function Tests
----------------------------

#. Add a function ``uint8_t testABC_DoThis()`` in the appropriate section in the
   test file ``tests/unit/app/driver/abc/test_abc.c``.
   This function implements the tests for ``ABC_DoThis()``.
   The prefix ``test`` (**no** trailing underscore) is required for ceedling to
   detect the function as a *test*-function.
#. Write the test code.

Static Function Tests
---------------------

#. Static functions are not seen by other translation units, therefore the
   static functions must be made callable from other modules by creating a
   wrapper.
   For this example the wrapper will be
   ``extern uint8_t TEST_ABC_DoSomethingElse(uint8_t someArgument)`` (notice
   the ``TEST_`` prefix followed by the name of the static function).
#. The declaration of the wrapper needs to be placed in the appropriate section
   in the header of the driver (``src/app/driver/abc/abc.h``).
#. The definition of the wrapper needs to be placed in the appropriate section
   in the source of the driver (``src/app/driver/abc/abc.c``).
   The only thing this wrapper needs to do, is to verbatim pass all arguments
   to the original function (i.e., the static function) and return its result.
   The definition of the wrapper looks therefore like this

   .. code-block:: c
       :linenos:

       extern uint8_t testABC_DoSomethingElse(uint8_t someArgument) {
           return ABC_DoSomethingElse(someArgument);
       }

#. Add a function ``void testABC_DoSomethingElse(void)`` in the appropriate
   section in the test file ``tests/unit/app/driver/abc/test_abc.c``.
   This function implements the tests for ``ABC_DoSomethingElse()``.
   The prefix ``test`` (**no** trailing underscore) is required for ceedling to
   detect the function as a *test*-function.
   Note: The ``TEST_`` prefix of the *externalization* wrapper is removed and not
   part of the test function name.
#. Write test code.

Result
------

.. literalinclude:: ./abc.h
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Header of the ``abc``-driver (``src/app/driver/abc/abc.h``)

The wrapper function ``TEST_ABC_DoSomethingElse`` needs to be put inside the
``UNITY_UNIT_TEST`` guard, so that it is not build during target builds.

.. literalinclude:: ./abc.c
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Implementation of the ``abc``-driver (``src/app/driver/abc/abc.c``)

.. literalinclude:: ./test_abc.c
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Implementation of the ``abc``-driver test
             (``tests/unit/app/driver/abc/test_abc.c``)

How to exclude files from unit tests
====================================

Normally, all files should be covered by a (at least empty) unit test.
If a certain file is not meant to be covered by unit tests, it has to be
excluded in several locations in order to suppress checking mechanisms in the
toolchain.

Additionally, the main wscript contains a mechanism that checks that every
file has a corresponding test file in the proper location.
Untested files have to be added to ``excl`` in ``check_test_files(ctx)``.

Using ceedling directly
=======================

.. warning::
    Ceedling (the unit test runner that is used by this project) is wrapped by
    the Waf toolchain.
    The toolchain makes sure that the path handed to ceedling contains the
    necessary applications and files, so without the wrapper extensive
    knowledge of the project structure is helpful.

    That being said, it is normally not necessary to directly execute ceedling
    as the wrapper handles this.

The wscript of this project executes always the complete unit test-suite.
This can take some time.
When developing or debugging a unit test, it might be beneficial to have finer
control over which test is executed.

For this and especially for debugging the unit test integration it is possible
to directly access ceedling as follows:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 ceedling gcov:test_plausibility.c

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh ceedling gcov:test_plausibility.c

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh ceedling gcov:test_plausibility.c

.. _linux_specific_usage:

Linux specific Usage
====================

The unit test suite is developed on Windows and there works out of the box on
all setup that install the dependencies as they are specified in
:ref:`SOFTWARE_INSTALLATION`.
However, it is still possible to get the unit test suite working on Linux.
Internally it is tested with the following setup:

- ``cat /etc/redhat-release``: Red Hat Enterprise Linux release 8.8 (Ootpa)
- ``uname -mrs``: Linux 4.18.0-425.19.2.el8_7.x86_64 x86_64
- ``gcc --version``: gcc (GCC) 8.5.0 20210514 (Red Hat 8.5.0-18)
- ``ruby --version``: ruby 3.1.2p20 (2022-04-12 revision 4491bb740a)
  [x86_64-linux]
