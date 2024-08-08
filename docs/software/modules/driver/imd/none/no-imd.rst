.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _DUMMY_INSULATION_MEASUREMENT_DEVICE:

Dummy Insulation Measurement Device
===================================

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/none/no-imd.c``
- ``src/app/driver/imd/none/no-imd.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/none/test_no-imd.c``

Description
-----------

The insulation measurement is always set to be valid and an insulation
resistance of :math:`10000000\ k{\Omega}`  is written to the database entry.
