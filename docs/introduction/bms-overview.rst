.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _THE_FOXBMS_2_PLATFORM:

#####################
The foxBMS 2 Platform
#####################

The |foxbms| platform consists of two main elements:

- the |master| and
- the |slave|.

The |master| consists of 3 boards:

- the |bms-master|,
- the |bms-interface| and
- the |bms-extension|.

An ARM-based microcontroller (`Cortex-R5`_) is used on the |bms-master|.

The |bms-master| communicates with the outside world via a CAN bus.
The current flowing through the battery system is measured via a current sensor
connected to a CAN bus.
The current sensor is controlled via CAN by the |bms-master| and sends the
resulting measurement via CAN.

The |slaves| (based on |bms-slaves|) are used for measuring cell voltages and
cell temperatures, as well as for passive balancing in battery modules.
The |slaves| can be connected in series as a daisy-chain.

In order for the |master| to communicate with the |slaves|, an interface board
(|bms-interface|) is needed.
It implements the physical layer of the communication between the |bms-master|
and the |slaves|.

Control requests to the |master| are made via CAN messages.
They control the externally facing BMS behavior of the system such as opening
and closing the contactors.
This behavior based on the internal implementation and application dependent.
The |master| monitors the state of the battery system to ensure a safe
operation of the system.
The measurement data and all relevant information of the system is communicated
through a CAN interface to a superior control unit.

The |bms-master| also provides an interlock line that is closely monitored.
This fast acting interface that can be connected through crucial system
components such as the |master|, emergency stop switch, service disconnect
switches, high voltage connectors and similar devices.
The interlock line can be opened by any connected device.
It is possible to define a system behavior in the case that the interlock line
has been opened such as the transition to an application-specific safe-state.
In the default implementation, all contactors will be opened and the BMS will
switch to an error state.

See :numref:`start-high-level`.

.. figure:: img/high-level-overview.png
   :name: start-high-level
   :align: center
   :alt: High level overview of |foxbms|

   High level overview of |foxbms|

In case that an application requires more inputs, outputs or specific hardware
functions, these can be implemented through a |bms-extension|.
This is a specialized board that connects through a set of connectors to
the |bms-master| and can implement application-specific hardware for the
|master|.

This description reflects the current state of |foxbms|. Due to the open nature
of the system, many other possibilities can be implemented, like for example:

- Use of other types of current sensors (e.g., shunt-based or Hall-effect
  based)
- No |slave| needs to be used: a direct measurement of the cell voltages and
  cell temperatures can be performed by the |master|
- A higher number of contactors can be controlled (e.g., up to 9)
- etc...

.. _Cortex-R5:
   https://developer.arm.com/ip-products/processors/cortex-r/cortex-r5
