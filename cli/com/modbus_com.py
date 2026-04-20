#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# - "This product uses parts of foxBMS®"
# - "This product includes parts of foxBMS®"
# - "This product is derived from foxBMS®"

"""Implements the a basic Modbus communication functionalities"""

import json
import signal
from queue import Empty
from time import sleep

from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ModbusException
from pymodbus.pdu import ExceptionResponse

from ..helpers.logger import add_queue_handler, logger
from .com_interface import ComInterface, ProcessInterface
from .parameter import ComControl, ModbusParameter


class Modbus(ComInterface):
    """High-level adapter for Modbus process-based communication.

    This class manages the lifecycle of a ModbusProcess and provides
    queue-based read/write methods for commands and results.
    """

    def __init__(self, name: str, parameter: ModbusParameter) -> None:
        """Initialize the Modbus communication interface.

        Args:
            name (str): A human-readable name for the interface (used for logging).
            parameter (ModbusParameter): Configuration parameters for Modbus TCP.
        """
        super().__init__(name=name)
        # Register the background process that will own the Modbus client.
        self._processes[ModbusProcess.__name__] = ModbusProcess(
            name, self.control, parameter
        )

    def read(self, block: bool = False, timeout: float | None = None) -> dict | None:
        """Read a result dictionary from the output queue.

        Args:
            block (bool): Whether to block until a result is available.
            timeout (float | None): Optional timeout (seconds) when blocking.

        Returns:
            dict | None: A command/result dictionary (including 'values') if
            available, otherwise None.

        Notes:
            Returns None if the process is not alive and the output queue is empty,
            to avoid blocking forever.
        """
        # If the related process is not alive and the queue is empty, reading would block.
        if self.control.output.empty() and not self.is_alive():
            debug_text = (
                "Input queue can not be read, because related process is not running."
            )
            logger.debug(debug_text)
            return None
        return self.control.output.get(block=block, timeout=timeout)

    def write(self, command: dict) -> None:
        """Send a Modbus command to the input queue for execution.

        Args:
            command (dict): The Modbus command as a dictionary. Expected keys:
                'date', 'code', 'address', 'length', 'values'.

        Raises:
            RuntimeError: If the Modbus process is not running.
        """
        if not self.is_alive():
            error_text = "Output queue can not be written, because related process is not running."
            raise RuntimeError(error_text)
        # Enqueue the command for the background process to consume.
        self.control.input.put(command)


