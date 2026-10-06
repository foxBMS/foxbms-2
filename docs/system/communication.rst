.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _COMMUNICATION:

#############
Communication
#############

***
CAN
***

|foxbms| supports communication with other high level devices (e.g., the VCU)
through CAN.


DBC File
========

These files describe the CAN interface used by |foxbms|.
The .dbc-file has been created using PCAN Symbol Editor
|version_pcan_symbol_editor| from symbol file |version_sym_file|.

The .dbc-file and .sym-file are located in ``tools/dbc``.

.. include:: ./../../build/docs/supported_can_messages.txt

CAN TX Behavior
===============

Cyclic Messages
---------------

The cyclic transmit list is configured in ``can_txMessages[]`` in
``src/app/driver/config/can_cfg_tx_cyclic.c``.

The cyclic messages are **not** sent immediately after startup.
Periodic TX is globally gated by ``can_state.periodicEnable`` in
``CAN_MainFunction()``:

- If ``periodicEnable == false``, no cyclic TX is executed.
- If ``periodicEnable == true``, ``CAN_PeriodicTransmit()`` sends all
  configured cyclic messages according to period and phase.

This gate is enabled during SYS initialization.
After that point, cyclic transmission runs continuously.

The BMS cyclically transmits these messages:

- ``f_SysState``
- ``f_BmsState``
- ``f_BmsStateDetails``
- ``f_CellVoltages``
- ``f_CellTemperatures``
- ``f_PackLimits``
- ``f_PackMinimumMaximumVoltage``
- ``f_PackMinimumMaximumTemp``
- ``f_PackStateEstimation``
- ``f_PackValuesP0``
- ``f_PackValuesP1``
- ``f_StringState``
- ``f_StringMinimumMaximumVoltage``
- ``f_StringMinimumMaximumTemp``
- ``f_StringStateEstimation``
- ``f_StringValuesP0``
- ``f_StringValuesP1``

On-demand Messages
------------------

The following transmitted messages are sent on request, not cyclically:

- ``f_DebugResponse``, including requested payload variants
  such as version information, commit hash, RTC time, boot timestamp and
  uptime.
- ``f_DebugBuildConfiguration``.
- ``f_DebugIdentifyHardware``.
- ``f_DebugUnsupportedMultiplexerValues`` when an unsupported
  debug multiplexer value is received.

These responses are triggered by received ``f_Debug`` frames in
``CANRX_Debug()``.

Further Asynchronous Transmitted Messages
-----------------------------------------

Some asynchronous TX messages are event-driven and therefore neither cyclic nor
request/response from ``f_Debug``:

- ``f_BmsState`` is additionally sent asynchronously when BMS state or
  substate changes (``CANTX_TransmitBmsState()``).
- ``f_BmsFatalError`` is sent by the diagnosis module when
  fatal errors are set/cleared and periodically resent while active.
- ``f_CrashDump`` is sent on detected stack overflow.

********
Ethernet
********

Additionally to to CAN communication an ethernet communication interface is
provided.
Currently plain |tcp-ip| is supported for user defined application tasks.
These tasks can be placed in ``ethernet.c``.
As an example and for testing purposes an echo server is implemented there.
More details can be found in :ref:`ETHERNET_APPLICATION`.
