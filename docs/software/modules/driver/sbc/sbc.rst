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

- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_assert.h`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x__assert_8h.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x__assert_8h_source.html>`__)
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_common.h`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x__common_8h.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x__common_8h_source.html>`__)
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_communication.c`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x__communication_8c.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x__communication_8c_source.html>`__)
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_communication.h`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x__communication_8h.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x__communication_8h_source.html>`__)
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x_map.h`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x__map_8h.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x__map_8h_source.html>`__)
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x.c`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x_8c.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x_8c_source.html>`__)
- ``src/app/driver/sbc/fs8x_driver/sbc_fs8x.h`` (`API <../../../../_static/doxygen/src/html/sbc__fs8x_8h.html>`__, `source <../../../../_static/doxygen/src/html/sbc__fs8x_8h_source.html>`__)

Driver
^^^^^^

- ``src/app/driver/sbc/sbc.c`` (`API <../../../../_static/doxygen/src/html/sbc_8c.html>`__, `source <../../../../_static/doxygen/src/html/sbc_8c_source.html>`__)
- ``src/app/driver/sbc/sbc.h`` (`API <../../../../_static/doxygen/src/html/sbc_8h.html>`__, `source <../../../../_static/doxygen/src/html/sbc_8h_source.html>`__)
- ``src/app/driver/sbc/nxpfs85xx.c`` (`API <../../../../_static/doxygen/src/html/nxpfs85xx_8c.html>`__, `source <../../../../_static/doxygen/src/html/nxpfs85xx_8c_source.html>`__)
- ``src/app/driver/sbc/nxpfs85xx.h`` (`API <../../../../_static/doxygen/src/html/nxpfs85xx_8h.html>`__, `source <../../../../_static/doxygen/src/html/nxpfs85xx_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/nxpfs85xx_cfg.h`` (`API <../../../../_static/doxygen/src/html/nxpfs85xx__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/nxpfs85xx__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/sbc/test_sbc.c`` (`API <../../../../_static/doxygen/tests/html/test__sbc_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__sbc_8c_source.html>`__)
- ``tests/unit/app/driver/sbc/test_nxpfs85xx.c`` (`API <../../../../_static/doxygen/tests/html/test__nxpfs85xx_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__nxpfs85xx_8c_source.html>`__)

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
