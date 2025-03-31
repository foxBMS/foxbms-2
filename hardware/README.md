# Hardware

The hardware designs of foxBMS 2 can be found at the following address:
<https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release>

## Hardware Versioning

The foxBMS 2 hardware uses basically the same versioning pattern as the main
foxBMS repository, i.e, this repository, with a slight modification. The basic versioning pattern is
``MAJOR.MINOR.PATCH``
where ``MAJOR``, ``MINOR`` and ``PATCH`` are numbers.

However, sometimes it is necessary to fix things in the Altium projects of a
hardware that are not related to the schematic or the layout and there it
does not make sense to bump any of the ``MAJOR``, ``MINOR`` and ``PATCH`` are
numbers, as the ``MAJOR.MINOR.PATCH`` version string is printed on the
hardware.
For that reason, a suffix is added to the version.

The full version identifier of a hardware release is therefore
``MAJOR.MINOR.PATCH-SUFFIX`` where ``SUFFIX`` is a number.

This README always points to a generic link of the latest version  of a
released hardware, i.e., the version with the latest suffix.

## Releases

The hardware design files belonging to this release are:

### BMS-Master

#### TI TMS570LC4357-based BMS-Master

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| TI BMS-Master                 | 1.2.3     | [TI TMS570LC4357 Master v1.2.3](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/master-v1.2.3.latest.zip)                         |

### BMS-Interfaces

#### LTC-based BMS-Interfaces

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| LTC Interface (LTC6820)       | 1.0.3     | [LTC LTC6820 Interface v1.0.3](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/interface-ltc-ltc6820-v1.0.3.latest.zip)           |

#### NXP-based BMS-Interfaces

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| NXP Interface (MC33664)       | 1.0.2     | [NXP MC33664 Interface v1.0.2](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/interface-nxp-mc33664-v1.0.2.latest.zip)           |

#### MAXIM-based BMS-Interfaces

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| MAXIM Interface (MAX17841b)   | 1.0.0     | [MAXIM MAX17841b Interface v1.0.0](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/interface-maxim-max17841b-v1.0.0.latest.zip)   |

### BMS-Slaves

#### LTC-based BMS-Slaves

##### 12 Cells LTC-based BMS-Slaves

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| LTC 12 Cell Slave (LTC6811-1) | 2.1.5     | [LTC LTC6811-1 12 Cell Slave v2.1.5](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/slave-12-ltc-ltc6811-1-v2.1.5.latest.zip)    |
| LTC 12 Cell Slave (LTC6811-1) | 2.1.7     | [LTC LTC6811-1 12 Cell Slave v2.1.7](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/slave-12-ltc-ltc6811-1-v2.1.7.latest.zip)    |

##### 18 Cells LTC-based BMS-Slaves

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| LTC 18 Cell Slave (LTC6813-1) | 1.1.3     | [LTC LTC6813-1 18 Cell Slave v1.1.3](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/slave-18-ltc-ltc6813-1-v1.1.3.latest.zip)    |
| LTC 18 Cell Slave (LTC6813-1) | 1.1.5     | [LTC LTC6813-1 18 Cell Slave v1.1.5](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/slave-18-ltc-ltc6813-1-v1.1.5.latest.zip)    |
| LTC 18 Cell Slave (LTC6813-1) | 1.1.6     | [LTC LTC6813-1 18 Cell Slave v1.1.6](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/slave-18-ltc-ltc6813-1-v1.1.6.latest.zip)    |

#### NXP-based BMS-Slaves

#### 14 Cells NXP-based BMS-Slaves

| Item                          | Version   | File                                                                                                                                                  |
|-------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| NXP 14 Cell Slave (MC33775A)  | 1.0.3     | [NXP MC33775A 14 Cell Slave v1.0.3](https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/hardware/release/slave-14-nxp-mc33775a-v1.0.3.latest.zip)      |

#### MAXIM-based BMS-Slaves

TODO
