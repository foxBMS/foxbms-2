.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _OPERATING_SYSTEM:

################
Operating System
################

Folder Structure
----------------

.. code-block::

   src
   └── os
        ├── freertos
        |       ├── freertos
        |       └── freertos-plus
        |               └── freertos-plus-tcp
        └── safertos

|freertos|
----------

|tcp-ip-stack|
--------------

The |tcp-ip-stack| is the FreeRTOS-Plus-TCP Library configured for |foxbms|.
For the |tcp-ip| communication the |ti-tms570| uses the Ethernet Media Access
Controller (EMAC) to communicate with the ethernet Physical Layer Transceiver
(PHY).

.. drawio-figure:: ethernet-communication.drawio
   :format: svg
   :alt: Block diagram of the ethernet communication
   :name: block-diagram-ethernet-communication
   :width: 130px

   Block diagram of the ethernet communication

To reference this, the |tcp-ip| communication is structured as shown in the figure below.

.. drawio-figure:: tcp-structure.drawio
   :format: svg
   :alt: Software structure of the ethernet communication
   :name: tcp-structure
   :width: 130px

   Software structure of the ethernet communication

The related drivers are structured as in the physical structure.
The phy specific settings and configuration is made in the phy driver ``dp83869.c``.
Then in the next layer the EMAC driver controlls the EMAC to transmit and receive
ethernet packages. The EMAC is connected by hardware to the PHY.

The basic usage of the |tcp-ip-stack| with sending a ping signal is explained
in  :ref:`HOW_TO_PING_FOXBMS`.
