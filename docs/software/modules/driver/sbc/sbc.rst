.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _SBC:

SBC
===

Module Files
------------

Vendor Driver
^^^^^^^^^^^^^

.. note::

   This driver has been provided by NXP Semiconductors, Inc. under the 3-Clause
   BSD License. It has been adapted to pass the interface configuration as
   parameter into the functions to enable the usage of multiple ICs in the
   system.

- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_assert.h``
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_common.h``
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_communication.c``
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_communication.h``
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_map.h``
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x.c``
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x.h``

Driver
^^^^^^

- ``src/app/driver/sbc/sbc.c``
- ``src/app/driver/sbc/sbc.h``
- ``src/app/driver/sbc/nxpfs85xx.c``
- ``src/app/driver/sbc/nxpfs85xx.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/sbc/test_sbc.c``
- ``tests/unit/app/driver/sbc/test_nxpfs85xx.c``

Description
-----------

As long as the Ignition signal is present, the SBC is operating normally in
Running mode.
If the Ignition signal is removed, the MCU will request to send the SBC to
Standby mode.
If no Ignition signal is present in Standby mode, the SBC will power down and
also stop powering the MCU, which will then also shut down.
When the Ignition signal is detected again at the SBC, it will start up and
power the MCU. The MCU will then initialize the SBC, which will return to
normal operation in Running mode after the initialization is complete.
This sequence is shown in :numref:`sbc_state_machine_wakeup`.

.. graphviz:: sbc_state_machine_wakeup.dot
    :caption: SBC wakeup state chart
    :name: sbc_state_machine_wakeup


FAQs
----

**Question:**

The MC33FS830A0ES needs to be programmed.
Can the programming procedure for NXP MC33FS8530A0ES be shared?

**Answer:**

Unfortunately not.
The details of how to program the MC33FS830A0ES cannot be shared due to NDAs.
You need to get in contact with the manufacturer for this information.
