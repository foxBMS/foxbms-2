.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _TEMPERATURE_SENSOR_API:

Temperature Sensor API
======================

|foxbms| supports various temperature sensors from different manufacturers as
shown in :ref:`SUPPORTED_TEMPERATURE_SENSORS`.
This is achieved by drivers that implement the Temperature Sensor Interface
(TSI).

This document describes how the TSI works.

An example how to implement a new TSI compatible driver is shown in
:ref:`HOW_TO_IMPLEMENT_A_NEW_TEMPERATURE_SENSOR_DRIVER`.

Configuration
-------------

The TSI is configured through the ``bms.json`` described in
:numref:`BMS_APPLICATION`. As of now it is only possible to configure one
temperature sensor implementation. The configuration is specified as in
:numref:`tsi-bms-json`.

.. code-block:: json
   :linenos:
   :emphasize-lines: 4,5,6
   :caption: snippet from ``bms.json`` specifying the TSI
   :name: tsi-bms-json

    {
        "slave-unit": {
            "temperature-sensor": {
                "manufacturer": "epcos",
                "model": "b57251v5103j060",
                "method": "polynomial"
            }
        }
    }

The key `manufacturer` describes the manufacturer of the temperature sensor.
The key `model` describes the exact part number of the temperature sensor.
The key `method` describes the implementation that is used in order to
calculate the temperature of the sensor. While this is dependent of the
implementation typical values of this key are `lookup-table` and `polynomial`.

Usage
-----

The TSI exposes the function ``TSI_GetTemperature`` on its public API that
allows to calculate a temperature from a voltage measurement value.
Through the configuration file this function uses internally the specified
temperature sensor implementation.

Internal implementation
-----------------------

Internally this variable implementation is done by conditionally compiling and
linking the library that is specified through the configuration file.
The values of the specified keys are used to resolve the path to the
implementation. The TSI implementations are sorted in a directory structure
like ``manufacturer/model/manufacturer_model_method.c``.
