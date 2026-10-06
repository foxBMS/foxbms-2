.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _UNIT_TESTS:

Unit Tests
==========

We are using |cmock|, |unity| and |c_exception| for unit testing integrated
into the |waf| build toolchain.

`Unit testing <https://en.wikipedia.org/wiki/Unit_testing>`_ tries to test
properties of the interfaces of the smallest units in your software.
(The linked Wikipedia article is a good read for getting into the topic.)
This test tries not to catch every detail of the implementation on the specific
MCU or the behavior of modules to each other.
For this reason the tests are compiled on a development machine instead of the
target.

.. note::

   All module, file and function names used in this document are generic
   placeholders and do not refer to any specific module of the code base.

Usage
-----

The tool is encapsulated in our Waf tool.
Additionally it is automatically executed on our CI-jobs.
The usage as a Waf tool is explained in
:ref:`How to Use Unit tests <HOW_TO_USE_UNIT_TESTS>`.
This part also explains how to use the tool directly in a shell.
The benefit is that the output in the shell is colored and single tests can be
executed.

Writing tests
-------------

A unit test should always cover exactly one file from our source code.
See the test-files in the ``tests/unit`` subdirectory for reference.

Headers of unit tests
^^^^^^^^^^^^^^^^^^^^^

Typically, the header of a unit-test-file consists of three parts.

.. code-block:: c

    /* part one: include the unity-header */
    #include "unity.h"

    /* part two: include Mocks */
    #include "Mockdependency_a.h"
    #include "Mockdependency_b.h"
    #include "Mockdependency_c.h"

    /* part three: include the code under test */
    #include "module.h"

With this information |cmock| will gather all necessary c-files to these
headers and compile a test-runner.

Using mocks
^^^^^^^^^^^

Most of the time the code that we want to test will rely on other parts of the
codebase.
A proper unit test should however only test single units and not more.
(This would otherwise be a integration- or high-level-test.)

In order to isolate our code from the rest, we can replace the calls to the
surrounding software with so-called
`Mocks <https://en.wikipedia.org/wiki/Mock_object>`_.

You can tell |cmock| to create a mock of a file by including the header file
and prepending the word ``Mock`` to it.
The mocks will be created on demand.
If you are running into issues with certain parts of the API not being
available in your Mock, then check the generated header file, if they are
present.
|cmock| will have issues parsing the header if the functions are marked with
special attributes or if they are defined as inline.
In this case it might be necessary to tell |cmock| that those attributes are
"strippable" with the ``:strippables:`` yaml in ``project.yml``.

Before calling a function in your test that is calling one of your mocked
functions internally, you must tell the test-runner that you are expecting the
mock to be called.
The generated mocks will have a ``*_Expect``-function for this.

Using assertions
^^^^^^^^^^^^^^^^

The main part of a test will be asserting that the output of a function is as
expected.
|unity| supplies many assertions that are summed up pretty well on
`GitHub <https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md>`_.

A typical usage might look like this:

.. code-block:: c

    /* the function to be tested is sum(a, b) */
    TEST_ASSERT_EQUAL(5, sum(2, 3));

Getting started with a new test
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As a start for a unit test you can create an empty file that compiles a
test-runner together with all needed mocks and the file under test.

You can start this by creating the test-c-file.
If the file that you want to test is called ``module.c`` with header
``module.h`` then you would name the unit test ``test_module.c``.
The directory structure of our application is mirrored in the
unit-test-directory.

Start with the basic includes:

.. code-block:: c

    #include "unity.h"
    #include "module.h"

When you compile with this, the test-runner will complain about missing
symbols.
Read the output closely and find where the called functions are defined.
Then add mocks for those by including their headers:

.. code-block:: c

    #include "unity.h"
    #include "Mockdependency_a.h"
    #include "Mockdependency_b.h"
    #include "Mockdependency_c.h"

    #include "module.h"

When you are done with that there will sometimes still be missing variables.
In this example definitions from a configuration file ``module_cfg.c`` are
missing.
You have two options:
add the variables directly to the test-file (this can be useful if you have to
access them in the unit test) or adding the header that introduces those
variables:

.. code-block:: c

    #include "unity.h"
    #include "Mockdependency_a.h"
    #include "Mockdependency_b.h"
    #include "Mockdependency_c.h"

    #include "module_cfg.h"

    #include "module.h"

You can start with this variant in order to get the unit test to compile and
then later on add the variables directly to your test and remove the added
header.

Special care has to be taken when registers are directly accessed in code.
This will most certainly lead to a SEGFAULT in the test-runner, as this access
to some random address is not allowed or sensible on the native-platform.
The solution is to replace these register accesses with variables, but since we
are using a HAL it can often be enough to just replace HAL-calls with mocks.

