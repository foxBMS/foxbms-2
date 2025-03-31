#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements the functionalities behind the 'log' command"""

from dataclasses import asdict
from enum import Enum
from multiprocessing import Event, Process, Queue, synchronize
from pathlib import Path
from signal import SIG_IGN, SIGINT, signal
from time import sleep, time

from can import Bus, CanInitializationError, CanOperationError, Message
from can.io import SizedRotatingLogger
from click import secho

from ..helpers.click_helpers import recho
from ..helpers.fcan import CanBusConfig

MAX_CAN_OPERATION_ERRORS_PER_HOUR = 10


class LoggerExitCodes(Enum):
    """Exit codes"""

    NO_ERROR = 0
    RECEIVE_PROCESS_NOT_STARTED = 1
    CAN_BUS_INITIALIZATION_FAILED = 2
    LOGGER_NOT_STARTED = 3


class StopLogging(Exception):
    """Stop logging"""


def receive_can_message(
    data_q: "Queue[Message]", bus_cfg: CanBusConfig, network_ok: synchronize.Event
) -> None:
    """Receives the CAN messages from the bus."""
    signal(SIGINT, SIG_IGN)  # keyboard interrupt is caught in the parent process
    try:
        with Bus(**asdict(bus_cfg)) as bus:
            network_ok.set()  # ready to log
            # We should tolerate a few receive errors, to not lose an entire
            # log just because of a single event. If the error occurs several
            # times, then something is actually wrong, and we can stop logging
            op_errs: list[float] = []
            while True:
                try:  # recv may raise CanOperationError
                    while True:
                        if msg := bus.recv(0.001):
                            data_q.put(msg)
                        if not network_ok.is_set():
                            # the parent process requests to stop, therefore
                            # we raise a StopError to escape the while-loop
                            raise StopLogging
                except CanOperationError:
                    err_time = time()
                    op_errs = [i for i in op_errs if not (err_time - i) > 3600] + [
                        err_time
                    ]
                    if len(op_errs) >= MAX_CAN_OPERATION_ERRORS_PER_HOUR:
                        # if we get more than a certain number of errors per
                        # hour then something seems off and we can stop
                        # receiving messages
                        recho("Too many errors occurred while receiving messages.")
                        raise StopLogging  # pylint: disable=raise-missing-from
    except CanInitializationError:
        # Create a user facing error and exit gracefully
        recho("Could not initialize CAN bus.")
    except StopLogging:
        secho("Stop receiving messages.")
    network_ok.clear()


def log_can_message(
    data_q: "Queue[Message]", network_ok: synchronize.Event, logger: SizedRotatingLogger
) -> None:
    """Logs the CAN message to a file."""
    first_timestamp: float = 0
    try:
        secho("Start Logging", fg="green")
        # get the first message
        while network_ok.is_set():
            # store timestamp of first message to have the correct start time
            if msg := data_q.get():
                first_timestamp = msg.timestamp
                msg.timestamp = 0
                logger(msg)
                break
        while network_ok.is_set():
            if msg := data_q.get():
                msg.timestamp = msg.timestamp - first_timestamp  # Correct timestamp
                logger(msg)
    finally:
        if first_timestamp:  # log remaining CAN messages, if we received at least one
            while msg := data_q.get():
                msg.timestamp = msg.timestamp - first_timestamp
                logger(msg)
        logger.stop()


def log(bus_cfg: CanBusConfig, output: Path, log_file_size: int = 200000) -> int:
    """Logs received CAN messages to file(s)."""
    network_ok = Event()
    data_q: "Queue[Message]" = Queue()
    p_recv = Process(
        target=receive_can_message,
        args=(data_q, bus_cfg, network_ok),
        name="p_recv",
        daemon=True,
    )
    try:
        p_recv.start()
    except (OSError, RuntimeError, ValueError):
        recho("Could not start receive process.\nExiting...")
        return LoggerExitCodes.RECEIVE_PROCESS_NOT_STARTED.value
    if not network_ok.wait(3):  # Wait for bus setup process
        recho("Could not initialize CAN bus. Timeout\nShutdown...")
        return LoggerExitCodes.CAN_BUS_INITIALIZATION_FAILED.value

    output.mkdir(parents=True, exist_ok=True)

    try:
        logger = SizedRotatingLogger(
            base_filename=output / Path("foxBMS_CAN_log.txt"),
            max_bytes=log_file_size,
            encoding="utf-8",
            rollover_count=0,
        )
    except ValueError:
        recho("Could not create logger object.")
        network_ok.clear()
        p_recv.join(timeout=2)
        if p_recv.is_alive():
            recho("Could could not cancel the receive process gracefully.")
            recho("Terminating...")
            p_recv.terminate()
        recho("Shutdown...")
        return LoggerExitCodes.LOGGER_NOT_STARTED.value

    try:
        secho("Use Ctrl+C to stop logging.", fg="green")
        log_can_message(data_q, network_ok, logger)
    except KeyboardInterrupt:
        network_ok.clear()
        sleep(0.5)
        p_recv.join(timeout=2)
        if p_recv.is_alive():
            recho("Could could not cancel the receive process gracefully.")
            recho("Terminating...")
            p_recv.terminate()
    secho("Shutdown...", fg="green")
    return LoggerExitCodes.NO_ERROR.value
