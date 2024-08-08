.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _DIAGNOSIS_MODULE:

Diagnosis Module
================

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/diag/diag.c``
- ``src/app/engine/diag/diag.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/engine/config/diag_cfg.c``
- ``src/app/engine/config/diag_cfg.h``
- ``src/app/engine/diag/cbs/diag_cbs_can.c``
- ``src/app/engine/diag/cbs/diag_cbs_contactor.c``
- ``src/app/engine/diag/cbs/diag_cbs_current.c``
- ``src/app/engine/diag/cbs/diag_cbs_current-sensor.c``
- ``src/app/engine/diag/cbs/diag_cbs_deep-discharge.c``
- ``src/app/engine/diag/cbs/diag_cbs_dummy.c``
- ``src/app/engine/diag/cbs/diag_cbs_interlock.c``
- ``src/app/engine/diag/cbs/diag_cbs_afe.c``
- ``src/app/engine/diag/cbs/diag_cbs_plausibility.c``
- ``src/app/engine/diag/cbs/diag_cbs_power-measurement.c``
- ``src/app/engine/diag/cbs/diag_cbs_sbc.c``
- ``src/app/engine/diag/cbs/diag_cbs_sys-mon.c``
- ``src/app/engine/diag/cbs/diag_cbs_temperature.c``
- ``src/app/engine/diag/cbs/diag_cbs_voltage.c``
- ``src/app/engine/diag/cbs/diag_cbs_aerosol-sensor.c``

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/config/test_diag_cfg.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_aerosol-sensor.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_afe.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_can.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_contactor.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_current.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_current-sensor.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_deep-discharge.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_dummy.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_interlock.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_plausibility.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_power-measurement.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_sbc.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_sys-mon.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_temperature.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_voltage.c``
- ``tests/unit/app/engine/diag/test_diag.c``

Detailed Description
--------------------

A detailed description on how to use the diagnosis module can be found in
:ref:`HOW_TO_USE_THE_DIAGNOSIS_MODULE`.

The diagnosis module is implemented as a central table
``diag_diagnosisIdConfiguration`` that maps the entries of ``DIAG_ID_e`` to
callbacks.
Whenever the diagnosis handler is called in the application code the
diagnosis module selects based on this table the appropriate callback.
It is in the responsibility of the callback to handle the reported issue.

In most implementations the callback sets according to the reported issue an
entry in the database for convenient use of error conditions.
An example is the CAN module that reports set error conditions from the
relevant tables in the database.
These entries are set from the callbacks of the diagnosis module.

In addition to the callbacks, several parameters of the occurring issue can be
set.
As an example the table configures whether an diagnosis entry has a fatal
severity.
The diagnosis module makes convenience functions available that allow for
scanning for fatal errors in order to react on any of these issues.
