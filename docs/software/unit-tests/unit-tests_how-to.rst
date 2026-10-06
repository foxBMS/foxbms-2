.. include:: ./../../macros.txt
.. include:: ./../../units.txt

..
   cspell:ignore Ootpa

.. _HOW_TO_USE_UNIT_TESTS:

How to use Unit Tests
=====================

Verify that the unit testing framework is working as expected:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 waf build_app_unit_test_gcc

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh waf build_app_unit_test_gcc

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh waf build_app_unit_test_gcc

Typical usage and more information on the unit tests can be found in
:ref:`Unit tests <UNIT_TESTS>`.

Guidelines for the Unit Test Skeleton
-------------------------------------

.. note::

   This example

In this example a driver that resides in ``src/app/driver/<driver>/<driver>.c`` and
``src/app/driver/<driver>/<driver>.h`` is added and therefore the accompanying unit tests
need to be added also.
The module ``abc.c`` implements the public function
``extern uint8_t ABC_DoThis(void)`` and the static function
``static uint8_t ABC_DoSomethingElse(uint8_t someArgument)``.
Therefore, there are now two functions that need to be united tested.

At first the accompanying unit test file needs to be created in
``tests/unit/app/driver/abc/test_abc.c`` (notice the prefix ``test``) based on
the template in ``conf/tpl/test_c.c``.

Public/Extern Function Tests
----------------------------

#. Add a function ``uint8_t testABC_DoThis()`` in the appropriate section in the
   test file ``tests/unit/app/driver/abc/test_abc.c``.
   This function implements the tests for ``ABC_DoThis()``.
   The prefix ``test`` (**no** trailing underscore) is required for |cmock| to
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
   in the header of the driver (``src/app/driver/<driver>/<driver>.h``).
#. The definition of the wrapper needs to be placed in the appropriate section
   in the source of the driver (``src/app/driver/<driver>/<driver>.c``).
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
   The prefix ``test`` (**no** trailing underscore) is required for |cmock| to
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
   :caption: Header of the ``abc``-driver (``src/app/driver/<driver>/<driver>.h``)

The wrapper function ``TEST_ABC_DoSomethingElse`` needs to be put inside the
``UNITY_UNIT_TEST`` guard, so that it is not build during target builds.

.. literalinclude:: ./abc.c
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Implementation of the ``abc``-driver (``src/app/driver/<driver>/<driver>.c``)

.. literalinclude:: ./test_abc.c
   :language: C
   :linenos:
   :start-after: start-include-in-doc
   :end-before: stop-include-in-doc
   :caption: Implementation of the ``abc``-driver test
             (``tests/unit/app/driver/abc/test_abc.c``)

.. _linux_specific_usage:

Linux specific Usage
====================

The unit test suite is developed on Windows and there works out of the box on
all setup that install the dependencies as they are specified in
:ref:`SOFTWARE_INSTALLATION`.
However, it is still possible to get the unit test suite working on Linux.
Internally it is tested with the following setup:

- ``cat /etc/redhat-release``: AlmaLinux release 10.2 (Lavender Lion)
- ``uname -mrs``: Linux 6.12.0-211.7.3.el10_2.x86_64 x86_64
- ``gcc --version``: gcc (GCC) 14.3.1 20251022 (Red Hat 14.3.1-4)
- ``ruby --version``: ruby 3.1.2p20 (2022-04-12 revision 4491bb740a)
  [x86_64-linux]


Unit Test Macros
================

- ``UNITY_UNIT_TEST``: shall be used to exclude code that is not needed for the
  target build, but required for unit testing (e.g., making a static function
  testable).
  It shall not be used to change functional behavior when compiling (some rare
  exceptions to this rule can be found in the code, but it is generally not
  preferred to do so, although sometimes it is needed.)
- ``COMPILE_FOR_UNIT_TEST``: shall be used when code needs to be compiled
  differently in oder to make it unit testable on the host.
  Do **not** use ``UNITY_UNIT_TEST`` to achieve the same (see above).
