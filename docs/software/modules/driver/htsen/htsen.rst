.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HUMIDITY_TEMPERATURE_SENSOR_MODULE:

Humidity/Temperature Sensor Module
==================================

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/htsen/htsen.c``
- ``src/app/driver/htsen/htsen.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/htsen/test_htsen.c``

Detailed Description
--------------------

The driver addresses the Sensirion SHT35-DIS I2C humidity/temperature sensor.

It triggers a humidity and temperature measurement and reads the result.
It checks the read CRC value: if it does not match the one that the
driver computes, the measurement value is discarded.
The result is stored in the corresponding database entry.
The driver does not use clock stretching to avoid problems on the
|I2C| bus.
