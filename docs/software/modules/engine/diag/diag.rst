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

A configurable debounce counter is implemented for the monitoring of all
diagnosis entries, to avoid unintentional jitter of fault/good conditions.
The individual configuration for each monitored parameter is responsible if and
how the BMS reacts on a violation of the limit value.
The following parameters have to be configured for the individual warnings and
errors:

- The **Response time/Threshold** for which the error condition needs to be
  present permanently before the according flag is set.
  The flag may be configured to raise on the first occurrence or after multiple
  violations in order to avoid false alarms, e.g. resulting from short-term
  current peaks.
- The BMS decides depending on the configured **Severity** how a raised flag is
  treated.
  Flags can be either configured with a **Info**, **Warning** or a **Fatal
  Error** severity.
  An informative or warning flag will result in no direct action from the BMS,
  whereas an error flag will lead to an opening of the contactors.
- The configured **Delay time** defines the time between raising an error flag
  and switching the BMS to **Error State**, i.e., that means an opening of the
  contactors.

Whenever the diagnosis handler is called in the application code, the
diagnosis module selects based on its configuration table the appropriate
callback if the threshold level is reached or reset again.
It is in the responsibility of the callback to handle the reported issue, in
addition to the raised/cleared flag in the diagnosis module.

In most implementations the callback sets according to the reported issue a
flag in the database for convenient use of error conditions.
An example is the CAN module that reports set error conditions from the
relevant tables in the database.
These entries are set from the callbacks of the diagnosis module.

In addition to the callbacks, several parameters of the occurring issue can be
set.
As an example the table configures whether an diagnosis entry has a fatal
severity.
The diagnosis module makes convenience functions available that allows
scanning for fatal errors in order to react on any of these issues.
