.. include:: ../../../macros.txt
.. include:: ../../../units.txt

.. _APPLICATION_MAIN:

Main
====

Module Files
------------

Driver
^^^^^^

- ``src/app/main/main.c``
- ``src/app/main/include/main.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/main/test_main.c``

Description
-----------

The main module provides the application entry point and performs the
platform startup sequence before handing over control to the operating system
scheduler.

At startup the following steps are performed:

- reads and stores the reset source,
- initializes hardware drivers and low-level peripherals (e.g., SPI, GPIO)
- initializes diagnostics and executes startup self-tests,
- initializes the operating system objects (queues, mutexes, events and tasks),
- enables IRQs after OS/task setup to avoid early interrupt handling on
  not-yet-created task handles,
- checks OS boot state and traps on initialization failures,
- stores the scheduler start tick and starts the scheduler.

Once the scheduler is started, normal operation continues in the configured
tasks.
Returning from ``main`` is an error.

For unit tests, the entry function is exposed as ``unit_test_main()`` instead
of ``main()`` so the startup sequence can be called from the test harness
without conflicting with the host test runner entry point.
