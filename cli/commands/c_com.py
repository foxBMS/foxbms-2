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

"""Click commands for communication test utilities (CAN, Modbus, MQTT)."""

from pathlib import Path

import click
from pymodbus.datastore import (
    ModbusServerContext,
    ModbusSlaveContext,
    ModbusSparseDataBlock,
)
from pymodbus.server import StartTcpServer

from ..cmd_com.com_impl import can_main, modbus_main, mqtt_main
from ..helpers import TOOL
from ..helpers.click_helpers import HELP_NAMES, secho, verbosity_option


@click.command(
    context_settings=HELP_NAMES,
    epilog="""
    Examples:

    As config should be passed a yaml file defining the communication of the
    CAN bus as the example below:

        \b
        connection:
            interface: pcan
            bitrate: 500000
            channel: PCAN_USBBUS2
        logger:
            max_bytes: int = 65536
            rollover_count: int = 0
            log_can_files: bool = False

    As input file should be used a text file with line deliminated json
    objects as below:

        \b
        {"id": 291, "data": [1, 2, 3, 4]}
        {"id": 291, "data": [5, 6, 7, 8]}

    The output file will contain the received CAN messages in a line
    deliminated json object format.""",
)
@click.option(
    "-c",
    "--config",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="Path to configuration yaml file",
)
@click.option(
    "-i",
    "--input",
    "input_file",
    type=click.Path(exists=False, file_okay=True, dir_okay=False, path_type=Path),
    required=False,
    help="Path to the input file",
)
@click.option(
    "-o",
    "--output",
    "output_file",
    type=click.Path(exists=False, file_okay=True, dir_okay=False, path_type=Path),
    required=False,
    help="Path to the output file",
)
@verbosity_option
@click.pass_context
def can(
    ctx: click.Context,
    config: Path,
    input_file: Path | None,
    output_file: Path | None,
    verbose: int,
) -> None:
    """Send and receive CAN messages using a YAML connection config."""
    can_main(config, input_file, output_file)
    ctx.exit(0)


@click.command(
    context_settings=HELP_NAMES,
    epilog="""Available register:

    - coil: address 10, length 4, default value 0

    - discrete input: address 20, length 8, default value 1

    - input register:

               - address 40, length 4, default values 20, 210, 450, 800

               - address 60, length 1, default value 100

    - holding register:

               - address 10, length 1, default value 225

               - address 20, length 1, default value 400

On this device, the registers don't share a memory. The client command uses
zeromode for the request addresses, therefore to read from coil at address 10,
the request address must be 9.

Example request:

    - {"date": "2025-11-25T00:00:00","code": "read_coils", "address": 9, "length": 1, "values": null}

""",
)
@click.option(
    "-a",
    "--address",
    type=str,
    default="localhost",
    help="Ip address of the Modbus device.",
)
@click.option(
    "-p",
    "--port",
    type=int,
    default=502,
    help="Path to configuration yaml file",
)
@verbosity_option
@click.pass_context
def device(
    ctx: click.Context,
    address: str,
    port: int,
    verbose: int,
) -> None:
    """Start a local Modbus TCP device (server) for testing and development.

    The call blocks until the server is stopped (e.g., via Ctrl+C).
    """
    coil_block = ModbusSparseDataBlock({10: [0] * 4})
    di_block = ModbusSparseDataBlock({20: [1] * 8})
    ir_block = ModbusSparseDataBlock({40: [20, 210, 450, 800], 60: [100]})
    hr_block = ModbusSparseDataBlock({10: [225], 20: [400]})
    slave = ModbusSlaveContext(di=di_block, co=coil_block, ir=ir_block, hr=hr_block)
    server_context = ModbusServerContext(slaves=slave, single=True)
    secho("Server has started")
    StartTcpServer(context=server_context, address=(address, port))
    secho("Server has stopped")
    ctx.exit(0)


@click.command(
    context_settings=HELP_NAMES,
    epilog=f"""
The configuration file defines connection and register mapping details.
The input file contains the Modbus commands (requests).

Example execution:

    {TOOL} modbus client -c config.yaml -i input.jsonl -o output.jsonl

Example command:

    {{"date": "2025-01-01T00:00:00.000", "code": "read_coils", "address": 9, \
        "length": 1, "values": null}}

""",
)
@click.option(
    "-c",
    "--config",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="Path to configuration yaml file",
)
@click.option(
    "-i",
    "--input",
    "input_file",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="Path to the input file",
)
@click.option(
    "-o",
    "--output",
    "output_file",
    type=click.Path(exists=False, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="Path to the output file",
)
@verbosity_option
@click.pass_context
def client(
    ctx: click.Context,
    config: Path,
    input_file: Path,
    output_file: Path,
    verbose: int,
) -> None:
    """Run the Modbus client workflow using a YAML configuration.

    Reads requests from ``input_file`` and writes results to ``output_file``.
    """
    modbus_main(config, input_file, output_file)
    ctx.exit(0)


@click.group(context_settings=HELP_NAMES)
def modbus() -> None:
    """Modbus subcommand group entry point."""


modbus.add_command(client)
modbus.add_command(device)


@click.command(
    context_settings=HELP_NAMES,
    epilog="""
    Examples:

    As config should be passed a yaml file defining the communication to the
    MQTT broker as the example below:

        \b
        broker: "broker.emqx.io"
        port: 1883
        subscribe: ["test_fox"]

    As input file should be used a text file with line deliminated json
    objects as below:

        \b
        {"topic": "test_fox", "data": "msg_1"}
        {"topic": "test_fox", "data": "msg_2"}

    The output file will contain the received MQTT messages in a line
    deliminated json object format.""",
)
@click.option(
    "-c",
    "--config",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="Path to configuration yaml file",
)
@click.option(
    "-i",
    "--input",
    "input_file",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=False,
    help="Path to the input file",
)
@click.option(
    "-o",
    "--output",
    "output_file",
    type=click.Path(exists=False, file_okay=True, dir_okay=False, path_type=Path),
    required=False,
    help="Path to the output file",
)
@verbosity_option
@click.pass_context
def mqtt(
    ctx: click.Context,
    config: Path,
    input_file: Path | None,
    output_file: Path | None,
    verbose: int,
) -> None:
    """Send and receive MQTT messages using a YAML connection config."""
    mqtt_main(config, input_file, output_file)
    ctx.exit(0)


@click.group(context_settings=HELP_NAMES)
def com_test() -> None:
    """Communication command group entry point.

    These scripts are used to test the communication with foxCLI-COM wrapper.
    """


com_test.add_command(can)
com_test.add_command(modbus)
com_test.add_command(mqtt)
