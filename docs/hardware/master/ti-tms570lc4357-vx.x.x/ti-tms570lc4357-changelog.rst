.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _CHANGELOG_FOR_MASTER_TMS570:

Changelog for Master TMS570
===========================

.. _CHANGELOG_FOR_MASTER_TMS570_V1_1_5:

v1.1.5
------

- Fixed Farnell order code for 100nF/50V/0402
- Updated project BOM

.. _CHANGELOG_FOR_MASTER_TMS570_V1_1_4:

v1.1.4
------

- Improved SBC wake-up pulse generation circuit by changing value of R7233 to 470kOhm

.. _CHANGELOG_FOR_MASTER_TMS570_V1_1_3:

v1.1.3
------

- fixed swapped INTA/INTB pinout of U6400 (PCF2131TF)
- added OSHWA logo on PCB
- modified power MOSFET and according control circuitry (Q4400-Q4402) for
  switching external insulation monitor (current rating of previous design was
  too weak)

.. _CHANGELOG_FOR_MASTER_TMS570_V1_1_2:

v1.1.2
------

- fixed assignment of memory devices (MRAM, FRAM, FLASH) to SPI3 chip select
  pins (net labels were swapped)
- split oscillator (Y6100) into two different part numbers and BOM items (one
  part for 20 MHz and 25 MHz)
- added voltage limiting Zener diodes to gates of Q6401 and Q6404
- added 100 |nF| capacitor to "ON" pin of U7300 in order to improve transient
  immunity
- fixed faulty solder paste layer on component U7100
- replaced Ethernet socket J2001 due to availability issues (old part number:
  Wuerth 615008185121, new part number: Wuerth 615008137421)


.. _CHANGELOG_FOR_MASTER_TMS570_V1_1_1:

v1.1.1
------

- added clamping diodes for contactor feedback resistor dividers in order to
  protect inputs of port expander
- added clamping diodes to insulation monitor PWM and OK inputs
- adjusted contactor connectors pinout to match foxBMS 1 Master pinout
- replaced R8014 and R8017 10k resistors with 10R resistors due to amplifier
  gain error
- added placement option to use ADUM6401 as a digital isolator for the CAN2
  interface
- fixed a typo in EMIF connector silk screen
- improved free space around debug connector (debug connector was moved towards
  upper edge of board)
- changed global polygon connect style to relief connect in order to improve
  solderabilty of through-hole connectors
- fixed solder paste areas for Mictor debug connector
- modified all mounting holes to fit Wuerth SMT spacers (outer diameter = 6mm)
- removed interlock high-side switch from default component variant due to
  availability issues
- added HW board version coding using IO1_0 to IO1_7 of port-expander1

.. _CHANGELOG_FOR_MASTER_TMS570_V1_1_0:

v1.1.0
------

- replaced CAN transceiver with TCAN1043HDQ1 (SOIC-14 package) due to
  availability issues
- added battery holder for CR1225 RTC battery (BR2470 battery is now optional)
- changed value of all I2C pull-up resistors from 5.1 |kOhm| to 2.49 |kOhm|
- removed buck-boost converter IC from "default" component variant due to
  availability issues
- removed Ethernet phy IC from "default" component variant due to availability
  issues
- finalized PCB layout
- added mechanical dimensions to draftman document

.. _CHANGELOG_FOR_MASTER_TMS570_V1_0_2:

v1.0.2
------

- added digital isolator with integrated power for CAN2 interface
- added connectors for interface and extension boards
- added bypass for buck-boost converter
- split board supply into clamp 30 and clamp 30C
- added additional protection circuit for clamp 30C supply input
- removed isolated USB interface
- added test points for interlock circuit

.. _CHANGELOG_FOR_MASTER_TMS570_V1_0_1:

v1.0.1
------

- corrected the supply of Vdd of the smart power switch (from 12V to 5V)

.. _CHANGELOG_FOR_MASTER_TMS570_V1_0_0:

v1.0.0
------

- initial version
