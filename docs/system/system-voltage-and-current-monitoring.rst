.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _SYSTEM_VOLTAGE_AND_CURRENT_MONITORING:

#####################################
System Voltage And Current Monitoring
#####################################

Several different voltages and currents inside a battery system need to be
measured.
The exact number of voltages and currents is dependent on the system topology
and the configuration of the battery system.

:numref:`measurements-in-multi-string-system` shows the voltages and currents
that need to measured in a multi-string system, here for example for a 3-string
system.

The left side of the image shows the *Battery Pack*.
The *Battery Junction Box* is depicted in the center of the figure with an
indicated connected *Application* shown on the right side the of the picture.

.. figure:: ../../build/docs/docs/system/img/battery-system-setup-multi-string.svg
   :alt: Voltages and currents to be measured in a multi-string system
   :name: measurements-in-multi-string-system
   :width: 800px

   Voltages and currents of interest in a multi-string system
   (click to enlarge)

.. csv-table:: Color description
   :name: colors-in-system-block-diagram
   :header-rows: 1
   :delim: ;
   :file: ./colors-in-system-block-diagram.csv

*******************
Multi-String System
*******************

The following section describes important parameters within a multi-string
battery pack.

Measurements Inside the Battery Pack
====================================

.. figure:: ../../build/docs/docs/system/img/battery-system-setup-pack-measurements.svg
   :alt: Voltages and currents to be measured in a multi-string system in the pack
   :name: measurements-in-multi-string-system-in-pack
   :width: 500px

   Voltages and currents to be measured in a multi-string system inside the
   battery pack (click to enlarge)

The strings are depicted in ascending order from right to left starting with
string 1 (``S1``) until left-most string m (``Sm``).
Each string consists of ``n`` modules, where every module has its own
|bms-slave|.
Each string features a current sensor, a string fuse and one or two string
contactors.
Thus, the following voltages need to be measured in each string:

- |string-voltage-m-abbr-full| where ``m`` indicates the number of
  the string.
- |fused-string-voltage-m-abbr-full| where ``m`` indicates the number
  of the string.
  It is measured between the lowest module and after the string fuse.
- |pack-voltage-m-abbr-full| where ``m`` indicates the number
  of the string.
  It is measured between the negative pole of the lowest module and behind the
  positive string contactor.
  This voltage is identical for all connected strings and enables measurement
  validation.

The current sensor additional measures each string current.

Comprehension of measured voltages and current is found in
:numref:`measured-voltages-and-current-in-the-pack`.

.. csv-table:: List of measured voltages and current in the pack
   :name: measured-voltages-and-current-in-the-pack
   :header-rows: 1
   :delim: ;
   :file: ./measured-voltages-and-current-in-the-pack.csv

********************
Single-String System
********************

A single-string system reduces the amount of required voltage and current
measurements.
:numref:`measurements-in-single-string-system` shows the single-string
topology.

.. figure:: ../../build/docs/docs/system/img/battery-system-setup-single-string.svg
   :alt: Voltages and current to be measured in a single-string system
   :name: measurements-in-single-string-system
   :width: 800px

   Voltages and current to be measured in a single-string system
   (click to enlarge)

The list of measurements is therefore reduced to
:numref:`pack-measurements-single-string-system` and no further measurements
inside the string are required.

.. csv-table:: List of measurements in single-string system
   :name: pack-measurements-single-string-system
   :header-rows: 1
   :delim: ;
   :file: ./pack-measurements-single-string-system.csv

********************************************
Measurements Inside the Battery Junction Box
********************************************

.. figure:: ../../build/docs/docs/system/img/battery-system-setup-bjb-measurements.svg
   :alt: Voltages to be measured in a multi string system in the BJB
   :name: voltages-in-multi-string-system-in-bjb
   :width: 500px

   Voltages to be measured in a multi-string system in the BJB (click to
   enlarge)

- The pack voltage is measured before the main fuse.
- The fused pack voltage is measured after the main fuse.
- The system voltage is measured after the contactors.
- **optional**: If the system uses a second power path, the second power path
  voltage is measured after the contactors of the second power path.

Comprehension of measured voltages and current in the battery junction box:

.. csv-table:: List of measurements in multi-string system
   :name: pack-measurements-multi-string-system
   :header-rows: 1
   :delim: ;
   :file: ./bjb-measurements.csv

.. _MAPPING_OF_DOCUMENTED_SYSTEM_TERMS_TO_CODE_NAMES:

************************************************
Mapping Of Documented System Terms To Code Names
************************************************

The voltage and current names in this chapter are system-level documentation
terms.
In the implementation, measured and validated values are represented by
database entries and fields.

The following table maps the documented voltage and current terms to the
currently used implementation symbols.

When reading this mapping, distinguish between measurement source and
application value:

- Measurement source refers to a direct sensor channel value, for example
   ``DATA_BLOCK_SYSTEM_VOLTAGE_x_s.highVoltage_mV[m]``.
- Application value refers to the value that is used by application logic
   after plausibility checking and fallback handling, for example
   ``DATA_BLOCK_PACK_VALUES_s.*`` fields written in redundancy validation.

.. csv-table:: Mapping of documented voltage terms to implementation symbols
   :name: voltage-term-to-code-mapping
   :header-rows: 1
   :delim: ;
   :file: ./voltage-term-to-code-mapping.csv
