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

"""Defines all needed parameters for the supported communication."""

from dataclasses import dataclass, field
from multiprocessing import Event, Queue
from multiprocessing.synchronize import Event as EventClass
from pathlib import Path


@dataclass
class ComControl:
    """Data class for managing inter-process communication and control signals.
    Contains input/output queues, events, and logging facilities for processes.
    """

    # field with default_factory is needed, because otherwise all classes with
    # a ComControl would use the same queues and event.
    input: Queue = field(default_factory=Queue)
    output: Queue = field(default_factory=Queue)
    ready: EventClass = field(default_factory=Event)
    shutdown: EventClass = field(default_factory=Event)
    # Each process get its own logging queue and listener to prevent printing
    # same logging message multiple times.
    logger: Queue = field(default_factory=Queue)
    # Log level has to be passed to each process, because at the moment a
    # process is created the log level of the fox.py logger is always warning.
    log_level: int = 50


@dataclass(slots=True, frozen=True)
class ModbusParameter:
    """Immutable configuration for a Modbus TCP connection.

    Holds connection parameters used by a Modbus client.

    Attributes:
        host: IP address or hostname of the Modbus server.
        port: TCP port of the Modbus server. Defaults to 502.
        timeout: Request/response timeout in seconds. Defaults to 1.
        retries: Number of retry attempts on transient I/O errors. Defaults to 3.
        ignore: If True, specific exceptions are ignored.

    Notes:
        Instances are immutable (frozen=True) and use __slots__ for reduced memory footprint.
    """

    host: str
    port: int = 502
    timeout: int = 1
    retries: int = 3
    ignore: bool = False


@dataclass(slots=True, frozen=True)
class MQTTParameter:
    """Data class for MQTT connection parameters.

    Attributes:
        broker (str): MQTT broker address.
        port (int): Port number for MQTT broker.
        subscribe (list[str]): List of topics to subscribe to.
        tls_cert (str | None): Path to TLS certificate (optional).
        username (str | None): Username for authentication (optional).
        password (str | None): Password for authentication (optional).
    """

    broker: str
    port: int
    subscribe: list[str]
    tls_cert: str | None = None
    username: str | None = None
    password: str | None = None


@dataclass
class CANLoggerParameter:
    """Data class for CAN logger configuration parameters.

    Attributes:
        max_bytes (int): Maximum size of the log file in bytes before rollover.
        rollover_count (int): Number of rollover log files to keep.
        log_can_files (bool): Whether to enable CAN logging to files.
    """

    max_bytes: int = 65536
    rollover_count: int = 0
    log_can_files: bool = False


@dataclass
class FileParameter:
    """Data class for file-based communication configuration.

    Attributes:
        input_file (Path | None): Path to the input file (optional).
        output_file (Path | None): Path to the output file (optional).
        encoding (str): File encoding (default "utf-8").
        can_logger (CANLoggerParameter): CAN logger parameters.
    """

    input_file: Path | None = None
    output_file: Path | None = None
    encoding: str = "utf-8"
    can_logger: CANLoggerParameter = field(default_factory=CANLoggerParameter)

    def __post_init__(self) -> None:
        """Ensures that input_file and output_file are Path objects after initialization."""
        if self.input_file:
            self.input_file = Path(self.input_file)
        if self.output_file:
            self.output_file = Path(self.output_file)
