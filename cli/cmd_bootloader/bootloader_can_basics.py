#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""This file implement the basic functions of CAN module (CanInterface) to send
and receive bootloader relevant CAN messages via CAN bus."""

import logging
import sys
import time
from enum import Enum
from pathlib import Path
from typing import Optional, Union, cast

import can
from can.interfaces.pcan import PcanBus
from can.interfaces.virtual import VirtualBus
from cantools import database
from cantools.typechecking import DecodeResultType, SignalDictType

from ..helpers.misc import BOOTLOADER_DBC_FILE
from .bootloader_can_messages import (
    AcknowledgeMessageType,
    BootloaderFsmStatesType,
    BootloaderVersionInfoType,
    DataTransferInfoType,
    Messages,
    StatusCode,
)


class BootloaderCanBasics:
    """
    This class implements all CAN relevant functions and can be used to wait or
    receive specified messages on CAN bus.
    """

    def __init__(
        self,
        can_bus: Union[VirtualBus, PcanBus, can.BusABC],
        dbc_file: Path = BOOTLOADER_DBC_FILE,
    ):
        """Init function.

        Args:
            can_bus: CAN bus object
        """
        if not dbc_file.is_file():
            sys.exit(f"File '{dbc_file}' does not exist.")
        db = database.load_file(dbc_file)
        self.db = cast(database.can.database.Database, db)
        self.can_bus = can_bus
        self.messages = Messages()

    # pylint: disable=too-many-arguments,too-many-positional-arguments
    def wait_can_message(
        self,
        arbitration_id: int,
        dbc_file: Optional[Path] = None,
        mux_value: Optional[str] = None,
        params: Optional[dict] = None,
        timeout_total: float = 1.0,
        timeout_bus_recv: float = 1.0,
    ) -> Optional[DecodeResultType]:
        """This function wait for a specified CAN message.
        Args:
            arbitration_id: id of the CAN message e.g., 0x480
            params: a dictionary that contains the variable name
            and its value to be filtered. e.g., {'AcknowledgeFlag':'Received'}.
            In this case, a message (with the required arbitration_id) will
            be thrown away if the value of 'AcknowledgeFlag' is different
            from 'Received'.
            timeout_total: the time limit [s] for the whole waiting process.
            timeout_bus_recv: the time limit [s] to receive one message.

        Returns:
            The waited message, which value will be replaced with a string of
            enums if it is defined in the dbc file.
        """

        if not dbc_file:
            db = self.db
        else:
            if not dbc_file.is_file():
                sys.exit(f"File '{dbc_file}' does not exist.")
            else:
                db = database.load_file(dbc_file)

        if not params:
            params = {}
        goto_next_message = False
        start_time = time.time()
        while (not timeout_total) or (time.time() - start_time <= timeout_total):
            message = self.can_bus.recv(timeout=timeout_bus_recv)
            if (message is not None) and (message.arbitration_id == arbitration_id):
                msg = db.decode_message(message.arbitration_id, message.data)
                msg = cast(SignalDictType, msg)
                if mux_value:
                    # Exit in advance if mux value does not match
                    mux_key_name = list(msg.keys())[0]
                    if "_Mux" not in mux_key_name:
                        return None
                    if msg[mux_key_name] != mux_value:
                        return None
                    msg.pop(mux_key_name)
                for key in params.keys():
                    if key in msg.keys():
                        if msg[key] != params[key]:
                            goto_next_message = True
                            break
                if goto_next_message:
                    goto_next_message = False
                    continue
                logging.debug("Received message: %s", msg)
                return msg
        return None

    def send_can_message_to_bootloader(self, *args: dict) -> None:
        """The function is to send any CAN messages contained in the dbc file.

        Args:
            *args: list of CAN messages to be sent.
        """
        # Send message on the bus
        for msg in args:
            msg_name = msg["Name"]
            msg.pop("Name")
            logging.debug("Sending CAN message: '%s': '%s'", msg_name, msg)
            message = self.db.get_message_by_name(msg_name)
            data = message.encode(msg)
            message_send = can.Message(
                arbitration_id=message.frame_id, data=data, is_extended_id=False
            )
            try:
                self.can_bus.send(message_send)
                logging.debug("Message sent on '%s'.", self.can_bus.channel_info)
            except can.CanError as e:
                logging.error("'%s', message not sent.", e)

    def send_request_to_bootloader(self, request_code: Enum) -> None:
        """The function is to send a request CAN message to bootloader.

        Args:
            request_code: a member of the enum RequestCode.
        """
        self.send_can_message_to_bootloader(
            self.messages.get_message_request_msg(request_code)
        )

    def send_data_to_bootloader(self, data_8_bytes: int) -> None:
        """The function is to send a data CAN message (8 bytes) to bootloader.

        Args:
            data_8_bytes: maximal 0xffffffffffffffff.
        """
        self.send_can_message_to_bootloader(
            self.messages.get_message_data_8_bytes(data_8_bytes)
        )

    def send_crc_to_bootloader(self, crc_8_bytes: int) -> None:
        """The function is to send a CRC CAN message (8 bytes) to bootloader.

        Args:
            crc_8_bytes: maximal 0xffffffffffffffff.
        """
        self.send_can_message_to_bootloader(
            self.messages.get_message_crc_8_bytes(crc_8_bytes)
        )

    def send_transfer_program_info_to_bootloader(
        self, len_of_program_in_bytes: int, num_of_transfer_loops: int
    ) -> None:
        """The function is to send a transfer process info CAN message to the
        bootloader.

        Args:
           len_of_program: the length of the program in bytes.
           num_of_transfer_loops: the number of total transfer loops.
        """
        # len_of_program: check int value (uint32 max?)
        self.send_can_message_to_bootloader(
            self.messages.get_message_transfer_program_info(
                len_of_program_in_bytes, num_of_transfer_loops
            )
        )

    def send_loop_number_to_bootloader(self, num_of_loop: int) -> None:
        """Send loop number CAN message to bootloader.

        Args:
            num_of_loop: the number of data transfer loops.
        """
        # num_of_loop: check int value (uint16 max?)
        self.send_can_message_to_bootloader(
            self.messages.get_message_loop_info(num_of_loop)
        )

    def wait_bootloader_state_msg(self) -> Optional[BootloaderFsmStatesType]:
        """Waits for the CAN messages containing the state of bootloader.

        Returns:
            CAN message containing FSM states of bootloader.
        """
        arbitration_id_bootloader_fsm_states = self.db.get_message_by_name(
            "f_BootloaderFsmStates"
        ).frame_id
        msg = self.wait_can_message(arbitration_id_bootloader_fsm_states)
        msg_bootloader_fsm_states = cast(BootloaderFsmStatesType, msg)
        return msg_bootloader_fsm_states

    def wait_data_transfer_info_msg(self) -> Optional[DataTransferInfoType]:
        """This function is to wait for the messages contain the information of the data transfer.

        Returns:
            CAN message containing data transfer info (the current loop number).
        """
        arbitration_id_data_transfer_info = self.db.get_message_by_name(
            "f_DataTransferInfo"
        ).frame_id
        msg = self.wait_can_message(arbitration_id_data_transfer_info)
        msg_data_transfer_info = cast(DataTransferInfoType, msg)
        return msg_data_transfer_info

    def wait_bootloader_version_info_msg(self) -> Optional[BootloaderVersionInfoType]:
        """This function is to wait for the messages contain the version information
        of the bootloader.

        Returns:
            CAN message containing data transfer info (the current loop number).
        """
        arbitration_id_bootloader_version_info = self.db.get_message_by_name(
            "f_BootloaderVersionInfo"
        ).frame_id
        msg = self.wait_can_message(
            arbitration_id_bootloader_version_info, mux_value="BootloaderVersionInfo"
        )
        msg_bootloader_version_info = cast(BootloaderVersionInfoType, msg)
        return msg_bootloader_version_info

    def wait_can_ack_msg(
        self,
        acknowledge_msg: Enum,
        timeout_total: float = 1.0,
        timeout_ack: float = 0.3,
        processed_level: Enum = StatusCode.ReceivedAndProcessed,
    ) -> Optional[AcknowledgeMessageType]:
        """Wait for a certain CAN acknowledge message

        Args:
            acknowledge_msg: the type of ACK message
            timeout_total: total time to wait
            timeout_ack: time to wait for a single message
            processed_level: the ACK message need to have at least this
                processed_level

        Returns:
            acknowledged message
        """
        start_time = time.time()
        while time.time() - start_time <= timeout_total:
            message = self.can_bus.recv(timeout=timeout_ack)
            if message and (message.arbitration_id == 0x480):
                msg = self.db.decode_message(message.arbitration_id, message.data)
                msg_ack = cast(AcknowledgeMessageType, msg)
                if (
                    (msg_ack["AcknowledgeFlag"] == "Received")
                    and (msg_ack["AcknowledgeMessage"] == acknowledge_msg.name)
                    and msg_ack["StatusCode"] == processed_level.name
                ):
                    return msg_ack
        return None
