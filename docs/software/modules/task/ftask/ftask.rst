.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _FTASK_MODULE:

FTASK Module
============

Module Files
------------

Driver
^^^^^^

- ``src/app/task/ftask/ftask.c`` (`API <../../../../_static/doxygen/src/html/ftask_8c.html>`__, `source <../../../../_static/doxygen/src/html/ftask_8c_source.html>`__)
- ``src/app/task/ftask/ftask.h`` (`API <../../../../_static/doxygen/src/html/ftask_8h.html>`__, `source <../../../../_static/doxygen/src/html/ftask_8h_source.html>`__)

FreeRTOS
""""""""

- ``src/app/task/ftask/freertos/ftask_freertos.c`` (`API <../../../../_static/doxygen/src/html/ftask__freertos_8c.html>`__, `source <../../../../_static/doxygen/src/html/ftask__freertos_8c_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/task/config/ftask_cfg.c`` (`API <../../../../_static/doxygen/src/html/ftask__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/ftask__cfg_8c_source.html>`__)
- ``src/app/task/config/ftask_cfg.h`` (`API <../../../../_static/doxygen/src/html/ftask__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/ftask__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/task/config/test_ftask_cfg.c``
- ``tests/unit/app/task/ftask/test_ftask.c``

Detailed Description
--------------------

.. note::

    The module name, and therefore all related file names, are prefixed with an **f**. The rationale behind this naming
    convention is that the operating system already provides a file named ``task.h`` and with using the **f**
    prefix it is unambiguous what file is meant.

|foxbms| uses five tasks, which are all configured in the ``ftask`` module. :numref:`ftask-function-overview`) shows

- the name by which the task is commonly referred to,
- the task creator function that creates the actual task,
- the user code function that is called periodically and
- purpose of the function.

The Engine task is the task with the highest priority and should not be modified
(see :numref:`ftask_special_task_engine`).

The Idle task is not counted as a *real* task and is only mentioned for the sake of completeness and should not be
modified (see :numref:`ftask_special_task_idle`).

.. csv-table:: *common* task names, the function that creates the task and the function that is run cyclically
   :name: ftask-function-overview
   :widths: auto
   :header-rows: 1
   :delim: ;
   :file: ./ftask-function-overview.csv

The following sections show how tasks are created, configured and how task behavior can be changed. Code sections that
are not meant to be modified by user are indicated in the code, e.g., for the Engine task:


.. code-block:: c
   :linenos:
   :emphasize-lines: 2-3,6-7
   :caption: Example of code that should not be modified by the user
   :name: ftsk-user-code-engine-example

    void FTSK_RunUserCodeEngine(void) {
        /* Warning: Do not change the content of this function */
        /* See function definition doxygen comment for details */
        DATA_Task();               /* Call database manager */
        SYSM_CheckNotifications(); /* Check notifications from tasks */
        /* Warning: Do not change the content of this function */
        /* See function definition doxygen comment for details */
    }

.. _ftask_task_creation:

Task Creation
^^^^^^^^^^^^^

.. warning::

    Task Creator Functions generally do not need to be modified by the user.
    User/application specific code should be implemented in the User Code Functions.

``FTSK_CreateTasks`` creates these 4 tasks on startup. The procedure is the same for all tasks:

#. Try to create a static task (using the Task Creator Function)
#. Assert if this does not work.

The Task Creator Functions bind User Code Functions into the tasks. This is best explained using the example of the
Engine task:

- Line 3: Run an initializer function before the task starts.
- Line 5: Delay the phase as specified in the configuration.
- Line 6: The task should run forever.
- Line 10: Bind the User Code Function to the task.

Before and after the the User Code Function is run, the :ref:`SYSTEM_MONITORING_MODULE` is notified that either the
User Code Function will be run or has run. This enables to determine if a task returns within the expected time frame.


.. code-block:: c
   :linenos:
   :emphasize-lines: 3,6,7,11
   :caption: Example of Task Creator Function
   :name: ftsk-task-creator-function-example

    void FTSK_CreateTaskEngine(void) {
        os_boot = OS_SCHEDULER_RUNNING;
        FTSK_InitializeUserCodeEngine();
        os_boot = OS_ENGINE_RUNNING;

        OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionEngine.Phase);
        while (1) {
            /* notify system monitoring that task will be called */
            SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_ENTER, OS_GetTickCount());
            /* user code implementation */
            FTSK_RunUserCodeEngine();
            /* notify system monitoring that task has been called */
            SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        }
    }

The other cyclic tasks are basically generated the same way. The main difference is that there is not a separate
initialization function but one common for all other cyclic tasks. These other cyclic tasks do not start until the Task
Creator Function ``FTSK_CreateTaskEngine`` has set the boot state to ``os_boot = OS_ENGINE_RUNNING`` **and** the
common initialization function for these tasks has finished.

.. _ftask_task_configuration:

Task Configuration
^^^^^^^^^^^^^^^^^^

The tasks are configured in ``ftask_cfg.c`` regarding their startup phase, cycle time, priority and stack size.

.. _ftask_special_tasks:

Special User Tasks
^^^^^^^^^^^^^^^^^^

There are four special user functions, three of these should not be modified by
the user/application ( ``FTSK_InitializeUserCodeEngine`` and
``FTSK_RunUserCodeEngine``, ``FTSK_RunUserCodeIdle``) and one that is for
user/application code (``FTSK_InitializeUserCodePreCyclicTasks``).
All tasks share the suffix ``FTSK_RunUserCode`` for a consistent implementation
of the Task Creator Functions and are not meant to be changed.


FTSK_InitializeUserCodeEngine
"""""""""""""""""""""""""""""

Before any tasks can start the database needs to be initialized. The database initialization is done by
``FTSK_InitializeUserCodeEngine()``.

.. _ftask_special_task_engine:

FTSK_RunUserCodeEngine
""""""""""""""""""""""

**This task should not be modified.**

This task triggers the database and the system monitoring modules. The database module copies all data from the queue
into the database variables. The system monitoring checks that all tasks run within their specified time frames.


.. _ftask_special_task_idle:

FTSK_RunUserCodeIdle
""""""""""""""""""""

**This task should not be modified.**

This task is bound to the operating system idle hook (``vApplicationIdleHook``).

FTSK_InitializeUserCodePreCyclicTasks
"""""""""""""""""""""""""""""""""""""

Peripherals and resources that need to be usable as soon as the periodic tasks are running are initialized here.

.. _ftask_user_tasks:

User Tasks
^^^^^^^^^^

The functions shown in :numref:`ftask-user-code-functions` are provided by the ``ftask`` module to run user/application
specific code:

.. csv-table:: ``ftask`` User Code Functions
   :name: ftask-user-code-functions
   :widths: auto
   :header-rows: 1
   :delim: ;
   :file: ./ftask-user-code-functions.csv

Further Reading
---------------

A How-to is found in :ref:`HOW_TO_USE_THE_FTASK_MODULE`.
