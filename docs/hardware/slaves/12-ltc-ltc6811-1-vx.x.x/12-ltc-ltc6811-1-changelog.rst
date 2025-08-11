.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE:

Changelog for the |ltc-ltc6811-1|\ -based 12 cell |bms-slave|
=============================================================

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_7:

v2.1.7
------

- Modify component designators to be compatible with 18cell versions.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_6:

v2.1.6
------

- Improve EMI (layout).
- Add RC filters on NTC sensor inputs.
- Add DC/DC converters for 5V LTC supplies.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_5:

v2.1.5
------

- Replace opamps, port expanders and optocouplers with AEC-Q100 compliant ones.
- Modify silkscreen texts.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_4:

v2.1.4
------

- Change primary software timer to be now switched on by default.
- Add layermarker on PCB.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_3:

v2.1.3
------

- Replace EOL port expander with PCF8574.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_2:

v2.1.2
------

- Port schematics and layout to Altium Designer.
- Create hierarchical design.
- Introduce minor improvements to design.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_1:

v2.1.1
------

- Improved isolation distances between external DC/DC converter supply and
  battery module signals.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_1_0:

v2.1.0
------

- Add 8-24 V isolated external power supply.
- Replace I2C EEPROM 24AA02UID with M24M02-DR (ECC).
- Switch connection of balancing resistors from serial to parallel.
- Replace isoSPI transformers HX1188NL with HM2102NL.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_0_3:

v2.0.3
------

- Fix isoSPI transformer CMC issue.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_0_2:

v2.0.2
------

- Replace LTC1380 multiplexer with ADG728 (400 kHz I2C).
- Adjust connection of 100 ohm resistors for V+/V_REG supply.
- Reduce value of I2C pull-up resistors to 1k2.

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_0_1:

v2.0.1
------

- Add missing cooling areas on bottom side, adjusted silk screen.
- Enlarge PCB tracks, R201/202/301/302 other package.
- Enlarge T201/301 cooling area.
- Replace PCF8574 with PCA8574 (400 kHz I2C).

.. _CHANGELOG_FOR_THE_LTC_LTC6811_1_BASED_12_CELL_BMS_SLAVE_V2_0_0:

v2.0.0
------

- Initial release.
