.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _SOFTWARE_STRUCTURE:

******************
Software Structure
******************

.. highlight:: C

The following section describes the diagnostics and safety focused BMS software
architecture as depicted in :numref:`sw-structure`.
This layer-based architecture facilitates hardware and operating independent
BMS implementations applying the design paradigms, that

- **All application code runs in a simple operating system context** and
- **MCU and external hardware dependent drivers are abstracted by the provided
  wrappers/abstraction layers.**

.. drawio-figure:: img/sw-architecture.drawio
   :format: svg
   :alt: Software structure
   :name: sw-structure
   :width: 800px
   :align: center

   |foxbms| - Software architecture

A **hardware abstraction layer (HAL)** provides various interfaces to directly
access the hardware and its peripherals.
This enables encapsulation of the actual BMS software implementation from the
hardware and eases porting the |foxbms| software to different microcontrollers.

The open-source real-time operating system |freertos| is the centerpiece of
the software architecture.
Its reliable kernel is ideally suited to ensure the compliance of all soft and
hard real-time requirements of a battery-management system.
Furthermore, it provides a migration path to SafeRTOS, which includes
certifications for the medical, automotive and industrial sector, if a
certification is required by the application.

The |foxbms| software itself, executed within the operating system context, is
grouped into three different layers:

- A dedicated **foxBMS Driver Layer** uses the MCU-Wrapper to provide different
  communication interfaces (e.g., CAN, UART, SPI) to acquire measurement data,
  monitor status of hardware components (e.g., supply voltages, transceivers,
  real-time clock) as the well as the communication with the |bms-slaves|.
- Diagnostic functions and error handling, system monitoring (for hard- and
  software) and interfaces to the data-exchange module are the most important
  tasks of the **foxBMS Engine Layer**.
  The data-exchange module, sometimes also called database ensures a reliable
  and safe asynchronous data exchange between different tasks and/or software
  modules.
  It is implemented based on a producer/consumer pattern.
  The exchanged data is always produced by a single data producer and then
  stored in the data-exchange module.
  Afterwards, it can be used by multiple consumers while the data integrity is
  always ensured.
- The actual BMS implementation including the monitoring of the safety critical
  parameters (e.g., safe-operating area, contactor state, communication
  errors), state estimation functionalities (e.g., state-of-charge, state-of-
  health, state-of-energy) and the application specific BMS application
  (e.g., balancing, plausibility checks) are implemented within the
  **foxBMS Application Layer**.
  Additionally, an algorithm module provides an interface to execute advanced
  computation intensive algorithms.

The Software structure is explained further in the following chapters.

.. toctree::
  :maxdepth: 1

  ./operating-system.rst
  ./operating-system-configuration.rst
  ./application.rst
