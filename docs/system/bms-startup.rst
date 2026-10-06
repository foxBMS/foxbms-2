.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _BMS_STARTUP:

###########
BMS Startup
###########

The low level initialization of the |mcu| is described in
:ref:`EMBEDDED_SOFTWARE_STARTUP`.
In the following, the high level initialization of the BMS software is
described in chronological order.

.. _CAN_BOOT_MESSAGE_SEQUENCE:

*************************
CAN Boot Message Sequence
*************************

Boot Message Bootloader
************************

The following messages will be received on startup via
``f_BootloaderVersionInfo`` and ``f_BootloaderAcknowledgeMessage``:

#. ``f_BootloaderVersionInfo.BootInformation`` with ``0xFEFEFEFEFEFEFE``
#. ``f_BootloaderVersionInfo.BootloaderVersionInfo`` with the current
   version information
#. ``f_BootloaderVersionInfo.CommitHashHigh7`` with the first 7 high characters
#. ``f_BootloaderVersionInfo.CommitHashLow7`` with the last 7 low characters
#. ``f_BootloaderVersionInfo.BootInformation`` with ``0x01010101010101``
#. ``f_BootloaderAcknowledgeMessage`` with ``0xD300000000000000``

Boot Message Application
************************

The following messages will be be send by |foxbms| on startup via ``f_DebugResponse``:

#. ``f_DebugResponse.BootInformation`` with ``0xFEFEFEFEFEFEFE``
#. ``f_DebugResponse.BmsSoftwareVersionInfo`` with the current
   version information
#. ``f_DebugResponse.CommitHashHigh7`` with the first 7 high characters
#. ``f_DebugResponse.CommitHashLow7`` with the last 7 low characters
#. ``f_DebugResponse.McuUniqueDieId`` with the Device ID
#. ``f_DebugResponse.McuLotNumber`` with the Lot number
#. ``f_DebugResponse.McuWaferInformation`` with the Wafer Information
#. ``f_DebugResponse.BootTimestamp`` with the boot Boot timestamp
#. ``f_DebugResponse.BootInformation`` with ``0x01010101010101``
