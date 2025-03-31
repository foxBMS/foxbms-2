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

The basic usage of the |tcp-ip-stack| with sending a ping signal is explained
in  :ref:`HOW_TO_PING_FOXBMS`.
