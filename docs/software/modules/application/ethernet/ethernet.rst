.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _ETHERNET_MODULE:

Ethernet Module
===============

Module Files
------------

Driver
^^^^^^

- ``src/app/application/ethernet/ethernet.c``
- ``src/app/application/ethernet/ethernet.h``

|freertos|
""""""""""
- ``src/app/application/ethernet/ethernet_freertos.c``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/config/ethernet_cfg.c``
- ``src/app/application/config/ethernet_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/config/test_ethernet_cfg.c``
- ``tests/unit/app/application/ethernet/test_ethernet.c``
- ``tests/unit/app/application/ethernet/test_ethernet_freertos.c``

Detailed Description
--------------------

The ethernet module contains the high level application code for the ethernet
communication. It also provides the external function implementations required
by the |freertos-plus-tcp-library|.

Initialization
^^^^^^^^^^^^^^

During the initialization tree things happen:

#.  A struct that holds the driver for the network interface
    (EMAC + PHY) is filled. The |tcp-ip-stack| accesses the relevant
    functions via this struct.
#.  The necessary needed network parameters are stored and made available for
    the |tcp-ip-stack|.
#.  The |tcp-ip-stack| is initialised and the tasks that use the network
    are created in the function ``vApplicationIPNetworkEventHook()``

This is sufficient to respond to ICMP ping requests.
How this works is explained in :ref:`HOW_TO_TEST_TCP_IP`.

|echo-server|
^^^^^^^^^^^^^

As an application example, a tcp |echo-server| is implemented in
``ethernet.c``.
To implement the echo server, a two-stepped approach was chosen.
The |echo-server| consists of two |ip-application-tasks| that are integrated in
the task scheme as described in :ref:`OS_CONFIGURATION`.
:numref:`echo-server-sequence` shows this communication in a simplified way.

.. drawio-figure:: img/echo_server.drawio
   :format: svg
   :alt: Echo Server Sequence Diagram
   :name: echo-server-sequence
   :width: 800px
   :align: center

   Echo Server Sequence Diagram

The task ``ETH_ListenForConnection`` is created in the :ref:`IP_EVENT_HOOK`.
This task creates a socket that listens for incoming connections.
If a TCP handshake completes successfully, a new connection socket is created
and the actual server task ``ETH_EchoServerInstance`` is started.

The server task first performs some basic configuration and then waits for
incoming messages.
When a message is received, the data is simply echoed back.
If the receive timeout is reached, the socket is closed and the task is
deleted.
The usage is explained in detail in :ref:`HOW_TO_TEST_TCP_IP`.

Random Numbers
^^^^^^^^^^^^^^

The externalized functions `ulApplicationGetNextSequenceNumber()` and
`xApplicationGetRandomNumber()` return just a simple random number.

 .. warning::
    The random number generator is presently implemented in a simple approach.
    It does not apply to any security recommendations.
    It is not suitable for security-relevant use
    (e.g., exposed to untrusted networks).

.. _IP_EVENT_HOOK:

IP Event Hook
^^^^^^^^^^^^^

The `vApplicationIPNetworkEventHook_Multi()` is called by the
|freertos-plus-tcp-library| when the network connects or disconnects.
This can be used to implement server tasks that depend on network connectivity.

DNS (Domain Name System resolution)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
|freertos| provides the function prototype ``xApplicationDNSQueryHook()``
to implement a DNS resolution.
This function is called by the |tcp-ip-stack| to check whether the name
received is the same as the one device is looking for.

See Also
--------

- :ref:`PHY_MODULE`
- :ref:`NETWORK_INTERFACE`
- :ref:`EMAC`
- :ref:`HOW_TO_IMPLEMENT_ETHERNET_PORT`
- :ref:`HOW_TO_TEST_TCP_IP`
