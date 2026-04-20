.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _APPLICATION:

###########
Application
###########

This chapter describes the function principle of the application after the
startup.
For more information on the startup procedure refer to
:ref:`EMBEDDED_SOFTWARE_STARTUP`.

***********************
Database and Diagnosis
***********************

The database runs with the highest priority in the system and provides
asynchronous data exchange for the whole system.
:numref:`database-data-exchange` shows the data exchanges implemented via the
database for exemplary modules.
In this example there are two functions (run in the |1ms-task|) that
create voltage (U), temperature (T) data (function ``MEAS_Control()``) and
receive CAN messages (Msgs) (function ``CAN_ReadRxBuffer()``).
The ``BMS_Trigger()`` function in the |10ms-task| requires all this data
and reads it from the database.

.. drawio-figure:: img/database_concept.drawio
   :format: svg
   :alt: Database data exchange
   :name: database-data-exchange
   :width: 520px

   Asynchronous data exchange with the |foxbms| database

The dependency between the database and the diagnosis module are shown in
:numref:`dependency-database-diagnosis`.

.. drawio-figure:: img/database-and-diagnosis.drawio
   :format: svg
   :alt: dependency between database and diagnosis
   :name: dependency-database-diagnosis
   :width: 520px

   Dependency between Database and Diagnosis module

**********************
Task-Function Mapping
**********************

To get a first overview, how the software works, the task-function mapping is
shown in :numref:`task-function-mapping`.

.. drawio-figure:: img/task-function-mapping.drawio
   :format: svg
   :alt: Task-Function mapping
   :name: task-function-mapping
   :width: 520px

   Task-function mapping in |foxbms|

A very reduced overview of how the ``BMS`` module works is shown below.

.. drawio-figure:: img/high-level-application-view.drawio
   :format: svg
   :alt: High-level view of the BMS module
   :name: high-level-application-view
   :width: 520px

   High-level view of the BMS module

The two key modules used are:

- ``SYS``
- ``BMS``

``SYS`` has a lower priority than the database and a higher priority than
``BMS``.
Both modules are implemented as a state machine, with a trigger function that
implements the transition between the states.
The trigger functions of ``SYS`` and ``BMS`` are called in
``FTSK_RunUserCodeCyclic10ms()``.

``SYS`` controls the operating state of the system. It starts the other
state machines (e.g., ``BMS``).

``BMS`` gathers info on the system via the database and takes decisions based
on this data.
The BMS is driven via CAN. Requests are made via CAN to go either
in STANDBY mode (i.e., contactors are open) or
in NORMAL mode (i.e., contactors are closed).
A safety feature is that these requests must be sent periodically every 100ms.
``BMS`` retrieves the state requests received via CAN from the database and
analyses them.
If the requests are not sent correctly, this means that the controlling unit
driving the BMS has a problem and the correctness of the orders sent to the BMS
may not be given anymore.
As a consequence, in this case ``BMS`` makes a call to ``CONT`` to open the
contactors.
Currently, ``BMS`` checks the cell voltages, the cell temperatures and the
global battery current.
If one of these physical quantities violates the safe operating area, ``BMS``
makes the corresponding call to ``CONT`` to open the contactors.
``BMS`` is started via an initial state request made in ``SYS``.

A watchdog instance is needed in case one of the aforementioned tasks hangs.
This watchdog is made by the System Monitor module which monitors all
important tasks (e.g., ``Database``, ``SYS``, ``BMS``): if any of the
monitored tasks hangs, this will be detected.

A last barrier is present in case all the preceding measures fail: the
hardware watchdog timer.
In case it is not triggered periodically, it resets the system.
