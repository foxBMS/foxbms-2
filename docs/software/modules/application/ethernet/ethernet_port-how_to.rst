.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_IMPLEMENT_ETHERNET_PORT:

How to Implement an Ethernet Port
=================================

|foxbms| uses the |freertos| |tcp-ip| port for ethernet communication.
This is already implemented and can be activated as described in
:ref:`CONFIGURE_OS`.
If the implementation needs to be modified, the required steps are described
here.
For instructions on using the existing implementation, refer to
:ref:`HOW_TO_TEST_TCP_IP`.

Configure the Hardware
----------------------

The |ti-tms570| is already configured for the DP83869-PHY mounted on the
|bms-master|.
In case the hardware is changed, the following steps have to be done in
|ti-halcogen|.
If the |bms-master| is unchanged, these steps can be skipped.

..
   cspell:ignore Pinmux

#. Configure Pinmux:

   * Enable the MII peripherals

#. Configure GIOs

   * Configure GIOA[1] for Reset
   * Configure GIOB[6] for Power Down

#. Configure EMAC

   * EMAC Configuration

      * Select an appropiate EMAC Address
      * Set the correct Physical Address (1)
      * Enable Transmit, Receive, MII
      * Ensure that Loopback Communication is disabled

   * PHY

      * Select Custom driver for **your** PHY-Driver.

#. Configure Interrupts

   * For the ethernet connection the Interrupt handler 77 and 79 are relevant.
     They have to be activated and have to be used in the PHY-Driver.

PHY-Driver
----------

With |foxbms| a driver for the DP83869-PHY which is mounted on the |foxbms|
master is provided. For other PHYs the changes are:

* Different register map, including extended register addresses
* Need for activating MII mode
* Loopback settings
* Selecting transmission speed
* Auto-negotiation process

Porting FreeRTOS-Plus-TCP
-------------------------

In order to use the FreeRTOS-Plus-TCP stack it has to be adapted to the used
Compiler :ref:`EMAC`-Driver and PHY-Driver.

In addition to adapting the compiler, EMAC driver and PHY driver, the memory
and buffer configuration of the |tcp-ip-stack| must be checked carefully.
Relevant parameters are configured primarily in ``FreeRTOSIPConfig.h`` and in
the application startup code.
In particular, the number and size of network
buffers (for example ``ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS`` and the
associated buffer sizes), the stack sizes and priorities of the TCP/IP task and
network-interface tasks, and the overall heap configuration
(``configTOTAL_HEAP_SIZE`` or the linker-based heap setup) must be chosen
according to the expected network load.
An undersized configuration can lead to dropped packets or connection
failures, while an oversized configuration may waste RAM.

Compiler configuration
^^^^^^^^^^^^^^^^^^^^^^

The compiler configuration is done in
``src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/Compiler``.
This is already configured for |foxbms|.

Network Interface Port layer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The network interface port layer connects the EMAC driver with the
|tcp-ip-stack|.
This porting is implemented in the :ref:`NETWORK_INTERFACE`.

The following functions must be provided:

* ``extern int pxTMS570_FillInterfaceDescriptor( BaseType_t xEMACIndex, NetworkInterface_t * pxInterface )``
* ``BaseType_t xNetworkInterfaceInitialise( struct xNetworkInterface * pxDescriptor )``
* ``extern void xNetworkInterfaceInput( void *pvParameters )``
* ``BaseType_t xNetworkInterfaceOutput(struct xNetworkInterface * pxDescriptor, NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend )``
* ``void uxNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )``

See Also
--------

- :ref:`PHY_MODULE`
- :ref:`NETWORK_INTERFACE`
- :ref:`EMAC`
- :ref:`ETHERNET_MODULE`
- :ref:`HOW_TO_TEST_TCP_IP`
