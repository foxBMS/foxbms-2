.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _BMS_VALUES_APPLICATION:

BMS Values
==========

Module Files
------------

Driver
^^^^^^

- ``src/app/application/bms/bms-values.c``
- ``src/app/application/bms/bms-values.h``

Configuration
^^^^^^^^^^^^^

- none

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/bms/test_bms-values.c``
- ``tests/unit/app/application/bms/test_bms-values-redundancy.c``

Detailed Description
--------------------

The BMS Values module is the data-conditioning stage between low-level
measurements and the :ref:`BMS_APPLICATION` state machine.
Its responsibility is to provide validated and derived values that are safe to
consume by control logic.

Execution context
^^^^^^^^^^^^^^^^^

The module is executed cyclically from the |10ms-task| every 50ms via
``BMSVL_UpdateSystemValues()``.
In the same |10ms-task|, ``BMS_Trigger()`` is called afterwards.
This ordering ensures that the BMS state machine operates on the newest
validated values.

Inputs and Outputs
^^^^^^^^^^^^^^^^^^

The update routine reads low-level measurement entries and computes validated
pack-level and statistics values.

Main input sources include:

- ``DATA_BLOCK_ID_CURRENT``
- ``DATA_BLOCK_ID_SYSTEM_VOLTAGE_1``
- ``DATA_BLOCK_ID_SYSTEM_VOLTAGE_3``
- ``DATA_BLOCK_ID_POWER``

If redundant cell-voltage and cell-temperature measurements are **NOT**
enabled, the module reads in the base measurements from:

- ``DATA_BLOCK_ID_CELL_VOLTAGE_BASE``
- ``DATA_BLOCK_ID_CELL_TEMPERATURE_BASE``

If redundant cell-voltage and cell-temperature measurements are enabled, the
redundancy module reads the base and redundant measurements from:

- ``DATA_BLOCK_ID_CELL_VOLTAGE_BASE``
- ``DATA_BLOCK_ID_CELL_TEMPERATURE_BASE``
- ``DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANT``
- ``DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANT``

The outputs are then:

- ``DATA_BLOCK_ID_PACK_VALUES`` (validated string/pack values and validity
  bits)
- ``DATA_BLOCK_ID_CELL_VOLTAGE`` (validated cell voltage values)
- ``DATA_BLOCK_ID_CELL_TEMPERATURE`` (validated cell temperature values)
- ``DATA_BLOCK_ID_MIN_MAX`` (derived min/max/average values)

Processing Sequence
^^^^^^^^^^^^^^^^^^^

At each invocation, ``BMSVL_UpdateSystemValues()`` performs the following high
level steps:

1. Read required database entries for current, voltage, and power.
2. Obtain validated cell voltage/temperature source data:

   - If redundancy support is enabled, use the redundancy module.
   - Otherwise, copy base measurements into the validated working tables.

3. Run plausibility checks for current, string voltage, battery voltage,
   high-voltage bus voltage, and power.
4. Derive min/max/average values for cell voltage and temperature.
5. Re-run derived values where needed after spread-based invalidation.
6. Write resulting pack and min/max values back to the database.

Interaction with Plausibility and Redundancy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The module delegates measurement validation logic to the plausibility module
(see :ref:`PLAUSIBILITY_APPLICATION`) and, if configured, cross-checking of
redundant measurements to the redundancy module
(see :ref:`REDUNDANCY_APPLICATION`).
Plausibility and redundancy are helper modules:
they validate and qualify measurement data, but they do not control BMS
states or contactors.
Control authority remains in :ref:`BMS_APPLICATION`, using values prepared by
BMS Values.
In short:

- BMS Values orchestrates,
- :ref:`PLAUSIBILITY_APPLICATION` validates, and
- :ref:`BMS_APPLICATION` consumes.

.. figure:: ../../../../../build/docs/docs/software/modules/application/bms/bms-values.svg
   :alt: BMS values block diagram
   :name: bms-values-block-diagram
   :width: 800px

   BMS values block diagram

.. figure:: ../../../../../build/docs/docs/software/modules/application/bms/bms-values-details.svg
   :alt: BMS values calling sequence block diagram
   :name: bms-values-calling-sequence-block-diagram
   :width: 800px

   Detailed calling sequence of BMS values update routine

Further Reading
---------------

- :ref:`BMS_APPLICATION`
- :ref:`PLAUSIBILITY_APPLICATION`
- :ref:`REDUNDANCY_APPLICATION`
