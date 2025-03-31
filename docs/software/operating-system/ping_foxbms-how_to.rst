.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_PING_FOXBMS:

How to Ping |foxbms|
=========================

This documentation shows how to send a simple ping message to foxbms.
In order to send this message, the |tcp-ip-stack| has to be configured.
This procedure is explained in the next steps.

Configure the Hardware
-----------------------

In order to configure the |ti-tms570| for using the the DP83869-PHY mounted on
the |foxbms| master these steps are necessary.
The configuration has to be done in |ti-halcogen|.

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

      * Select Custom driver for **your** DP83869-Driver.
        The driver for the PHY is currently not provided by |foxbms| and has
        to be implemented by the user.
        A |foxbms| version is coming soon.

#. Configure Interrupts

   * For the ethernet connection the Interrupt handler 77 and 79 are relevant.
     They have to be activated and have to be used in the DP83869-Driver.

Writing the DP83869-Driver
--------------------------

Even though the DP83869-PHY is very similar to the DP83640 the DP83640-Driver,
provided by |ti-halcogen| can not be used directly for the  DP83869.
The changes are:

* Different register map, including extended register addresses
* Need for activating MII mode
* Loopback settings
* Selecting transmission speed
* Auto-negotiation process

Porting FreeRTOS-Plus-TCP
-------------------------

In order to use the FreeRTOS-Plus-TCP stack it has to be adapted to the used
Compiler and EMAC/PHY-Driver.
This is also explained in detail
`here <https://freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/10-Porting/01-FreeRTOS_TCP_Porting>`_.

Compiler configuration
^^^^^^^^^^^^^^^^^^^^^^

The compiler configuration is done in ``src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/Compiler``.
This is already configured for |foxbms|.

Network Interface Port layer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The network interface port layer connects the EMAC driver with the
|tcp-ip-stack|.
Therefore the following functions have to be implemented as described in the
documentation:

* ``extern int pxTMS570_FillInterfaceDescriptor( BaseType_t xEMACIndex, NetworkInterface_t * pxInterface )``
* ``BaseType_t xNetworkInterfaceInitialise( struct xNetworkInterface * pxDescriptor )``
* ``extern void xNetworkInterfaceInput( void *pvParameters )``
* ``BaseType_t xNetworkInterfaceOutput(struct xNetworkInterface * pxDescriptor, NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend )``
* ``void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )``

Using the |tcp-ip-stack| for Pinging
------------------------------------

The |tcp-ip-stack| answers by default to ping messages without a sever
initialized.
For enabeling |foxbms| to answer to a ping message the interface
descriptor has to be initialized with ``pxTMS570_FillInterfaceDescriptor``.
The IP Address and other network parameters have to be set with
``FreeRTOS_FillEndPoint``.
For initializing the |tcp-ip-stack| ``FreeRTOS_IPInit_Multi`` is called.
The tasks that use the network are created in the
``vApplicationIPNetworkEventHook`` hook function below.
The hook function is called when the network connects.

Ping is a standard program for checking an ethernet connection.
When calling ``ping <ip address>`` it should return

.. code-block:: powershell

   PS > ping 169.254.107.24
   Pinging 169.254.107.24 with 32 bytes of data:
   Reply from 169.254.107.24: bytes=32 time<1ms TTL=255
   Reply from 169.254.107.24: bytes=32 time=1ms TTL=255
   Reply from 169.254.107.24: bytes=32 time=1ms TTL=255
   Reply from 169.254.107.24: bytes=32 time<1ms TTL=225

   Ping statistics for 169.254.107.24:
   Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
   Approximate round trip times in milli-seconds:
   Minimum = 0ms, Maximum = 1ms, Average = 0ms
