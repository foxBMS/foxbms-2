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

"""Testing file 'cli/cmd_bootloader/bootloader.py'."""

import io
import shutil
import sys
import time
import unittest
from pathlib import Path
from typing import cast
from unittest.mock import patch

import can
from cantools import database

# Redirect message or not
MSG_REDIRECT = True

# Other paths
PATH_TEMP = Path(__file__).parent / "temp"

sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))

# pylint: disable=protected-access
# pylint: disable=wrong-import-position
from cli.cmd_bootloader.bootloader import Bootloader  # noqa: E402
from cli.cmd_bootloader.bootloader_binary_file import BootloaderBinaryFile  # noqa: E402
from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan  # noqa: E402
from cli.cmd_bootloader.bootloader_can_messages import (  # noqa: E402
    BootFsmState,
    CanFsmState,
)
from cli.helpers.misc import BOOTLOADER_DBC_FILE  # noqa: E402


class TestBootloader(unittest.TestCase):
    """Class to test Bootloader class."""

    def setUp(self):
        # Redirect the sys.stdout to the StringIO object
        if MSG_REDIRECT:
            sys.stdout = io.StringIO()

        # Initialize virtual CAN bus instance
        self.can_bus = can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        )

        with patch.object(
            BootloaderBinaryFile, "_check_app_size"
        ) as mock_check_app_size:
            mock_check_app_size.return_value = True
            # Generate a fake binary
            PATH_TEMP.mkdir(parents=True, exist_ok=True)
            self.path_binary_data = PATH_TEMP / "test.bin"
            self.path_binary_data.write_bytes(bytes(100))
            self.path_crc_64_table = PATH_TEMP / "test_crc_64.csv"

            # Initialize a Bootloader instance
            interface = BootloaderInterfaceCan(can_bus=self.can_bus)
            self.bd = Bootloader(
                interface=interface,
                path_app_binary=self.path_binary_data,
                app_size=0x100,
                path_crc_64_table=self.path_crc_64_table,
            )

            # Load dbc file
            self.db = database.load_file(BOOTLOADER_DBC_FILE)

    def tearDown(self):
        # Delete the temporal directory
        shutil.rmtree(PATH_TEMP)
        self.can_bus.shutdown()

    @patch.object(BootloaderBinaryFile, "_check_app_size")
    def test_init(self, mock_check_app_size):
        """Function to test function __init__()."""
        mock_check_app_size.return_value = True
        interface = BootloaderInterfaceCan(can_bus=self.can_bus)
        bd = Bootloader(
            interface=interface,
            path_app_binary=None,
            app_size=0x100,
            path_crc_64_table=self.path_crc_64_table,
        )
        self.assertIsNone(bd.binary_file)

    def test_get_sub_sector_loops(self):
        """Function to test function _get_sub_sector_loops()."""
        with self.assertRaises(SystemExit) as cm:
            self.bd._get_sub_sector_loops(idx=0, max_idx=1024)
        self.assertEqual(
            cm.exception.code,
            "The index of the loop number should start from 1, it is now 0.",
        )
        with self.assertRaises(SystemExit) as cm:
            self.bd._get_sub_sector_loops(idx=1, max_idx=1023)
        self.assertEqual(
            cm.exception.code,
            "The max_idx is smaller than size_of_sub_sector_in_loops 1024.",
        )
        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            1, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 1)
        self.assertEqual(i_loop_sub_sector_end, 1024)
        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            10, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 10)
        self.assertEqual(i_loop_sub_sector_end, 1024)
        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            1024, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 1024)
        self.assertEqual(i_loop_sub_sector_end, 1024)

        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            1025, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 1025)
        self.assertEqual(i_loop_sub_sector_end, 2048)
        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            2048, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 2048)
        self.assertEqual(i_loop_sub_sector_end, 2048)

        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            2049, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 2049)
        self.assertEqual(i_loop_sub_sector_end, 2050)
        i_loop_sub_sector_start, i_loop_sub_sector_end = self.bd._get_sub_sector_loops(
            2050, 2050
        )
        self.assertEqual(i_loop_sub_sector_start, 2050)
        self.assertEqual(i_loop_sub_sector_end, 2050)

    def test_get_sector_size_using_num_of_data_loops(self):
        """Function to test function _get_sector_size_using_num_of_data_loops()."""
        i_loops_start = 1
        for i_sector in range(7, 32):
            if i_sector in list(range(7, 10)) + list(range(16, 32)):
                size_of_sector_bytes = 0x20000
            else:
                size_of_sector_bytes = 0x40000
            # Smallest data loop number
            size_of_sector_in_loops = self.bd._get_sector_size_using_num_of_data_loops(
                i_loops_start
            )
            self.assertEqual(size_of_sector_in_loops, int(size_of_sector_bytes / 8))
            # Largest data loop number
            size_of_sector_in_loops = self.bd._get_sector_size_using_num_of_data_loops(
                i_loops_start + size_of_sector_in_loops - 1
            )
            self.assertEqual(size_of_sector_in_loops, int(size_of_sector_bytes / 8))

            # Update for testing the next sector
            i_loops_start += size_of_sector_in_loops
        # None return
        self.assertIsNone(self.bd._get_sector_size_using_num_of_data_loops(0x3E0001))

    @patch.object(BootloaderInterfaceCan, "get_bootloader_version_num")
    @patch.object(BootloaderInterfaceCan, "get_current_num_of_loops")
    @patch.object(BootloaderInterfaceCan, "get_bootloader_state")
    def test_check_target(
        self,
        mock_get_bootloader_state,
        mock_get_current_num_of_loops,
        mock_get_bootloader_version_num,
    ):
        """Function to test function check_target()."""
        # False Case
        mock_get_bootloader_state.return_value = (None, None)
        mock_get_current_num_of_loops.return_value = None
        mock_get_bootloader_version_num.return_value = (None, None, None)
        self.assertFalse(self.bd.check_target())
        # True Case 1
        mock_get_bootloader_state.return_value = (None, "BootFsmStateWait")
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            2,
        )
        self.assertTrue(self.bd.check_target())
        # True Case 2
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            None,
        )
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            2,
        )
        self.assertTrue(self.bd.check_target())
        # True Case 3
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = None
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            2,
        )
        self.assertTrue(self.bd.check_target())
        # True Case 4
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            2,
        )
        self.assertTrue(self.bd.check_target())

    @patch.object(BootloaderInterfaceCan, "send_crc")
    @patch.object(BootloaderInterfaceCan, "wait_can_ack_msg")
    @patch.object(BootloaderInterfaceCan, "send_data_to_bootloader")
    def test_send_and_validate_vector_table(
        self,
        mock_send_data_to_bootloader,
        mock_wait_can_ack_msg,
        mock_send_crc,
    ):
        """Function to test function send_and_validate_vector_table()."""
        mock_send_data_to_bootloader.return_value = None
        # False Exit Case 1
        mock_wait_can_ack_msg.return_value = None
        mock_send_crc.return_value = (True, True)
        self.assertFalse(self.bd.send_and_validate_vector_table())
        # False Exit Case 2
        mock_wait_can_ack_msg.return_value = {"fake message": 1}
        mock_send_crc.return_value = (False, True)
        self.assertFalse(self.bd.send_and_validate_vector_table())
        # False Exit Case 3
        mock_wait_can_ack_msg.return_value = {"fake message": 1}
        mock_send_crc.return_value = (False, False)
        self.assertFalse(self.bd.send_and_validate_vector_table())
        # False Exit Case 4
        mock_wait_can_ack_msg.return_value = {"fake message": 1}
        mock_send_crc.return_value = (True, False)
        self.assertFalse(self.bd.send_and_validate_vector_table())
        # True Exit Case
        mock_wait_can_ack_msg.return_value = {"fake message": 1}
        mock_send_crc.return_value = (True, True)
        self.assertTrue(self.bd.send_and_validate_vector_table())

    @patch.object(BootloaderInterfaceCan, "wait_can_ack_msg")
    @patch.object(BootloaderInterfaceCan, "send_data_to_bootloader")
    @patch.object(BootloaderInterfaceCan, "send_loop_number_to_bootloader")
    @patch.object(BootloaderBinaryFile, "get_crc_and_data_by_index")
    def test_send_data_as_a_sub_sector(
        self,
        mock_get_crc_and_data_by_index,
        mock_send_loop_number_to_bootloader,
        mock_send_data_to_bootloader,
        mock_wait_can_ack_msg,
    ):
        """Function to test function send_data_as_a_sub_sector()."""
        mock_send_loop_number_to_bootloader.return_value = None
        # False Case
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_wait_can_ack_msg.return_value = None
        self.assertFalse(self.bd.send_data_as_a_sub_sector(1, 1024))
        # True Case
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_wait_can_ack_msg.return_value = {"fake message": 1}
        self.assertTrue(self.bd.send_data_as_a_sub_sector(1, 1024))
        # Check if the data has been sent 1024 times
        mock_send_data_to_bootloader.reset_mock()
        self.bd.send_data_as_a_sub_sector(1, 1024)
        self.assertEqual(mock_send_data_to_bootloader.call_count, 1024)
        # Check if the data has been sent 1 times
        mock_send_data_to_bootloader.reset_mock()
        self.bd.send_data_as_a_sub_sector(1024, 1024)
        self.assertEqual(mock_send_data_to_bootloader.call_count, 1)

    @patch.object(BootloaderInterfaceCan, "send_crc")
    @patch.object(BootloaderBinaryFile, "get_crc_and_data_by_index")
    @patch.object(Bootloader, "send_data_as_a_sub_sector")
    def test_send_data_as_a_sector(
        self,
        mock_send_data_as_a_sub_sector,
        mock_get_crc_and_data_by_index,
        mock_send_crc,
    ):
        """Function to test function send_data_as_a_sector()."""
        # Case 1: each subsector has been successfully sent.
        mock_send_data_as_a_sub_sector.return_value = True
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (True, True)
        self.bd.send_data_as_a_sector(
            i_loop=1,
            total_num_of_loops=32768,
            size_of_sector_in_loops=16384,
            times_of_repeat=0,
        )
        self.assertEqual(mock_send_data_as_a_sub_sector.call_count, 16)
        # Case 2: one subsector has been not successfully sent.
        mock_send_data_as_a_sub_sector.reset_mock()
        mock_send_data_as_a_sub_sector.return_value = False
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (True, True)
        self.assertFalse(
            self.bd.send_data_as_a_sector(
                i_loop=1,
                total_num_of_loops=16384,
                size_of_sector_in_loops=16384,
                times_of_repeat=0,
            )
        )
        self.assertEqual(mock_send_data_as_a_sub_sector.call_count, 1)
        # Case 3: one subsector has been not successfully sent, but
        # times_of_repeat is 1 .
        mock_send_data_as_a_sub_sector.reset_mock()
        mock_send_data_as_a_sub_sector.return_value = False
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (True, True)
        self.assertFalse(
            self.bd.send_data_as_a_sector(
                i_loop=1,
                total_num_of_loops=16384,
                size_of_sector_in_loops=16384,
                times_of_repeat=1,
            )
        )
        self.assertEqual(mock_send_data_as_a_sub_sector.call_count, 2)
        # Case 4: CRC has not been received by bootloader.
        mock_send_data_as_a_sub_sector.reset_mock()
        mock_send_data_as_a_sub_sector.return_value = True
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (False, True)
        self.assertFalse(
            self.bd.send_data_as_a_sector(
                i_loop=1,
                total_num_of_loops=16384,
                size_of_sector_in_loops=16384,
                times_of_repeat=1,
            )
        )
        # Case 5: the validation process in bootloader fails.
        mock_send_data_as_a_sub_sector.reset_mock()
        mock_send_data_as_a_sub_sector.return_value = True
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (True, False)
        self.assertFalse(
            self.bd.send_data_as_a_sector(
                i_loop=1,
                total_num_of_loops=16384,
                size_of_sector_in_loops=16384,
                times_of_repeat=1,
            )
        )
        # Case 6: true case.
        mock_send_data_as_a_sub_sector.reset_mock()
        mock_send_data_as_a_sub_sector.return_value = True
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (True, True)
        self.assertTrue(
            self.bd.send_data_as_a_sector(
                i_loop=1,
                total_num_of_loops=16384,
                size_of_sector_in_loops=16384,
                times_of_repeat=1,
            )
        )

    @patch.object(Bootloader, "_get_sector_size_using_num_of_data_loops")
    @patch.object(Bootloader, "send_data_as_a_sector")
    def test_send_app_data(
        self, mock_send_data_as_a_sector, mock_get_sector_size_using_num_of_data_loops
    ):
        """Function to test function send_app_data()."""
        with self.assertRaises(SystemExit) as cm:
            self.bd.send_app_data(i_loop=0)
        self.assertEqual(
            cm.exception.code,
            "The input loop number is smaller than the minimum (1).",
        )
        self.bd.binary_file = cast(BootloaderBinaryFile, self.bd.binary_file)
        self.bd.binary_file.len_of_program_in_8_bytes = 32768
        # Case return false 1: resume sending with the current loop number of 16384
        mock_get_sector_size_using_num_of_data_loops.return_value = None
        mock_send_data_as_a_sector.reset_mock()
        mock_send_data_as_a_sector.return_value = True
        self.assertFalse(self.bd.send_app_data(1))
        # Case return false 2: resume sending with the current loop number of 16384
        mock_get_sector_size_using_num_of_data_loops.return_value = 16384
        mock_send_data_as_a_sector.reset_mock()
        mock_send_data_as_a_sector.return_value = False
        self.assertFalse(self.bd.send_app_data(1))
        # Case return true: resume sending with the current loop number of 16384
        mock_get_sector_size_using_num_of_data_loops.return_value = 16384
        mock_send_data_as_a_sector.reset_mock()
        mock_send_data_as_a_sector.return_value = True
        self.assertTrue(self.bd.send_app_data(1))

    @patch.object(BootloaderInterfaceCan, "reset_bootloader")
    def test_reset_bootloader(self, mock_reset_bootloader):
        """Function to test function reset_bootloader()."""
        # Case 1: return True.
        mock_reset_bootloader.return_value = True
        self.assertTrue(self.bd.reset_bootloader())
        # Case 2: return False.
        mock_reset_bootloader.return_value = False
        self.assertFalse(self.bd.reset_bootloader())

    @patch.object(BootloaderInterfaceCan, "run_app_on_bootloader")
    def test_run_app(self, mock_run_app_on_bootloader):
        """Function to test function run_app()."""
        # Case 1: False case.
        mock_run_app_on_bootloader.return_value = False
        self.assertFalse(self.bd.run_app())
        # Case 2: True case.
        mock_run_app_on_bootloader.return_value = True
        self.assertTrue(self.bd.run_app())

    @patch.object(BootloaderInterfaceCan, "send_program_info")
    @patch.object(BootloaderInterfaceCan, "start_transfer")
    def test_send_pre_info(self, mock_start_transfer, mock_send_program_info):
        """Function to test function send_pre_info()."""
        # Case 1: start_transfer fails.
        mock_start_transfer.return_value = False
        mock_send_program_info.return_value = True
        self.assertFalse(self.bd.send_pre_info())
        # Case 2: send_program_info fails.
        mock_start_transfer.return_value = True
        mock_send_program_info.return_value = False
        self.assertFalse(self.bd.send_pre_info())
        # Case 3: successfully sent.
        mock_start_transfer.return_value = True
        mock_send_program_info.return_value = True
        self.assertTrue(self.bd.send_pre_info())

    def test_check_if_data_transfer_resumable(self):
        """Function to test function _check_if_data_transfer_resumable()."""
        self.assertFalse(
            self.bd._check_if_data_transfer_resumable(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
                current_num_of_loops=100,
            )
        )
        self.assertFalse(
            self.bd._check_if_data_transfer_resumable(
                can_fsm_state=CanFsmState.CanFsmStateFinishedFinalValidation.name,
                boot_fsm_state=BootFsmState.BootFsmStateWait.name,
                current_num_of_loops=100,
            )
        )
        self.assertFalse(
            self.bd._check_if_data_transfer_resumable(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
                current_num_of_loops=100,
            )
        )
        self.assertFalse(
            self.bd._check_if_data_transfer_resumable(
                can_fsm_state=CanFsmState.CanFsmStateFinishedFinalValidation.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
                current_num_of_loops=0,
            )
        )
        self.assertTrue(
            self.bd._check_if_data_transfer_resumable(
                can_fsm_state=CanFsmState.CanFsmStateFinishedFinalValidation.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
                current_num_of_loops=100,
            )
        )

    def test_check_if_bootloader_at_the_beginning(self):
        """Function to test function _check_if_bootloader_at_the_beginning()."""
        self.assertFalse(
            self.bd._check_if_bootloader_at_the_beginning(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
                current_num_of_loops=0,
            )
        )
        self.assertFalse(
            self.bd._check_if_bootloader_at_the_beginning(
                can_fsm_state=CanFsmState.CanFsmFinishedTransferVectorTable.name,
                boot_fsm_state=BootFsmState.BootFsmStateWait.name,
                current_num_of_loops=0,
            )
        )
        self.assertFalse(
            self.bd._check_if_bootloader_at_the_beginning(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
                current_num_of_loops=0,
            )
        )
        self.assertFalse(
            self.bd._check_if_bootloader_at_the_beginning(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateWait.name,
                current_num_of_loops=2,
            )
        )
        self.assertTrue(
            self.bd._check_if_bootloader_at_the_beginning(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateWait.name,
                current_num_of_loops=0,
            )
        )

    def test_check_if_only_send_vector_table(self):
        """Function to test function _check_if_only_send_vector_table()."""
        self.assertFalse(
            self.bd._check_if_only_send_vector_table(
                can_fsm_state=CanFsmState.CanFsmFinishedTransferVectorTable.name,
                boot_fsm_state=BootFsmState.BootFsmStateWait.name,
            )
        )
        self.assertFalse(
            self.bd._check_if_only_send_vector_table(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
            )
        )
        self.assertFalse(
            self.bd._check_if_only_send_vector_table(
                can_fsm_state=CanFsmState.CanFsmStateNoCommunication.name,
                boot_fsm_state=BootFsmState.BootFsmStateWait.name,
            )
        )
        self.assertTrue(
            self.bd._check_if_only_send_vector_table(
                can_fsm_state=CanFsmState.CanFsmFinishedTransferVectorTable.name,
                boot_fsm_state=BootFsmState.BootFsmStateLoad.name,
            )
        )

    @patch.object(time, "sleep", return_value=None)
    @patch.object(Bootloader, "send_and_validate_vector_table")
    @patch.object(Bootloader, "send_app_data")
    @patch.object(Bootloader, "reset_bootloader")
    @patch.object(Bootloader, "send_pre_info")
    @patch.object(Bootloader, "_check_if_only_send_vector_table")
    @patch.object(Bootloader, "_check_if_data_transfer_resumable")
    @patch.object(Bootloader, "_check_if_bootloader_at_the_beginning")
    @patch.object(BootloaderInterfaceCan, "get_current_num_of_loops")
    @patch.object(BootloaderInterfaceCan, "get_bootloader_state")
    @patch.object(BootloaderBinaryFile, "_check_app_size")
    # pylint: disable-next=too-many-statements,too-many-arguments,too-many-positional-arguments
    def test_send_app_binary(
        self,
        mock_check_app_size,
        mock_get_bootloader_state,
        mock_get_current_num_of_loops,
        mock_check_if_bootloader_at_the_beginning,
        mock_check_if_data_transfer_resumable,
        mock_check_if_only_send_vector_table,
        mock_send_pre_info,
        mock_reset_bootloader,
        mock_send_app_data,
        mock_send_and_validate_vector_table,
        mock_sleep,
    ):
        """Function to test function send_app_binary()."""
        mock_sleep.return_value = None
        # Case 1-1: bootloader is at the initial status, every thing runs well.
        mock_check_app_size.return_value = True
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 0
        mock_check_if_bootloader_at_the_beginning.return_value = True
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        interface = BootloaderInterfaceCan(can_bus=self.can_bus)
        bd = Bootloader(
            interface=interface,
            path_app_binary=self.path_binary_data,
            app_size=0x100,
            path_crc_64_table=self.path_crc_64_table,
        )
        self.assertTrue(bd.send_app_binary())

        # Case 1-2: bootloader is at the initial status, but PC cannot get
        # the current loop number from bootloader.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = None
        mock_check_if_bootloader_at_the_beginning.return_value = True
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        bd = Bootloader(
            interface=interface,
            path_app_binary=self.path_binary_data,
            app_size=0x100,
            path_crc_64_table=self.path_crc_64_table,
        )
        with self.assertRaises(SystemExit) as cm:
            bd.send_app_binary()
        self.assertEqual(cm.exception.code, "Cannot get all states of the bootloader.")

        # Case 2-1: bootloader is not at the initial status, data transfer is
        # resumable, the data has been not transferred totally.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateReceived8BytesCrc",
            "BootFsmStateLoad",
        )
        mock_get_current_num_of_loops.return_value = 16384
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = True
        mock_check_if_only_send_vector_table.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertTrue(bd.send_app_binary())

        # Case 2-2: bootloader is not at the initial status, data transfer is
        # resumable, the data has been transferred totally.
        mock_get_bootloader_state.return_value = (
            "CanFsmFinishedTransferVectorTable",
            "BootFsmStateLoad",
        )
        mock_get_current_num_of_loops.return_value = 16384
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = True
        mock_check_if_only_send_vector_table.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertTrue(bd.send_app_binary())

        # Case 3-1: bootloader is not at the initial status,
        # data transfer is not resumable, reset bootloader fails
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertFalse(bd.send_app_binary())

        # Case 3-2: bootloader is not at the initial status,
        # data transfer is not resumable, reset bootloader successfully,
        # send pre-info fails,
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = False
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertFalse(bd.send_app_binary())

        # Case 3-3: bootloader is not at the initial status,
        # data transfer is not resumable, reset bootloader successfully,
        # send pre-info successfully, send app data fails.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.reset_mock()
        mock_send_app_data.return_value = False
        mock_send_and_validate_vector_table.reset_mock()
        mock_send_and_validate_vector_table.return_value = True
        self.assertFalse(bd.send_app_binary())
        self.assertEqual(mock_send_app_data.call_count, 4)
        self.assertEqual(mock_send_and_validate_vector_table.call_count, 0)

        # Case 3-4: bootloader is not at the initial status,
        # data transfer is not resumable, reset bootloader successfully,
        # send pre-info successfully, send app data successfully, send and
        # validate vector table fails.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.reset_mock()
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.reset_mock()
        mock_send_and_validate_vector_table.return_value = False
        self.assertFalse(bd.send_app_binary())
        self.assertEqual(mock_send_app_data.call_count, 1)
        self.assertEqual(mock_send_and_validate_vector_table.call_count, 4)

        # Case 3-5: bootloader is not at the initial status,
        # data transfer is not resumable, reset bootloader successfully,
        # send pre-info successfully, send app data successfully, send and
        # validate vector table successfully.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_data_transfer_resumable.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertTrue(bd.send_app_binary())


if __name__ == "__main__":
    unittest.main()
