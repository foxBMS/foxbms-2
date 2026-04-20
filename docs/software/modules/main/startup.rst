.. include:: ../../../macros.txt
.. include:: ../../../units.txt

.. _EMBEDDED_SOFTWARE_STARTUP:

Embedded Software Startup
=========================

Module Files
------------

Driver
^^^^^^

- ``src/app/main/fstartup.c``
- ``src/app/main/include/fstartup.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/main/test_fstartup.c``

Description
-----------

The startup code begins in the function ``_c_int00()`` in
``app/main/fstartup.c``.
After initialization of the main microcontroller registers, memory, system
clock and interrupts, the C function ``main()`` is called.
In ``general/main.c``, interrupts are enabled and the initializations of the
microcontroller unit, peripherals and software modules are done (e.g.,
hardware modules like SPI and DMA).
The OS is then started.
The steps are indicated by the global variable ``os_boot``.
At the end of the main function, the operating system resources (tasks, queues)
are configured in ``OS_InitializeOperatingSystem()`` (``src/app/task/os/os.c``)
and the scheduler is started.
All configured tasks (|freertos| threads) are then started depending on
their priority.
The successful activation of the tasks is indicated by
``os_boot = OS_RUNNING``.

The OS-scheduler first calls the highest priority task.
All other cyclic tasks are blocked in a while-loop until the initialization of
this task finishes.
At the beginning of the task, ``FTSK_InitializeUserCodeEngine()`` is called.
In this function, the database is initialized.
Once finished, this is indicated by ``os_boot = OS_ENGINE_RUNNING``.
The function ``FTSK_RunUserCodeEngine()`` is then called, where the diagnostic
module and the database are managed.

Once ``os_boot = OS_ENGINE_RUNNING``, the |1ms-task| is unblocked.
The function ``FTSK_InitializeUserCodePreCyclicTasks()`` is called once first.
This function is called when the OS and the database are running but before
the cyclic tasks run.
Once ``FTSK_InitializeUserCodePreCyclicTasks()`` has finished, all cyclic tasks
are unblocked from there while-loop and run periodically.
