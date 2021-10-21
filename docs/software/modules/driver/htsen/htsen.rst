.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HUMIDITY_TEMPERATURE_SENSOR_MODULE:

Humidity/Temperature Sensor Module
==================================

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/htsen/htsen.c`` (`API <../../../../_static/doxygen/src/html/htsen_8c.html>`__, `source <../../../../_static/doxygen/src/html/htsen_8c_source.html>`__)
- ``src/app/driver/htsen/htsen.h`` (`API <../../../../_static/doxygen/src/html/htsen_8h.html>`__, `source <../../../../_static/doxygen/src/html/htsen_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/htsen/test_htsen.c`` (`API <../../../../_static/doxygen/tests/html/test__htsen_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__pex_8c_source.html>`__)

Detailed Description
--------------------

The driver addresses the Sensirion SHT35-DIS I2C humidity/temperature sensor.

It triggers a humidity and temperature measurement and reads the result.
It checks the read CRC value: if it does not match the one that the
driver computes, the measurement value is discarded.
The result is stored in the corresponding database entry.
The driver does not use clock stretching to avoid problems on the
|I2C| bus.