To finish it, your test needs a startup and teardown function.
These functions will be called between each single test in a file.

Do not be too explicit
^^^^^^^^^^^^^^^^^^^^^^

Take care to make your tests not too explicit.
That means that you should not care to much about internals of the functions
that you are testing. Focus on the public interfaces of the functions.

One option is to ignore the calls to irrelevant mock functions.
You can add a call of ``*_Ignore()``-mocks at the start of your test.

If necessary, tests can be supported with helper functions in the file under
test.
For example it is possible to add a getter in order to obtain the internal
instance of the struct that tracks the state of a state-machine of a module.
This should however be kept to a minimum. If a function is not testable at all,
it is better to rewrite the function than to try to add complicated tests for
it.

Checklist for unit tests
^^^^^^^^^^^^^^^^^^^^^^^^

This section aims to be a help for writing complete unit tests.
It is an adaption of the checklist described in :cite:`Medoff2014` with
comments based on experience.

#. Unit tests **MUST** show that the module under test performs its intended
   action. For example if a module transforms values, then a test should show
   this mechanism in action.
#. Unit tests **SHOULD** show that the module under test does not perform any
   unintended side effects.
   This is not a strict requirement, as it is hard to prove.
   Generally, side effects should be omitted whenever possible in
   order to make the code clearer and in the end more testable.
   During development and execution of the unit tests special care should be
   taken in order to spot potential side effects.
#. Equivalence classes **SHALL** be created for every input of a module.
   An equivalence class describes which class of input values takes the same
   internal path inside the function.
   For example, when writing a function that validates a value against a valid
   range, all values inside the range would be one group and all values outside
   the range would be another group.
   All values that are loaded by the module or are passed to the module when
   calling it are considered input.
#. Boundary values **MUST** be tested for every equivalence class.
   By testing the values at the limits of an equivalence class, it is made sure
   that the function is able to handle the outer limits.
   This is for many implementations a good heuristic.
#. The value Zero **SHALL** be tested as input value.
   This applies to both variables and pointers.
   Zero value is a good indicator for unveiling critical edge-cases such as a
   division by zero or access to a null-pointer.
#. Outputs **SHALL** be forced to their limits.
   By writing a test that forces the output values to their maximum values, it
   can be made sure that the function is able to handle them.
#. Outputs **SHOULD** be forced above their limits.
   Depending on the module and its implementation, it can be impossible to
   force the output values of the module beyond its limits.
   Otherwise, this should be tested.
#. The first and last element of a sequence **SHALL** be tested.
   Especially for arrays, the first and last element shall be checked.
   This can help to unveil off-by-one and out-of-bounds-access to arrays.
#. Sequences **SHALL** be tested with zero, one, two and maximum elements.
   If the module under test consumes a sequence such as an array, the cases
   where the array has zero, one, two and the maximum number of elements shall
   be tested.
#. The test **SHOULD** reach full branch and line-coverage.
   This is only a requirement for safety critical modules.
   However, it is helpful nevertheless for writing better code by avoiding
   untestable conditions.

Typical test failures
---------------------

This part tries to sum up typical causes for failing unit tests.

SEGFAULT of the test-runner
^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the test-runner aborts an |unity| provides not much output this most of the
time means that you have run into a segmentation fault.
This happens when the code that is currently tested tries to access a register
of the MCU directly.
In a native build this register address lies outside of the program space and
will thus be terminated by your operating system.

Linker issues and unknown symbols
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In most other cases it will be a missing symbol during compilation or linking.
Read the output closely and try to mock or add any symbols that are missing.

Using coverage reports
----------------------

When you build the unit tests with coverage enabled you will find a coverage
report locally in ``build/app_unit_test_gcc/coverage`` as HTML.

This report knows two metrics: line-coverage and branch-coverage.
Line-coverage is the percentage of lines of code that have been passed while
running the unit test.
Branch-coverage is the percentage of decisions (e.g., when passing an
if-instruction) that have been covered by the test-code.

The goal is to find uncovered lines and branches and to extend the unit tests
in order to reach finally 100% coverage.

Full example of a unit test
----------------------------

This section explores how a full unit test could look like.
As an example we have a generic module ``arithmetic.h``:

.. code-block:: c
    :linenos:
    :caption: Header for ``arithmetic.h``
    :name: arithmetic-h

    extern uint16_t addition(uint8_t a, uint8_t b);

This header is implemented in ``arithmetic.c``:

