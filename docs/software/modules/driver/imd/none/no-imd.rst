.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _NO_IMD_DRIVER:

No IMD
======

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/none/none/none_none.c``
- ``src/app/driver/imd/none/none/none_none.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/none/none/test_none_none.c``

Description
-----------

The insulation measurement is always set to be valid and an insulation
resistance of :math:`10000000\ k{\Omega}`  is written to the database entry.
