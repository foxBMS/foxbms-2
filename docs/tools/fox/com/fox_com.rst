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

- **Unified Interface:** Provides a common interface for different communication backends (e.g., file, MQTT), which can be easily integrated into user scripts.
- **Process-based Architecture:** Each communication backend (file or MQTT) runs in its own managed process, ensuring non-blocking operation and improved reliability through multiprocessing.
- **Inter-Process Communication:** Utilizes `multiprocessing.Queue` for safe, efficient message transfer between user code and communication processes.
- **Event-based Control:** Startup, readiness, and shutdown of communication processes is controlled via events, ensuring predictable and robust lifecycle management.
- **Extensible:** Easily add new communication backends by implementing the provided abstract base classes (`ComInterface`, `ProcessInterface`).

**How it works:**

- When you instantiate a communication object (e.g., `File` or `MQTT`), you provide a name and a parameter object describing the configuration.
- Calling `.start()` launches the appropriate communication process(es) in the background.
- Use `.read()` and `.write()` methods to exchange data with the communication backend. These methods interact with the process-safe queues and handle process health checks automatically.
- Call `.shutdown(block=True)` to terminate all background processes cleanly when done.

**Module Structure:**

- ``__init__.py``: Declares the package and provides the high-level package docstring.
- ``com_interface.py``: Defines the base classes for all communication interfaces and processes, handling process control, lifecycle, and inter-process events.
- ``can_com.py``: Implements CAN-based communication with a background process managing the CAN connection.
- ``file_com.py``: Implements file-based communication via separate reader and writer processes.
- ``mqtt_com.py``: Implements MQTT-based communication with a background process managing the MQTT client connection and message routing.
- ``parameter.py``: Contains all data classes for configuration and process control (including `ComControl`, `MQTTParameter`, `FileParameter`, and `CANLoggerParameter`).

This framework is particularly useful for applications that require
decoupled or parallel data transfer.

Parameter Objects
-----------------

- :class:`FileParameter`
    - `input_file` (str or Path, optional): File to read from.
    - `output_file` (str or Path, optional): File to write to.
    - `encoding` (str): File encoding (default: "utf-8").
- :class:`MQTTParameter`
    - `broker` (str): MQTT broker address.
    - `port` (int): Broker port.
    - `subscribe` (list of str): Topics to subscribe to.
    - `tls_cert` (str, optional): Path to TLS certificate.
    - `username` (str, optional): MQTT username.
    - `password` (str, optional): MQTT password.
- :class:`CANParameter`
    - `interface` (str): Used CAN interface.
    - `channel` (str | int, optional): Channel name or number.
    - `bitrate` (int, optional): Bitrate used for CAN communication.
    - `dbc` (Path, optional): Path to a .dbc file used for encoding/decoding.

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

.. drawio-figure:: ./communication_file.drawio
   :format: svg
   :alt: File Composition Diagram
   :name: File Composition Diagram
   :width: 70 %
   :align: center

   File Communication Architecture

.. drawio-figure:: ./communication_mqtt.drawio
   :format: svg
   :alt: MQTT Composition Diagram
   :name: MQTT Composition Diagram
   :width: 70 %
   :align: center

   MQTT Communication Architecture

.. drawio-figure:: ./communication_can.drawio
   :format: svg
   :alt: CAN Composition Diagram
   :name: CAN Composition Diagram
   :width: 70 %
   :align: center

   CAN Communication Architecture


can
===

This program implements a command line interface (CLI) to test sending and
receiving of CAN messages using a configuration and optional a input/output
files.

Usage
-----

.. include:: ./../../../../build/docs/fox_com-test_can_help.txt

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 com-test can -c <config_file> -i [<input_file>] -o [<output_file>]

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh com-test can -c <config_file> -i [<input_file>] -o [<output_file>]

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh com-test can -c <config_file> -i [<input_file>] -o [<output_file>]

Features
--------

- Reads CAN connection parameters from the provided configuration file.
- Sends CAN messages specified in the input file.
- Writes received CAN messages to the output file in the python-can format.
- Supports DBC-based encoding of signal dictionaries (if a DBC file is provided).
- Tolerates transient CAN receive errors; stops if too many errors occur in a short time.

Input File Format
-----------------

Each line in the input file must be a valid JSON object describing a CAN message to send.
Two formats are supported:

1) Raw payload (no DBC encoding):

.. code-block:: json

   { "id": 291, "data": [1, 2, 3, 4, 5, 6, 7, 8] }

- id: integer message identifier in decimal representation (standard ID expected).
- data: list of integers (0-255), up to 8 bytes.

2) DBC-based signals (when a DBC is configured):

.. code-block:: json

   { "id": 291, "data": {"SignalA": 42, "SignalB": 1 }}

- id: integer message identifier that exists in the DBC file.
- data: object mapping signal names to values; signals are encoded using the DBC definition.

