.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _COMMUNICATION:

Communication
=============

CAN
----
|foxbms| supports communication with other high level devices (e.g., the VCU)
through CAN.


DBC File
^^^^^^^^^^

These files describe the CAN interface used by |foxbms|.
The .dbc-file has been created using PCAN Symbol Editor
|version_pcan_symbol_editor| from symbol file |version_sym_file|.

The .dbc-file and .sym-file are located in ``tools/dbc``.

.. include:: ./../../build/docs/supported_can_messages.txt

Ethernet
---------
Additionally to to CAN communication an ethernet communication interface is
provided.
Currently plain |tcp-ip| is supported for user defined application tasks.
These tasks can be placed in ``ethernet.c``.
As an example and for testing purposes an echo server is implemented there.
More details can be found in :ref:`ETHERNET_MODULE`.
