#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing file 'cli/cmd_etl/etl/can_filter.py'."""

import sys
import unittest
from pathlib import Path

try:
    from cli.cmd_etl.etl.can_filter import CANFilter
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.etl.can_filter import CANFilter


class TestInit(unittest.TestCase):
    """Tests the init method of the CANFilter class"""

    def test_can_filter_init_all_parameter(self) -> None:
        """Tests the init method of a CANFilter class with all parameter
        passed
        """
        test_obj = CANFilter(ids=["23A", "120"], id_pos=4, sampling={"23A": 5})
        expected_dict = {
            "_ids": ["23A", "120"],
            "_id_pos": 4,
            "_sampling": {"23A": 5},
            "_occurrence": {"23A": 0},
            "input": None,
            "output": None,
        }
        self.assertDictEqual(test_obj.__dict__, expected_dict)
        self.assertEqual(
            str(test_obj),
            "ID pos: 4, IDs: ['23A', '120'], sampling: {'23A': 5}, occurrence: {'23A': 0}",
        )

    def test_can_filter_init_all_mandatory_parameter(self) -> None:
        """Tests the init method of a CANFilter class with all
        mandatory parameter passed
        """
        test_obj = CANFilter(ids=["23A", "120"], id_pos=4)
        expected_dict = {
            "_ids": ["23A", "120"],
            "_id_pos": 4,
            "_sampling": None,
            "input": None,
            "output": None,
        }
        self.assertDictEqual(test_obj.__dict__, expected_dict)
        self.assertEqual(
            str(test_obj),
            "ID pos: 4, IDs: ['23A', '120'], sampling: None",
        )


class TestFilterMsg(unittest.TestCase):
    """Tests the filter_msg method of the CANFilter class"""

    def test_filter_msg_valid_msg(self) -> None:
        """Tests the filter_msg method of the CANFilter class with valid CAN msg"""
        can_filter = CANFilter(ids=["130", "120"], id_pos=2)
        msg = "  0.000237 7  130        Rx D 5  00  F0  FC  FF  FF"
        self.assertEqual(can_filter.filter_msg(msg), msg)

    def test_filter_msg_wrong_id(self) -> None:
        """Tests the filter_msg method of the CANFilter class with wrong id"""
        can_filter = CANFilter(ids=["130", "120"], id_pos=2)
        msg = "  0.000237 7  170        Rx D 5  00  F0  FC  FF  FF"
        self.assertEqual(can_filter.filter_msg(msg), None)

    def test_filter_msg_invalid_msg(self) -> None:
        """Tests the filter_msg method of the CANFilter class with invalid CAN msg"""
        can_filter = CANFilter(ids=["130", "120"], id_pos=2)
        msg = "Some meta data at the beginning of a file"
        self.assertEqual(can_filter.filter_msg(msg), None)

    def test_filter_msg_with_sampling(self) -> None:
        """Tests the filter_msg method of the CANFilter class with sampling"""
        can_filter = CANFilter(ids=["130", "120"], id_pos=2, sampling={"130": 2})
        msg = "  0.000237 7  130        Rx D 5  00  F0  FC  FF  FF"
        self.assertEqual(can_filter.filter_msg(msg), None)
        self.assertEqual(can_filter.filter_msg(msg), msg)

    def test_filter_msg_index_out_of_range(self) -> None:
        """Tests the filter_msg method of the CANFilter class with an index
        out of range exception
        """
        can_filter = CANFilter(ids=["130", "120"], id_pos=10)
        msg = "  0.000237 7  130  "
        self.assertEqual(can_filter.filter_msg(msg), None)


class TestExtendIds(unittest.TestCase):
    """Tests the extend_ids method of the CANFilter class"""

    def test_extend_ids_correct_extension(self) -> None:
        """Tests the static extend_ids function of the
        CANFilter class with a valid id extension
        """
        extended_ids = CANFilter.extend_ids(["1-3", "5", "79-7A"])
        expected_result = ["5", "1", "2", "3", "79", "7A"]
        self.assertListEqual(extended_ids, expected_result)

    def test_extend_ids_overlapping_ids(self) -> None:
        """Tests the static extend_ids function of the
        CANFilter class with overlapping ids
        """
        extended_ids = CANFilter.extend_ids(["1-8", "5"])
        expected_result = ["5", "1", "2", "3", "4", "6", "7", "8"]
        self.assertListEqual(extended_ids, expected_result)


if __name__ == "__main__":
    unittest.main()
