.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOC__COULOMB_COUNTING:

SOC: Coulomb Counting
=====================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/counting/soc_counting_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/soc/counting/test_soc_counting.c``

Detailed Description
--------------------

By default, |foxbms| uses the internal integrator of the current sensor.
If the current sensor integrator is not available, the integration is made
directly in the |foxbms| software.

The check of the presence of the integrator and the software integration are
made in the :ref:`cc-function-name`.

.. literalinclude:: ./../../../../../../../src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE cc-documentation-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE cc-documentation-stop-include */
   :caption: Function implementing Coulomb-counting
   :name: cc-function-name

SE_CalculateStateOfCharge
-------------------------

Charge and Discharge Throughput can be calculated in two ways, depending if an
external analog current counting value is available or not. The algorithm is
implemented as follows:

If analog current counting value is available:

1. Calculate the charge difference between the latest and the previous current counting value.
1. Check current flow direction to evaluate if charge/discharge is ongoing.
1. Accumulate either charge or discharge throughput in |soc| database entry.
1. Save time step
1. Save current counting value

If analog current counting value is *NOT* available:

1. Manually integrate current to calculate amount of charge since last call.
1. Check current flow direction to evaluate if charge/discharge is ongoing.
1. Accumulate either charge or discharge throughput in |soc| database entry.
1. Save time step
