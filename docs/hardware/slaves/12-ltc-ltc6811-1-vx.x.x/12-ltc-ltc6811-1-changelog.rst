.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _CHANGELOG_FOR_SLAVE_LTC6811_1:

Changelog for Slave LTC6811-1
=============================

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_7:

v2.1.7
------

- modified component designators to be compatible with 18cell versions

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_6:

v2.1.6
------

- EMI improvements (layout)
- added RC filters on NTC sensor inputs
- added DC/DC converters for 5V LTC supplies

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_5:

v2.1.5
------

- Replaced opamps, port expanders and optocouplers with AEC-Q100 compliant ones
- Modified silkscreen texts

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_4:

v2.1.4
------

- Primary software timer is now switched on by default
- added layermarker on PCB

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_3:

v2.1.3
------

- replaced EOL port expander with PCF8574

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_2:

v2.1.2
------

- ported schematics and layout to Altium Designer
- created hierarchical design
- introduced minor improvements to design

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_1:

v2.1.1
------

- improved isolation distances between external DC/DC converter supply and
  battery module signals

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_1_0:

v2.1.0
------

- added 8-24 V isolated external power supply
- replaced I2C EEPROM 24AA02UID with M24M02-DR (ECC)
- switched connection of balancing resistors from serial to parallel
- replaced isoSPI transformers HX1188NL with HM2102NL

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_0_3:

v2.0.3
------

- fixed isoSPI transformer CMC issue

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_0_2:

v2.0.2
------

- replaced LTC1380 MUXs with ADG728 (400 kHz I2C)
- adjusted connection of 100 ohm resistors for V+/V_REG supply
- reduced value of I2C pull-up resistors to 1k2

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_0_1:

v2.0.1
------

- added missing cooling areas on bottom side, adjusted silk screen
- enlarged PCB tracks, R201/202/301/302 other package
- enlarged T201/301 cooling area
- Replaced PCF8574 with PCA8574 (400 kHz I2C)

.. _CHANGELOG_FOR_SLAVE_LTC6811_1_v2_0_0:

v2.0.0
------

- initial release