class ModbusProcess(ProcessInterface):
    """Background process that owns a ModbusTcpClient and executes commands.

    The process:

    - Connects to a Modbus TCP server.
    - Pulls commands from the input queue.

    - Executes the appropriate pymodbus method based on 'code'.
    - Pushes results (including 'values') onto the output queue.

    """

    def __init__(
        self, name: str, control: ComControl, parameter: ModbusParameter
    ) -> None:
        """Initialize the Modbus process.

        Args:
            name (str): Process name (used for logging).
            control (ComControl): Inter-process control and queues.
            parameter (ModbusParameter): Modbus TCP configuration parameters.
        """
        super().__init__(name=name, daemon=True)
        self.name = name
        self.control = control
        self.parameter = parameter
        self._client: ModbusTcpClient | None = None

    def __del__(self) -> None:
        """Ensures Modbus client is closed."""
        if isinstance(self._client, ModbusTcpClient):
            self._client.close()

    def run(self) -> None:
        """Process entry point.

        - Ignores SIGINT to avoid duplicate handling in child processes.
        - Adds a queue-based logging handler for inter-process logs.

        - Connects the Modbus TCP client.
        - Signals readiness, then starts the command execution loop.

        """
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        add_queue_handler(self.control.logger)
        logger.setLevel(self.control.log_level)
        # The client object must be created here (not in __init__)
        # to avoid "cannot pickle '_thread.lock' object" exceptions.
        self._client = self._connect()
        logger.debug("Modbus client connected.")
        if isinstance(self._client, ModbusTcpClient):
            self.control.ready.set()
            self._execute_command()

    def _connect(self) -> ModbusTcpClient | None:
        """Create and connect a ModbusTcpClient.

        Returns:
            ModbusTcpClient | None: A connected client; None if the connection
            could not be established (the process will be shut down).
        """
        client = ModbusTcpClient(
            host=self.parameter.host,
            port=self.parameter.port,
            timeout=self.parameter.timeout,
            retries=self.parameter.retries,
        )
        connected = client.connect()
        if not connected:
            # Connection failed; request a clean shutdown.
            self.shutdown()
            return None
        return client

    def _execute_command(self) -> None:
        """Execute commands with consolidated error handling.

        This wrapper catches JSON and Modbus-related exceptions, logs them,
        and ensures the process is shut down afterward.
        """
        try:
            self._execute_command_sub()
        except json.decoder.JSONDecodeError as e:
            logger.error(
                "The command '%s' in the '%s' queue "
                "can not be converted to a dictionary.",
                e.doc,
                self.name,
            )
        except KeyError:
            logger.error(
                "One command from the input read by '%s' does "
                "not contain the keyword date', 'code', 'address', 'length' "
                "and 'values'.",
                self.name,
            )
        except ModbusException as err:
            logger.error("Could not send request: %s", err)
        # Stop the process after handling the command loop.
        self.shutdown()

    def _execute_command_sub(self) -> None:
        """Core command execution loop.

        Expects commands with keys: 'code', 'address', 'length', 'values'.

        For read operations:

            - 'values' is populated from the result (bits/registers)

        For write operations:

            - 'values' is echoed from the result (if available)

        Unknown codes and Modbus exception responses are either logged as debug
        or error depending on ModbusParameter.ignore. Results are put on the
        output queue for the caller to consume.
        """
        if not isinstance(self._client, ModbusTcpClient):
            logger.error("Client has not yet started.")
            self.shutdown()
            return
        result = None
        while not self.control.shutdown.is_set():
            try:
                # Non-blocking read of the next command.
                command = self.control.input.get(block=False)
                code = command["code"]
                address = command["address"]
                length = command["length"]
                values = command["values"]

                # Dispatch the command based on 'code'.
                match code:
                    case "read_coils":
                        result = self._client.read_coils(
                            address=address, count=length, slave=0
                        )
                        # Slice to requested length to avoid trailing padding.
                    case "read_discrete_input":
                        result = self._client.read_discrete_inputs(
                            address=address, count=length, slave=0
                        )
                    case "read_input_register":
                        result = self._client.read_input_registers(
                            address=address, count=length, slave=0
                        )
                    case "read_holding_register":
                        result = self._client.read_holding_registers(
                            address=address, count=length, slave=0
                        )
                    case "write_coils":
                        result = self._client.write_coils(
                            address=address, values=values, slave=0
                        )
                    case "write_holding_register":
                        result = self._client.write_registers(
                            address=address, values=values, slave=0
                        )
                    case _:
                        # Unknown command code: log and optionally stop.
                        if self.parameter.ignore:
                            logger.debug("Unkown command code: '%s'", code)
                            values = None
                            command["error"] = "Unknown Code"
                        else:
                            logger.error("Unkown command code: '%s'", code)
                            break
                # Check for Modbus exception responses from the server.
                if isinstance(result, ExceptionResponse):
                    if self.parameter.ignore:
                        logger.debug("Command exception: %s", str(result))
                        values = None
                        command["error"] = ExceptionResponse
                    else:
                        logger.error("Command exception: %s", str(result))
                        break
                    values = [ExceptionResponse]
                elif result is not None:
                    if code in ["read_coils", "read_discrete_input", "write_coils"]:
                        values = result.bits[0:length]
                    else:
                        values = result.registers
                command["values"] = values
                self.control.output.put(command)
            except Empty:
                # No command available; sleep briefly to avoid busy-waiting.
                sleep(0.01)
                continue
