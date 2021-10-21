.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _PORT_EXPANDER_MODULE:

Port Expander Module
====================

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/pex/pex.c`` (`API <../../../../_static/doxygen/src/html/pex_8c.html>`__, `source <../../../../_static/doxygen/src/html/pex_8c_source.html>`__)
- ``src/app/driver/pex/pex.h`` (`API <../../../../_static/doxygen/src/html/pex_8h.html>`__, `source <../../../../_static/doxygen/src/html/pex_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/pex_cfg.c`` (`API <../../../../_static/doxygen/src/html/pex__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/pex__cfg_8c_source.html>`__)
- ``src/app/driver/config/pex_cfg.h`` (`API <../../../../_static/doxygen/src/html/pex__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/pex__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/pex/test_pex.c`` (`API <../../../../_static/doxygen/tests/html/test__pex_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__pex_8c_source.html>`__)

Detailed Description
--------------------

The driver addresses the port expander devices through getter and setter functions.

The driver works with the NXP PCA9539 port expander IC, which has 16 GPIOs.
It is also compatible with the TI TCA9539.
