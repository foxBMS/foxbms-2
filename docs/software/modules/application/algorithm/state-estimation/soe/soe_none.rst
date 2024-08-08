.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOE__NONE:

SOE: None
=========

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/soe/none/soe_none.c``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/algorithm/state_estimation/soe/none/soe_none_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/soe/none/test_soe_none.c``

Detailed Description
--------------------

State-of-energy estimation is deactivated. No state-of-energy estimation is
performed and respective database values are not calculated.
