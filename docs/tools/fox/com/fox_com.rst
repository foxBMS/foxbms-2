.. include:: ./../../../macros.txt

.. _FOX_COM:

===
com
===

.. contents:: Table of Contents
    :depth: 1
    :local:

Wrapper
=======

This fox.py module implements a modular, process-based communication wrapper,
designed for use in Python script that require robust and flexible data
exchange via files or communication interfaces typically used in the battery
and IoT applications.

**Main Features:**

- **Unified Interface:** Provides a common interface for different
  communication backends (e.g., file, MQTT), which can be easily integrated
  into user scripts.
- **Process-based Architecture:** Each communication backend (file or MQTT)
  runs in its own managed process, ensuring non-blocking operation and improved
  reliability through multiprocessing.
- **Inter-Process Communication:** Utilizes ``multiprocessing.Queue`` for safe,
  efficient message transfer between user code and communication processes.
- **Event-based Control:** Startup, readiness, and shutdown of communication
  processes is controlled via events, ensuring predictable and robust lifecycle
  management.
- **Extensible:** Easily add new communication backends by implementing the
  provided abstract base classes (``ComInterface``, ``ProcessInterface``).

**How it works:**

- When you instantiate a communication object (e.g., ``File`` or ``MQTT``), you
  provide a name and a parameter object describing the configuration.
- Calling ``.start()`` launches the appropriate communication process(es) in
  the background.
- Use ``.read()`` and ``.write()`` methods to exchange data with the
  communication backend.
  These methods interact with the process-safe queues and handle process health
  checks automatically.
- Call ``.shutdown(block=True)`` to terminate all background processes cleanly
  when done.

**Module Structure:**

- ``__init__.py``: Declares the package and provides the high-level package
  docstring.
- ``com_interface.py``: Defines the base classes for all communication
  interfaces and processes, handling process control, lifecycle, and
  inter-process events.
- ``can_com.py``: Implements CAN-based communication with a background process
  managing the CAN connection.
- ``file_com.py``: Implements file-based communication via separate reader and
  writer processes.
- ``mqtt_com.py``: Implements MQTT-based communication with a background
  process managing the MQTT client connection and message routing.
- ``parameter.py``: Contains all data classes for configuration and process
  control (including ``ComControl``, ``MQTTParameter``, ``FileParameter``, and
  ``CANLoggerParameter``).

This framework is particularly useful for applications that require
decoupled or parallel data transfer.

Parameter Objects
-----------------

- :class:`cli.com.parameter.FileParameter`
    - ``input_file`` (str or Path, optional): File to read from.
    - ``output_file`` (str or Path, optional): File to write to.
    - ``encoding`` (str): File encoding (default: "utf-8").
- :class:`cli.com.parameter.MQTTParameter`
    - ``broker`` (str): MQTT broker address.
    - ``port`` (int): Broker port.
    - ``subscribe`` (list of str): Topics to subscribe to.
    - ``tls_cert`` (str, optional): Path to TLS certificate.
    - ``username`` (str, optional): MQTT username.
    - ``password`` (str, optional): MQTT password.
- :class:`cli.helpers.fcan.CanBusConfig`
    - ``interface`` (str): Used CAN interface.
    - ``channel`` (str | int, optional): Channel name or number.
    - ``bitrate`` (int, optional): Bitrate used for CAN communication.
    - ``dbc`` (Path, optional): Path to a .dbc file used for encoding/decoding.

Example
-------

**File Communication Example**

.. code-block:: python

   # Setup parameters for file communication
   file_para = FileParameter(
       input_file="input.txt",
       output_file="output.txt"
   )
   file_com = File("File Communication", file_para)
   file_com.start()

   # Read lines from input file (if input_file is set)
   line = file_com.read()
   while line:
       print("Read from file:", line)
       line = file_com.read()

   # Write a line to output file (if output_file is set)
   file_com.write("Hello output file!")

   # Shutdown after communication and block until all processes have terminated
   file_com.shutdown(block=True)

Architecture
------------

.. figure:: ../../../../build/docs/docs/tools/fox/com/communication_file.svg
   :alt: File Composition Diagram
   :name: File Composition Diagram
   :width: 70 %
   :align: center

   File Communication Architecture

.. figure:: ../../../../build/docs/docs/tools/fox/com/communication_mqtt.svg
   :alt: MQTT Composition Diagram
   :name: MQTT Composition Diagram
   :width: 70 %
   :align: center

   MQTT Communication Architecture

.. figure:: ../../../../build/docs/docs/tools/fox/com/communication_can.svg
   :alt: CAN Composition Diagram
   :name: CAN Composition Diagram
   :width: 70 %
   :align: center

   CAN Communication Architecture


CAN Configuration
-----------------

The configuration file is a YAML document with at least the following sections:

- connection: parameters to initialize the CAN bus (python-can).
- logger: parameters for CAN logging.

An example skeleton (adjust to your environment):

.. code-block:: yaml

   connection:
     interface: socketcan        # e.g., 'socketcan', 'pcan', 'kvaser', ...
     channel: can0               # e.g., 'can0', 'PCAN_USBBUS1', ...
     bitrate: 500000             # bus bitrate in bit/s
     dbc: ./example/example.dbc  # optional path to a DBC file

   logger:                       # optional settings for the CAN logger
     max_bytes: 65536            # max. number of bytes in each log file
     rollover_count: 0           # The starting number for each log file

Modbus Configuration File
-------------------------

The YAML configuration file for the Modbus TCP parameters is depicted below:

.. literalinclude:: example/modbus_config.yaml
   :language: yaml
   :start-after: start-include-in-docs
   :end-before: stop-include-in-docs
   :caption: Configuration for the modbus client subcommand

MQTT Configuration File
-----------------------

An example the configuration file for MQTT parameters is depicted below.:

.. literalinclude:: example/mqtt_config.yaml
   :language: yaml
   :start-after: start-include-in-docs
   :end-before: stop-include-in-docs
   :caption: Configuration for the mqtt subcommand
