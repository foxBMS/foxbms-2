.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER:

Changelog for the |ti-tms570|\ -based |bms-master|
==================================================


.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_2_3:

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_2_3_0:

v1.2.3-0
--------

- Fix wiring of U6601 in schematic and layout.
- Fix vendor number of Y6100 (SiT1618BA-32-33N-20.000000) in BOM.
- Include a suffix in the version declaration.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_2_2:

v1.2.2
------

- Update RTC INTA output circuitry to work independent of backup battery
  presence.
- Add design variant "production" as primary configuration for board
  production.
- Add SBC config to release file (check NOTICE).

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_2_1:

v1.2.1
------

- Update RTC INTA output circuitry for compatibility with higher supply
  voltages.
- Rearrange wiring of SBC wake signals: only ignition signal on WAKE1, RTC and
  CAN1 on WAKE2.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_2_0:

v1.2.0
------

- Optimize design  by removing the second SBC (peripheral SPC), now
  featuring only a single SBC.
- Fix wiring of Ethernet PHY in schematic and layout.
- Control of CAN1 enable and !standby signals directly from MCU instead of port
  expander to increase compatibility with upcoming CAN bootloader.
- Update schematic comment related to SBC debug pulse length.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_1_5:

v1.1.5
------

- Fix Farnell order code for 100nF/50V/0402.
- Update project BOM.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_1_4:

v1.1.4
------

- Improve SBC wake-up pulse generation circuit by changing value of R7233 to
  470kOhm.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_1_3:

v1.1.3
------

- Fix swapped INTA/INTB pinout of U6400 (PCF2131TF).
- Add OSHWA logo on PCB.
- Modify power MOSFET and according control circuitry (Q4400-Q4402) for
  switching external insulation monitor (current rating of previous design was
  too weak).

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_1_2:

v1.1.2
------

- Fix assignment of memory devices (MRAM, FRAM, FLASH) to SPI3 chip select
  pins (net labels were swapped).
- Split oscillator (Y6100) into two different part numbers and BOM items (one
  part for 20 MHz and 25 MHz).
- Add voltage limiting Zener diodes to gates of Q6401 and Q6404.
- Add 100 |nF| capacitor to "ON" pin of U7300 in order to improve transient
  immunity.
- Fix faulty solder paste layer on component U7100.
- Replace Ethernet socket J2001 due to availability issues (old part number:
  Wuerth 615008185121, new part number: Wuerth 615008137421).

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_1_1:

v1.1.1
------

- Add clamping diodes for contactor feedback resistor dividers in order to
  protect inputs of port expander.
- Add clamping diodes to insulation monitor PWM and OK inputs.
- Adjust contactor connectors pinout to match foxBMS 1 Master pinout.
- Replace R8014 and R8017 10k resistors with 10R resistors due to amplifier
  gain error.
- Add placement option to use ADUM6401 as a digital isolator for the CAN2
  interface.
- Fix a typo in EMIF connector silk screen.
- Improve free space around debug connector (debug connector was moved towards
  upper edge of board).
- Change global polygon connect style to relief connect in order to improve
  solderabilty of through-hole connectors.
- Fix solder paste areas for Mictor debug connector.
- Modify all mounting holes to fit Wuerth SMT spacers (outer diameter = 6mm).
- Remove interlock high-side switch from default component variant due to
  availability issues.
- Add hardware board version coding using IO1_0 to IO1_7 of port-expander1.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_1_0:

v1.1.0
------

- Replace CAN transceiver with TCAN1043HDQ1 (SOIC-14 package) due to
  availability issues.
- Add battery holder for CR1225 RTC battery (BR2470 battery is now optional).
- Change value of all I2C pull-up resistors from 5.1 |kOhm| to 2.49 |kOhm|.
- Remove buck-boost converter IC from "default" component variant due to
  availability issues.
- Remove Ethernet PHY IC from "default" component variant due to availability
  issues.
- Finalize PCB layout.
- Add mechanical dimensions to draftman document.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_0_2:

v1.0.2
------

- Add digital isolator with integrated power for CAN2 interface.
- Add connectors for interface and extension boards.
- Add bypass for buck-boost converter.
- Split board supply into clamp 30 and clamp 30C.
- Add additional protection circuit for clamp 30C supply input.
- Remove isolated USB interface.
- Add test points for interlock circuit.

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_0_1:

v1.0.1
------

- Correct the supply of V:sub:`dd` of the smart power switch (from 12V to 5V).

.. _CHANGELOG_FOR_THE_TI_TMS570_BASED_BMS_MASTER_V1_0_0:

v1.0.0
------

- Initial version.
