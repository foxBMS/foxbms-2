.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_REACH_STANDBY_STATE:

How to reach STANDBY state
==========================

In *STANDBY* state, the BMS is monitoring and controlling the balancing,
recalibrates the |soc| and is waiting for a state request to close the
contactors.

Following conditions MUST be met, that the BMS switches into *STANDBY* state:

#. No error from a diagnosis entry with a severity of ``DIAG_FATAL_ERROR``
   defined in ``src/app/engine/config/diag_cfg.c`` MUST be detected.
#. Depending on the configuration of *BS_CHECK_CAN_TIMING* in file
   ``src/app/application/config/battery_system_cfg.c`` the state request
   message **foxBMS_BmsStateRequest** MUST be received periodically via CAN.
#. **STANDBY** state MUST be requested via signal **foxBMS_modeRequest** in
   message **foxBMS_BmsStateRequest**.

How to recalibrate the SOC
==========================

The |soc| is automatically recalibrated by the BMS in STANDBY state if the
battery system is in a rest phase (**BMS_AT_REST**).
This state is reached if for duration of **BS_RELAXATION_PERIOD_10ms**, the
current drawn from or supplied to the battery system is below
**BS_REST_CURRENT_mA**.
The default values for these defines are 200 |mA| and 600000 |ms|.
This means that with the default configuration, the battery system will enter
the rest phase if for 10 minutes no current above 200 |mA| is drawn from or
supplied to the battery system.
Both defines are configured in file
``src/app/application/config/battery_system_cfg.c`` the system will begin the
relaxation phase (**BMS_RELAXATION**).


.. _HOW_TO_REINITIALIZE_NVRAM:

How to reinitialize NVRAM
=============================

The NVRAM (e.g., FRAM) can be reinitialized with the ``f_Debug`` CAN message
by setting the value for ``InitializeFram`` to one.
(See :ref:`COMMUNICATION` under the ``Debug`` section)


.. _HOW_TO_RESET_DEEP_DISCHARGE_FLAG:

How to reset deep-discharge flag
================================

The deep-discharge flag is a non-volatile flag that is stored in the FRAM to
prevent the battery system from engaging after a deep-discharge event has been
detected. The BMS will not allow a closing of the contactors until this cell is
replaced. To reset the flag, signal ``ResetPersistentFlags`` in CAN message
``f_BmsStateRequest`` must be sent to the BMS.

.. warning::
   Resetting the flag carelessly without replacing the discharged cell poses a
   significant safety risk!
