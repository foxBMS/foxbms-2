.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _CHANGELOG_FOR_SLAVE_LTC6813_1:

Changelog for Slave LTC6813-1
=============================

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_1_5:

v1.1.5
------

- EMI layout improvements
- adapted component variants to other changes

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_1_4:

v1.1.4
------

- replaced DC/DC converter power inductor with AEC-Q compliant one
- added circuit for switching off DC/DC converters in LTC sleep mode
- added pull-ups on all GPIOs of the LTCs

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_1_3:

v1.1.3
------

- schematic cleanup, improved fonts and sizes on PCB

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_1_2:

v1.1.2
------

- replaced ACPL-247 with ACPL-217 optocoupler in order to be able to use
  automotive components

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_1_1:

v1.1.1
------

- replaced port expander with TCA6408APWR (automotive)
- replaced analog buffer opamp with AD8628ARTZ-R2 (automotive)
- replaced DC/DC buck controller with LM5161QPWPRQ1 (automotive)

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_1_0:

v1.1.0
------

- ported schematics and layout to Altium Designer
- created hierarchical design
- introduced minor improvements to design
- replaced linear regulation (PNP transistor) for LTC power supply with DC/DC
  converters
- improved isolation distances between external DC/DC converter supply and
  battery module signals
- added 8-24 V isolated external power supply
- replaced I2C EEPROM 24AA02UID with M24M02-DR (ECC)
- replaced isoSPI transformers HX1188NL with HM2102NL
- reduced balancing resistors from 2x 68 Ohm to 2x 130 Ohm due to shrinked
  cooling areas
- added layermarker on PCB
- Primary discharge timer is now switched on by default

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_0_1:

v1.0.1
------

- replaced all LTC1380 MUXs with ADG728 MUXs

.. _CHANGELOG_FOR_SLAVE_LTC6813_1_V1_0_0:

v1.0.0
------

- initial release
