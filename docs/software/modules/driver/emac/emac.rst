.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _EMAC:

EMAC
====

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/emac/emac-low-level.c``
- ``src/app/driver/emac/emac-low-level.h``
- ``src/app/driver/emac/emac.c``
- ``src/app/driver/emac/emac.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/emac_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/emac/test_emac.c``

Description
-----------

The |emac|-Driver driver handles DMA-based packet transmit (Tx) and receive
(Rx) as part of the TCP/IP stack.
The |tcp-ip| communication is described in more detail in the section
:ref:`OPERATING_SYSTEM`.

Initialization
^^^^^^^^^^^^^^

The |emac| Module has its own RAM.
This is used to store the Tx and Rx buffer descriptors.
Each *Buffer Descriptor* (``EMAC_TX_BUFFER_DESCRIPTOR_s``) and
(``EMAC_RX_BUFFER_DESCRIPTOR_s``) has fields:

* ``pBuffer``: pointer to data buffer
* ``bufferOffsetAndLength``: padding info and buffer length
* ``flagsAndPacketLength``: flags and packet length
* ``next``: link to the next descriptor

As the buffer descriptors are stored in a linked list, it contains first a
pointer to the next buffer descriptor and then the pointer to the actual
buffer.
The buffer offset is the offset from the start of the packet buffer to the
first byte of valid data.
The buffer length then describes the number of valid data in the buffer.
The flags contain information about the packet as:

* SOP (Start of Packet)
* EOP (End of Packet)
* OWNER (Ownership)
* EOQ (End of Queue)

The Packet Length field is only valid for data-carrying SOP (Start of Packet)
descriptors.
For these, it specifies the total length of the entire Ethernet frame,
regardless of whether the frame is stored in a single buffer or span across
multiple fragments.

:numref:`initialization-emac-dma` shows the buffer descriptors
after their Initialization.

.. drawio-figure:: img/data_processing_emac_initialize_dma.drawio
   :format: svg
   :alt: Initialization of |emac| DMA Module
   :name: initialization-emac-dma
   :width: 800px
   :align: center

   Initialization of |emac| DMA Module

During the initialization the buffer pointers are mapped to the buffer and
aligned to a linked list.
For each channel a struct is holding the relevant information for the network
interface.
For the receive part it is the free head, active head and active tail.
The transmit channel contains the free head, next buffer descriptor to process
and active tail.

Rx Channel
""""""""""

The struct Rx channel is prepared to manage the reception of several packets.
It consists of the pointer (``EMAC_RX_CHANNEL_s``):

- ``pFreeHead`` that points to next free head of the buffer list.
  It is set after the first reception.
- ``pActiveHead``, a pointer to the head of the list of buffer descriptors.
  The DMA engine will start to fill the buffer of this descriptor after
  reception.
- ``pActiveTail`` that points to the last descriptor in the active list.

Tx Channel
""""""""""

The transmit channel  is similar to the receive channel.
It also consists of a linked list of buffer descriptors
(``EMAC_TX_CHANNEL_s``).
But this list is circularly linked.
After initialization, ``pFreeHead`` and ``pNextBufferDescriptorToProcess``
point to the first buffer descriptor.
The buffer pointers get assigned during the transmission processes.

Receive
^^^^^^^

When an Ethernet frame arrives on the MAC, the EMAC receive logic writes the
frame into system memory using DMA.
The DMA engine uses the previously mentioned linked list of buffer descriptors.
For each descriptor, the DMA stores the address of the buffer, status
information, and the number of valid bytes in that buffer.
At the beginning, ``pActiveHead`` and ``pActiveTail`` are accessed.
``pFreeHead`` will point after the reception to the first descriptor.
This is used as temporary storage for the pointer.
The total packet length is recorded in the EOP descriptor, as illustrated in
:numref:`receive-emac-before`.
Once the EMAC has completed writing a frame
(i.e. the EOP descriptor has been filled and marked as owned by software), it
signals the processor by generating an interrupt.
At this point the software knows that at least one complete frame is available
on the receive descriptor list.
Starting with the first descriptor, they are processed in the
:ref:`NETWORK_INTERFACE`.


.. drawio-figure:: img/data_processing_emac_receive_before.drawio
   :format: svg
   :alt: Reception of Ethernet Packets
   :name: receive-emac-before
   :width: 800px
   :align: center

   Structure of buffer descriptors before reception

In the receive handler, the software walks the list of active descriptors,
starting at pActiveHead.
For each packet, it:

#. Locates the next SOP descriptor.
#. Follows the descriptor chain up to and including the corresponding EOP descriptor. This covers all buffers that contain fragments of the same Ethernet frame.
#. Copies the received data from these DMA buffers into a Ethernet packet buffer.
#. Passes the packet to the network interface, which then hands it off to the Ethernet protocol stack for further processing (e.g. IP, TCP/UDP).

After the buffer descriptor is processed, the flags are reset and
``pActiveHead`` and ``pActiveTail`` point to the last processed buffer
descriptor.


.. note::
   Currently, the |EMAC| driver only supports the reception of one buffer
   descriptor.

When the chain is processed, the linked list is shifted to the end of the
chain.
:numref:`receive-emac-after` describes the linked RX buffer descriptor list
after the reception.

.. drawio-figure:: img/data_processing_emac_receive_after.drawio
   :format: svg
   :alt: Reception of Ethernet Packets
   :name: receive-emac-after
   :width: 800px
   :align: center

   Structure of buffer descriptors after reception

Transmit
^^^^^^^^
:numref:`transmit-emac` describes the buffer descriptors during data
transmission.

.. drawio-figure:: img/data_processing_emac_transmit.drawio
   :format: svg
   :alt: Transmission of Ethernet Packets
   :name: transmit-emac
   :width: 800px
   :align: center

   Structure of buffer descriptors after transmission

In the transmission process, the ``pNextBufferDescriptorToProcess`` of the Tx
channel is filled first.
The total packet length, the start of packet length
(SOP) and the owner (OWNER) flag are set in the first buffer descriptor.
Then the payload is added.
As the driver currently does not accept fragmented packets every descriptor
ends with the end of packet (EOP) flag.
The last packet is the ``pActiveTail``.
``pFreeHead`` points to the next descriptor available for sending data.

See Also
--------

- :ref:`PHY_MODULE`
- :ref:`NETWORK_INTERFACE`
- :ref:`ETHERNET_MODULE`
- :ref:`HOW_TO_IMPLEMENT_ETHERNET_PORT`
- :ref:`HOW_TO_TEST_TCP_IP`
