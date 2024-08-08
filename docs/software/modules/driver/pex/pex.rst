.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _PORT_EXPANDER_MODULE:

Port Expander Module
====================

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/pex/pex.c``
- ``src/app/driver/pex/pex.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/pex_cfg.c``
- ``src/app/driver/config/pex_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/pex/test_pex.c``

Detailed Description
--------------------

The driver addresses the port expander devices through getter and setter functions.

The driver works with the NXP PCA9539 port expander IC, which has 16 GPIOs.
It is also compatible with the TI TCA9539.
