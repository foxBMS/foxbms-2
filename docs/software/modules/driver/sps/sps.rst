.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _SPS_DRIVER:

SPS
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/sps/sps.c``
- ``src/app/driver/sps/sps.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/sps_cfg.c``
- ``src/app/driver/config/sps_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/sps/test_sps.c``
- ``tests/unit/app/driver/config/test_sps_cfg.c``

Description
-----------

The |sps| (Smart Power Switch) module controls smart power-switch outputs that
are used for contactors and general-purpose IO channels.
The module supports multiple |sps| ICs connected in a daisy-chain and exchanges
commands via SPI.

``SPS_Initialize()`` configures the required reset/chip-select and feedback
enable IOs.

``SPS_Ctrl()`` is the time-triggered state machine entry point (intended to be
called every 10 ms).
Its startup and runtime sequence performs:

- hardware reset toggling of the |sps| chain,
- control-register configuration (normal mode, strong drive),
- cyclic triggering of current measurements,
- cyclic readout of output-current diagnostic registers for all four outputs,
- continuous application of requested channel states.

Per control step, the driver performs two SPI transactions:

- one register read/write command transaction,
- one output-control transaction that also clocks out read responses from the
  previous command (as defined by the daisy-chain protocol).

Channel requests are stored in ``sps_channelStatus`` and applied atomically:

- ``SPS_RequestContactorState()`` only accepts channels configured as
  ``SPS_AFF_CONTACTOR``.
- ``SPS_RequestGeneralIoState()`` only accepts channels configured as
  ``SPS_AFF_GENERAL_IO``.
- ``SPS_SwitchOffAllGeneralIoChannels()`` clears all channels affiliated as
  general IO.

Two feedback paths are provided:

- ``SPS_GetChannelCurrentFeedback()`` derives electrical state from measured
  channel current compared to a configurable threshold.
- ``SPS_GetChannelPexFeedback()`` reads mapped feedback pins via the port
  expander and supports normally-open and normally-closed feedback logic.

Static channel configuration (affiliation, thresholds, and
:ref:`PEX <PORT_EXPANDER_DRIVER>` mapping) is provided in ``sps_cfg.c`` and
``sps_cfg.h``.
