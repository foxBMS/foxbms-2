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

#. Calculate the energy difference between the latest and the previous energy
   counting value.
#. Check current flow direction to evaluate if charge/discharge is ongoing.
#. Accumulate energy throughput value in either charge or discharge direction
   in |soe| database entry.
#. Save time step
#. Save energy counting value

If external energy counting value is *NOT* available:

#. Manually calculate energy throughput since last call.
#. Check current flow direction to evaluate if charge/discharge is ongoing.
#. Accumulate energy throughput value in either charge or discharge direction
   in |soe| database entry.
#. Save time step
