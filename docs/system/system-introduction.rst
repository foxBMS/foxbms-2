.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _SYSTEM_INTRODUCTION:

System Introduction
===================





Conditions that lead to a transition to `ERROR` state
=====================================================

Logging after 20th event for errors connected related to the contactor
feedback. This value is chosen to be so large because of the time delay
between the request for a state and the actual physical response. It is
caused by the SPI transaction to the SPS module, the rise time of the
control signal and the actual opening/closing of the contactor. Only then
can the feedback be read correctly, which also takes some additional time
depending on the selected feedback source.


.. table:: Diagnosis entries with severity: ERROR
   :name: diagnosis-entries-errors
   :widths: grid

   +--------------------------------------------------+---------------------+------------------+
   | Diagnosis Entry                                  | Response time in ms | Delay time in ms |
   +==================================================+=====================+==================+
   | DIAG_ID_INTERLOCK_FEEDBACK                       | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST            | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL             | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL            | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL          | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL       | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL         | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL      | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL              | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL           | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL            | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL         | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL           | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL              | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_ON_OPEN_STRING                   | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_AFE_SPI                                  | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_AFE_COMMUNICATION_INTEGRITY              | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_AFE_MUX                                  | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_AFE_CONFIG                               | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CAN_TIMING                               | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_CC_RESPONDING             | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_EC_RESPONDING             | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_RESPONDING                | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_SBC_RSTB_ERROR                           | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_STRING_MINUS_CONTACTOR_FEEDBACK          | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK             | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_DEEP_DISCHARGE_DETECTED                  | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT              | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_MEASUREMENT_ERROR                | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT    | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_V2_MEASUREMENT_TIMEOUT    | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT    | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_POWER_MEASUREMENT_ERROR                  | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
   | DIAG_ID_ALERT_MODE                               | to be added         | to be added      |
   +--------------------------------------------------+---------------------+------------------+
