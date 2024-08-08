.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _REDUNDANCY_MODULE:

Redundancy Module
=================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/redundancy/redundancy.c``
- ``src/app/application/redundancy/redundancy.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/redundancy/test_redundancy.c``

Detailed Description
--------------------

|tbc|

FAQs
----

**Question:**

The database entries ``XXX_BASE`` and ``XXX_REDUNDANCY0`` for cell voltages and
cell temperatures are only read and not written in function
``MRC_ValidateAfeMeasurement()``.
Is that correct?

**Answer:**

It is correct that the cell voltage and cell temperature measurements from the
AFE is just read by the redundancy module.
Function ``MRC_ValidateCellVoltageMeasurement()`` receives the cell voltage
measurement values of the base and the redundant measurement as an input and
validates the measurements.
The validated cell voltages are then written into database entry
``DATA_BLOCK_ID_CELL_VOLTAGE``.
This database entry contains the validated measurements and is consecutively
used from all other modules throughout the BMS.

The same logic is applied to the cell temperature measurement values.
Here, the validated measurements are written into the database entry
``DATA_BLOCK_ID_CELL_TEMPERATURE``.

The database entries ``XXX_BASE`` and ``XXX_REDUNDANCY0`` are only intended to
be used by the redundancy module. All other modules **SHALL** only use the
validated database entries ``DATA_BLOCK_ID_CELL_VOLTAGE`` and
``DATA_BLOCK_ID_CELL_TEMPERATURE``.
