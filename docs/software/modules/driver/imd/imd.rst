.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _INSULATION_MEASUREMENT_DEVICE:

Insulation Measurement Device
=============================

Several different Insulation Measurement Devices (IMD) are supported.

.. toctree::
    :maxdepth: 1
    :caption: List of available Insulation Measurement Devices

    ./bender/bender_ir155.rst
    ./bender/bender_iso165c.rst
    ./none/no-imd.rst


A superimposed state machine is implemented that interacts with the actual IMD
implementation. There a different reasons for the state machine to transition
between the states. Generally three cases can happen:

- an external request to the state machine is received to either initialize
  the IMD or to start/stop the insulation resistance measurement
- the IMD implementation request a transition e.g., when the initialization is
  finished
- inherent determined transitions e.g., ``IMD_FSM_STATE_HAS_NEVER_RUN``
  transitions to state ``IMD_FSM_STATE_UNINITIALIZED``

The state machine consists of the following states:

- ``IMD_FSM_STATE_HAS_NEVER_RUN``
- ``IMD_FSM_STATE_UNINITIALIZED``
- ``IMD_FSM_STATE_INITIALIZATION``
- ``IMD_FSM_STATE_IMD_ENABLE``
- ``IMD_FSM_STATE_SHUTDOWN``
- ``IMD_FSM_STATE_RUNNING``
- ``IMD_FSM_STATE_ERROR``

The transitions between the main states of the IMD state machine is depicted
below.

.. graphviz:: imd-state-diagram.dot
    :caption: IMD state flow diagram
    :name: imd-state-diagram

These transitions will either be performed automatically or on request. The
following requests can be made to the state machine:

- ``IMD_STATE_INITIALIZE_REQUEST``
- ``IMD_STATE_SWITCH_ON_REQUEST``
- ``IMD_STATE_SHUTDOWN_REQUEST``

Brief state machine description
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The state machine trigger function ``IMD_Trigger()`` is called periodically
from the 100|_||ms| task.
It starts in state ``IMD_FSM_STATE_HAS_NEVER_RUN`` after startup and
transitions with the next call automatically to state
``IMD_FSM_STATE_UNINITIALIZED``.
The state machine waits in this state until the
``IMD_STATE_INITIALIZE_REQUEST`` has been submitted to the state machine
during the startup phase from the `sys` module.
The state machine transitions to state ``IMD_FSM_STATE_IMD_ENABLE`` after a
successful initialization of the required peripherals and the software modules
of the selected IMD driver implementation in state
``IMD_FSM_STATE_INITIALIZATION``.
Now, the state machine waits again to receive the
``IMD_STATE_SWITCH_ON_REQUEST`` from the application state machine activate the
IMD device and begin with the insulation resistance measurement and the
evaluation of the measurement results from IMD.
The state machine continuously monitors the insulation resistance of the
battery system in this state until the ``IMD_STATE_SHUTDOWN_REQUEST`` is
submitted to the state machine.
This can be necessary to prevent a mutual interference if multiple insulation
monitoring devices would be monitoring the battery system, e.g., when a vehicle
is connected to a charging station.


Description of the IMD state machine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the following, the implementations of all cases are explained in detail.

``IMD_FSM_STATE_HAS_NEVER_RUN``
"""""""""""""""""""""""""""""""

This is the default state after the startup of the BMS. The state machine will
immediately transition to state ``IMD_FSM_STATE_UNINITIALIZED`` with the next
call.

``IMD_FSM_STATE_UNINITIALIZED``
"""""""""""""""""""""""""""""""

The state machine will wait until an ``IMD_STATE_INITIALIZE_REQUEST`` is
received.
This request is submitted from the ``sys`` state machine during the startup
phase of the BMS.

``IMD_FSM_STATE_INITIALIZATION``
""""""""""""""""""""""""""""""""

The ``IMD_FSM_STATE_INITIALIZATION`` state is responsible to only initialize
the required peripherals and the software module of the selected IMD to prepare
the module for the measurement.
The actual measurement of the IMD shall not be started.
The state machine will switch to ``IMD_FSM_STATE_ERROR`` in case an error is
detected during the initialization. The interface is utilized using function
``IMD_ProcessInitializationState``.

``IMD_FSM_STATE_IMD_ENABLE``
""""""""""""""""""""""""""""

This state is responsible to activate the IMD device and start the insulation
measurement after a successful initialization.
Before this is done, the state request ``IMD_STATE_SWITCH_ON_REQUEST`` need to
be submitted to the state machine.
In the default software, this will be done by the ``bms`` state machine during
its initialization phase.
The ``imd`` state machine will switch to ``IMD_FSM_STATE_ERROR`` in case an
error is detected during the activation.
The state machine will switch to ``IMD_FSM_STATE_RUNNING`` once the IMD is
enabled.
The function ``IMD_ProcessEnableState`` provides the required interface for the
IMD implementation.

``IMD_FSM_STATE_SHUTDOWN``
""""""""""""""""""""""""""

The insulation measurement and the IMD are switched off in this case using the
interface function ``IMD_ProcessShutdownState``.
If this this successful, state ``IMD_FSM_STATE_IMD_ENABLE`` is executed next,
otherwise a transition into state ``IMD_FSM_STATE_ERROR`` will be done.

``IMD_FSM_STATE_RUNNING``
"""""""""""""""""""""""""

This the main state of the superimposed IMD state machine.
This state acquires the measurement results from the selected IMD implementation
using the function ``IMD_ProcessRunningState()`` and evaluates the measurement
results in function ``IMD_EvaluateInsulationMeasurement``.
This way the evaluation is encapsulated from the insulation measurement
performed by the IMD.
Additionally, the state machine will react on a ``IMD_STATE_SHUTDOWN_REQUEST``
request and subsequently switch to state ``IMD_FSM_STATE_SHUTDOWN``.

``IMD_FSM_STATE_ERROR``
"""""""""""""""""""""""

|tbc|

Interfaces for IMD driver implementation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following functions are used to interface the actual IMD implementation:


.. literalinclude:: ./../../../../../src/app/driver/imd/imd.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE imd-documentation */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE imd-documentation */
   :caption: Interface description for IMD implementations
   :name: imd-interface-description
