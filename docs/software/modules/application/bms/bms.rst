.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _BMS_MODULE:

BMS Module
==========

Module Files
------------

Driver
^^^^^^

- ``src/app/application/bms/bms.c``
- ``src/app/application/bms/bms.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/config/bms_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/bms/test_bms.c``

Detailed Description
--------------------

A finite state machine with two main states (**STANDBY** and **NORMAL**)
defines the BMS behaviour.
**STANDBY** corresponds to the state where all the contactors are open and
**NORMAL** corresponds to a state where the contactors are closed to allow
current flowing.
The transition between the main states is made in response to CAN requests read
from the data-exchange module (database).
The state machine transitions into an initializing state after start up and
waits until it is driven via CAN into **STANDBY**.
The BMS will automatically transition into **ERROR** state, once a malfunction
is detected within the battery system that could be potentially dangerous.
Additionally, the **ERROR** state prevents closing of the contactors and can
only be left, if the cause of failure has been removed and a **STANDBY**
request is sent to the state machine.
A simplified flow chart of the BMS state machine and the possible transitions
between the main states is depicted in the figure below.

.. drawio-figure:: bms-state-diagram.drawio
   :format: svg
   :alt: BMS state flow diagram
   :name: bms-state-diagram
   :width: 240px

   BMS state flow diagram

All diagnosis flags are transmitted via CAN, allowing the superior control
system to react accordingly.
Diagnosis entries with a severity of **Info** and **Warning** flags are
transmitted for information purpose only and no action is taken once a
parameter threshold is violated and the respective flag is raised.
In contrast, once a potentially hazardous situation is detected and an
**Error** flag is raised, the BMS will subsequently switch, with the configured
delay time to ERROR state and open the contactors.
As soon as an error condition is detected by the BMS, the BMS will signal this
the superimposed control system by setting **EmergencyShutoff** flag in CAN
message f_BmsState.

Special attention must be paid to overcurrent errors when disconnecting the
battery system from the application, because of the imminent danger of
permanently welded contactors caused by erroneous switching actions.
The contacts of a contactor can be welded, when attempting to disconnect the
battery system while a current higher than the maximum break current of the
contactor is flowing and thus the battery system cannot be disconnected from
the application.
In the worst-case, this could lead to permanent damage or even destruction of
the battery system, in case the current cannot be interrupted or controlled.
To prevent this from happening in an over-current scenario the BMS waits until
the fuse of the battery system has been triggered and the current has been
interrupted.
Only then will the contactors be opened.
This guarantees a maximum of safety while trying to maximize the contactor
service life.
