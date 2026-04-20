.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _NETWORK_INTERFACE:

Network Interface
=================

Module Files
------------

Portable
^^^^^^^^

- ``src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/NetworkInterface/tms570lc435/NetworkInterface.c``
- ``src/os/freertos/freertos-plus/freertos-plus-tcp/source/include/NetworkInterface.h``

Description
-----------

The NetworkInterface module serves as the hardware abstraction layer between
the FreeRTOS+TCP networking stack and the |ti-tms570| |emac| hardware.
It manages:

- Hardware initialization of the |emac| and PHY
- Transmission of Ethernet frames from the TCP stack to the hardware
- Reception of Ethernet frames from the hardware to the TCP stack
- Memory management for network buffers
- PHY link status monitoring

Connecting to the application
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Figure :numref:`tcp-structure-application` shows the connection of the
|freertos-plus-tcp| stack with the rest of the software.

.. drawio-figure:: img/tcp_structure.drawio
   :format: svg
   :alt: Connection of the FreeRTOS-Plus-TCP stack with the software
   :name: tcp-structure-application
   :width: 800px
   :align: center

   Connection of the |freertos-plus-tcp| stack with the software

**The application** is connected via berkley sockets to the |tcp-ip-stack|.
It offers funktions like:

- **Create sockets**: Establish communication endpoints for sending and
  receiving data.
- **Send and receive data**: Transmit data to other devices or receive data
  from them.
- **Establish and manage TCP connections**: Connect to servers, listen for
  incoming connections, and accept connections from clients.

**The Event queue** handles messages and requests to the IP-Task.
This is especially relevant for the Receive Task.
This is also used to inform the application of Network events.

**The Network Interface** is the connecting element to the hardware drivers and
responsible for the buffer management.

**The Receive Task**: This task works as deferred interrupt handler that is
waiting for incoming network traffic.

Integration with FreeRTOS+TCP Stack
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Network Interface module integrates with the FreeRTOS+TCP stack by
implementing the required network interface callback functions.
These functions are registered through the ``NIC_FillInterfaceDescriptor``
function which populates a NetworkInterface_t structure that is then added to
the TCP stack via ``FreeRTOS_AddNetworkInterface``.

Memory Management
^^^^^^^^^^^^^^^^^^
The module manages two separate memory pools:

#. **EMAC Buffers**: Managed internally by the EMAC driver for hardware
   operations.
#. **FreeRTOS Buffers**: Static array nic_networkBuffers allocated specifically
   for FreeRTOS network buffer descriptors.


Limitations
^^^^^^^^^^^^
- Currently supports only single-packet messages (no fragmentation)
- Uses statically allocated buffers
- Supports only one network interface instance

See Also
----------
- :ref:`EMAC`
- :ref:`PHY_MODULE`
- :ref:`ETHERNET_MODULE`
- :ref:`HOW_TO_IMPLEMENT_ETHERNET_PORT`
- :ref:`HOW_TO_TEST_TCP_IP`
