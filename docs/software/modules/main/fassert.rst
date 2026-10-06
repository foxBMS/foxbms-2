.. include:: ../../../macros.txt
.. include:: ../../../units.txt

.. _ASSERTION_MAIN:

Assertion
=========

Module Files
------------

Driver
^^^^^^

- ``src/app/main/fassert.c``
- ``src/app/main/include/fassert.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/main/test_fassert.c``

Detailed Description
--------------------

The assertion module provides the project-wide assertion interface via
``FAS_ASSERT()``.
It is used to detect programming errors and invalid internal states that must
never occur in correct program execution.

Runtime behavior is configured through ``FAS_ASSERT_LEVEL``:

- ``FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS``:
  on assertion failure, store assert location information, disable interrupts
  and stay in an infinite loop.
  This reliably leads to a watchdog reset.
- ``FAS_ASSERT_LEVEL_INF_LOOP_FOR_DEBUG``:
  on assertion failure, store assert location information and stay in an
  infinite loop to support debugging.
- ``FAS_ASSERT_LEVEL_NO_OPERATION``:
  assertion checks are compiled to no-operation behavior.

The helper ``FAS_TRAP`` can be passed to ``FAS_ASSERT`` to intentionally force
an assertion failure.

When an assertion fails, ``FAS_ASSERT_RECORD()`` captures the current location
and forwards it to ``FAS_StoreAssertLocation()``.
The implementation stores program-counter and line information in a local
assert-location object to keep failure locations unique and visible during
analysis.

In unit-test builds (``UNITY_UNIT_TEST``), ``FAS_ASSERT`` does not enter an
infinite loop.
Instead, it throws a ``CException`` to allow tests to verify assertion
behavior.
