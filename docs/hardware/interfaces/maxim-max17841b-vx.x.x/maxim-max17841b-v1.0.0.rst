.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _INTERFACE_MAX17841B___V1_0_0__:

Interface MAX17841B ``v1.0.0``
==============================

.. |local_interface_version|  replace:: ``1.0.0``

.. note::

   The design for this interface is not complete.
   If you are interested in a design, please contact us.

--------
Overview
--------

.. important::

   The following description only applies for the |max17841b|\ -based
   |bms-interface| hardware version |local_interface_version|.

.. hint::

   All connector pinouts described below follow the
   :ref:`CONVENTION_FOR_MOLEX_3_0_CONNECTOR_PIN_NUMBERING` and
   :ref:`CONVENTION_FOR_SAMTEC_EDGE_RATE_CONNECTOR_PIN_NUMBERING`.

The |bms-interface| is used to enable communication between the |bms-master|
and the |bms-slaves|.
The |bms-interface| is intended to be used together with a |bms-master|.

--------------
Specifications
--------------

^^^^^^^^^^^^^^^^^^^^^
Mechanical Dimensions
^^^^^^^^^^^^^^^^^^^^^

The size of the PCB is 70 |_| |mm| |_| x |_| 120 |_| |mm|.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Battery Management UART Insulation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The interface board is designed for a maximum continuous insulation voltage of
1250 |_| V |_| DC between all two Maxim Battery Management UART channels and
between each Maxim Battery Management UART channel and the |bms-master|.

The insulation is designed according to DIN |_| EN |_| 60664-1:2008-01 under
the following conditions:

- Type of insulation: functional
- Electrical field: homogeneous
- Pollution degree: 2
- PCB insulator class: 3b
- Transient voltage: 4300 |_| V |_| DC
- Continuous (working) voltage: 1250 |_| V |_| DC (selected due to design
  space constraints on PCB)

The PCB design complies with the following parameters:

- Clearance: 1.3 |_| |mm|
- Creepage distance: 12.5 |_| |mm|


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Maxim Battery Management UART communication
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The |bms-interface| offers up to two isolated Maxim Battery Management UART
communication channels using the |max17841b| transceiver chip from |mxm|.
The |max17841b| transceivers are controlled by the SPI signals from the
|bms-master|.

For the connection to the |bms-master|, a 40-pole mezzanine connector (J100)
is used.
