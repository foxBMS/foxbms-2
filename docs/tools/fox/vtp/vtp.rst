.. include:: ./../../../macros.txt

.. _FOX_VIRTUAL_TESTING_PLATFORM:

|virtual-testing-platform|
==========================

The idea of the  |virtual-testing-platform| (|vtp|) is enable testing a
high-level control unit HLCU (e.g., a VCU) together with the BMS without an physical
BMS hardware.
Therefore the |hlcu| is connected via CAN with the host and controls the BMS
via the state request control message (alternatively can the |hlcu| also be
virtualized and by that a full virtual test can be implemented).
The user provides the BMS internal measurement values, e.g.,  the cell voltage
measurement data via the interface.
Based on this input (measurement data and state request) the BMS derives it
internal state and communicates it via CAN.

.. drawio-figure:: virtual-bms.drawio
   :format: svg
   :alt: Workflow and purpose of the Virtual Testing Platform (VTP)
   :name: virtual-bms-workflow-and-purpose
   :width: 800px

   Workflow and purpose of the |virtual-testing-platform| (|vtp|)


The |virtual-testing-platform| is only available in the GUI.
