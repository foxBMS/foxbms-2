.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _SOA_MODULE:

SOA Module
==========

Module Files
------------

Driver
^^^^^^

- ``src/app/application/soa/soa.c``
- ``src/app/application/soa/soa.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/config/soa_cfg.c``
- ``src/app/application/config/soa_cfg.h``

Unit Test
^^^^^^^^^
- ``tests/unit/app/application/soa/test_soa_cfg.c``
- ``tests/unit/app/application/soa/test_soa.c``

Detailed Description
--------------------

Keeping the battery cells in the safe operating area (|soa|) is a safety goal of
the BMS. The |soa| includes cell voltage, cell temperature and battery current.

|foxbms| provides three different error levels. The first error level is called
Maximum operating limit (|mol|), the second error level is called Recommended
Safety Limit (|rsl|) and the third error level is called Maximum Safety Limit
(|msl|). The first two error levels indicate that a parameter is reaching the
limits of the recommended operating area and counter measures should be
initiated to prevent an unwanted opening of the contactors. A violation of a
|msl| means the safety of the system and the persons cannot be guaranteed anymore
and leads to an opening of the contactors.
