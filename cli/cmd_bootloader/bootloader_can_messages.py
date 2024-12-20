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

"""This file contains all enums of CAN messages and functions to get specfied
can messages in dict. The members of the Enum classes that end with 'Str' have
the string type of value."""

import sys
from enum import Enum
from pathlib import Path
from typing import Optional, TypedDict, cast

import cantools
import cantools.database

from ..helpers.misc import BOOTLOADER_DBC_FILE


def extract_enum_from_dbc_file(
    enum_name: str, dbc_file: Path = BOOTLOADER_DBC_FILE
) -> Optional[Enum]:
    """Extract enum from the dbc file.

    Args:
        dbc_file: .dbc file where the definition of CAN messages are saved.
        enum_name: the name of the enum.

    Returns:
        enum with the name of the enum.
    """

    # Load the DBC file
    if not dbc_file.is_file():
        sys.exit(f"The input '{dbc_file}' is not a file.")

    db = cantools.database.load_file(dbc_file)
    db = cast(cantools.database.can.database.Database, db)
    # Loop through all messages to find the right one with right signal name
    for message in db.messages:
        for signal in message.signals:
            if signal.name == enum_name:
                if not signal.choices:
                    sys.exit(f"There is no enum for signal '{enum_name}'.")
                choices = {str(value): key for key, value in signal.choices.items()}
                dynamic_enum = Enum(enum_name, choices)
                return dynamic_enum

    # If no such message available
    sys.exit(f"Cannot find signal '{enum_name}' in the dbc file.")


# Extracted enums from dbc file
YesNoAnswer = extract_enum_from_dbc_file("YesNoAnswer")
AcknowledgeFlag = extract_enum_from_dbc_file("AcknowledgeFlag")
AcknowledgeMessage = extract_enum_from_dbc_file("AcknowledgeMessage")
StatusCode = extract_enum_from_dbc_file("StatusCode")
RequestCode = extract_enum_from_dbc_file("RequestCode8Bits")
CanFsmState = extract_enum_from_dbc_file("CanFsmState")
BootFsmState = extract_enum_from_dbc_file("BootFsmState")


class AcknowledgeMessageType(TypedDict):
    """This class specify the type of the received acknowledge message."""

    AcknowledgeFlag: str
    AcknowledgeMessage: str
    StatusCode: str
    YesNoAnswer: str


class DataTransferInfoType(TypedDict):
    """This class specify the type of the received data transfer info message."""

    CurrentLoopNumber: int


class BootloaderVersionInfoType(TypedDict):
    """This class specify the type of the received bootloader version info message."""

    DirtyFlag: int
    MajorVersionNumber: int
    MinorVersionNumber: int
    PatchVersionNumber: int
    ReleaseDistance: int
    ReleaseDistanceOverflow: int
    UnderVersionControl: int


class BootloaderFsmStatesType(TypedDict):
    """This class specify the type of the bootloader FSM states."""

    CanFsmState: str
    BootFsmState: str


class Messages:
    """This class provides the methods to get valid messages"""

    def __init__(self, dbc_file: Path = BOOTLOADER_DBC_FILE):
        if not dbc_file.is_file():
            sys.exit(f"The provided dbc file '{dbc_file}' does not exist.")

        db = cantools.database.load_file(dbc_file)
        self.db = cast(cantools.database.can.database.Database, db)
        self.message_names = [
            message.name for message in getattr(self.db, "messages", [])
        ]
        if not self.message_names:
            sys.exit("There are no messages in the database file.")

    def _get_message(self, name: str, **kwargs):
        """Get valid CAN message.

        Args:
            name: name of this CAN message.

        Returns:
            A valid CAN message.
        """
        if name in self.message_names:
            can_message = self.db.get_message_by_name(name)
        else:
            sys.exit(
                f"The name of message '{name}' cannot be found in the CAN database."
            )
        message = {}
        message["Name"] = name
        signal_names = [signal.name for signal in can_message.signals]
        for key, value in kwargs.items():
            if key in signal_names:
                signal = can_message.get_signal_by_name(key)
                if not self._check_range(signal, value):
                    sys.exit(f"The value of the signal '{key}' is out of range.")
                if not self._check_enum(signal, value):
                    sys.exit(
                        f"The value of the signal '{key}' is not in the "
                        "corresponding enum."
                    )
                message[key] = value
            else:
                sys.exit(f"Cannot find the signal '{key}' in CAN message '{name}'.")
        return message

    def _check_range(
        self, signal: cantools.database.can.signal.Signal, signal_value: float
    ):
        """Check if the CAN signal value is in its range.

        Args:
            signal: signal of the CAN message.
            signal_value: CAN signal value to be checked.

        Returns:
            True if the CAN signal value is in range.
        """
        if (signal_value > cast(float, signal.maximum)) or (
            signal_value < cast(float, signal.minimum)
        ):
            return False
        return True

    def _check_enum(
        self, signal: cantools.database.can.signal.Signal, signal_value: int
    ):
        """Check if the CAN signal value is one of its enum values,
        if this signal has enum.

        Args:
            signal: signal of the CAN message.
            signal_value: the signal value to be checked.

        Returns:
            True if either the CAN signal does not have enum or the signal
            value is one of the enum values.
        """
        if signal.choices:
            choices = [key for key, _ in signal.choices.items()]
            if signal_value not in choices:
                return False
        return True

    def get_message_request_msg(self, request_code: Enum) -> dict:
        """Get request CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(
            name="f_BootloaderActionRequest", RequestCode8Bits=request_code.value
        )

    def get_message_transfer_program_info(
        self, len_of_program_in_bytes: int, num_of_transfer_loops: int
    ) -> dict:
        """Get transfer program info CAN message for sending.

        Returns:
            dict: a CAN message.
        """
        return self._get_message(
            name="f_TransferProcessInfo",
            ProgramLength=len_of_program_in_bytes,
            RequiredTransferLoops=num_of_transfer_loops,
        )

    def get_message_data_8_bytes(self, data_8_bytes: int) -> dict:
        """Get data 8 bytes CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(name="f_Data8Bytes", Data=data_8_bytes)

    def get_message_crc_8_bytes(self, crc_8_bytes: int) -> dict:
        """Get CRC 8 bytes CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(name="f_Crc8Bytes", Crc=crc_8_bytes)

    def get_message_loop_info(self, num_of_loop: int) -> dict:
        """Get loop info CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(name="f_LoopInfo", LoopNumber=num_of_loop)
