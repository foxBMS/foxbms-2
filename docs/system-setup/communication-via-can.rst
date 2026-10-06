.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _COMMUNICATION_VIA_CAN:

#####################
Communication via CAN
#####################

The main communication interface of |foxbms| is the CAN bus.

******************************
Initial Communication Sequence
******************************

On startup, the |bms-master| will transmit a startup message.
This is described at :ref:`CAN_BOOT_MESSAGE_SEQUENCE`.

*************************
Request Debug Information
*************************

The high-level control unit can request debug information from the |bms-master|
by sending a ``f_DebugRequest`` message.
The |bms-master| will respond with a ``f_DebugResponse`` message.
This is described at :ref:`HOW_TO_USE_THE_F_DEBUG_CAN_MESSAGE`.

*****************
BMS State Request
*****************

|tbc|
