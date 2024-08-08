.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _OS_MODULE:

OS Module
=========

Module Files
------------

Driver
^^^^^^

- ``src/app/task/os/os.c``
- ``src/app/task/os/os.h``

|freertos|
""""""""""

- ``src/app/task/os/freertos/os_freertos.c``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/task/os/test_os.c``

Detailed Description
--------------------

The ``os`` module provides system relevant initializations and functions, such as e.g., the operating system timer.

At first the ``os`` module initializes all tasks form :ref:`ftask_task_configuration` by calling ``OS_InitializeOperatingSystem()``.
After that the scheduler takes over. The details of the task configuration are documented in the :ref:`FTASK_MODULE`.

The operating system timer incrementing function ``OS_TimerTrigger`` is implemented in the ``os`` module. The timer
is stored in the variable ``os_timer``.

Furthermore it enables the user configuration of the ``vApplicationIdleHook``
by calling ``FTSK_RunUserCodeIdle``.

Further Reading
---------------

Task configuration details are found in the :ref:`FTASK_MODULE` documentation.
