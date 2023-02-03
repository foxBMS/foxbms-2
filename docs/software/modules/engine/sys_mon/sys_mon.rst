.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _SYSTEM_MONITORING_MODULE:

System Monitoring Module
========================

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/sys_mon/sys_mon.c`` (`API <../../../../_static/doxygen/src/html/sys__mon_8c.html>`__, `source <../../../../_static/doxygen/src/html/sys__mon_8c_source.html>`__)
- ``src/app/engine/sys_mon/sys_mon.h`` (`API <../../../../_static/doxygen/src/html/sys__mon_8h.html>`__, `source <../../../../_static/doxygen/src/html/sys__mon_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/engine/config/sys_mon_cfg.c`` (`API <../../../../_static/doxygen/src/html/sys__mon__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/sys__mon__cfg_8c_source.html>`__)
- ``src/app/engine/config/sys_mon_cfg.h`` (`API <../../../../_static/doxygen/src/html/sys__mon__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/sys__mon__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/config/test_sys_mon_cfg.c`` (`API <../../../../_static/doxygen/tests/html/test__sys__mon__cfg_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__sys__mon__cfg_8c_source.html>`__)
- ``tests/unit/app/engine/sys_mon/test_sys_mon.c`` (`API <../../../../_static/doxygen/tests/html/test__sys__mon_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__sys__mon_8c_source.html>`__)

Detailed Description
--------------------

The system monitoring module allows to detect timing variations in the
execution of the system tasks on millisecond level.
This is done by capturing a timestamp on entry and exit of the task functions.
A continuously running monitoring task compares the actual runtime of the tasks
against a configured maximum allowed runtime.
In the case that the predefined thresholds are surpassed, a diagnosis entry is
created and, if configured, the violation is recorded in persistent memory.
The current state of the system monitoring is communicated through a CAN
message.

Configuration
^^^^^^^^^^^^^

The monitoring actions for each task can be configured in the array described
in :ref:`current-system-monitoring-configuration`.

.. literalinclude:: ./../../../../../src/app/engine/config/sys_mon_cfg.c
   :language: C
   :linenos:
   :start-after: /* DOCUMENTATION marker - sys_mon config entry */
   :end-before: /* DOCUMENTATION marker - sys_mon config exit */
   :caption: Current system monitoring configuration
   :name: current-system-monitoring-configuration

In this configuration, every task must have an entry.
For each task, the system monitoring can be enabled or disabled.
This is for example useful, when the algorithm task may violate the timings
from time to time.
Care should be taken, when a higher priority task is not monitored.
A timing violation on a higher priority task likely means that it takes up
all remaining resources leading to a block of the remaining lower priority
tasks.

Apart from settings such as the expected cycle time and timing variation, it is
possible to configure whether a flag in persistent memory shall be set for each
task.
If this is disabled, the flag still exists, but will always return false
(meaning no violation).

Diagnosis entries
^^^^^^^^^^^^^^^^^

When monitoring is enabled for a task and when the task violates its timings,
a diagnosis entry of type ``DIAG_ID_SYSTEM_MONITORING`` is created in the
:ref:`DIAGNOSIS_MODULE`.
The diagnosis handler for this ID sets the appropriate error flags in the
``DATA_BLOCK_ERROR_STATE_s`` database table.

These flags will not be cleared automatically.
The only actions that clear these flags are a power on reset cycle or sending
the appropriate flag in the ``foxBMS_Command`` CAN message.

Error recording
^^^^^^^^^^^^^^^

If enabled, the system monitoring sets flags in persistent memory.
This is handled through the :ref:`FRAM` module.
Flags are written to the ``FRAM_SYS_MON_RECORD_s`` entry and committed to
persistent memory through a handler that is called from the 10 millisecond
task.

These flags will not be cleared automatically.
The only action that clears these flags is sending the appropriate flag in the
``foxBMS_Command`` CAN message.

CAN interface
^^^^^^^^^^^^^

The system monitoring module communicates through the :ref:`CAN` interface.
Detected violations of timing expectations are communicated through two
messages.
A general flag indicating if any violation has occurred is sent as the
variable ``foxBMS_SysMonError`` in message ``foxBMS_State``.
This flag is a sum value of all recorded and current entries in the database
and the persistent memory.

A detail information on the current state is transmitted in the
``foxBMS_DetailState`` message.
This message sets flags for recorded and current violations of each task.

In order to reset all current and recorded flags, the ``foxBMS_resetFlags``
must be set in the ``foxBMS_Command`` message.
This calls the function ``SYSM_ClearAllTimingViolations()`` which resets
both the current and recorded flags and commits the update directly to the
persistent memory.
