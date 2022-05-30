.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _CONNECTORS:

Connectors
==========

This section summarizes the types of connectors used in |foxbms|.

|tbc|

..
    Comment: This section should contain: Molex connectors, the extension
    connectors, additional pin header, JTAG connectors (both cTI and Mictor)

.. _CONVENTION_FOR_MOLEX_3_0_CONNECTOR_PIN_NUMBERING:

Convention for Molex Micro-Fit 3.0 Connector Pin Numbering
----------------------------------------------------------

:numref:`molex-connector-numbering` presents the convention for the numbering
of pins in Molex connectors.
This type of connector is the main type of connector that is used for |foxbms|.
The numbering style is used throughout this documentation.

.. figure:: img/molex_connector-viewing-direction.png
   :alt: Description of pin numbering in Molex connectors.
   :name: molex-connector-numbering
   :width: 100 %

   Supply connector pin out, receptacle - rear view, header - front view
   (image source: MOLEX)

There are two types of connectors:

 * Header
 * Receptacle, plugged into the header

The numbering shown on the left in :numref:`molex-connector-numbering`
is always valid when viewing in the direction indicated by the arrow with the
indication ``viewing direction``.
This must be taken into account when crimping the receptacles.

.. _CONVENTION_FOR_SAMTEC_EDGE_RATE_CONNECTOR_PIN_NUMBERING:

Convention for Samtec Edge Rate connectors
------------------------------------------

|tbc|

.. _CONVENTION_FOR_MICTOR_CONNECTOR_PIN_NUMBERING:

Convention for Mictor Pin Numbering
-----------------------------------

|tbc|

.. _CONVENTION_FOR_WUERTH_WR_MJ_CONNECTOR_PIN_NUMBERING:

Convention for Wuerth WR-MJ connector
-------------------------------------

|tbc|

.. _DEBUG_ADAPTERS:

Debug adapters
--------------

The |master| is connected to the debugger through a debug port.
On the standard design, a Mictor 38 pin connector is used with the pin
numbering described in :ref:`CONVENTION_FOR_MICTOR_CONNECTOR_PIN_NUMBERING`.

The benefit of this connector is that it enables both a JTAG connection (for
debugging purposes) and a trace connection (for detailed trace analysis).
In order to interface this port to a normal debugger, an adapter as to be used.
As an example part number
`8.06.08 <https://www.segger.com/products/production/flasher/accessories/adapters/j-link-mictor-38-adapter/>`_
from Segger can be used.
This is just an example, other manufacturers such as Lauterbach have similar
adapters.

On more compact, derived designs of the |master| a connector according to the
compact TI 20 pin connector is used.
On this connector, only JTAG connections are supported.
This connector can be interfaced for example with part number
`8.06.06 <https://www.segger.com/products/production/flasher/accessories/adapters/ti-cti-20-adapter/>`_
from Segger.
