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

"""This file implement the can module (CanInterface) to communicate with
bootloader in a higher level."""

import logging
import time
from enum import Enum
from typing import Optional, Tuple, Union, cast

from can import BusABC
from can.interfaces.pcan import PcanBus
from can.interfaces.virtual import VirtualBus

from ..helpers.misc import APP_DBC_FILE
from .bootloader import BootloaderInterface
from .bootloader_can_basics import BootloaderCanBasics
from .bootloader_can_messages import (
    AcknowledgeMessage,
    AcknowledgeMessageType,
    BootFsmState,
    CanFsmState,
    RequestCode8Bits,
    StatusCode,
    YesNoAnswer,
)


class BootloaderInterfaceCan(BootloaderInterface):
    """
    This class implements all CAN relevant functions and can be used to wait "
    or receive specified messages on CAN bus.
    """

    def __init__(self, can_bus: Union[VirtualBus, PcanBus, BusABC]):
        """Init function.

        Args:
            can_bus: CAN bus object
        """
        self.can = BootloaderCanBasics(can_bus=can_bus)

    def send_crc(
        self, crc_8_bytes: int, is_crc_of_vector_table: bool = False
    ) -> Tuple[bool, bool]:
        """Send crc signature to bootloader and get the feedback from bootloader.

        Args:
            crc_8_bytes: maximal 0xffffffffffffffff.
            is_crc_of_vector_table: True if it is the crc of vector table.

        Returns:
            True, if a ACK message has been received.
            True, if the YesNoAnswer is "Yes".
        """
        self.can.send_crc_to_bootloader(crc_8_bytes)
        if is_crc_of_vector_table:
            ack_crc_msg = self.can.wait_can_ack_msg(
                AcknowledgeMessage.ReceivedCrcOfVectorTable,
                timeout_total=120,
                timeout_ack=30,
            )
        else:
            ack_crc_msg = self.can.wait_can_ack_msg(
                AcknowledgeMessage.Received8BytesCrc, timeout_total=120, timeout_ack=30
            )
        if not ack_crc_msg:
            logging.error("Cannot receive ACK message.")
            return False, False
        is_crc_valid = ack_crc_msg["YesNoAnswer"] == "Yes"
        if is_crc_valid:
            logging.debug("Successfully send 8 bytes crc to bootloader.")
        else:
            logging.debug("The validation for this sector failed.")
        return True, is_crc_valid

    def send_program_info(
        self,
        len_of_program_in_bytes: int,
        num_of_transfer_loops: int,
    ) -> bool:
        """Send program info to bootloader and check its states.

        Args:
            len_of_program_in_bytes: the length of the program in bytes.
            num_of_transfer_loops: the number of total transfer loops.

        Returns:
            True if bootloader has received info, False otherwise.
        """
        logging.debug("Start sending program info to bootloader ...")
        self.can.send_transfer_program_info_to_bootloader(
            len_of_program_in_bytes=len_of_program_in_bytes,
            num_of_transfer_loops=num_of_transfer_loops,
        )

        msg = self.can.wait_can_ack_msg(AcknowledgeMessage.ReceivedProgramInfo)
        if not msg:
            logging.error(
                "Cannot receive the ACK message containing 'ReceivedAndProcessed'."
            )
            return False

        if msg["YesNoAnswer"] != YesNoAnswer.Yes.name:
            logging.error(
                "The program info is not valid, check if the program length "
                "is larger than the maximum and/or if the "
                "num_of_transfer_loops is correct."
            )
            return False

        can_fsm_state, _ = self.get_bootloader_state()
        if can_fsm_state != CanFsmState.CanFsmStateWaitForDataLoops.name:
            logging.error(
                "Cannot change the state of CAN module to wait for data loops."
            )
            return False
        logging.debug("Successfully start the transfer process on bootloader.")
        return True

    def send_loop_number_to_bootloader(self, num_of_loop: int) -> None:
        """Send the current loop number of the to-be-transferred data to
        bootloader.

        Args:
            num_of_loop: the current loop number (starting from 1).
        """
        return self.can.send_loop_number_to_bootloader(num_of_loop=num_of_loop)

    def send_data_to_bootloader(self, data_8_bytes: int) -> None:
        """Send data in 8 bytes to bootloader.

        Args:
            data_8_bytes: data in 8 bytes.
        """
        # Add waiting time to prevent the error that the tx buffer is full
        time.sleep(0.0001)
        self.can.send_data_to_bootloader(data_8_bytes=data_8_bytes)

    def wait_can_ack_msg(
        self,
        acknowledge_msg: Enum,
        timeout_total: float = 1.0,
        timeout_ack: float = 0.3,
        processed_level: Enum = StatusCode.ReceivedAndProcessed,
    ) -> Optional[AcknowledgeMessageType]:
        """Wait for a certain ACK message.

        Args:
            acknowledge_msg: the type of ACK message.
            timeout_total: total time to wait.
            timeout_ack: time to wait for a single message.
            processed_level: the ACK message needs to have at least this
                processed_level.

        Returns:
            acknowledged message from bootloader
        """

        return self.can.wait_can_ack_msg(
            acknowledge_msg=acknowledge_msg,
            timeout_total=timeout_total,
            timeout_ack=timeout_ack,
            processed_level=processed_level,
        )

    def start_transfer(self) -> bool:
        """Inform bootloader to start the transfer program process.

        Returns:
           True if the bootloader has successfully started the process, False
           otherwise.
        """
        logging.debug(
            "Send request to start CAN communication to transfer the program..."
        )
        self.can.send_request_to_bootloader(RequestCode8Bits.CmdToTransferProgram)
        if not self.can.wait_can_ack_msg(
            AcknowledgeMessage.ReceivedCmdToTransferProgram
        ):
            logging.error(
                "Cannot receive the ACK message to start transferring the program."
            )
            return False
        can_fsm_state, _ = self.get_bootloader_state()
        if can_fsm_state != CanFsmState.CanFsmStateWaitForInfo.name:
            logging.error(
                "Cannot set the CAN FSM state of the bootloader to %s.",
                CanFsmState.CanFsmStateWaitForInfo.name,
            )
            return False
        logging.debug("Successfully started the transfer process on the bootloader.")
        return True

    def reset_bootloader(
        self, time_to_wait: float = 15.0, num_of_repeat: int = 5
    ) -> bool:
        """Reset bootloader.

        Args:
            time_to_wait: time to wait for bootloader to reset itself.

        Returns:
            True if the bootloader has been successfully reset, False
            otherwise.
        """
        logging.debug("Sending request to bootloader to reset boot process ...")
        self.can.send_request_to_bootloader(RequestCode8Bits.CmdToResetBootProcess)
        if not self.can.wait_can_ack_msg(
            AcknowledgeMessage.ReceivedCmdToResetBootProcess,
            processed_level=StatusCode.ReceivedAndInProcessing,
            timeout_total=10,
            timeout_ack=1,
        ):
            logging.error("Cannot receive ACK message to reset boot process.")
            return False
        time.sleep(time_to_wait)

        for _ in range(num_of_repeat):
            can_fsm_state, boot_fsm_state = self.get_bootloader_state()
            if can_fsm_state:
                break
            time.sleep(1)

        logging.debug(
            "can_fsm_state: %s, boot_fsm_state: %s", can_fsm_state, boot_fsm_state
        )
        current_num_of_loops = self.get_current_num_of_loops()
        if (
            (can_fsm_state != CanFsmState.CanFsmStateNoCommunication.name)
            or (boot_fsm_state != BootFsmState.BootFsmStateWait.name)
            or (current_num_of_loops != 0)
        ):
            logging.error("Bootloader cannot be successfully reset.")
            return False
        logging.debug("Successfully reset bootloader.")
        return True

    def run_app_on_bootloader(self) -> bool:
        """Run the BMS application on the BMS-Master (i.e., jump from the
        bootloader into the application via BMS state request.)

        Returns:
            True if the application has been started successfully,
            False otherwise.
        """
        logging.info("Sending request to run application...")
        self.can.send_request_to_bootloader(RequestCode8Bits.CmdToRunProgram)
        if not self.can.wait_can_ack_msg(
            AcknowledgeMessage.ReceivedCmdToRunProgram,
            processed_level=StatusCode.ReceivedAndInProcessing,
        ):
            logging.error("Did not receive ACK message to run application.")
            return False
        # Check if there is validated program available
        msg_ack = self.can.wait_can_ack_msg(
            AcknowledgeMessage.ReceivedCmdToRunProgram,
            timeout_total=5,
            timeout_ack=0.3,
            processed_level=StatusCode.ReceivedAndProcessed,
        )
        if not msg_ack:
            logging.error(
                "Did not receive ACK message with program availability message.\n"
                "There might be no application program available on the BMS-Master."
            )
            return False

        if msg_ack["YesNoAnswer"] != YesNoAnswer.Yes.name:
            logging.error("There is no valid program available on the BMS.")
            return False

        logging.info(
            "There is a validated program available on the BMS-Master.\n"
            "Jumping into application..."
        )
        time.sleep(2)
        logging.info(
            "Successfully informed bootloader to run the BMS application.\n"
            "Checking if the application is running..."
        )
        bms_state = self.get_foxbms_state()
        if not bms_state:
            logging.error("Could not jump into the application.")
            return False
        logging.info(
            "Successfully jumped into the application. foxBMS status: %s ",
            bms_state,
        )
        return True

    def get_foxbms_state(self) -> Optional[str]:
        """Get the current state of foxBMS application if it is running.

        Returns:
            None if no application is running, otherwise the current bms state.
        """
        msg = None
        bms_state = None
        msg = self.can.wait_can_message(
            arbitration_id=0x220,
            dbc_file=APP_DBC_FILE,
            timeout_total=0.05,
            timeout_bus_recv=0.001,
        )
        if msg:
            bms_state = cast(dict, msg)["BmsState"]
        return bms_state

    def get_bootloader_state(self) -> Tuple[Optional[str], Optional[str]]:
        """Ask bootloader for its current state.

        Returns:
            CanFsmState, BootFsmState.
        """
        logging.debug("Sending request to bootloader to get bootloader states...")
        self.can.send_request_to_bootloader(RequestCode8Bits.CmdToGetBootloaderInfo)
        msg_bootloader_fsm_states = self.can.wait_bootloader_state_msg()
        if msg_bootloader_fsm_states:
            can_fsm_state = msg_bootloader_fsm_states["CanFsmState"]
            boot_fsm_state = msg_bootloader_fsm_states["BootFsmState"]
            logging.debug(
                "can_fsm_state: %s, boot_fsm_state: %s", can_fsm_state, boot_fsm_state
            )
            return can_fsm_state, boot_fsm_state
        logging.error("Can not get the state of bootloader.")
        return None, None

    def get_current_num_of_loops(self) -> Optional[int]:
        """Ask the bootloader to get its current number of data transfer loops.

        Returns:
            The current number of data transfer loops on bootloader.
        """
        logging.debug("Sending request to bootloader to ask for number of loops...")
        self.can.send_request_to_bootloader(RequestCode8Bits.CmdToGetDataTransferInfo)
        msg_data_transfer_info = self.can.wait_data_transfer_info_msg()
        if msg_data_transfer_info:
            current_num_of_loops = msg_data_transfer_info["CurrentLoopNumber"]
            logging.debug("current_num_of_loops: %d", current_num_of_loops)
            return current_num_of_loops
        logging.error(
            "Can not get the current number of data transfer loops of bootloader."
        )

        return None

    def get_bootloader_version_num(
        self,
    ) -> Tuple[Optional[int], Optional[int], Optional[int]]:
        """Get the bootloader's version number.

        Returns:
            Current version number (MajorVersionNumber, MinorVersionNumber,
            PatchVersionNumber) of the bootloader.
        """
        logging.debug("Sending request to bootloader to ask for the version number...")
        self.can.send_request_to_bootloader(RequestCode8Bits.CmdToGetVersionInfo)

        msg_bootloader_version_info = self.can.wait_bootloader_version_info_msg()

        if msg_bootloader_version_info:
            major_version_number = msg_bootloader_version_info["MajorVersionNumber"]
            minor_version_number = msg_bootloader_version_info["MinorVersionNumber"]
            patch_version_number = msg_bootloader_version_info["PatchVersionNumber"]

            logging.debug(
                "The version number of bootloader: %d.%d.%d",
                major_version_number,
                minor_version_number,
                patch_version_number,
            )
            return major_version_number, minor_version_number, patch_version_number

        logging.error(
            "Can not get the current number of data transfer loops of bootloader."
        )
        return None, None, None
