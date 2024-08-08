.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOE__ENERGY_COUNTING:

SOE: Energy Counting
====================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/soe/counting/soe_counting.c``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/algorithm/state_estimation/soe/counting/soe_counting_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/soe/counting/test_soe_counting.c``

Detailed Description
--------------------
SE_CalculateStateOfCharge
-------------------------

Charge and Discharge energy throughput can be calculated in two ways.

If external energy counting value is available:

1. Calculate the energy difference between the latest and the previous energy counting value.
1. Check current flow direction to evaluate if charge/discharge is ongoing.
1. Accumulate energy throughput value in either charge or discharge direction in |soe| database entry.
1. Save time step
1. Save energy counting value


If external energy counting value is *NOT* available:

1. Manually calculate energy throughput since last call.
1. Check current flow direction to evaluate if charge/discharge is ongoing.
1. Accumulate energy throughput value in either charge or discharge direction in |soe| database entry.
1. Save time step


|tbc|
