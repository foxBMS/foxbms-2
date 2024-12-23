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

"""Testing file 'cli/cmd_bootloader/bootloader_binary_file.py'."""

import io
import shutil
import sys
import unittest
from json import JSONDecodeError
from pathlib import Path
from typing import List
from unittest.mock import patch

import numpy as np

# Redirect message or not
MSG_REDIRECT = True

# Path of temp where the temporal files generated by test function will be saved
PATH_TEMP = Path(__file__).parent / "temp"


try:
    from cli.cmd_bootloader.app_constants import APP_SIZE
    from cli.cmd_bootloader.bootloader_binary_file import (
        PATH_CRC_64_TABLE,
        BootloaderBinaryFile,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.cmd_bootloader.app_constants import APP_SIZE
    from cli.cmd_bootloader.bootloader_binary_file import (
        PATH_CRC_64_TABLE,
        BootloaderBinaryFile,
    )


def convert_list_of_uint64_to_bytes(list_of_8_bytes: List[int]) -> bytes:
    """Convert a list of unsigned 64-bits integer to bytes.

    Args:
        list_of_8_bytes: a list of unsigned 64-bits/8-bytes integer.

    Returns:
        return_bytes: the converted bytes.
    """
    return_bytes = b""
    for _, item in enumerate(list_of_8_bytes):
        return_bytes += item.to_bytes(8, "big")
    return return_bytes


class TestBootloaderBinaryFile(unittest.TestCase):
    """Class to test the BootloaderBinaryFile class."""

    # pylint:disable=too-many-instance-attributes

    def setUp(self):
        # Redirect the sys.stdout to the StringIO object
        if MSG_REDIRECT:
            sys.stdout = io.StringIO()

        # Create temporal directory for saving files generated by test functions
        PATH_TEMP.mkdir(parents=True, exist_ok=True)
        self.path_crc_64_table_1 = PATH_TEMP / "test_crc_64_1.csv"
        self.path_crc_64_table_2 = PATH_TEMP / "test_crc_64_2.csv"
        self.path_binary_data_1 = PATH_TEMP / "test_1.bin"
        self.path_binary_data_2 = PATH_TEMP / "test_2.bin"

        # Create data to be used for testing
        # Prepare vector table (list of uint64)
        self.test_vector_table = [
            16262709364579450888,
            720575995771297696,
            16402109846735564868,
            16321362090676846595,
        ]

        # Prepare test data 1
        list_test_data_1 = [
            16262709364579450888,
            720575995771297696,
            16402109846735564868,
        ]
        self.test_data_1_crc = [
            3772680413081354448,
            6161991488417852690,
            668399922976118821,
        ]
        self.test_data_1 = convert_list_of_uint64_to_bytes(list_test_data_1)
        self.test_data_1_with_zeros = self.test_data_1 + bytes([0] * 8)

        # Prepare test data 2
        list_test_data_2 = [
            16262709364579450888,
            720575995771297696,
            16402109846735564868,
            16321362090676846595,
        ]
        self.test_data_2 = convert_list_of_uint64_to_bytes(list_test_data_2)

        # Write to file
        self.path_binary_data_1.write_bytes(
            convert_list_of_uint64_to_bytes(self.test_vector_table) + self.test_data_1
        )
        self.path_binary_data_2.write_bytes(
            convert_list_of_uint64_to_bytes(self.test_vector_table) + self.test_data_2
        )

    def tearDown(self):
        # Delete the temporal directory
        shutil.rmtree(PATH_TEMP)

        # Reset sys.stdout to its original value
        sys.stdout = sys.__stdout__

    @patch.object(np, "genfromtxt")
    @patch.object(BootloaderBinaryFile, "_is_crc64_table_valid")
    @patch.object(BootloaderBinaryFile, "_check_app_size")
    def test_init(
        self, mock_check_app_size, mock_is_crc64_table_valid, mock_genfromtxt
    ):
        """Function to test function __init__()."""
        # Case 1: the input dbc file is not valid
        path_app_binary = Path("./fake")
        mock_is_crc64_table_valid.return_value = True
        mock_check_app_size.return_value = True
        with self.assertRaises(SystemExit) as cm:
            BootloaderBinaryFile(
                path_app_binary=path_app_binary,
                app_size=32,
                path_crc_64_table=self.path_crc_64_table_1,
            )
        self.assertEqual(
            cm.exception.code,
            f"Binary '{path_app_binary}' does not exist.",
        )

        # Case 2: the input app_size is None, use default app_size
        mock_is_crc64_table_valid.return_value = True
        mock_genfromtxt.return_value = None
        mock_check_app_size.return_value = True
        bbf = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_1,
            app_size=None,
            path_crc_64_table=self.path_crc_64_table_1,
        )
        self.assertEqual(len(bbf.byte_array), APP_SIZE)

        # Case 3: the app size is not valid
        mock_is_crc64_table_valid.return_value = True
        mock_genfromtxt.return_value = None
        mock_check_app_size.return_value = False
        with self.assertRaises(SystemExit) as cm:
            bbf = BootloaderBinaryFile(
                path_app_binary=self.path_binary_data_2,
                app_size=32,
                path_crc_64_table=self.path_crc_64_table_2,
            )
        self.assertEqual(
            cm.exception.code,
            "The app size is invalid.\n"
            "The app size must be the sum of sizes of the used sectors.",
        )

        # Case 4: the input path_crc_64_table is None
        mock_is_crc64_table_valid.return_value = True
        mock_genfromtxt.return_value = None
        mock_check_app_size.return_value = True
        bbf = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=None,
        )
        self.assertEqual(bbf.crc64_table_file, PATH_CRC_64_TABLE)

    @patch.object(BootloaderBinaryFile, "_check_app_size")
    def test_init_2(self, mock_check_app_size):
        """Function to test the init function of BootloaderBinaryFile."""
        mock_check_app_size.return_value = True
        bbf1 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_1,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_1,
        )
        bbf2 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_2,
        )
        self.assertEqual(bbf1.data64_vector_table, self.test_vector_table)
        self.assertEqual(bbf1.byte_array, self.test_data_1_with_zeros)
        self.assertEqual(bbf1.len_of_program_in_bytes, 32)
        self.assertEqual(bbf1.len_of_program_in_8_bytes, 4)
        self.assertEqual(bbf1.crc64_vector_table, 16000361463206193970)

        self.assertEqual(bbf2.data64_vector_table, self.test_vector_table)
        self.assertEqual(bbf2.byte_array, self.test_data_2)
        self.assertEqual(bbf2.len_of_program_in_bytes, 32)
        self.assertEqual(bbf2.len_of_program_in_8_bytes, 4)
        self.assertEqual(bbf2.crc64_vector_table, 16000361463206193970)

        self.assertTrue(self.path_crc_64_table_1.exists())
        self.assertTrue(self.path_crc_64_table_2.exists())
        array_csv_1 = np.genfromtxt(self.path_crc_64_table_1, delimiter=",", dtype=str)
        array_csv_2 = np.genfromtxt(self.path_crc_64_table_2, delimiter=",", dtype=str)

        self.assertEqual(int(array_csv_1[1][1], 16), 0xE1B0C000E92D4008)
        self.assertEqual(int(array_csv_1[1][2], 16), 0x345B40D8E0C9C0D0)
        self.assertEqual(int(array_csv_1[1][3]), 16262709364579450888)
        self.assertEqual(int(array_csv_1[1][4]), 3772680413081354448)
        self.assertEqual(int(array_csv_2[1][1], 16), 0xE1B0C000E92D4008)
        self.assertEqual(int(array_csv_2[1][2], 16), 0x345B40D8E0C9C0D0)
        self.assertEqual(int(array_csv_2[1][3]), 16262709364579450888)
        self.assertEqual(int(array_csv_2[1][4]), 3772680413081354448)

    @patch.object(BootloaderBinaryFile, "_check_app_size")
    def test_is_crc64_table_valid(self, mock_check_app_size):
        """Function to test function _is_crc64_table_valid()."""
        # Case 1: If crc64_table_file is not a file
        mock_check_app_size.return_value = True
        bbf1 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_2,
        )
        bbf1.crc64_table_file = Path("./fake")
        # pylint: disable-next=protected-access
        self.assertFalse(bbf1._is_crc64_table_valid())

        # Case 2: If app_hash_file is not a file
        mock_check_app_size.return_value = True
        bbf2 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_2,
        )
        bbf2.app_hash_file = Path("./fake")
        # pylint: disable-next=protected-access
        self.assertFalse(bbf2._is_crc64_table_valid())

        # Case 3: If app_hash_file is invalid
        mock_check_app_size.return_value = True
        bbf3 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_2,
        )
        with patch(
            "json.loads", side_effect=JSONDecodeError("Error", "app_hash_file", 0)
        ):
            # pylint: disable-next=protected-access
            self.assertFalse(bbf3._is_crc64_table_valid())

        # Case 4: If app_file.as_posix does not exist in app_hash_file
        mock_check_app_size.return_value = True
        bbf4 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_2,
        )
        bbf4.app_file = Path("fake")
        # pylint: disable-next=protected-access
        self.assertFalse(bbf4._is_crc64_table_valid())

        # If everything goes well
        bbf5 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_2,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_2,
        )
        # pylint: disable-next=protected-access
        self.assertTrue(bbf5._is_crc64_table_valid())

    def test_check_app_size(self):
        """Function to test the function _check_app_size."""
        # pylint: disable=protected-access
        # Invalid app sizes
        self.assertFalse(BootloaderBinaryFile._check_app_size(32))
        self.assertFalse(BootloaderBinaryFile._check_app_size(0x400000))
        # Valid app sizes
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x20000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x40000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x60000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0xA0000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0xE0000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x120000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x160000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x1A0000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x1E0000))
        self.assertTrue(BootloaderBinaryFile._check_app_size(0x3E0000))

    @patch.object(BootloaderBinaryFile, "_check_app_size")
    def test_get_crc_and_data_by_index(self, mock_check_app_size):
        """Function to test the function get_crc_and_data_by_index."""
        mock_check_app_size.return_value = True
        bbf1 = BootloaderBinaryFile(
            path_app_binary=self.path_binary_data_1,
            app_size=32,
            path_crc_64_table=self.path_crc_64_table_1,
        )
        crc_64_bits, data_64_bits = bbf1.get_crc_and_data_by_index(2)
        self.assertEqual(crc_64_bits, 668399922976118821)
        self.assertEqual(data_64_bits, 16402109846735564868)
        bbf1.array_csv[2 + 1][1] = 0
        with self.assertRaises(SystemExit) as cm:
            bbf1.get_crc_and_data_by_index(2)
        self.assertEqual(
            cm.exception.code,
            "The data in byte_array does not match the one in array_csv, the "
            "CRC table might be outdated.",
        )


if __name__ == "__main__":
    unittest.main()