.. code-block:: c
    :linenos:
    :caption: Implementation of ``arithmetic.h``
    :name: arithmetic-c

    #include "arithmetic.h"

    uint16_t addition(uint8_t a, uint8_t b) {
        return a + b;
    }

In order to make sure that the function is implemented as expected, we can
test its properties with a unit test. Since this is an addition-function it
should be commutative.
That means ``a + b == b + a``. Another issue could be that the implementation
has an overflow when adding two full-scale ``uint8_t``.
The following unit-test-fixture has two tests that will be executed one after
another.

.. code-block:: c
    :linenos:
    :caption: Unit test for ``arithmetic.h``
    :name: test-arithmetic-c

    /* The test-library */
    #include "unity.h"

    /* The file under test */
    #include "arithmetic.h"

    void setUp(void) {
        /* this function is called before executing a test;
        we can use it in more complex setups in order to initialize
        the code under test into a defined state */
    }

    void tearDown(void) {
        /* this function is called after executing a test;
        we can use it for cleaning up if needed */
    }

    /* The title that we use should be as explicit as possible */
    void testAdditionIsCommutative(void) {
        /* In order to test whether this function is commutative we have
        different options. For a unit test it is OK to have hard-coded values. */

        /* we call the function addition with 2 and 3 and
        expect the output to be 5 */
        TEST_ASSERT_EQUAL(5u, addition(2u, 3u));

        /* the other way around calling the function with 3 and 2
        should also return 5 */
        TEST_ASSERT_EQUAL(5u, addition(3u, 2u));
    }

    void testAdditionOfTwoUint8Max(void) {
        /* The result of adding two UINT8_MAX should be 510 */
        TEST_ASSERT_EQUAL(510, addition(UINT8_MAX, UINT8_MAX));
    }

The described code is isolated from any other code.
In "real" implementations, however, we will often find code that depends on
other modules and functions.
Unit testing aims to test code-fragments in insulation.
Therefore it is often used with a mocking-framework which replaces dependencies
with stubs whose behavior can be controlled by test.

Let's assume that we want to test a function in another module that depends on
``arithmetic.h``.
This module checks whether the sum of two values matches an expected value.

.. code-block:: c
    :linenos:
    :caption: Header of ``checker.h``
    :name: checker-h

    /* return 0 if the sum does not match, 1 if it matches */
    extern uint8_t check_sum_matches(uint8_t value_1,
                                     uint8_t value_2,
                                     uint16_t expected_sum);

.. code-block:: c
    :linenos:
    :caption: Implementation of ``checker.h``
    :name: checker-c

    #include "checker.h"

    #include "arithmetic.h"

    uint8_t check_sum_matches(uint8_t value_1,
                              uint8_t value_2,
                              uint16_t expected_sum) {
        uint16_t calculated_sum = addition(value_1, value_2);
        return calculated_sum == expected_sum;
    }

In the unit test we replace ``addition`` with a mock.
That means we do not care if ``arithmetic.h`` is implemented correctly and
replace it with our own implementation.

.. code-block:: c
    :linenos:
    :caption: Unit test for ``checker.h``
    :name: test-checker-c

    #include "unity.h"

    /* CMock will recognize by this that we want to create a mock of
    arithmetic.h and analyze the file and create it */
    #include "Mockarithmetic.h"

    /* this is the file under test */
    #include "checker.h"

    void setUp(void) {
    }

    void tearDown(void) {
    }

    void testCheckSumMatchesWithMatchingValues(void) {
        /* we tell the mock to expect that it is called with 5 and 4 and that
        it should return 9; This will be added to a list and the mocks will
        be "consumed" from this list during execution */
        addition_ExpectAndReturn(5, 4, 9);

        /* assert that check_sum_matches returns true */
        TEST_ASSERT(check_sum_matches(5, 4, 9));
    }

    void testCheckSumMatchesWithMismatchingValues(void) {
        addition_ExpectAndReturn(5, 4, 9);

        /* assert that a mismatching expected sum will return false */
        TEST_ASSERT_FALSE(check_sum_matches(5, 4, 10));
    }

Both tests should pass.
The second test asserts that the function returns false if the calculated sum
does not match the expected value.
For both tests the mock is told what it will receive and what it should return.
Variants of these mock-functions exist that allow to ignore calls to this
function, or to add a callback for more complex return values.

Further reading
---------------

- `Introduction to CMock and mocking <http://www.throwtheswitch.org/cmock>`_
- `API of CMock <https://github.com/ThrowTheSwitch/CMock/blob/master/docs/CMock_Summary.md>`_
- `Assertion summary of Unity <https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md>`_
