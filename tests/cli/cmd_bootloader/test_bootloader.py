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

"""Testing file 'cli/cmd_bootloader/bootloader.py'."""

import io
import json
import logging
import sys
import time
import unittest
from pathlib import Path
from typing import cast
from unittest.mock import MagicMock, patch

import can
import numpy as np
from cantools import database

try:
    from cli.cmd_bootloader.bootloader import Bootloader
    from cli.cmd_bootloader.bootloader_binary_file import BootloaderBinaryFile
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_can_messages import BootFsmState, CanFsmState
    from cli.helpers.misc import BOOTLOADER_DBC_FILE, FOXBMS_BIN_FILE
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bootloader.bootloader import Bootloader
    from cli.cmd_bootloader.bootloader_binary_file import BootloaderBinaryFile
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_can_messages import BootFsmState, CanFsmState
    from cli.helpers.misc import BOOTLOADER_DBC_FILE, FOXBMS_BIN_FILE

# Redirect message or not
MSG_REDIRECT = True

# Other paths
PATH_TEMP = Path(__file__).parent / "temp"


# pylint: disable=protected-access
@patch.object(logging, "info", return_value=None)
@patch.object(logging, "warning", return_value=None)
@patch.object(logging, "error", return_value=None)
class TestBootloader(unittest.TestCase):
    """Class to test Bootloader class."""

    # pylint: disable=arguments-differ
    @patch("cli.cmd_bootloader.bootloader_binary_file.get_sha256_file_hash_str")
    @patch.object(np, "genfromtxt")
    @patch.object(json, "loads")
    @patch.object(Path, "read_text")
    def setUp(
        self, mock_read_text, mock_loads, mock_genfromtxt, mock_get_sha256_file_hash_str
    ):
        # Redirect the sys.stdout to the StringIO object
        if MSG_REDIRECT:
            sys.stdout = io.StringIO()

        # Initialize virtual CAN bus instance
        self.can_bus = can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        )

        # Prepare the fake information for bootloader BootloaderBinaryFile
        self.program = {
            "len_of_program_in_bytes": 16,
            "len_of_program_in_8_bytes": 2,
            "app_size": 262144,
            "bin_file": FOXBMS_BIN_FILE,
            "vector_table_crc": 10755722470602090755,
            "vector_table": [
                16861741595983085566,
                16861496787142180862,
                16933534594249061028,
                16510180501024141744,
            ],
            "hash_app": "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "hash_csv": "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        }
        self.array_csv = np.array(
            [
                [
                    "",
                    "data_64_bits_hex",
                    "crc_64_bits_hex",
                    "data_64_bits",
                    "crc_64_bits",
                ],
                [
                    "0",
                    "0x10178797a00021f",
                    "0x1b1fccbffe003729",
                    "72471532147835423",
                    "1954505888274331433",
                ],
                [
                    "1",
                    "0x3733626331303166",
                    "0xd7da68b1dbd05c5a",
                    "3977631074064806246",
                    "15553859376183794778",
                ],
            ]
        )

        # Initialize a Bootloader instance
        mock_read_text.return_value = None
        mock_loads.return_value = self.program
        mock_genfromtxt.return_value = self.array_csv
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        interface = BootloaderInterfaceCan(can_bus=self.can_bus)
        self.bd = Bootloader(interface=interface)

        # Load dbc file
        self.db = database.load_file(BOOTLOADER_DBC_FILE)

    def tearDown(self):
        self.can_bus.shutdown()

    def test_get_sub_sector_loops(self, *_):
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

    def test_get_sector_size_using_num_of_data_loops(self, *_):
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
    @patch.object(BootloaderInterfaceCan, "get_foxbms_state")
    @patch.object(BootloaderInterfaceCan, "get_current_num_of_loops")
    @patch.object(BootloaderInterfaceCan, "get_bootloader_state")
    def test_check_target(
        self,
        mock_get_bootloader_state,
        mock_get_current_num_of_loops,
        mock_get_foxbms_state,
        mock_get_bootloader_version_num,
        *_,
    ):
        """Function to test function check_target()."""
        # Case 1-1: return 3, the information of bootloader can not be reached.
        mock_get_bootloader_state.return_value = (None, None)
        mock_get_current_num_of_loops.return_value = None
        mock_get_foxbms_state.return_value = None
        mock_get_bootloader_version_num.return_value = (
            None,
            None,
            None,
        )
        self.assertEqual(3, self.bd.check_target())

        # Case 2-1: return 1, partial information of bootloader can not be reached.
        mock_get_bootloader_state.return_value = (None, "BootFsmStateWait")
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            2,
        )
        self.assertEqual(1, self.bd.check_target())

        # Case 2-2: return 1, partial information of bootloader can not be reached.
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
        self.assertEqual(1, self.bd.check_target())

        # Case 2-3: return 1, partial information of bootloader can not be reached.
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
        self.assertEqual(1, self.bd.check_target())

        # Case 2-4: return 1, partial information of bootloader can not be reached.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            None,
            1,
            2,
        )
        self.assertEqual(1, self.bd.check_target())

        # Case 2-5: return 1, partial information of bootloader can not be reached.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            6,
            None,
            2,
        )
        self.assertEqual(1, self.bd.check_target())

        # Case 2-6: return 1, partial information of bootloader can not be reached.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 2
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            None,
        )
        self.assertEqual(1, self.bd.check_target())

        # Case 2-6: return 1, partial information of bootloader can not be reached.
        mock_get_bootloader_state.return_value = (
            None,
            None,
        )
        mock_get_current_num_of_loops.return_value = None
        mock_get_foxbms_state.return_value = None
        mock_get_bootloader_version_num.return_value = (
            6,
            1,
            None,
        )
        self.assertEqual(1, self.bd.check_target())

        # Case 3-1: return 2, foxBMS is running.
        mock_get_bootloader_state.return_value = (
            None,
            None,
        )
        mock_get_current_num_of_loops.return_value = None
        mock_get_foxbms_state.return_value = "test_bms_state"
        self.assertEqual(2, self.bd.check_target())

        # Case 4-1: return 0, bootloader is runnning, and all information can
        # be received.
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
        self.assertEqual(0, self.bd.check_target())

    @patch.object(BootloaderInterfaceCan, "send_crc")
    @patch.object(BootloaderInterfaceCan, "wait_can_ack_msg")
    @patch.object(BootloaderInterfaceCan, "send_data_to_bootloader")
    def test_send_and_validate_vector_table(
        self, mock_send_data_to_bootloader, mock_wait_can_ack_msg, mock_send_crc, *_
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
        *_,
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
        *_,
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

        # Case 7: true case, show progressbar.
        mock_send_data_as_a_sub_sector.reset_mock()
        mock_send_data_as_a_sub_sector.return_value = True
        mock_get_crc_and_data_by_index.return_value = (
            0xFFFFFFFFFFFFFFFF,
            0xFFFFFFFFFFFFFFFF,
        )
        mock_send_crc.return_value = (True, True)
        with patch("click.progressbar") as mock_progressbar:
            mock_progressbar.return_value.__enter__.return_value = MagicMock()
            self.assertTrue(
                self.bd.send_data_as_a_sector(
                    i_loop=1,
                    total_num_of_loops=16384,
                    size_of_sector_in_loops=16384,
                    times_of_repeat=1,
                    progressbar=mock_progressbar,
                    progressbar_sector_steps=20,
                )
            )

    @patch.object(Bootloader, "_get_sector_size_using_num_of_data_loops")
    @patch.object(Bootloader, "send_data_as_a_sector")
    def test_send_app_data(
        self,
        mock_send_data_as_a_sector,
        mock_get_sector_size_using_num_of_data_loops,
        *_,
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
    def test_reset_bootloader(self, mock_reset_bootloader, *_):
        """Function to test function reset_bootloader()."""
        # Case 1: return True.
        mock_reset_bootloader.return_value = True
        self.assertTrue(self.bd.reset_bootloader())
        # Case 2: return False.
        mock_reset_bootloader.return_value = False
        self.assertFalse(self.bd.reset_bootloader())

    @patch.object(BootloaderInterfaceCan, "run_app_on_bootloader")
    def test_run_app(self, mock_run_app_on_bootloader, *_):
        """Function to test function run_app()."""
        # Case 1: False case.
        mock_run_app_on_bootloader.return_value = False
        self.assertFalse(self.bd.run_app())
        # Case 2: True case.
        mock_run_app_on_bootloader.return_value = True
        self.assertTrue(self.bd.run_app())

    @patch.object(BootloaderInterfaceCan, "send_program_info")
    @patch.object(BootloaderInterfaceCan, "start_transfer")
    def test_send_pre_info(self, mock_start_transfer, mock_send_program_info, *_):
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

    def test_check_if_data_transfer_resumable(self, *_):
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

    def test_check_if_bootloader_at_the_beginning(self, *_):
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

    def test_check_if_only_send_vector_table(self, *_):
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
    @patch.object(Bootloader, "_check_if_bootloader_at_the_beginning")
    @patch.object(BootloaderInterfaceCan, "get_current_num_of_loops")
    @patch.object(BootloaderInterfaceCan, "get_bootloader_state")
    @patch("cli.cmd_bootloader.bootloader_binary_file.get_sha256_file_hash_str")
    @patch.object(np, "genfromtxt")
    @patch.object(json, "loads")
    @patch.object(Path, "read_text")
    # pylint: disable-next=too-many-statements,too-many-arguments,too-many-positional-arguments
    def test_send_app_binary(
        self,
        mock_read_text,
        mock_loads,
        mock_genfromtxt,
        mock_get_sha256_file_hash_str,
        mock_get_bootloader_state,
        mock_get_current_num_of_loops,
        mock_check_if_bootloader_at_the_beginning,
        mock_check_if_only_send_vector_table,
        mock_send_pre_info,
        mock_reset_bootloader,
        mock_send_app_data,
        mock_send_and_validate_vector_table,
        mock_sleep,
        *_,
    ):
        """Function to test function send_app_binary()."""
        mock_sleep.return_value = None
        mock_read_text.return_value = None
        mock_loads.return_value = self.program
        mock_genfromtxt.return_value = self.array_csv

        # Case 1-8: bootloader is at the initial status, every thing runs well,
        # the progress progressbar will be shown.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 0
        mock_check_if_bootloader_at_the_beginning.return_value = True
        mock_check_if_only_send_vector_table.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        interface = BootloaderInterfaceCan(can_bus=self.can_bus)
        bd = Bootloader(interface=interface)
        with patch("click.progressbar") as mock_progressbar:
            mock_progressbar.return_value.__enter__.return_value = MagicMock()
            self.assertTrue(bd.send_app_binary(show_progressbar=True))

        # Case 2-8: bootloader is at the initial status, every thing runs well.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 0
        mock_check_if_bootloader_at_the_beginning.return_value = True
        mock_check_if_only_send_vector_table.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        interface = BootloaderInterfaceCan(can_bus=self.can_bus)
        bd = Bootloader(interface=interface)
        self.assertTrue(bd.send_app_binary(show_progressbar=False))

        # Case 3-8: bootloader is at the initial status, but PC cannot get
        # the current loop number from bootloader.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateNoCommunication",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = None
        mock_check_if_bootloader_at_the_beginning.return_value = True
        mock_check_if_only_send_vector_table.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        bd = Bootloader(interface=interface)
        with self.assertRaises(SystemExit) as cm:
            bd.send_app_binary(show_progressbar=False)
        self.assertEqual(cm.exception.code, "Cannot get all states of the bootloader.")

        # Case 4-8: bootloader is not at the initial status, reset bootloader
        # fails.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = False
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertFalse(bd.send_app_binary(show_progressbar=False))

        # Case 5-8: bootloader is not at the initial status, reset bootloader
        # successfully, send pre-info fails.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = False
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertFalse(bd.send_app_binary(show_progressbar=False))

        # Case 6-8: bootloader is not at the initial status, reset bootloader
        # successfully, send pre-info successfully, send app data fails, show progressbar.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.reset_mock()
        mock_send_app_data.return_value = False
        mock_send_and_validate_vector_table.reset_mock()
        mock_send_and_validate_vector_table.return_value = True
        with patch("click.progressbar") as mock_progressbar:
            mock_progressbar.return_value.__enter__.return_value = MagicMock()
            self.assertFalse(bd.send_app_binary(show_progressbar=True))
        self.assertEqual(mock_send_app_data.call_count, 3)
        self.assertEqual(mock_send_and_validate_vector_table.call_count, 0)

        # Case 6-8: bootloader is not at the initial status, reset bootloader
        # successfully, send pre-info successfully, send app data fails.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.reset_mock()
        mock_send_app_data.return_value = False
        mock_send_and_validate_vector_table.reset_mock()
        mock_send_and_validate_vector_table.return_value = True
        self.assertFalse(bd.send_app_binary(show_progressbar=False))
        self.assertEqual(mock_send_app_data.call_count, 3)
        self.assertEqual(mock_send_and_validate_vector_table.call_count, 0)

        # Case 7-8: bootloader is not at the initial status, reset bootloader
        # successfully, send pre-info successfully, send app data successfully,
        # send and validate vector table fails, show progressbar.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.reset_mock()
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.reset_mock()
        mock_send_and_validate_vector_table.return_value = False
        with patch("click.progressbar") as mock_progressbar:
            mock_progressbar.return_value.__enter__.return_value = MagicMock()
            self.assertFalse(bd.send_app_binary(show_progressbar=True))
        self.assertEqual(mock_send_app_data.call_count, 1)
        self.assertEqual(mock_send_and_validate_vector_table.call_count, 3)

        # Case 7-8: bootloader is not at the initial status, reset bootloader
        # successfully, send pre-info successfully, send app data successfully,
        # send and validate vector table fails.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.reset_mock()
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.reset_mock()
        mock_send_and_validate_vector_table.return_value = False
        self.assertFalse(bd.send_app_binary(show_progressbar=False))
        self.assertEqual(mock_send_app_data.call_count, 1)
        self.assertEqual(mock_send_and_validate_vector_table.call_count, 3)

        # Case 8-8: bootloader is not at the initial status, reset bootloader
        # successfully, send pre-info successfully, send app data successfully,
        # send and validate vector table successfully.
        mock_get_bootloader_state.return_value = (
            "CanFsmStateError",
            "BootFsmStateWait",
        )
        mock_get_current_num_of_loops.return_value = 100
        mock_check_if_bootloader_at_the_beginning.return_value = False
        mock_check_if_only_send_vector_table.return_value = False
        mock_reset_bootloader.return_value = True
        mock_send_pre_info.return_value = True
        mock_send_app_data.return_value = True
        mock_send_and_validate_vector_table.return_value = True
        self.assertTrue(bd.send_app_binary(show_progressbar=False))

    # def test_can_bus_config(self, *_):
    #     """Test bootloader configuration helper."""
    #     with self.assertRaises(SystemExit) as cm:
    #         CanBusConfig(interface="pcan", channel=0)
    #     self.assertEqual(
    #         cm.exception.code,
    #         "Invalid channel choice for interface 'pcan'.",
    #     )
    #     with self.assertRaises(SystemExit) as cm:
    #         CanBusConfig(interface="pcan", channel="foo")
    #     self.assertEqual(
    #         cm.exception.code,
    #         "Invalid channel choice for interface 'pcan'.",
    #     )
    #     with self.assertRaises(SystemExit) as cm:
    #         CanBusConfig(interface="kvaser", channel="foo")
    #     self.assertEqual(
    #         cm.exception.code,
    #         "Invalid channel choice for interface 'kvaser'.",
    #     )
    #     with self.assertRaises(SystemExit) as cm:
    #         CanBusConfig(interface="bla", channel="foo")
    #     self.assertEqual(
    #         cm.exception.code,
    #         "Unsupported interface 'bla'.",
    #     )


if __name__ == "__main__":
    unittest.main()
