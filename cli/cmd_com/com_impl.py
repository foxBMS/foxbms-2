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

"""Implements a CLI command to test the available foxCLI-COM functionalities."""

import sys
from json import loads
from json.decoder import JSONDecodeError
from pathlib import Path
from queue import Empty

from ..com.can_com import CAN
from ..com.file_com import File
from ..com.modbus_com import Modbus
from ..com.mqtt_com import MQTT
from ..com.parameter import (
    CANLoggerParameter,
    FileParameter,
    ModbusParameter,
    MQTTParameter,
)
from ..helpers.click_helpers import recho, secho
from ..helpers.config import read_config
from ..helpers.fcan import CanBusConfig
from ..helpers.logger import logger


def can_main(config: Path, input_file: Path | None, output_file: Path | None) -> None:
    """Implementation of the can command"""
    config_dict = read_config(config)
    logger.debug(config_dict)
    can_para = CanBusConfig(**config_dict["connection"])
    can_com = CAN("CAN Connection", can_para)
    if output_file:
        output_file.parent.mkdir(exist_ok=True, parents=True)
    file_para = FileParameter(
        input_file=input_file,
        output_file=output_file,
        can_logger=CANLoggerParameter(**config_dict.get("logger", {})),
    )
    file_handler = File("Read and Write", file_para)
    logger.info("Init communication processes.")
    try:
        can_com.start()
        file_handler.start()
    except ChildProcessError as e:
        recho(str(e))
        sys.exit(1)
    logger.info("Start writing and reading with CAN bus")
    if output_file:
        secho("Logging is stopped with Ctrl+C.")
    while True:
        try:
            if input_file:
                if user_input := file_handler.read():
                    user_input_dict = loads(user_input)
                    logger.debug("Sent CAN message: %s", user_input_dict)
                    can_com.write(user_input_dict)
            if output_file:
                msg = can_com.read(block=True, timeout=1)
                logger.debug("Received CAN message: %s", msg)
                if msg:
                    file_handler.write_can_message(msg)
        except JSONDecodeError:
            recho("Some lines in the input file are not in the json format.")
            break
        except RuntimeError:
            # Handle the case where the input queue is empty
            break
        except KeyboardInterrupt:
            break
    secho("End of communication")
    # Timeout needed because a full queue will keep the CAN process and
    # logger will keep file_handler alive
    can_com.shutdown(block=True, timeout=1)
    file_handler.shutdown(block=True, timeout=1)


def modbus_main(config: Path, input_file: Path, output_file: Path) -> None:
    """CLI entry point that performs Modbus transactions based on an input file.

    It reads Modbus commands (as JSON lines) from input_file, sends them to a
    Modbus device, and writes the responses to output_file. The Modbus client
    and the file handler run in their own child processes. This function
    orchestrates those processes and the message flow via queues.

    Args:
        config (Path): Path to a configuration file containing Modbus parameters.
        input_file (Path): Path to the input file with JSON lines of commands.
        output_file (Path): Path to the output file to write the results.

    Exits:
        The process exits with code 1 if a child process cannot be started.

    """
    # Read Modbus configuration and construct a parameter object.
    config_dict = read_config(config)
    logger.debug(config_dict)

    # Build Modbus parameters (variable name kept as-is for compatibility).
    modbus_para = ModbusParameter(**config_dict)

    # Create Modbus client interface and the file handler.
    client = Modbus("Test Modbus connection", modbus_para)
    # Ensure the output directory exists before starting the file handler.
    output_file.parent.mkdir(exist_ok=True, parents=True)
    file_para = FileParameter(input_file=input_file, output_file=output_file)
    file_handler = File("Read and Write", file_para)

    logger.info("Init communication processes.")
    try:
        # Start child processes (Modbus and file I/O).
        client.start()
        file_handler.start()
    except ChildProcessError as e:
        # Starting processes failed; report and exit.
        recho(str(e))
        sys.exit(1)

    secho("Start the communication.")
    while True:
        try:
            # Read a single JSON command line from the file input.
            if user_input := file_handler.read():
                user_input_dict = loads(user_input)
                logger.info("Sent Modbus values: %s", user_input_dict)

                # Send the command to the Modbus process.
                client.write(user_input_dict)

                # Read the execution result (blocks up to timeout).
                result_dict = client.read(block=True, timeout=1)
                logger.info("Received Modbus values: %s", result_dict)

                # Write result back to the output file if available.
                file_handler.write(str(result_dict))
            else:
                # No more input; terminate the loop.
                break
        except Empty:
            # Queue is empty (no result); terminate gracefully.
            recho("Empty queue, process stopped.")
            break
        except JSONDecodeError:
            # Input contained malformed JSON.
            recho("Some lines in the input file are not in the json format.")
            break
        except KeyboardInterrupt:
            # Allow user interruption to stop the loop.
            break

    secho("End of communication.")

    # Ensure clean shutdown of child processes.
    client.shutdown(block=True, timeout=1)
    file_handler.shutdown(block=True, timeout=1)


def mqtt_main(config: Path, input_file: Path | None, output_file: Path | None) -> None:
    """Implementation of the mqtt command"""
    config_dict = read_config(config)
    logger.debug(config_dict)
    mqtt_para = MQTTParameter(**config_dict)
    client = MQTT("Test MQTT broker", mqtt_para)
    if output_file:
        output_file.parent.mkdir(exist_ok=True, parents=True)
    file_para = FileParameter(input_file=input_file, output_file=output_file)
    file_handler = File("Read and Write", file_para)
    logger.info("Init communication processes.")
    try:
        client.start()
        file_handler.start()
    except ChildProcessError as e:
        recho(str(e))
        sys.exit(1)
    logger.info("Start writing and reading")
    if output_file:
        secho("Logging is stopped with Ctrl+C.")
    while True:
        try:
            if input_file:
                if user_input := file_handler.read():
                    user_input_dict = loads(user_input)
                    logger.debug("Sent MQTT message: %s", user_input_dict)
                    client.write(user_input_dict)
            if output_file:
                message_dict = client.read(block=True, timeout=1)
                logger.debug("Received MQTT message: %s", message_dict)
                if message_dict:
                    file_handler.write(str(message_dict))
        except JSONDecodeError:
            recho("Some lines in the input file are not in the json format.")
            break
        except KeyboardInterrupt:
            break
    secho("End of communication.")
    client.shutdown(block=True, timeout=1)
    file_handler.shutdown(block=True, timeout=1)
