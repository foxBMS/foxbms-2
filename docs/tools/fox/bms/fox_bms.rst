.. include:: ./../../../macros.txt

.. _FOX_BMS:

bms
===

The bms tool provides a shell that enables communication with the |foxbms|
through a CAN bus.
This makes low-level debugging possible through only a terminal.


Usage
-----

.. include:: ./../../../../build/docs/fox_bms_help.txt


The bms command opens the bms-shell in which commands are implemented for
communicating with the |foxbms|.

There are the following commands:

- ``help``: Prints all commands and gives further information on the commands
  when typing ``help <command>``.
- ``init``: Initializes the CAN bus. All other commands can only be executed
  after passing this command.
- ``exit``: Exits the bms-shell and shuts the CAN bus down if necessary.
- ``boottimestamp``: Requests the Boot Timestamp of the |foxbms|.
- ``buildconfig``: Requests the Build Configuration of the |foxbms|.
- ``commithash``: Requests the commit hash the software version was built with.
- ``fram``: Instructs the |foxbms| to initialize the FRAM.
- ``getrtc``: Requests the current time of the |foxbms|.
- ``log [ID] [optional: #] [optional: FILE]``: Logs the specified amount of
  received messages with ``ID`` to the terminal or to
  ``build/logs/foxBMS_CAN_bms_log.txt`` if  ``FILE`` has been passed.
  If the size of the file reaches 200000 bytes a new file is created with the
  timestamp added to the name.
  ``ID`` can be given in decimal format or in hexadecimal format
  (e.g. ``0x300``, ``300h``).
  If no amount has been specified, only one message will be logged.
- ``mcuid``: Requests the unique ID of the MCU.
- ``mculotnumber``: Requests the lot number of the MCU.
- ``mcuwaferinfo``: Requests the wafer information the MCU was made of.
- ``rtc``: Sets the time of the |foxbms| to the current time.
- ``softwarereset``: Instructs the |foxbms| to perform a software reset.
- ``softwareversion``: Requests the software version of the |foxbms|.
- ``uptime``: Requests the uptime of the |foxbms|.

Structure
---------

.. drawio-figure:: ./img/bms_overview.drawio
   :format: svg
   :alt: bms tool overview
   :name: bms-tool-overview
   :width: 60 %
   :align: center

   Overview of the bms tool

.. drawio-figure:: ./img/bms_processes.drawio
   :format: svg
   :alt: bms tool processes
   :name: bms-tool-processes
   :width: 70 %
   :align: center

   Processes of the bms tool
