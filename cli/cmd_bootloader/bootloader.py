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

"""This file implements the bootloader class to communicate with the bootloader
inside hardware (foxbms board) to assist the booting process of foxbms (send
binary files and commands)."""

import logging
import math
import sys
import time
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Optional, Tuple, TypedDict, cast

from .app_constants import (
    APP_MEMORY_MAP,
    NUM_OF_BYTES_PER_DATA_LOOPS,
    SIZE_OF_SUB_SECTOR_IN_LOOPS,
)
from .bootloader_binary_file import BootloaderBinaryFile
from .bootloader_can_messages import (
    AcknowledgeMessage,
    AcknowledgeMessageType,
    BootFsmState,
    CanFsmState,
    StatusCode,
)


class CanBusConfigHelper(TypedDict):
    """Helper"""

    interface: str
    channel: str
    bitrate: int


@dataclass
class CanBusConfig:
    """Type of CAN connection (must be supported by python-can)."""

    interface: str
    channel: str
    bitrate: int

    def as_dict(self) -> CanBusConfigHelper:
        """Return the"""
        tmp: CanBusConfigHelper = {
            "interface": self.interface,
            "channel": self.channel,
            "bitrate": self.bitrate,
        }
        return tmp


class BootloaderInterface(ABC):
    """Abstract class of the bootloader communication interface."""

    @abstractmethod
    def run_app_on_bootloader(self) -> bool:
        """Run the application on bootloader.

        Returns:
            True if the application has been started successfully, False
            otherwise.
        """

    @abstractmethod
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

    @abstractmethod
    def get_bootloader_state(self) -> Tuple[Optional[str], Optional[str]]:
        """Get the bootloader's current state.

        Returns:
            CanFsmState, BootFsmState.
        """

    @abstractmethod
    def get_current_num_of_loops(self) -> Optional[int]:
        """Get the bootloader's current transfer data loop number.

        Returns:
            Current number of data transfer loops.
        """

    @abstractmethod
    def get_bootloader_version_num(
        self,
    ) -> Tuple[Optional[int], Optional[int], Optional[int]]:
        """Get the bootloader's version number.

        Returns:
            Current version number (MajorVersionNumber, MinorVersionNumber,
            PatchVersionNumber) of the bootloader.
        """

    @abstractmethod
    def start_transfer(self) -> bool:
        """Inform bootloader to start the transfer program process.

        Returns:
            True if bootloader has successfully started the process, False
            otherwise.
        """

    @abstractmethod
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

    @abstractmethod
    def send_loop_number_to_bootloader(self, num_of_loop: int) -> None:
        """Send the current loop number of the to-be-transferred data to
        bootloader.

        Args:
            num_of_loop: the current loop number (starts from 1).
        """

    @abstractmethod
    def send_data_to_bootloader(self, data_8_bytes: int):
        """Send data in 8 bytes to bootloader.

        Args:
            data_8_bytes: data in 8 bytes.
        """

    @abstractmethod
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

    @abstractmethod
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
            processed_level: the ACK message need to have at least this
                processed_level.

        Returns:
            Acknowledged message from bootloader
        """


class Bootloader:
    """
    Implementation of the communication between host-implementation of the
    bootloader and the embedded bootloader.
    """

    def __init__(
        self,
        interface: BootloaderInterface,
        path_app_binary: Optional[Path] = None,
        app_size: Optional[int] = None,
        path_crc_64_table: Optional[Path] = None,
    ):
        """Init function.

        Args:
            interface: the communication interface of the bootloader
            path_app_binary: the path of the binary file
            app_size: the size of the binary file of the application
            path_crc_64_table: the path of the temporary file that stores the
                CRC table
        """

        # Initialize the binary file object for bootloader
        self.binary_file = None
        if path_app_binary:
            self.binary_file = BootloaderBinaryFile(
                path_app_binary=path_app_binary,
                app_size=app_size,
                path_crc_64_table=path_crc_64_table,
            )

        # Get the communication interface
        self.interface = interface

    def check_target(self) -> bool:
        """Check if the bootloader can be reached and print its current status.

        Returns:
            True if the bootloader can be reached, False otherwise.
        """

        # Get and check the bootloader state and the loop number
        can_fsm_state, boot_fsm_state = self.interface.get_bootloader_state()
        current_num_of_loops = self.interface.get_current_num_of_loops()
        if not (can_fsm_state or boot_fsm_state or current_num_of_loops):
            # Check if the foxBMS application is running
            bms_state = self.interface.get_foxbms_state()
            if bms_state:
                logging.info(
                    "FoxBMS application is running. FoxBMS status: %s ",
                    bms_state,
                )
                return True
            logging.error(
                "Cannot get any status of bootloader successfully, check "
                "the CAN connection and the power status of the hardware."
            )
            return False

        # Get and check the version info of bootloader
        major_version_number, minor_version_number, patch_version_number = (
            self.interface.get_bootloader_version_num()
        )
        if (
            (not major_version_number)
            or (not minor_version_number)
            or (not patch_version_number)
        ):
            logging.error(
                "Cannot get the version number of bootloader successfully, abort."
            )
            return False
        logging.info(
            "Successfully communicated with bootloader (v%d.%d.%d): "
            "can_fsm_state: '%s', boot_fsm_state: '%s', "
            "current_num_of_loops: '%s'",
            major_version_number,
            minor_version_number,
            patch_version_number,
            can_fsm_state,
            boot_fsm_state,
            current_num_of_loops,
        )
        return True

    def send_and_validate_vector_table(self) -> bool:
        """Send vector table to the bootloader

        Returns:
            True if vector table has been successfully sent, False otherwise
        """

        logging.info("Sending vector table...")

        # Send 4 * 8 bytes of vector table to bootloader
        self.binary_file = cast(BootloaderBinaryFile, self.binary_file)
        for i_vector in range(4):
            self.interface.send_data_to_bootloader(
                self.binary_file.data64_vector_table[i_vector]
            )
        msg = self.interface.wait_can_ack_msg(AcknowledgeMessage.ReceivedVectorTable)
        if not msg:
            logging.error("Cannot send the vector table.")
            return False
        logging.info("Successfully sent vector table.")

        # Send 8 bytes of CRC signature of the vector table to bootloader
        is_crc_received, is_crc_valid = self.interface.send_crc(
            self.binary_file.crc64_vector_table, is_crc_of_vector_table=True
        )
        if not is_crc_received:
            logging.error("Cannot send CRC of vector table.")
            return False
        if not is_crc_valid:
            logging.error("Vector table is not validated.")
            return False
        logging.info("The vector table has been successfully validated.")
        return True

    def send_data_as_a_sub_sector(
        self, i_loop_start: int, i_loop_end: int, is_first_sub_sector: bool = True
    ) -> bool:
        """Send one sub-sector of data to bootloader.

        Args:
            i_loop_start: start index of data loops to send (min. 1)
            i_loop_end: end index of data loops to send

        Returns:
            True if the data has been successfully sent, False otherwise
        """
        i_loop = i_loop_start
        time_start = time.time()
        self.binary_file = cast(BootloaderBinaryFile, self.binary_file)
        while i_loop <= i_loop_end:
            # Since the minimal i_loop is 1, the index will be i_loop - 1
            _, data_8_bytes = self.binary_file.get_crc_and_data_by_index(i_loop - 1)
            self.interface.send_loop_number_to_bootloader(num_of_loop=i_loop)

            # There is an additional process in the first subsector to run
            # in bootloader, because of that, there should be some delay to
            # send the data to prevent dropping data
            if is_first_sub_sector and (i_loop == i_loop_start):
                time.sleep(0.01)
            self.interface.send_data_to_bootloader(data_8_bytes)
            i_loop += 1

        # Wait ACK message for the last loop in this subsector loops
        msg = self.interface.wait_can_ack_msg(AcknowledgeMessage.ReceivedSubSectorData)
        if not msg:
            logging.error(
                "loop_%s to loop_%s data could not be send.",
                i_loop_start,
                i_loop_end,
            )
            return False
        logging.info(
            "loop_%s to loop_%s data have been successfully sent! It takes in total %s s",
            i_loop_start,
            i_loop_end,
            time.time() - time_start,
        )
        return True

    def _get_sub_sector_loops(self, idx: int, max_idx: int) -> Tuple[int, int]:
        """Get the start and end loop number (i_loop) of one subsector.

        Args:
            idx: the start index of the loop number (min. 1)
            max_idx: the maximum index of the loop number, which is the total
                loop number.

        Returns:
            the start i_loop and the end i_loop of the subsector
        """
        if idx < 1:
            sys.exit(
                f"The index of the loop number should start from 1, it is now {idx}."
            )
        size_of_sub_sector_in_loops = 1024
        if max_idx < size_of_sub_sector_in_loops:
            sys.exit(
                "The max_idx is smaller than size_of_sub_sector_in_loops "
                f"{size_of_sub_sector_in_loops}."
            )
        i_loop_sub_sector_start = idx
        i_max_sub_sector_loop = (
            math.floor(max_idx / size_of_sub_sector_in_loops)
            * size_of_sub_sector_in_loops
        )
        if idx > i_max_sub_sector_loop:
            i_loop_sub_sector_end = max_idx
        else:
            i_loop_sub_sector_end = (
                math.ceil(idx / size_of_sub_sector_in_loops)
                * size_of_sub_sector_in_loops
            )
        return i_loop_sub_sector_start, i_loop_sub_sector_end

    def send_data_as_a_sector(
        self,
        i_loop: int,
        total_num_of_loops: int,
        size_of_sector_in_loops: int,
        times_of_repeat: int = 3,
    ) -> bool:
        """Send data of one sector to the embedded bootloader.

        Args:
            i_loop: start loop number
            total_num_of_loops: total loop number
            times_of_repeat: time to repeat if a subsector cannot be
                successfully sent

        Returns:
            True if this sector data has been successfully sent, False
            otherwise
        """
        self.binary_file = cast(BootloaderBinaryFile, self.binary_file)
        size_of_sub_sector_in_loops = SIZE_OF_SUB_SECTOR_IN_LOOPS
        i_loop_start = i_loop
        if (i_loop + size_of_sector_in_loops) > total_num_of_loops:
            #  Last sector
            i_loop_end = total_num_of_loops
        else:
            i_loop_end = i_loop + size_of_sector_in_loops - 1
        logging.info("This sector is from i_loop %d to %d", i_loop_start, i_loop_end)
        is_first_sub_sector = True
        time_start = time.time()
        while i_loop <= i_loop_end:
            i_loop_sub_sector_start, i_loop_sub_sector_end = self._get_sub_sector_loops(
                idx=i_loop,
                max_idx=total_num_of_loops,
            )
            # Send one sector of data (16384 loops)
            ret_val = False
            for _ in range(times_of_repeat + 1):
                ret_val = self.send_data_as_a_sub_sector(
                    i_loop_start=i_loop_sub_sector_start,
                    i_loop_end=i_loop_sub_sector_end,
                    is_first_sub_sector=is_first_sub_sector,
                )
                if ret_val:
                    break
            if not ret_val:
                logging.error("Error when sending data as subsector.")
                return False
            is_first_sub_sector = False
            i_loop += size_of_sub_sector_in_loops

        # Send crc data
        logging.info(
            "Finished sending '%s' to '%s' data for this sector. Starting validation...",
            i_loop_start,
            i_loop_end,
        )
        crc_8_bytes, _ = self.binary_file.get_crc_and_data_by_index(i_loop_end - 1)
        is_crc_received, is_crc_valid = self.interface.send_crc(crc_8_bytes=crc_8_bytes)
        if not is_crc_received:
            logging.error("The CRC cannot be received by the embedded bootloader.")
            return False
        if not is_crc_valid:
            logging.error(
                "The validation process for this sector in the bootloader failed."
            )
            return False

        logging.info(
            "Successfully sent sector (loop_%s to loop_%s) (took %s s).",
            i_loop_start,
            i_loop_end,
            time.time() - time_start,
        )
        return True

    @staticmethod
    def _get_sector_size_using_num_of_data_loops(i_loop: int):
        start_num_of_data_loops = 1
        for flash_sector in APP_MEMORY_MAP:
            size_of_sector_in_loops = int(
                flash_sector.size_in_bytes / NUM_OF_BYTES_PER_DATA_LOOPS
            )
            end_num_of_data_loops = (
                start_num_of_data_loops + size_of_sector_in_loops - 1
            )
            if start_num_of_data_loops <= i_loop <= end_num_of_data_loops:
                return size_of_sector_in_loops
            start_num_of_data_loops += size_of_sector_in_loops
        return None

    def send_app_data(
        self,
        i_loop: int,
    ) -> bool:
        """Send the application binary file as data in 8 bytes (and calculated
        crc in 8 bytes) to the embedded bootloader.

        Args:
           i_loop: the start data loop number (must be 1 or greater).
        """
        if i_loop < 1:
            sys.exit("The input loop number is smaller than the minimum (1).")
        self.binary_file = cast(BootloaderBinaryFile, self.binary_file)

        # Start transferring data in loops
        start_time = time.time()
        while i_loop <= self.binary_file.len_of_program_in_8_bytes:
            size_of_sector_in_loops = self._get_sector_size_using_num_of_data_loops(
                i_loop
            )
            if not size_of_sector_in_loops:
                logging.error(
                    "Cannot find the size of the sector using the loop number '%s'.",
                    i_loop,
                )
                return False
            time.sleep(0.3)
            if not self.send_data_as_a_sector(
                i_loop=i_loop,
                total_num_of_loops=self.binary_file.len_of_program_in_8_bytes,
                size_of_sector_in_loops=size_of_sector_in_loops,
            ):
                logging.error("Cannot transfer this sector of data.")
                return False
            i_loop += size_of_sector_in_loops

        logging.info(
            "Successfully finish sending loop data (took %s s).",
            time.time() - start_time,
        )
        return True

    def reset_bootloader(self) -> bool:
        """Reset bootloader

        Returns:
            True if the bootloader has been successfully reset, False
            otherwise.
        """
        logging.info("Resetting bootloader ...")
        if self.interface.reset_bootloader() is not True:
            logging.error("Cannot reset bootloader.")
            return False
        logging.info("Successfully reset bootloader.")
        return True

    def run_app(self) -> bool:
        """Run application on bootloader

        Returns:
            True if bootloader has been successfully informed to run
            application, False otherwise.
        """
        if self.interface.run_app_on_bootloader() is not True:
            logging.error("Cannot inform embedded bootloader to run app.")
            return False
        return True

    def send_pre_info(self) -> bool:
        """Send CAN message to the embedded bootloader to let the embedded
        bootloader prepare for receiving the application data.

        Returns:
            True if the pre-info has been successfully sent to
            the bootloader, False otherwise.
        """
        # Send command to start the transmission of program
        logging.info("Prepare bootloader to start receiving program info...")
        if not self.interface.start_transfer():
            logging.error("Cannot start to send the application program.")
            return False

        # Send program relevant information
        self.binary_file = cast(BootloaderBinaryFile, self.binary_file)
        logging.info(
            "Sending program info: len_of_program_in_bytes %s, "
            "num_of_transfer_loops %s...",
            self.binary_file.len_of_program_in_bytes,
            self.binary_file.len_of_program_in_8_bytes,
        )
        if not self.interface.send_program_info(
            len_of_program_in_bytes=self.binary_file.len_of_program_in_bytes,
            num_of_transfer_loops=self.binary_file.len_of_program_in_8_bytes,
        ):
            logging.error("Cannot send pre-info to the bootloader.")
            return False
        logging.info("Successfully send pre-info to the bootloader.")
        return True

    @staticmethod
    def _check_if_data_transfer_resumable(
        can_fsm_state: str, boot_fsm_state: str, current_num_of_loops: int
    ) -> bool:
        """Check if the data transfer operation is in a resumable state.

        Args:
            can_fsm_state: can_fsm_state
            boot_fsm_state: boot_fsm_state
            current_num_of_loops: current_num_of_loops

        Returns:
            True if data transfer resumable, False otherwise.
        """

        # Check boot_fsm_state
        if boot_fsm_state != BootFsmState.BootFsmStateLoad.name:
            return False

        # Check can_fsm_state
        if can_fsm_state not in (
            CanFsmState.CanFsmStateReceived8BytesData.name,
            CanFsmState.CanFsmStateReceived8BytesCrc.name,
            CanFsmState.CanFsmStateFinishedFinalValidation.name,
        ):
            return False

        # Check the current number of loops
        if current_num_of_loops < 2:
            return False

        return True

    @staticmethod
    def _check_if_bootloader_at_the_beginning(
        can_fsm_state: str, boot_fsm_state: str, current_num_of_loops: int
    ) -> bool:
        """Check if bootloader at the start states.

        Args:
            can_fsm_state: can_fsm_state
            boot_fsm_state: boot_fsm_state
            current_num_of_loops: current_num_of_loops

        Returns:
            True if bootloader at the start states, False otherwise.
        """

        # Check boot_fsm_state
        if boot_fsm_state != BootFsmState.BootFsmStateWait.name:
            return False

        # Check can_fsm_state
        if can_fsm_state != CanFsmState.CanFsmStateNoCommunication.name:
            return False

        # Check the current number of loops
        if current_num_of_loops != 0:
            return False

        return True

    @staticmethod
    def _check_if_only_send_vector_table(
        can_fsm_state: str, boot_fsm_state: str
    ) -> bool:
        """Check if only the vector table needs to be transmitted.

        Args:
            can_fsm_state: can_fsm_state
            boot_fsm_state: boot_fsm_state

        Returns:
            True if only the vector table needs to be transmitted, False
            otherwise.
        """
        # Check boot_fsm_state
        if boot_fsm_state != BootFsmState.BootFsmStateLoad.name:
            return False

        # Check can_fsm_state
        if can_fsm_state != CanFsmState.CanFsmFinishedTransferVectorTable.name:
            return False

        return True

    def send_app_binary(self):
        """Transfer the app binary to the embedded bootloader"""
        # The times to repeat sending app if it fails.
        times_of_repeat = 3

        # Check the state of bootloader (hardware side)
        logging.info(
            "Checking the state of the bootloader and the current loop number..."
        )
        can_fsm_state, boot_fsm_state = self.interface.get_bootloader_state()
        current_num_of_loops = self.interface.get_current_num_of_loops()
        logging.info(
            "can_fsm_state: %s, boot_fsm_state: %s, current_num_of_loops: %s",
            can_fsm_state,
            boot_fsm_state,
            current_num_of_loops,
        )
        # Check there is a value for each input argument
        if (
            (not can_fsm_state)
            or (not boot_fsm_state)
            or (current_num_of_loops is None)
        ):
            sys.exit("Cannot get all states of the bootloader.")

        # Estimate if the data transfer need to be started from the beginning
        is_send_pre_info = self._check_if_bootloader_at_the_beginning(
            can_fsm_state, boot_fsm_state, current_num_of_loops
        )

        # Check if the data transfer is resumable
        is_resume_data_transfer = self._check_if_data_transfer_resumable(
            can_fsm_state, boot_fsm_state, current_num_of_loops
        )

        # Send app binary based on if the data transfer is resumable
        i_loop = 1
        only_send_vector_table = False

        # If not send_pre_info
        if not is_send_pre_info:
            if is_resume_data_transfer:
                logging.info("The data transfer is resumable, try to resume...")
                only_send_vector_table = self._check_if_only_send_vector_table(
                    can_fsm_state, boot_fsm_state
                )
                i_loop = current_num_of_loops
            else:
                logging.warning(
                    "Cannot resume sending data to bootloader. Resetting bootloader."
                )
                if not self.reset_bootloader():
                    return False
                is_send_pre_info = True

        # Send pre-info to bootloader
        if is_send_pre_info:
            if not self.send_pre_info():
                return False

        # Send app data
        ret_val = False
        for _ in range(times_of_repeat + 1):
            if only_send_vector_table:
                ret_val = self.send_and_validate_vector_table()
            else:
                if self.send_app_data(i_loop):
                    ret_val = self.send_and_validate_vector_table()
                    only_send_vector_table = True
            if ret_val:
                break
            time.sleep(3)

        # Check if the app data and vector table has been successfully sent
        if not ret_val:
            logging.error("Error while sending app data.")
            return False

        logging.info(
            "Successfully finished sending application data and vector table "
            "to the bootloader."
        )
        return True
