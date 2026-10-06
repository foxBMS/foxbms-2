.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _PLAUSIBILITY_APPLICATION:

Plausibility
============

Module Files
------------

Driver
^^^^^^

- ``src/app/application/plausibility/plausibility.h``
- ``src/app/application/plausibility/validate-battery-voltage.c``
- ``src/app/application/plausibility/validate-cell-temperature.c``
- ``src/app/application/plausibility/validate-cell-voltage.c``
- ``src/app/application/plausibility/validate-current-measurement.c``
- ``src/app/application/plausibility/validate-high-voltage-bus.c``
- ``src/app/application/plausibility/validate-power-measurement.c``
- ``src/app/application/plausibility/validate-string-voltage.c``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/config/plausibility_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/plausibility/test_validate-battery-voltage.c``
- ``tests/unit/app/application/plausibility/test_validate-cell-temperature.c``
- ``tests/unit/app/application/plausibility/test_validate-cell-voltage.c``
- ``tests/unit/app/application/plausibility/test_validate-current-measurement.c``
- ``tests/unit/app/application/plausibility/test_validate-high-voltage-bus.c``
- ``tests/unit/app/application/plausibility/test_validate-power-measurement.c``
- ``tests/unit/app/application/plausibility/test_validate-string-voltage.c``

Detailed Description
--------------------

The plausibility module validates measurement consistency and freshness for the
application layer.
It is used by :ref:`BMS_VALUES_APPLICATION` to transform raw low-level
measurements into trusted values for the :ref:`BMS_APPLICATION` state machine.

Role in the Application
^^^^^^^^^^^^^^^^^^^^^^^

The module itself does not own a scheduler-triggered task.
Its routines are called from ``BMSVL_UpdateSystemValues()`` in the BMS Values
module.
This keeps validation logic centralized while allowing BMS Values to control
execution order.
Plausibility is a helper module only:
it validates and qualifies data but does not control :ref:`BMS_APPLICATION`
states and does **not** switch contactors.

Validation Responsibilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The module covers:

- current measurement validation with timeout and timestamp checks,
- power measurement validation with optional fallback from current and voltage,
- string-voltage validation with source plausibility checks and fallback logic,
- battery-voltage aggregation from valid string voltages,
- high-voltage-bus validation based on connected strings and freshness,
- spread checks for cell voltage and cell temperature.

Most routines update validity flags in the associated database structures and
emit diagnostic events to track timeout, plausibility, and error conditions.

Cooperation with BMS Values and BMS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The interaction is intentionally layered:

1. Low-level drivers publish measurements to the database.
2. BMS Values calls plausibility functions to validate and derive values.
3. BMS Values writes validated pack and min/max values.
4. BMS state machine consumes these values in its control decisions.

This separation ensures that BMS control logic can operate on consistently
validated inputs while keeping validation and diagnostics reusable and
testable.

Further Reading
---------------

- :ref:`BMS_VALUES_APPLICATION`
