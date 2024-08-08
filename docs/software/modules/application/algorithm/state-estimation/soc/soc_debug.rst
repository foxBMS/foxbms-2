.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOC__DEBUG_DEFAULT:

SOC: Debug Default
==================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/debug/soc_debug.c``

Configuration
^^^^^^^^^^^^^

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/soc/debug/test_soc_debug.c``

Detailed Description
--------------------

The debug implementation is a fake implementation that continuously outputs
a SOC of 50%.