.. note::

   - Extended identifiers are not configurable via the input; standard IDs are used.
   - If the input line is not valid JSON, that line is ignored.

Output File Format
------------------

Each received CAN message is written into a text file.
A typical record contains the timestamp, arbitration id and data, for example:

.. code-block:: text

   Timestamp:        0.000000    ID:      123    S Rx                DL:  8    01 02 03 04 05 06 07 08

The exact structure may include additional fields depending on the python-can
message representation used by the logger.

.. important::

   The received CAN messages are saved into a file with the suffix .txt
   If a different file suffix is used, the SizedRotatingLogger from python-can
   will not be created and therefore the logging will not start!

Configuration
-------------

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

Example
-------

The following example uses a PCAN interface on Linux with a 500 kbit/s bus.
The command:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 com-test can -c can_config_send.yaml -i can_input.jsonl

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh com-test can -c can_config_send.yaml -i can_input.jsonl

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh com-test can -c can_config_send.yaml -i can_input.jsonl

With the configuration file can_config_send.yaml as seen below:

.. literalinclude:: example/can_config_send.yaml
   :language: yaml
   :start-after: start-include-in-docs
   :end-before: stop-include-in-docs
   :caption: Configuration for the can subcommand to send messages.

The input file could contain the following lines:

.. literalinclude:: example/can_input.jsonl
   :language: json
   :caption: Input file for the can subcommand.

Running the following command in a second terminal:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 com-test can -c can_config_recv.yaml -o can_output.txt

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh com-test can -c can_config_recv.yaml -o can_output.txt

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh com-test can -c can_config_recv.yaml -o can_output.txt

With the configuration file can_config_send.yaml as seen below:

.. literalinclude:: example/can_config_recv.yaml
   :language: yaml
   :start-after: start-include-in-docs
   :end-before: stop-include-in-docs
   :caption: Configuration for the can subcommand to recv messages.

The output file will contain the following lines:

.. literalinclude:: example/can_output.txt
   :language: text
   :caption: Output file for the can subcommand.

.. note::

   The SizedRotatingLogger from python-can will always keep the last line in
   memory, therefore in the output file the last line is missing.

.. hint::

   While sending CAN messages or logging to an output file, you can stop with
   Ctrl+C.
   The program will then shut down the CAN connection and the file logger
   gracefully.

The example configuration and the input can be downloaded below:

| :download:`Send Configuration <example/can_config_send.yaml>`
| :download:`Input <example/can_input.jsonl>`
| :download:`Recv Configuration <example/can_config_recv.yaml>`
| :download:`Output <example/can_output.txt>`

modbus
======

This program implements a command line interface (CLI) to execute Modbus TCP
commands (requests) as a client using configuration and input/output files.
It also provides a simple built-in Modbus TCP test device (server).

Subcommands
-----------

- ``modbus client`` - runs the Modbus client that reads JSON line commands,
  sends them to a Modbus device and writes response to another file.
- ``modbus device`` - starts a local Modbus TCP device (server) for testing.

Usage
-----

Client:
.......

.. include:: ./../../../../build/docs/fox_com-test_modbus_client_help.txt

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 modbus client -c <config_file> -i <input_file> -o <output_file>

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh modbus client -c <config_file> -i <input_file> -o <output_file>

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh modbus client -c <config_file> -i <input_file> -o <output_file>

Device (test server):
.....................

.. include:: ./../../../../build/docs/fox_com-test_modbus_device_help.txt

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 modbus device -a [address] -p [port]

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh modbus device -a [address] -p [port]

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh modbus device -a [address] -p [port]

Client Features
---------------

- Reads Modbus TCP connection parameters from the provided configuration file.
- Executes Modbus commands specified in the input file (JSON per line).
- Writes command results to the output file (JSON per line).
- Supports common Modbus function codes for coils, discrete inputs, input registers, and holding registers.
- Handles I/O via background processes for Modbus and file operations.

Supported Command Codes
-----------------------

.. csv-table:: Modbus device command arguments
   :file: command_codes.csv
   :widths: 50 50
   :header-rows: 1
   :align: center

Configuration File
------------------

The YAML configuration file for the Modbus TCP parameters is depicted below:

.. literalinclude:: example/modbus_config.yaml
   :language: yaml
   :start-after: start-include-in-docs
   :end-before: stop-include-in-docs
   :caption: Configuration for the modbus client subcommand

Input File Format
-----------------

Each line in the input file represents a command (request) and must be a
valid JSON object with the following structure:

.. code-block:: json

   { "date": "2025-01-01T12:00:00", "code": "read_coils", "address": 9, "length": 4, "values": null }
   { "date": "2025-01-01T12:00:01", "code": "write_holding_register", "address": 19, "length": 2, "values": [100, 200] }

.. important::

   In the used Python package ``PyModbus``, register addresses start at 0.
   Hence if a register in a device starts at 10, the command requires the address 9.

.. note::

   - The parameter ``values`` is ignored for read operations.
   - For write operations, ``values`` must be provided and is echoed in the response (if available).

