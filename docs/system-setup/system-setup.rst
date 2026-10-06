.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _SYSTEM_SETUP:

############
System Setup
############

The following sections describe the actual system setup of a battery system
using |foxBMS|.

.. warning::

   This is a general description of the system setup.
   The actual system setup is specific to each battery system and will vary.
   It is the responsibility of the user to adapt the system setup to their
   specific battery system and use case.

The system assumes a single-string battery system.

******************
|bms-slaves| Setup
******************

The |bms-interface| is only used for the connection between the |bms-master|
and the lowest |bms-slave|.
The |bms-slaves| are connected in daisy chain from the lowest battery module in
the system to the highest battery module in the system.

.. note::

   All wiring between the |bms-interface| and the |bms-slaves|, as well as
   between the |bms-slaves|, **SHALL** be done using **twisted pair cables**
   to ensure signal integrity.
   The cables **SHALL** be kept as short as possible to minimize signal
   degradation.

The following describes the connection of the |bms-slaves| for a single string
battery system.

.. note::

   For these |bms-slave| pinouts see
   :ref:`pinout-overview-for-18-ltc-ltc6813-1-v1.1.3_pinout-table`,
   :ref:`pinout-overview-for-12-ltc-ltc6811-1-v2.1.6_pinout-table`, or
   :ref:`pinout-overview-for-14-nxp-mc33775a-v1.0.3_pinout-table`
   depending on the used |bms-slave|.

|adi| or |ltc|\ -based Daisy Chain
==================================

- Connect |isospi1| of the |bms-interface| with X500 of the first |bms-slave|.
- Connect X501 of the first |bms-slave| with X500 of the second |bms-slave|.
- Connect X501 of the second |bms-slave| with X500 of the third |bms-slave|.
- and so on ...

|nxp|\ -based Daisy Chain
=========================

- Connect |tpl1| of the |bms-interface| with |tpl-high| of the first
  |bms-slave|.
- Connect |tpl-low| of the first |bms-slave| with |tpl-high| of the second
  |bms-slave|.
- Connect |tpl-low| of the second |bms-slave| with |tpl-high| of the third
  |bms-slave|.
- and so on ...

*****************************
System Value Monitoring Setup
*****************************

Setting up current and voltage sensing at system level is more complex than the
|bms-slaves| setup, as it heavily depends on the system topology and the
configuration of the battery system.

When working with the default BMS state machine implemented in
``src/applications/bms/``, the following values need to be measured at system
level.

See also :ref:`SYSTEM_VOLTAGE_AND_CURRENT_MONITORING` for more details.

Current
=======

The current sensor **SHALL** be placed in the negative main current path of the
battery system, between the lowest battery module and

- the |msd| (if existing) or
- before the main negative contactor of the battery system.

The current direction **SHALL** then be configured accordingly in the software
configuration of the system.

Voltage
=======

The default BMS state machine implementation requires |pack-voltage| and the
|system-voltage| to be measured.
The implementation is then depending on the used current sensor.

See also :ref:`MAPPING_OF_DOCUMENTED_SYSTEM_TERMS_TO_CODE_NAMES` for more
details.

Isabellenhuette IVT-S
---------------------

This setup requires the CAN messages of the IVT-S to be configured as
implemented in ``src/app/driver/config/can_cfg_rx-message-definitions.h``.

- IVT-S V1 **SHALL** represent the pack voltage and be connected between the
  highest battery module and fuse of the battery system.
  This voltage is stored in the data block ``DATA_BLOCK_SYSTEM_VOLTAGE_1_s``
  and used to derive |pack-voltage-abbr-full|.
- IVT-S V2 is unused.
- IVT-S V3 **SHALL** represent the system voltage and be connected after main
  positive contactor.
  This voltage is stored in the data block ``DATA_BLOCK_SYSTEM_VOLTAGE_3_s``
  and used to derive |system-voltage-abbr-full|.
