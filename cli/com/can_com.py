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

"""Implements the basic CAN communication functionalities"""

import signal
from dataclasses import asdict
from queue import Empty
from threading import Thread
from time import sleep, time
from typing import cast

from can import (
    BusABC,
    CanInitializationError,
    CanOperationError,
    Message,
    ThreadSafeBus,
)
from cantools import database
from cantools.database.can.database import Database
from cantools.database.errors import UnsupportedDatabaseFormatError

from ..helpers.click_helpers import recho
from ..helpers.fcan import CanBusConfig
from ..helpers.logger import add_queue_handler, logger
from .com_interface import ComInterface, ProcessInterface
from .parameter import ComControl

MAX_CAN_OPERATION_ERRORS_PER_HOUR = 10


class CAN(ComInterface):
    """Communication interface for CAN bus operations.
    Handles initialization and interaction with CANProcess for reading and writing messages.
    """

    def __init__(self, name: str, parameter: CanBusConfig) -> None:
        """Initializes the CAN communication interface.

        Args:
            name (str): Name of the CAN interface.
            parameter (CanBusConfig): Configuration parameters for the CAN bus.
        """
        super().__init__(name=name)
        self._processes[CANProcess.__name__] = CANProcess(name, self.control, parameter)

    def read(self, block: bool = False, timeout: float | None = None) -> Message | None:
        """Reads a CAN message from the output queue.

        Args:
            block (bool): Whether to block if the queue is empty.
            timeout (float | None): Maximum time to wait for a message.

        Returns:
            Message | None: The received CAN message, or None if not available.
        """
        # If the related process is not alive, this method would block otherwise.
        if self.control.output.empty():
            if not self.is_alive():
                debug_text = "Input queue can not be read, because related process is not running."
                logger.debug(debug_text)
            return None
        return self.control.output.get(block=block, timeout=timeout)

    def write(self, msg: Message | dict) -> None:
        """Sends a CAN message by placing it on the input queue.

        Args:
            msg (Message): The CAN message to send.

        Raises:
            RuntimeError: If the related CAN process is not running.
        """
        if not self.is_alive():
            error = "Output queue can not be written, because related process is not running."
            raise RuntimeError(error)
        self.control.input.put(msg)


class CANProcess(ProcessInterface):
    """Process for handling CAN bus communication.
    Manages low-level connections, sending, and receiving of CAN messages.
    """

    def __init__(self, name: str, control: ComControl, parameter: CanBusConfig) -> None:
        """Initializes the CAN process.

        Args:
            name (str): Name of the process.
            control (ComControl): Communication control object.
            parameter (CanBusConfig): CAN bus configuration parameters.
        """
        super().__init__(name=name, daemon=True)
        self.name = name
        self.control = control
        self.parameter = parameter
        self._bus: BusABC | None = None
        self._op_errs: list[float] = []

    def run(self) -> None:
        """Starts the CAN process:
        - Connects to the CAN bus
        - Starts a thread for sending messages
        - Receives CAN messages in the main thread
        """
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        add_queue_handler(self.control.logger)
        logger.setLevel(self.control.log_level)
        self._bus = self._connect()
        if self._bus:
            logger.info("CAN bus connected.")
            self.control.ready.set()
            Thread(target=self._receive_can_messages, daemon=True).start()
            self._write_can_messages()

    def _connect(self) -> BusABC | None:
        """Creates a connection to the CAN bus.

        Returns:
            Any | None: The CAN bus object if successful, None otherwise.

        Handles:
            CanInitializationError: If the CAN bus cannot be initialized.
        """
        try:
            return ThreadSafeBus(**asdict(self.parameter))
        except CanInitializationError:
            # Create a user facing error and exit gracefully
            recho("Could not initialize CAN bus.")
        self.shutdown()
        return None

    def _write_can_messages(self) -> None:
        """Continuously sends CAN messages from the input queue to the CAN bus.
        Runs in a separate thread.
        """
        if self._bus:
            dbc: Database | None = None
            if self.parameter.dbc:
                dbc = self._load_database()
            start_time = time()
            with self._bus as bus:
                logger.info("Start writing CAN messages to the bus.")
                try:
                    while not self.control.shutdown.is_set():
                        try:
                            msg = self.control.input.get(block=False)
                            if can_msg := CANProcess._get_can_msg(msg, start_time, dbc):
                                bus.send(can_msg)
                            else:
                                logger.debug(
                                    "Message '%s' is ignored. DBC file not valid or it "
                                    "is not a dict or Message object.",
                                    msg,
                                )
                        except Empty:
                            sleep(0.1)
                except KeyError:
                    logger.error(
                        "Provided json dictionary does not contain "
                        "at least one mandatory key id or data in the "
                        "in the defined CAN message."
                    )
                    self.shutdown()

    def _receive_can_messages(self) -> None:
        """Continuously receives CAN messages from the bus and puts them into the output queue.
        Implements error tolerance for CAN operation errors.
        """
        # We should tolerate a few receive errors to not lose an entire
        # log just because of a single event. If the error occurs several
        # times, then something is actually wrong, and we can stop logging.
        if self._bus:
            with self._bus as bus:
                logger.info("Start reading CAN messages from the bus.")
                while not self.control.shutdown.is_set():
                    try:
                        self._receive_can_message(bus)
                    except CanOperationError:
                        err_time = time()
                        self._op_errs = [
                            i for i in self._op_errs if not (err_time - i) > 3600
                        ] + [err_time]
                        if len(self._op_errs) >= MAX_CAN_OPERATION_ERRORS_PER_HOUR:
                            # if we get more than a certain number of errors per
                            # hour then something seems off and we can stop
                            # receiving messages
                            recho(
                                "Too many CAN errors occurred while receiving messages."
                            )
                            self.shutdown()

    def _receive_can_message(self, bus: BusABC) -> None:
        """Receives a CAN message from the CAN bus and puts it into the output queue.

        Args:
            bus: The CAN bus object to receive messages from.
        """
        while not self.control.shutdown.is_set():
            if msg := bus.recv(0.1):
                self.control.output.put(msg, timeout=0.1)

    def _load_database(self) -> Database | None:
        """Loads the CAN database from a dbc file"""
        try:
            return cast(
                Database, database.load_file(self.parameter.dbc, encoding="utf-8")
            )
        except UnsupportedDatabaseFormatError:
            recho("DBC format is not valid!")
            self.shutdown()
        except FileNotFoundError:
            recho("DBC file is not found!")
            self.shutdown()
        return None

    @staticmethod
    def _get_can_msg(
        msg: Message | dict, start_time: float, dbc: Database | None
    ) -> Message | None:
        """Provides a valid CAN message with respect to the passed parameter."""
        if isinstance(msg, Message):
            return msg
        if isinstance(msg, dict):
            msg_id = msg["id"]
            if isinstance(msg["data"], dict):
                if dbc:
                    msg_def: database.can.message.Message = dbc.get_message_by_frame_id(
                        msg_id
                    )
                    msg_data = msg_def.encode(msg["data"], padding=True)
                else:
                    return None
            else:
                msg_data = msg["data"]
            current_time = time() - start_time
            return Message(
                timestamp=current_time,
                is_extended_id=False,
                arbitration_id=msg_id,
                data=msg_data,
            )
        return None
