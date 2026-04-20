.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _PHY_MODULE:

PHY Module
==========

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/phy/dp83869.c``
- ``src/app/driver/phy/dp83869.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/phy_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/phy/test_dp83869.c``

Detailed Description
--------------------

The DP83869 driver implements the physical layer interface for the
Texas Instruments DP83869HM Ethernet PHY.
It provides functions for initializing, configuring, and managing the PHY,
including link status detection, auto-negotiation, and loopback configuration.

This driver uses the MDIO interface to communicate with the PHY and integrates
with the system's network stack via the :ref:`EMAC` module.

Initialization
^^^^^^^^^^^^^^

The PHY initialization sequence includes:

#. Hardware reset via GPIO pin.
#. Software reset of the PHY.
#. Verification of PHY ID.
#. Configuration of operation mode (e.g., MII).
#. Link establishment and auto-negotiation.

The initialization status is stored in the global data block ``phy_tablePhy``.

Registers
---------

The driver accesses both standard and extended PHY registers via MDIO.

Standard Registers
^^^^^^^^^^^^^^^^^^

+------------------+-------+
| Register Name    | Offset|
+==================+=======+
| BMCR             | 0x00  |
+------------------+-------+
| BMSR             | 0x01  |
+------------------+-------+
| PHYIDR1          | 0x02  |
+------------------+-------+
| PHYIDR2          | 0x03  |
+------------------+-------+
| ANAR             | 0x04  |
+------------------+-------+
| ALNPAR           | 0x05  |
+------------------+-------+
| STATUS           | 0x11  |
+------------------+-------+
| BIST             | 0x16  |
+------------------+-------+

Extended Registers
^^^^^^^^^^^^^^^^^^

+------------------+----------+
| Register Name    | Offset   |
+==================+==========+
| REGCR            | 0x0D     |
+------------------+----------+
| ADDAR            | 0x0E     |
+------------------+----------+
| GEN_CTRL         | 0x1F     |
+------------------+----------+
| STRAP_STS        | 0x6E     |
+------------------+----------+
| OP_MODE          | 0x1DF    |
+------------------+----------+
| FX_INT_STS       | 0xC19    |
+------------------+----------+

See Also
--------

- :ref:`EMAC`
- :ref:`NETWORK_INTERFACE`
- :ref:`ETHERNET_MODULE`
