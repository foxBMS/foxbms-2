.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _CONNECTORS:

Connectors
==========

This section summarizes the types of connectors used in |foxbms|.

.. _MOLEX_MICRO_FIT_3_0_CONNECTOR:

Molex Micro-Fit 3.0 Connector
-----------------------------

This type of connector is the main type of connector that is used for |foxbms|.

There are two types of connectors:

- the *header* and
- the *receptacle*, plugged into the header.

The pin numbering is always counted from the view of the cables going into the
header or Receptacle.
This must be taken into account when crimping the receptacles.

.. _MICTOR_CONNECTOR:

Mictor Connector
----------------

This connector is only used for the hardware debugger interface between the
host PC and the |bms-master|.

The benefit of this connector is that it enables both a JTAG connection (for
debugging purposes) and a trace connection (for detailed trace analysis).
An adapter is needed to interface this port to a debugger.
Examples are

- Converter IDC20A to Mictor-38 from Lauterbach (part number ``LA-3722``)
- J-Link Mictor 38 Adapter from Segger (part number ``8.06.08``)
- TI-CTI-20 Adapter from Segger (part number ``8.06.06``)

.. _SAMTEC_EDGE_RATE_CONNECTOR:

Samtec Edge Rate Connector
--------------------------

This connector is only used internally on the BMS hardware to connect the
|bms-interface| to the |bms-master|.

.. _WUERTH_WR_MJ_CONNECTOR:

Wuerth WR-MJ connector
----------------------

This connector is only used for Ethernet and adheres to the common internet
standards.
