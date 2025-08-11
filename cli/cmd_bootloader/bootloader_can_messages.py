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

"""This file contains all enums of CAN messages and functions to get specfied
can messages in dict. The members of the Enum classes that end with 'Str' have
the string type of value."""

from enum import Enum
from pathlib import Path
from typing import TypedDict

from cantools import database

from ..helpers.misc import BOOTLOADER_DBC_FILE


# pylint:disable=invalid-name
class YesNoFlag(Enum):
    """Enum from dbc file"""

    No = 0
    Yes = 1


class AcknowledgeFlag(Enum):
    """Enum from dbc file"""

    NotReceived = 0
    Received = 1


class AcknowledgeMessage(Enum):
    """Enum from dbc file"""

    ReceivedCmdToTransferProgram = 1
    ReceivedProgramInfo = 2
    ReceivedLoopNumber = 3
    ReceivedSubSectorData = 4
    Received8BytesCrc = 5
    ReceivedFinal8BytesCrcSignature = 6
    ReceivedVectorTable = 7
    ReceivedCrcOfVectorTable = 8
    ReceivedCmdToRunProgram = 9
    ReceivedCmdToResetBootProcess = 10


class StatusCode(Enum):
    """Enum from dbc file"""

    ReceivedButNotProcessed = 0
    ReceivedAndInProcessing = 1
    ReceivedAndProcessed = 2
    Error = 3


class BootloaderAction(Enum):
    """Enum from dbc file"""

    CmdToTransferProgram = 1
    CmdToResetBootProcess = 2
    CmdToRunProgram = 3
    CmdToGetBootloaderInfo = 4
    CmdToGetDataTransferInfo = 5
    CmdToGetVersionInfo = 6


class CanFsmState(Enum):
    """Enum from dbc file"""

    CanFsmStateNoCommunication = 1
    CanFsmStateWaitForInfo = 2
    CanFsmStateWaitForDataLoops = 3
    CanFsmStateReceivedLoopNumber = 4
    CanFsmStateReceived8BytesData = 5
    CanFsmStateReceived8BytesCrc = 6
    CanFsmStateFinishedFinalValidation = 7
    CanFsmFinishedTransferVectorTable = 8
    CanFsmStateValidatedVectorTable = 9
    CanFsmStateError = 10
    CanFsmStateResetBoot = 11
    CanFsmStateRunProgram = 12


class BootFsmState(Enum):
    """Enum from dbc file"""

    BootFsmStateWait = 1
    BootFsmStateReset = 2
    BootFsmStateRun = 3
    BootFsmStateLoad = 4
    BootFsmStateError = 5


# pylint:enable=invalid-name


class AcknowledgeMessageType(TypedDict):
    """This class specify the type of the received acknowledge message."""

    AcknowledgeFlag: str
    AcknowledgeMessage: str
    StatusCode: str
    Response: str


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

    def __init__(self, dbc_file: Path = BOOTLOADER_DBC_FILE) -> None:
        if not dbc_file.is_file():
            raise SystemExit(f"File '{dbc_file}' does not exist.")
        db = database.load_file(dbc_file)
        if not isinstance(db, database.can.database.Database):
            raise SystemExit(
                f"Expected '{dbc_file}' to contain a CAN database, but "
                f"type is '{type(db)}'."
            )
        self.db = db
        self.message_names = [
            message.name for message in getattr(self.db, "messages", [])
        ]
        if not self.message_names:
            raise SystemExit("There are no messages in the database file.")

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
            raise SystemExit(
                f"The name of message '{name}' cannot be found in the CAN database."
            )
        message = {}
        message["Name"] = name
        signal_names = [signal.name for signal in can_message.signals]
        for key, value in kwargs.items():
            if key in signal_names:
                signal = can_message.get_signal_by_name(key)
                if not self._check_range(signal, value):
                    raise SystemExit(
                        f"The value of the signal '{key}' is out of range."
                    )
                if not self._check_enum(signal, value):
                    raise SystemExit(
                        f"The value of the signal '{key}' is not in the "
                        "corresponding enum."
                    )
                message[key] = value
            else:
                raise SystemExit(
                    f"Cannot find the signal '{key}' in CAN message '{name}'."
                )
        return message

    def _check_range(
        self, signal: database.can.signal.Signal, signal_value: float
    ) -> bool:
        """Check if the CAN signal value is in its range.

        Args:
            signal: signal of the CAN message.
            signal_value: CAN signal value to be checked.

        Returns:
            True if the CAN signal value is in range.
        """
        if signal.maximum is not None and (signal_value > signal.maximum):
            return False
        if signal.minimum is not None and (signal_value < signal.minimum):
            return False
        return True

    def _check_enum(
        self, signal: database.can.signal.Signal, signal_value: int
    ) -> bool:
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
            name="f_BootloaderActionRequest", BootloaderAction=request_code.value
        )

    def get_message_transfer_program_info(
        self, len_of_program_in_bytes: int, num_of_transfer_loops: int
    ) -> dict:
        """Get transfer program info CAN message for sending.

        Returns:
            dict: a CAN message.
        """
        return self._get_message(
            name="f_BootloaderTransferProcessInfo",
            ProgramLength=len_of_program_in_bytes,
            RequiredTransferLoops=num_of_transfer_loops,
        )

    def get_message_data_8_bytes(self, data_8_bytes: int) -> dict:
        """Get data 8 bytes CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(name="f_BootloaderData8Bytes", Data=data_8_bytes)

    def get_message_crc_8_bytes(self, crc_8_bytes: int) -> dict:
        """Get CRC 8 bytes CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(name="f_BootloaderCrc8Bytes", Crc=crc_8_bytes)

    def get_message_loop_info(self, num_of_loop: int) -> dict:
        """Get loop info CAN message for sending.

        Returns:
            dict: message.
        """
        return self._get_message(name="f_BootloaderLoopInfo", LoopNumber=num_of_loop)
