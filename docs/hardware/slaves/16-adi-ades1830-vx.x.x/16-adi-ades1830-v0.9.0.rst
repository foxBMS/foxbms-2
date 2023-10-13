.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _ADI_ADES1830_BASED_16_CELL_SLAVE___V0_9_0__:

ADI ADES1830-based 16-Cell Slave ``v0.9.0``
===========================================

.. note::

   The changelog for this release is found at
   :ref:`CHANGELOG_FOR_SLAVE_ADI_ADES1830_V0_9_0`.

.. |local_slave_version|  replace:: ``0.9.0``

--------
Overview
--------

.. important::

   The following description only applies for the |adi-ades1830|\ -based 16
   cell |BMS-Slave| hardware version |local_slave_version|.

.. hint::

   All connector pinouts described below follow the
   :ref:`CONVENTION_FOR_MOLEX_3_0_CONNECTOR_PIN_NUMBERING`.

--------------
Specifications
--------------

^^^^^^^^^^^^^^^^^^
Electrical Ratings
^^^^^^^^^^^^^^^^^^

The current consumption from the module has been measured at 57.6 |_| V module
voltage, which is equivalent to a cell voltage of 3.6 |_| V per cell.
No sense lines have been connected for this measurement (as the impact of cell
voltage sensing is negligible on the current consumption).


.. csv-table:: Electrical Ratings
   :name: adi_ades1830_slave_18cell_v0.9.0_electrical_ratings
   :header-rows: 1
   :delim: ;
   :file: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0_electrical-ratings.csv

^^^^^^^^^^^^^^^^^^^^^
Mechanical Dimensions
^^^^^^^^^^^^^^^^^^^^^

.. csv-table:: Mechanical Dimensions
   :name: adi_ades1830_slave_18cell_v0.9.0_mechanical_dimensions
   :header-rows: 1
   :delim: ;
   :file: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0_mechanical-dimensions.csv

^^^^^^^^^^^^^
Block Diagram
^^^^^^^^^^^^^

The block diagram of the |BMS-Slave| is shown in
:numref:`Fig. %s <block_diagram_adi_ades1830_slave_18cell_v0.9.0>`.

.. _block_diagram_adi_ades1830_slave_18cell_v0.9.0:
.. figure:: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0.png
   :width: 100 %

   |BMS-Slave| 16-Cell Block Diagram

^^^^^^^^^^^^^^^^^^^^^^^^^^
Schematic and Board Layout
^^^^^^^^^^^^^^^^^^^^^^^^^^

More information about the board schematic and layout files can be found in
section :ref:`DESIGN_RESOURCES`.

---------
Functions
---------

^^^^^^^^^^^^^^^^^^^^^^^^
Cell Voltage Measurement
^^^^^^^^^^^^^^^^^^^^^^^^

The cell voltage sense lines are input on the connector J400.
The pinout is described in
:numref:`Table %s <adi_ades1830_slave_16cell_v0.9.0_cell_sense_connector>`.

.. figure:: ./../../img/molex_connector-24pin.png
   :width: 50 %

.. csv-table:: Cell voltage sense connector
   :name: adi_ades1830_slave_16cell_v0.9.0_cell_sense_connector
   :header-rows: 1
   :delim: ;
   :file: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0_cell_voltage-sense-connector.csv


The |VBAT+| and |VBAT-| connection is used for the internal power supply of the
slave board.

The cell input lines are filtered by differential capacitor
filter:
both possibilities are provided on the PCB of the |BMS-Slave|.
More information on the corner frequency of this filtering can be found in the
schematic.
The differential capacitor filter can be used when noise is less frequent or
the design is subjected to cost optimization.

^^^^^^^^^^^^^^^^^^^^^^
Passive Cell Balancing
^^^^^^^^^^^^^^^^^^^^^^

The passive balancing circuit is realized by a series connection of two
30 |Ohm| discharge-resistors that can be connected to each single cell in
parallel.
The balancing process is controlled by the |adi-ades1830| monitoring IC.
The resistor value of 2x 30 |_| |Ohm| results in a balancing current of about
60 |_| mA at a cell voltage of 3.6 |_| V.
This current results in a power dissipation of about 0.2W per balancing
channel (at 3.6 |_| V).

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Temperature Sensor Measurement
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The cell temperature sensors are connected to the connectors J403.

The pinout is described in
:numref:`Table %s <adi_ades1830_slave_16cell_v0.9.0_temp_sense_connector>`.

.. figure:: ./../../img/molex_connector-20pin.png
   :width: 30 %

.. csv-table:: Temperature sensor connector
   :name: adi_ades1830_slave_16cell_v0.9.0_temp_sense_connector
   :header-rows: 1
   :delim: ;
   :file: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0_temperature-sensor-connector.csv

Standard 10 |_| |kOhm| NTC resistors are recommended for use.
When using other values than these, the series resistors
(R517-R526) on the board may have to be adjusted.
Please note that the accuracy of the internal voltage reference VREF2 decreases
heavily with a load of over 3 |_| mA.
Using 10x 10 |_| |kOhm| NTC resistors with the corresponding 10 |_| |kOhm|
series resistors results in a current of 1.5mA (at 20 |_| °C) which is drawn
from VREF2.

Each 10 temperature sensors are connected to an GPIO pin of the |adi-ades1830|.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
isoSPI Daisy Chain Connection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The data transmission between the slaves and between the slaves and BMS master
uses the isoSPI interface.
The isoSPI signals are input/output on the connectors J501/J402.
The isoSPI ports are bidirectional, that means they can be used in forward and
reverse direction.
The isoSPI connections are isolated galvanically using pulse transformers
(T500/T501).


The pinout of the isoSPI connectors is described in
:numref:`Table %s <adi_ades1830_slave_16cell_v0.9.0_daisy_input_connectors>`
and
:numref:`Table %s <adi_ades1830_slave_16cell_v0.9.0_daisy_output_connectors>`.

.. figure:: ./../../img/molex_connector-2pin.png
   :width: 5 %

.. csv-table:: isoSPI Daisy Chain Input Connectors
   :name: adi_ades1830_slave_16cell_v0.9.0_daisy_input_connectors
   :header-rows: 1
   :delim: ;
   :file: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0_daisy-input-connectors.csv

.. csv-table:: isoSPI Daisy Chain Output Connectors
   :name: adi_ades1830_slave_16cell_v0.9.0_daisy_output_connectors
   :header-rows: 1
   :delim: ;
   :file: ./16-adi-ades1830-v0.9.0/16-adi-ades1830-v0.9.0_daisy-output-connectors.csv

------------------------------
Software Driver Implementation
------------------------------

The driver implementation for this AFE is documented at :ref:`ADI_ADES1830`.
