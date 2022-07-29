.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _DIAGNOSIS_MODULE:

Diagnosis Module
================

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/diag/diag.c`` (`API <../../../../_static/doxygen/src/html/diag_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag_8c_source.html>`__)
- ``src/app/engine/diag/diag.h`` (`API <../../../../_static/doxygen/src/html/diag_8h.html>`__, `source <../../../../_static/doxygen/src/html/diag_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/engine/config/diag_cfg.c`` (`API <../../../../_static/doxygen/src/html/diag__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cfg_8c_source.html>`__)
- ``src/app/engine/config/diag_cfg.h`` (`API <../../../../_static/doxygen/src/html/diag__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/diag__cfg_8h_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_can.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__can_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__can_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_contactor.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__contactor_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__contactor_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_current.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__current_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__current_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_current-sensor.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__current-sensor_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__current-sensor_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_deep-discharge.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__deep-discharge_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__deep-discharge_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_dummy.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__dummy_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__dummy_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_interlock.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__interlock_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__interlock_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_afe.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__afe_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__afe_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_plausibility.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__plausibility_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__plausibility_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_power-measurement.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__power-measurement_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__power-measurement_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_sbc.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__sbc_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__sbc_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_sys-mon.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__sys-mon_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__sys-mon_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_temperature.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__temperature_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__temperature_8c_source.html>`__)
- ``src/app/engine/diag/cbs/diag_cbs_voltage.c`` (`API <../../../../_static/doxygen/src/html/diag__cbs__voltage_8c.html>`__, `source <../../../../_static/doxygen/src/html/diag__cbs__voltage_8c_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/config/test_diag_cfg.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cfg_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cfg_8c_source.html>`__)
- ``tests/unit/app/engine/diag/test_diag.c`` (`API <../../../../_static/doxygen/tests/html/test__diag_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_can.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__can_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__can_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_contactor.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__contactor_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__contactor_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_current.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__current_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__current_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_current-sensor.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__current-sensor_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__current-sensor_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_deep-discharge.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__deep-discharge_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__deep-discharge_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_dummy.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__dummy_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__dummy_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_interlock.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__interlock_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__interlock_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_afe.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__afe_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__afe_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_plausibility.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__plausibility_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__plausibility_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_power-measurement.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__power-measurement_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__power-measurement_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_sbc.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__sbc_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__sbc_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_sys-mon.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__sys-mon_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__sys-mon_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_temperature.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__temperature_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__temperature_8c_source.html>`__)
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_voltage.c`` (`API <../../../../_static/doxygen/tests/html/test__diag__cbs__voltage_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__diag__cbs__voltage_8c_source.html>`__)

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
