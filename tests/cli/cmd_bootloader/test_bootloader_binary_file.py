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

"""Testing file 'cli/cmd_bootloader/bootloader_binary_file.py'."""

import json
import sys
import unittest
from json import JSONDecodeError
from pathlib import Path
from unittest.mock import patch

import numpy as np

try:
    from cli.cmd_bootloader.bootloader_binary_file import BootloaderBinaryFile
    from cli.helpers.misc import (
        FOXBMS_APP_CRC_FILE,
        FOXBMS_APP_INFO_FILE,
        FOXBMS_BIN_FILE,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bootloader.bootloader_binary_file import BootloaderBinaryFile
    from cli.helpers.misc import (
        FOXBMS_APP_CRC_FILE,
        FOXBMS_APP_INFO_FILE,
        FOXBMS_BIN_FILE,
    )


class TestBootloaderBinaryFile(unittest.TestCase):
    """Class to test the BootloaderBinaryFile class."""

    def setUp(self):
        pass

    def tearDown(self):
        pass

    @patch("cli.cmd_bootloader.bootloader_binary_file.get_sha256_file_hash_str")
    @patch.object(np, "genfromtxt")
    @patch.object(json, "loads")
    @patch.object(Path, "read_text")
    # pylint: disable-next=too-many-statements
    def test_init(
        self, mock_read_text, mock_loads, mock_genfromtxt, mock_get_sha256_file_hash_str
    ):
        """test the init function"""
        program = {
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
        array_csv = np.array(
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
        # Case 1: no early exit
        mock_read_text.return_value = None
        mock_loads.return_value = program
        mock_genfromtxt.return_value = array_csv
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        bb = BootloaderBinaryFile()

        # Case 2: json file does not exit
        mock_read_text.reset_mock()
        mock_get_sha256_file_hash_str.reset_mock()
        mock_read_text.side_effect = FileNotFoundError
        mock_loads.return_value = program
        mock_genfromtxt.return_value = array_csv
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"{Path(FOXBMS_APP_INFO_FILE)} not found, please run 'waf build_app_embedded' "
            "command to build the project first/again, exit.",
        )

        # Case 3: Other error- JSONDecodeError produced by json.load()
        mock_read_text.reset_mock()
        mock_get_sha256_file_hash_str.reset_mock()
        mock_read_text.return_value = None
        mock_read_text.side_effect = None
        mock_loads.return_value = None
        mock_loads.side_effect = JSONDecodeError("fake", "fake", 0)
        mock_genfromtxt.return_value = array_csv
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"Can not load {FOXBMS_APP_INFO_FILE}, exit.",
        )

        # Case 4: Other error- TypeError by json.load()
        mock_read_text.reset_mock()
        mock_get_sha256_file_hash_str.reset_mock()
        mock_read_text.return_value = None
        mock_read_text.side_effect = None
        mock_loads.side_effect = TypeError
        mock_loads.return_value = None
        mock_genfromtxt.return_value = array_csv
        mock_get_sha256_file_hash_str.side_effect = None
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"Can not load {FOXBMS_APP_INFO_FILE}, exit.",
        )

        # Case 5: binary file not found
        mock_read_text.reset_mock()
        mock_get_sha256_file_hash_str.reset_mock()
        mock_read_text.return_value = None
        mock_read_text.side_effect = None
        mock_loads.side_effect = None
        mock_loads.return_value = program
        mock_genfromtxt.return_value = array_csv
        mock_get_sha256_file_hash_str.side_effect = FileNotFoundError
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"{FOXBMS_BIN_FILE} not found, please run 'waf build_app_embedded' "
            "command to build the project first/again, exit.",
        )

        # Case 6: csv file not found
        mock_read_text.reset_mock()
        mock_get_sha256_file_hash_str.reset_mock()
        mock_read_text.return_value = None
        mock_read_text.side_effect = None
        mock_loads.return_value = program
        mock_genfromtxt.side_effect = FileNotFoundError
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"{FOXBMS_APP_CRC_FILE} not found, please run 'waf build_app_embedded' "
            "command to build the project first/again, exit.",
        )

        # Case 7: app != Path(program["bin_file"]) -> early exit
        mock_loads.reset_mock()
        mock_get_sha256_file_hash_str.reset_mock()
        program["bin_file"] = "bin_file_invalid"
        mock_loads.return_value = program
        mock_genfromtxt.return_value = array_csv
        mock_genfromtxt.side_effect = None
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"{FOXBMS_BIN_FILE} does not match {FOXBMS_APP_INFO_FILE}"
            " Please try to remove all these files using 'waf clean_app_embedded'"
            " and build them again using 'waf build_app_embedded'. Abort!",
        )

        # Case 8: hash_app_current != hash_app -> early exit
        mock_loads.reset_mock()
        program["bin_file"] = FOXBMS_BIN_FILE
        program["hash_app"] = "000"
        program["hash_csv"] = (
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a"
        )
        mock_loads.return_value = program
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"{FOXBMS_BIN_FILE} does not match {FOXBMS_APP_INFO_FILE}"
            " Please try to remove all these files using 'waf clean_app_embedded'"
            " and build them again using 'waf build_app_embedded'. Abort!",
        )

        # Case 9: hash_csv != hash_csv_current -> early exit
        mock_loads.reset_mock()
        program["bin_file"] = FOXBMS_BIN_FILE
        program["hash_app"] = (
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389"
        )
        program["hash_csv"] = "000"
        mock_loads.return_value = program
        mock_get_sha256_file_hash_str.side_effect = [
            "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
            "716ef6584531e57dd90e1bf8df14f261f36a234803e2dd8f9c4d1d374dd2e60a",
        ]
        with self.assertRaises(SystemExit) as cm:
            _ = BootloaderBinaryFile()
        self.assertEqual(
            cm.exception.code,
            f"{FOXBMS_BIN_FILE} does not match {FOXBMS_APP_INFO_FILE}"
            " Please try to remove all these files using 'waf clean_app_embedded'"
            " and build them again using 'waf build_app_embedded'. Abort!",
        )

        # Test get_crc_and_data_by_index
        self.assertEqual(
            bb.get_crc_and_data_by_index(0), (1954505888274331433, 72471532147835423)
        )


if __name__ == "__main__":
    unittest.main()
