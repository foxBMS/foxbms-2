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

"""Testing file 'cli/cmd_bootloader/crc64_ti_impl.py'."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parents[2] / "tools/waf-tools"))
import crc64_ti_impl  # pylint: disable=import-error,wrong-import-position


class TestCRC64TIImpl(unittest.TestCase):
    """Class to test the the functions inside crc64_ti_impl.py"""

    def test_generate_crc_64_for_tms570_on_board_crc_calculation(self):
        """Function to test generate_crc_64_for_tms570_on_board_crc_calculation()"""
        # Test one single 64-bits data with 0 as crc root value
        test_data_1 = 0x1122334455667788
        crc_1 = crc64_ti_impl.generate_crc_64_for_tms570_on_board_crc_calculation(
            data_64_bits=test_data_1
        )
        self.assertEqual(crc_1, 12129596691791633497)

        # Test two 64-bits data with the crc result of one input as the crc input for the next
        test_data_2 = 0x99AABBCCDDEEFF00
        crc_2 = crc64_ti_impl.generate_crc_64_for_tms570_on_board_crc_calculation(
            data_64_bits=test_data_2, crc_64_bits=crc_1
        )
        self.assertEqual(crc_2, 13985017789367110321)

        # Test 1024 repeated data
        test_data_3 = 0x123456789ABCDEF0
        crc_3 = 0
        for _ in range(1024):
            crc_3 = crc64_ti_impl.generate_crc_64_for_tms570_on_board_crc_calculation(
                data_64_bits=test_data_3, crc_64_bits=crc_3
            )
        self.assertEqual(crc_3, 11957925961466737347)


if __name__ == "__main__":
    unittest.main()
