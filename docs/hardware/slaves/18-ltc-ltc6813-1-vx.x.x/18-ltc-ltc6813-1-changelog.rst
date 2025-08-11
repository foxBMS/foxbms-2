.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE:

Changelog for the |ltc-ltc6813-1|\ -based 18 cell |bms-slave|
=============================================================

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_6:

v1.1.6
------

- Add new design variant "automotive_single-AFE" with only primary AFE
  populated.
- Update schematic design with new template.
- Add PNP data to release.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_5:

v1.1.5
------

- Improve EMI layout.
- Adapt component variants to other changes.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_4:

v1.1.4
------

- Replace DC/DC converter power inductor with AEC-Q compliant one.
- Add circuit for switching off DC/DC converters in LTC sleep mode.
- Add pull-ups on all GPIOs of the LTCs.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_3:

v1.1.3
------

- Cleanup schematic.
- Improve fonts and sizes on PCB.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_2:

v1.1.2
------

- Replace ACPL-247 with ACPL-217 optocoupler in order to be able to use
  automotive components.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_1:

v1.1.1
------

- Replace port expander with TCA6408APWR (automotive).
- Replace analog buffer opamp with AD8628ARTZ-R2 (automotive).
- Replace DC/DC buck controller with LM5161QPWPRQ1 (automotive).

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_1_0:

v1.1.0
------

- Port schematics and layout to Altium Designer.
- Create hierarchical design.
- Introduce minor improvements to design.
- Replace linear regulation (PNP transistor) for LTC power supply with DC/DC
  converters.
- Improve isolation distances between external DC/DC converter supply and
  battery module signals.
- Add 8-24 V isolated external power supply.
- Replace I2C EEPROM 24AA02UID with M24M02-DR (ECC).
- Replace isoSPI transformers HX1188NL with HM2102NL.
- Reduce balancing resistors from 2x 68 Ohm to 2x 130 Ohm due to shrunk
  cooling areas.
- Add layermarker on PCB.
- Change primary software timer to be now switched on by default.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_0_1:

v1.0.1
------

- Replace all LTC1380 multiplexer with ADG728 multiplexer.

.. _CHANGELOG_FOR_THE_LTC_LTC6813_1_BASED_18_CELL_BMS_SLAVE_V1_0_0:

v1.0.0
------

- Initial release.
