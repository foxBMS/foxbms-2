.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _OS_CONFIGURATION:

##############################
Operating System Configuration
##############################

As previously stated, one design paradigm is that all application code runs
within the context of an operating system.
In detail, this means that various tasks with different priorities are used to
ensure the necessary real-time behavior of the BMS as shown in
:numref:`sw-tasks-priorities`.

.. drawio-figure:: img/sw-tasks-setup-tasks-and-priorities.drawio
   :format: svg
   :alt: Tasks & Priorities
   :name: sw-tasks-priorities
   :width: 800px
   :align: center

   |foxbms| Tasks & Priorities

Two different task implementations are used:

- **Cyclic executed task in non-blocking mode with a fixed period (1ms, 10ms,
  100ms)**
- **Consecutive running task in blocking-mode**

Four scheduled tasks with a period of 1\ |ms|, 10\ |ms|, and 100\ |ms|, are
configured to execute the various deterministic finite-state machines that
describe the behavior of the BMS as shown in :numref:`sw-tasks-cyclic`.

.. drawio-figure:: img/sw-tasks-setup-cyclic.drawio
   :format: svg
   :alt: Task Model - Cyclic Tasks
   :name: sw-tasks-cyclic
   :width: 800px
   :align: center

   |foxbms| Task Model - Cyclic Tasks

Time-sensitive software modules (e.g., diagnostics, measurement, CAN reception)
are called within the |1ms-task|, whereas less
time critical modules (e.g., CAN transmission, interlock, BMS) are implemented
inside the |10ms-task|.
Software modules that are temporally uncritical (e.g., state estimation,
balancing) are handled by the |100ms-task|.
The additional |100ms-algorithms-task| can be used for advanced
user specific algorithms (e.g., moving averages, computation intensive state
estimation algorithms).

Additional functionality is provided in through continuous running tasks
that are implemented in blocking-mode.
This means, that the detailed function implementation in these tasks defines
the actual timing behavior (e.g., running time, blocking duration).
The continuous running tasks are shown in
:numref:`sw-tasks-continuous-blocking`.

.. drawio-figure:: img/sw-tasks-setup-continuous.drawio
   :format: svg
   :alt: Task Model - Continuous Running Tasks
   :name: sw-tasks-continuous-blocking
   :width: 800px
   :align: center

   |foxbms| Task Model - Continuous Running Tasks

The |engine-task| is used to implement a data-exchange layer between the
different tasks and processes.
This data-exchange layer runs with the highest priority of all tasks and is
interfaced using queues to either send or to receive data entries.
These |freertos| queues are formally verified for memory safety, thread safety
and functional correctness.
Depending on the selected AFE, the respective driver is either implemented as
non-blocking variant (executed in the |1ms-task|) or blocking and
then running in the |afe-task|.
All drivers for hardware peripherals with an :ref:`I2C_MODULE` interface on the
|foxbms-bms-master| are called within the |i2c-task|.
The |uart-task| is used to handle the |uart| software flowcontrol.
The |idle-task| is executed if all other tasks are currently blocked or are
waiting to be executed again.

In general all |tcp-ip| related tasks have a lower priority than the tasks that
are relevant for the core functionalities of the BMS. The |tcp-ip| related tasks
are the |emac-task|, the |ip-task| and the |ip-application-tasks| which
can be more than one.
These Tasks are displayed in figure :numref:`sw-tasks-priority-diagram`.

.. drawio-figure:: img/task-priority-diagram.drawio
   :format: svg
   :alt: Task Model - Priority diagram
   :name: sw-tasks-priority-diagram
   :width: 800px
   :align: center

   |foxbms| Task Model - Priority Diagram

In the |emac-task| the reception of the :ref:`EMAC` is handled.
In line with the |freertos| recommendations the receiving task (|emac-task|)
has the highest priority from the |tcp-ip| related tasks.
It is directly responding to hardware interrupts.
The |ip-task| runs with lower priority as it handles more complex operations.
This allows the Receive Task to do its job without interruption.
The |ip-application-tasks| use the |tcp-ip| API to send and receive data have
the lowest priority. This prevents them from starving the time-sensitive
network handling done by the |ip-task|.

The TCP/IP related tasks and |uart-task| are disabled by default.
To enable them the configuration needs to be adapted as in
:ref:`CONFIGURE_OS` or respectively in :ref:`CONFIGURE_DEBUG`.
