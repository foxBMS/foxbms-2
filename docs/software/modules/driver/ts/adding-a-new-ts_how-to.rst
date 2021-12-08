.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_IMPLEMENT_A_NEW_TEMPERATURE_SENSOR_DRIVER:

How to Implement a New Temperature Sensor Driver
================================================

This section gives a guideline how add a new temperature sensor to the project.

The Example
-----------

This example shows adding the TEMP123 sensor from Great Sensor Corp.
(Great-Sensor) to the project.
The sensor is a NTC thermistor with a resistance value of 10k at room
temperature.
The steps of adding the sensor contain:

  * explanation of the directory structure,
  * adding the sensor to the documentation,
  * adding the sensor to the codebase (with lookup table and polynomial),
  * adding the sensor to the unit tests,
  * making known the relevant paths to |code| and
  * using the sensor through ``bms.json``.

Basic Directory Structure
-------------------------

#. Adapt ``conf/bms/schema/bms.schema.json`` to recognize the new manufacturer
   and the temperature sensor.
#. Add the *Great-Sensor* manufacturer directory ``great-sensor``:
   ``src/app/driver/ts/great-sensor``.
#. Add the temperature sensor ``temp123`` directory:
   ``src/app/driver/ts/great-sensor/temp123``.

Example: For the mentioned temperature sensor the directory structure
is as follows:

.. code-block::

   src
   └── app
       └── src
           └── driver
               ├── ts
               │   └── great-sensor
               │       └── temp123
               │           ├── lookup-table
               │           │   └── great-sensor_temp123_lookup-table.c # actual lookup-table implementation
               │           ├── polynomial
               │           │   └── great-sensor_temp123_polynomial.c   # actual polynomial implementation
               │           ├── great-sensor_temp123.c                  # wrapper-implementation that exposes the sensor to the BMS
               │           └── great-sensor_temp123.h                  # header file for the wrapper implementation
               └── wscript                                             # register the new files in this wscript

Extending the documentation
---------------------------

The documentation (apart from the |doxygen| documentation that is handled in
each source and header file) is contained in :ref:`SUPPORTED_TEMPERATURE_SENSORS`.
In order to add a new temperature sensor, create a ``RST`` file for the sensor
similar to the existing files and link it in the referenced document.
Additionally, assign a short name for the temperature sensor and add it to
``ts-short-names.csv``.
This short name is used by convention in the function calls of the actual
sensor implementation in order to avoid collisions with other sensor
implementations.

Implementing the sensor
-----------------------

There are two common options for implementing the temperature calculation of
a NTC or PTC:

a. Using a lookup-table or
b. calculating the temperature with a polynomial function.

Both methods are based on the actual sensor parameters that have to be
supplied by the manufacturer.
Often, the manufacturer supplies additional tools or tables in the data sheet
that allow to calculate both methods easily.

For the lookup-table, a lookup-table containing the relation between
temperature and resistance of the sensor has to be generated and stored in the
file implementing it.
It the responsibility of the implementation to make sure that

#. interpolation of values between two lookup-table entries,
#. handling of input values outside the range of the lookup-table and
#. general error-checking of inputs and outputs

is implemented correctly.

The polynomial implementation has to calculate the correct temperature from
the measured voltage and uses for this purpose a set of pre-calculated
polynomial coefficients.

The same rules as for the lookup-table considering the robust implementation
of this function apply also to the polynomial implementation.

A temperature sensor does not have to implement both methods.
In the case that one of the methods is not implemented the missing function
must execute ``FAS_ASSERT(FAS_TRAP);`` when being called in order to make sure
that this incorrect configuration is uncovered during debugging.

Implementing the unit tests
---------------------------

The unit tests should be implemented similar to the existing unit tests for
the other sensors.
Important points to consider are:

#. Testing actual values,
#. testing the range of the implementation and
#. testing out of range input.

As the unit test build system discovers the files to be tested independently of
the main build system, special care has to be taken that the correct source
files are attracted.
This mainly applies to the implementations of the lookup-table and polynomial
function as they do not have a separate header file.
Use ``TEST_FILE("good-sensor_temp123_polynomial.c")`` in the test file in order
to hint the correct source file to the unit test build system.

Updating the IDE configuration
------------------------------

In order to be able to discover the new files in |code|, it is necessary to
add the path to new sensor to the ``env`` node in
``tools/ide/vscode/c_cpp_properties.json.jinja2`` and rerun the ``configure``
step of the build system.
For the aforementioned example the following line would have to be added:
``"${workspaceFolder}/src/app/driver/ts/good-sensor/temp123",``.

Using the sensor
----------------

In order to use the new temperature sensor it has to be configured in
``bms.json``.
For the example the configuration would have to be:

.. code-block::

  "temperature-sensor": {
    "manufacturer": "good-sensor",
    "model": "temp123",
    "method": "lookup-table"
  }

After this, the temperature sensor is exposed through the
:ref:`temperature_sensor_api`.
