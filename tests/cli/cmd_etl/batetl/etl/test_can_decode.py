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

"""Testing file 'cli/cmd_etl/batetl/etl/can_decode.py'."""

import logging
import os
import sys
import unittest
from pathlib import Path
from tempfile import mkstemp
from unittest.mock import Mock

try:
    from cli.cmd_etl.batetl.cmd.can_decode_helper import get_cantools_database
    from cli.cmd_etl.batetl.etl.can_decode import CANDecode
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[5]))
    from cli.cmd_etl.batetl.cmd.can_decode_helper import get_cantools_database
    from cli.cmd_etl.batetl.etl.can_decode import CANDecode


class TestCANDecode(unittest.TestCase):
    """Test cases for the CANDecode class"""

    def test_candecode_init(self) -> None:
        """Tests the creation of a CANDecode object"""
        # Case 1: Create CANDecode object with valid config
        test_obj = CANDecode(None, 0, 2, 6)
        self.assertEqual(test_obj._database, None)  # pylint: disable=W0212
        self.assertEqual(test_obj._timestamp_pos, 0)  # pylint: disable=W0212
        self.assertEqual(test_obj._id_pos, 2)  # pylint: disable=W0212
        self.assertEqual(test_obj._data_pos, 6)  # pylint: disable=W0212

    def test_decode_msg(self) -> None:
        """Tests the decode_msg method from the CANDecode class,
        especially the exception handling"""
        # Case 1: Test execute _handle_decoding
        tf, temp_path = mkstemp()
        with open(tf, mode="w", encoding="utf-8") as f:
            f.write("BO_ 860 CurrentSensor_Current: 6 Vector__XXX\n")
            f.write(
                "SG_ CurrentSensor_SIG_Current : 23|32@0- (1,0)"
                ' [-2147483648|2147483647] "mA" Vector__XXX\n'
            )
        dbc = get_cantools_database(temp_path)
        test_obj = CANDecode(dbc=dbc, timestamp_pos=0, id_pos=2, data_pos=6)
        test_obj._handle_decoding = Mock(return_value="test")  # pylint: disable=W0212
        self.assertEqual("test", test_obj.decode_msg("test"))
        test_obj = CANDecode(dbc=dbc, timestamp_pos=0, id_pos=2, data_pos=6)
        list_of_test_cases = [
            {"msg": "test", "logging": "Index error at msg: test"},
            {
                "msg": "0.1 2 test 2 3 5 6",
                "logging": "CAN id or data in msg 0.1 2 test 2 3 5 6 contain"
                " non-hexadecimal values. Wrong id-pos or data-pos ?",
            },
            {
                "msg": "925.201998 8  12A        Rx D 6  00  04  FF  FF  F4  A4",
                "logging": "CAN id in msg 925.201998 8  12A   "
                "     Rx D 6  00  04  FF  FF  F4  A4 is not "
                "listed in the DBC file",
            },
            {
                "msg": "925.201998 8  35C        Rx D 6  00",
                "logging": "CAN msg 925.201998 8  35C        Rx D 6  00 can "
                "not be decoded",
            },
        ]
        for i in list_of_test_cases:
            with self.subTest(f"Case: {i['logging']}"):
                with self.assertLogs(level=logging.INFO) as al:
                    msg_name, decoded_msg = test_obj.decode_msg(i["msg"])
                # check if key word id_pos is part of the error message
                self.assertTrue(i["logging"] in al.output[0])
                self.assertEqual(msg_name, None)
                self.assertEqual(decoded_msg, None)
        os.remove(temp_path)

    def test_handle_decoding(self) -> None:
        """Tests the handle_decoding method of the CANDecode class"""
        # Case 1: non-mulitplexed message
        tf, temp_path = mkstemp()
        with open(tf, mode="w", encoding="utf-8") as f:
            f.write("BO_ 860 CurrentSensor_Current: 6 Vector__XXX\n")
            f.write(
                "SG_ CurrentSensor_SIG_Current : 23|32@0- (1,0)"
                ' [-2147483648|2147483647] "mA" Vector__XXX\n'
            )
        dbc = get_cantools_database(temp_path)
        test_obj = CANDecode(dbc=dbc, timestamp_pos=0, id_pos=2, data_pos=6)
        can_msg = "925.201998 8  35C        Rx D 6  00  04  FF  FF  F4  A4"
        msg_name, decoded_msg = test_obj._handle_decoding(can_msg)  # pylint: disable=protected-access
        self.assertEqual(msg_name, "CurrentSensor_Current")
        self.assertEqual(
            decoded_msg,
            '{"Timestamp": 925.201998,"860_CurrentSensor_SIG_Current_mA":-2908}\n',
        )
        os.remove(temp_path)
        # Case 2: mulitplexed message
        tf, temp_path = mkstemp()
        with open(tf, mode="w", encoding="utf-8") as f:
            f.write("BO_ 577 f_StringMinimumMaximumValues: 8 Vector__XXX\n")
            f.write(
                "SG_ f_StringMinimumMaximumValues_Mux M : 3|4@0+ "
                '(1,0) [0|0] "" Vector__XXX\n'
            )
            f.write(
                "SG_ MinimumCellVoltage m0 : 17|14@0- (1,0) [-8192|8191]"
                ' "mV" Vector__XXX\n'
            )
            f.write(
                "SG_ MaximumCellVoltage m0 : 15|14@0- (1,0) [-8192|8191]"
                ' "mV" Vector__XXX\n'
            )
            f.write(
                "SG_ MaximumCellTemperature m0 : 35|9@0- (0.5,0) "
                '[-128|127.5] "degC" Vector__XXX\n'
            )
            f.write(
                "SG_ MinimumCellTemperature m0 : 42|9@0- (0.5,0) "
                '[-128|127.5] "degC" Vector__XXX\n'
            )
        dbc = get_cantools_database(temp_path)
        test_obj = CANDecode(dbc=dbc, timestamp_pos=0, id_pos=2, data_pos=6)
        can_msg = "925.216222 8  241        Rx D 8  00  07  F2  3F  8D  FC  8F  E4"
        msg_name, decoded_msg = test_obj._handle_decoding(can_msg)  # pylint: disable=protected-access
        expected_decoded_msg = (
            '{"Timestamp": 925.216222,"577_MaximumCellVoltage_mV":'
            '508,"577_MinimumCellVoltage_mV":-7176,"577_MaximumCellTemperature_degC"'
            ':-32.5,"577_MinimumCellTemperature_degC":-110.5}\n'
        )
        self.assertEqual(msg_name, "f_StringMinimumMaximumValues")
        self.assertEqual(decoded_msg, expected_decoded_msg)
        os.remove(temp_path)
        # Case 3: two CAN message different CAN id
        tf, temp_path = mkstemp()
        with open(tf, mode="w", encoding="utf-8") as f:
            f.write("BO_ 860 CurrentSensor_Current1: 6 Vector__XXX\n")
            f.write(
                "SG_ CurrentSensor_SIG_Current : 23|32@0- (1,0)"
                ' [-2147483648|2147483647] "mA" Vector__XXX\n'
            )
            f.write("BO_ 830 CurrentSensor_Current2: 6 Vector__XXX\n")
            f.write(
                "SG_ CurrentSensor_SIG_Current : 23|32@0- (1,0)"
                ' [-2147483648|2147483647] "mA" Vector__XXX\n'
            )
        dbc = get_cantools_database(temp_path)
        test_obj = CANDecode(dbc=dbc, timestamp_pos=0, id_pos=2, data_pos=6)
        can_msg = "925.201998 8  33E        Rx D 6  00  04  FF  FF  F4  A4"
        msg_name, decoded_msg = test_obj._handle_decoding(can_msg)  # pylint: disable=protected-access
        self.assertEqual(msg_name, "CurrentSensor_Current2")
        self.assertEqual(
            decoded_msg,
            '{"Timestamp": 925.201998,"830_CurrentSensor_SIG_Current_mA":-2908}\n',
        )
        os.remove(temp_path)

    def test_data_format(self) -> None:
        """Tests the data_format method of the CANDecode class"""
        # Case 1: Decoded data with unit
        signal_mock = Mock()
        signal_mock.name = "test"
        signal_mock.unit = "mA"
        signals = [signal_mock, signal_mock]
        msg_id = "0"
        decoded_data = {"test": 20}
        data_format = CANDecode._data_format(msg_id, signals, decoded_data)  # pylint: disable=protected-access
        expected_data_format = '"0_test_mA":20,"0_test_mA":20'
        self.assertEqual(data_format, expected_data_format)
        # Case 2: Decoded data without unit
        signal_mock = Mock()
        signal_mock.name = "test"
        signal_mock.unit = None
        signals = [signal_mock, signal_mock]
        msg_id = "0"
        decoded_data = {"test": "20"}
        data_format = CANDecode._data_format(msg_id, signals, decoded_data)  # pylint: disable=protected-access
        expected_data_format = '"0_test_None":"20","0_test_None":"20"'
        self.assertEqual(data_format, expected_data_format)


if __name__ == "__main__":
    unittest.main()
