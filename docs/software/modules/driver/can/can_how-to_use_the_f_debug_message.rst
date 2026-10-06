.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_USE_THE_F_DEBUG_CAN_MESSAGE:

Using the ``f_Debug`` CAN Message
"""""""""""""""""""""""""""""""""

``f_Debug``
"""""""""""

The following information can be requested or action can be performed via the
``f_Debug`` message:

#. ``f_Debug.FramInitialization`` resets all entries stored within the FRAM to
   their default values.
#. ``f_Debug.IdentifyHardware`` requests the hardware information for every
   connected |bms-slave|.
#. ``f_Debug.Rtc`` requests to set the provided time as timestamp on the
   |bms-master|.
#. ``f_Debug.SoftwareReset`` requests the |bms-master| to perform a power cycle
   (not yet implemented).
#. ``f_Debug.TimeInfo`` requests to return the current time info of the |rtc|.
#. ``f_Debug.UptimeInfo``  requests to return the current uptime of the
   |bms-master|.
#. ``f_Debug.VersionInfo`` requests to return the current version information
   of the |bms-master| software.

``f_DebugResponse``
"""""""""""""""""""

The following ``f_DebugResponse`` signals are available:

#. ``f_DebugResponse.BmsSoftwareVersionInfo`` transmits the current version
   information.
#. ``f_DebugResponse.BootInformation`` may be send with either
   ``0xFEFEFEFEFEFEFE`` for start or ``0x01010101010101`` for end of the boot
   pattern.
#. ``f_DebugResponse.BootTimestamp`` transmits the system startup time.
#. ``f_DebugResponse.CommitHashHigh7`` transmits the first 7 high characters of
   the commit hash.
#. ``f_DebugResponse.CommitHashLow7`` transmits the last 7 low characters of
   the commit hash.
#. ``f_DebugResponse.McuLotNumber`` transmits the DieID high.
#. ``f_DebugResponse.McuUniqueDieId`` transmits the device ID.
#. ``f_DebugResponse.McuWaferInformation`` transmits the DieID low.
#. ``f_DebugResponse.RtcTime`` transmits the current device time.
#. ``f_DebugResponse.Uptime`` transmits the uptime since the first |1ms-task|
   started.

These messages are either sent by |foxbms| as part of its normal operation or
on request via a ``f_Debug`` message.