Output File Format
------------------

Each result is written as a line-delimited JSON object mirroring the input structure,
with ``values`` populated from the Modbus response:

.. code-block:: json

   { "date": "2025-01-01T12:00:00", "code": "read_coils", "address": 9, "length": 4, "values": [0, 0, 1, 1] }
   { "date": "2025-01-01T12:00:01", "code": "write_holding_register", "address": 19, "length": 2, "values": [100, 200] }

.. hint::

   If ``values`` is None, an exception occurred and the command (request) could
   not be processed.
   Additionally an ``error``: ``exception`` key-value-pair is added to the output.

Device Register Map
-------------------

The table below documents the initial register state of the built-in Modbus TCP
device (test server).

.. csv-table:: Modbus Test Device Register Map
   :file: modbus_device_registers.csv
   :widths: 24 12 10 20 26
   :header-rows: 1
   :align: center

.. note::

   - Coils and Holding Registers are writable in device (test server); Discrete Inputs
     and Input Registers are read-only.

   - Default values shown are the initial values when the server starts.

.. hint::

   The register memory is not shared between blocks. See `explanation
   <https://pymodbus.readthedocs.io/en/latest/source/library/simulator/config.html#device-entries>`_.

Example
-------

Start the local test server:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 modbus device -a localhost -p 502

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh modbus device -a localhost -p 502

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh modbus device -a localhost -p 502

Run the client:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 modbus client -c modbus_config.yaml -i modbus_input.jsonl -o modbus_output.jsonl

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh modbus client -c modbus_config.yaml -i modbus_input.jsonl -o modbus_output.jsonl

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh modbus client -c modbus_config.yaml -i modbus_input.jsonl -o modbus_output.jsonl

Prepare the input file:

.. literalinclude:: example/modbus_input.jsonl
   :language: json
   :caption: Example input file for the modbus client subcommand.

Resulting output file:

.. literalinclude:: example/modbus_output.jsonl
   :language: json
   :caption: Example output file for the modbus client subcommand.

The example configuration and the input/output can be downloaded below:

| :download:`Configuration <example/modbus_config.yaml>`
| :download:`Input <example/modbus_input.jsonl>`
| :download:`Output <example/modbus_output.jsonl>`


Implementation Notes
--------------------

- The CLI orchestrates two child processes:

  - A Modbus process that owns the ``pymodbus.client.ModbusTcpClient`` and executes commands.

  - A file handler that reads input JSON lines and writes output JSON lines.

- Communication between processes uses queues; the client blocks up to 1 second while waiting for a result.

- If the Modbus server returns an exception (``pymodbus.pdu.ExceptionResponse``), the behavior depends on ``ignore``:

  - ``ignore: true`` --> log as debug and continue.

  - ``ignore: false`` --> log as error and stop the loop.

mqtt
====

This program implements a command line interface (CLI) to test sending and
receiving of MQTT messages as client using a configuration and optional
a input/output files.

Usage
-----

.. include:: ./../../../../build/docs/fox_com-test_mqtt_help.txt

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 com-test mqtt -c <config_file> -i [<input_file>] -o [<output_file>]

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh com-test mqtt -c <config_file> -i [<input_file>] -o [<output_file>]

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh com-test mqtt -c <config_file> -i [<input_file>] -o [<output_file>]

Features
--------

- Reads MQTT connection parameters from the provided configuration file.
- Sends MQTT messages specified in the input file.
- Writes received MQTT messages to the output file.
- Handles input/output as JSON lines.

Input File Format
-----------------

Each line in the input file should be a valid JSON object with the following
structure:

.. code-block:: json

   { "topic": "your/topic", "data": "your message" }

Output File Format
------------------

Each received MQTT message is written as a line deliminated JSON object
identical to input example.

Example
-------

The following example uses the public 'broker.emqx.io' MQTT broker with no
password, username or certificate.

Run the command:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 com-test mqtt -c config.yaml -i input.jsonl -o output.jsonl

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.sh com-test mqtt -c config.yaml -i input.jsonl -o output.jsonl

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh com-test mqtt -c config.yaml -i input.jsonl -o output.jsonl

Use the configuration file shown below:

.. literalinclude:: example/mqtt_config.yaml
   :language: yaml
   :start-after: start-include-in-docs
   :end-before: stop-include-in-docs
   :caption: Configuration for the mqtt subcommand

Prepare the input file:

.. literalinclude:: example/mqtt_input.jsonl
   :language: json
   :caption: Input file for the mqtt subcommand

Resulting output file:

.. literalinclude:: example/mqtt_output.jsonl
   :language: json
   :caption: Output file produced by the mqtt subcommand

The example configuration and the input can be downloaded below:

| :download:`Configuration <example/mqtt_config.yaml>`
| :download:`Input <example/mqtt_input.jsonl>`
| :download:`Output <example/mqtt_output.jsonl>`
