.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _SUPPORTED_TEMPERATURE_SENSORS:

Supported Temperature Sensors
=============================

|foxbms| supports various temperature sensors from different manufacturers
as the list below shows.

These temperature sensors can be used inside the AFE drivers as they
implement the :ref:`TEMPERATURE_SENSOR_API`.

.. include:: ./../../../../../build/docs/supported_temperature_sensors.txt

These temperature sensors have *short names* in order to be able to write
shorter function and variable names. The *short names* are listed in
:numref:`temperature-sensor-short-names`.

.. csv-table:: Temperature sensor short names
   :name: temperature-sensor-short-names
   :header-rows: 1
   :delim: ;
   :file: ./ts-short-names.csv
