.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HARDWARE_INFO_ENGINE:

Hardware Info
=============

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/hw_info/master_info.c``
- ``src/app/engine/hw_info/master_info.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/hw_info/test_master_info.c``

Detailed Description
--------------------

The hardware info module stores and provides basic runtime information about
the master board state.

The module maintains an internal state with:

- source of the last reset,
- debug probe connection state,
- calculated supply voltage at clamp 30C.

Reset and Debugger Information
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- ``MINFO_SetResetSource()`` stores the reset source reported by the platform.
- ``MINFO_GetResetSource()`` returns the currently stored reset source.
- ``MINFO_SetDebugProbeConnectionState()`` stores whether a debug probe is
  connected.
- ``MINFO_GetDebugProbeConnectionState()`` returns the stored debug probe
  connection state.

Supply Voltage Monitoring (Clamp 30C)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``MINFO_CheckSupplyVoltageClamp30c()`` reads the ADC table from the database,
reconstructs the supply voltage using the configured resistor divider and
checks it against an undervoltage threshold.

Behavior of this check:

- If measured supply voltage is above or equal to the threshold, it reports
  ``DIAG_EVENT_OK`` for ``DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST``.
- If measured supply voltage is below the threshold, it reports
  ``DIAG_EVENT_NOT_OK`` for the same diagnosis entry.
- The calculated voltage value is stored in the module-internal master state.
