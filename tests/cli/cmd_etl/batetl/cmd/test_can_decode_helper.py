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

"""Testing file 'cli/cmd_etl/batetl/cmd/can_decode_helper.py'."""

import io
import os
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
from pathlib import Path
from tempfile import mkdtemp, mkstemp
from unittest.mock import Mock, call, mock_open, patch

try:
    from cli.cmd_etl.batetl.cmd.can_decode_helper import (
        can_decode_setup,
        get_cantools_database,
        run_decode2file,
        run_decode2stdout,
        validate_decode_config,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[5]))
    from cli.cmd_etl.batetl.cmd.can_decode_helper import (
        can_decode_setup,
        get_cantools_database,
        run_decode2file,
        run_decode2stdout,
        validate_decode_config,
    )


class TestCANDecodeHelper(unittest.TestCase):
    """Tests the helper functions for the decode command"""

    @patch("cli.cmd_etl.batetl.cmd.can_decode_helper.get_cantools_database")
    @patch("cli.cmd_etl.batetl.cmd.can_decode_helper.CANDecode")
    @patch("cli.cmd_etl.batetl.cmd.can_decode_helper.validate_decode_config")
    def test_can_decode_setup(
        self,
        validate_mock: Mock,
        can_decode_mock: Mock,
        db_mock: Mock,
    ) -> None:
        """test_can_decode_setup mocks all function calls in
        the can_decode_setup function to ensure the correctness
        of the setup procedure.

        :param validate_mock: Mock for validate_decode_config function.
        :param CANDecode_mock: Mock for the CANDecode class.
        """
        # Case 1: check whether all functions are called
        # once as intended
        test_config = {"dbc": Path("test.dbc")}
        db_mock.return_value = Path("test.dbc")
        can_decode_setup(test_config)
        validate_mock.assert_called_once_with(test_config)
        can_decode_mock.assert_called_once_with(**test_config)

    @patch("cantools.database.load_file", return_value=4)
    def test_get_cantools_database_invalid_dbc(self, _: Mock) -> None:
        """Test exit on invalid dbc file."""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            get_cantools_database(Path("dummy.dbc"))
        self.assertEqual("Not a 'Database'.\n", buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_get_cantools_database(self) -> None:
        """Tests the get_cantools_database, which a passed .dbc file and returns
        a cantools database object"""
        # Case 1: Valid dbc file
        tf, temp_path = mkstemp()
        with open(tf, mode="w", encoding="utf-8") as f:
            f.write("BO_ 860 CurrentSensor_Current: 6 Vector__XXX\n")
            f.write(
                "SG_ CurrentSensor_SIG_Current : 23|32@0- (1,0)"
                ' [-2147483648|2147483647] "mA" Vector__XXX\n'
            )
        can_db = get_cantools_database(Path(temp_path))
        self.assertEqual(
            "CurrentSensor_SIG_Current", can_db.messages[0].signals[0].name
        )
        os.remove(temp_path)

        # Case 2: invalid dbc file
        tf, temp_path = mkstemp()
        with open(tf, mode="w", encoding="utf-8") as f:
            f.write("Invalid DBC")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            get_cantools_database(Path(temp_path))
        self.assertEqual(buf.getvalue(), "Invalid DBC file.\n")
        self.assertEqual(cm.exception.code, 1)
        os.remove(temp_path)

        # Case 3: dbc file not found
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            get_cantools_database(Path("wrong_dbc_path"))
        self.assertEqual(buf.getvalue(), "DBC file not found.\n")
        self.assertEqual(cm.exception.code, 1)

    def test_validate_decode_config_invalid_dbc(self) -> None:
        """Check that invalid 'dbc' configurations are catched."""
        err = io.StringIO()
        with redirect_stderr(err), self.assertRaises(SystemExit) as cm:
            validate_decode_config({"dbc": None})
        self.assertEqual(err.getvalue(), "'dbc' configuration is not a Path.\n")
        self.assertEqual(cm.exception.code, 1)

    def test_validate_decode_config(self) -> None:
        """test_validate_decode_config checks the validate_decode_config function"""
        # Case 1: config as it should be
        test_config = {
            "dbc": Path("test.dbc"),
            "timestamp_pos": 0,
            "id_pos": 3,
            "data_pos": 5,
        }
        self.assertIsNone(validate_decode_config(test_config))

        # Case 2: check all possible wrong configurations
        test_cases = [
            {
                "config": {
                    "dbc": Path("test.dbc"),
                    "timestamp_pos": "test",
                    "id_pos": 3,
                    "data_pos": 5,
                },
                "echo": "'timestamp_pos' is not an integer.",
            },
            {
                "config": {
                    "dbc": Path("test.dbc"),
                    "timestamp_pos": 0,
                    "id_pos": "test",
                    "data_pos": 5,
                },
                "echo": "'id_pos' is not an integer.",
            },
            {
                "config": {
                    "dbc": Path("test.dbc"),
                    "timestamp_pos": 0,
                    "id_pos": 3,
                    "data_pos": "test",
                },
                "echo": "'data_pos' is not an integer.",
            },
        ]
        for i in test_cases:
            with self.subTest(f"Case: {i['echo']}"):
                buf = io.StringIO()
                with (
                    redirect_stderr(buf),
                    self.assertRaises(SystemExit) as cm,
                ):
                    validate_decode_config(i["config"])
                # check if key word id_pos is part of the error message
                self.assertTrue(i["echo"] in buf.getvalue())
                self.assertEqual(cm.exception.code, 1)

    @patch("sys.stdout", new_callable=StringIO)
    @patch("sys.stdin", new_callable=StringIO)
    def test_run_decode2stdout(self, stdin: StringIO, stdout: StringIO) -> None:
        """test_run_decode2stdout checks the correctness of the
        decode functionality execution with stdout as output

        :param stdin: A StringIO object to simulate the
            standard input
        :param stdout: A StringIO object to simulate the
            standard output
        """
        # Case 1: check correct behaviour with text as input
        stdin.write("test message")
        stdin.seek(0)
        decode_obj = Mock()
        decode_obj.decode_msg.return_value = "test id", "decoded test message\n"
        run_decode2stdout(decode_obj)
        decode_obj.decode_msg.assert_called_with("test message")
        stdout.seek(0)
        self.assertEqual("decoded test message\n", stdout.read())
        # Delete stream content
        stdin.truncate(0)
        stdin.seek(0)
        stdout.truncate(0)
        stdout.seek(0)
        # Case 2: multiple incoming messages
        stdin.write("test message\ntest message\ntest message\n")
        stdin.seek(0)
        run_decode2stdout(decode_obj)
        stdout.seek(0)
        self.assertListEqual(
            [
                "decoded test message\n",
                "decoded test message\n",
                "decoded test message\n",
            ],
            stdout.readlines(),
        )
        # Delete stream content
        stdin.truncate(0)
        stdin.seek(0)
        stdout.truncate(0)
        stdout.seek(0)
        # Case 3: empty decoded message
        stdin.write("test message")
        stdin.seek(0)
        decode_obj.decode_msg.return_value = None, "test"
        stdout.seek(0)
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            run_decode2stdout(decode_obj)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(err, "No CAN message was decoded. Check configuration file.\n")
        self.assertEqual(out, "")

    def test_run_decode2file_invalid_output_directory(self):
        """Check that not the provided object has required 'output_directory'
        member."""
        err = io.StringIO()
        with redirect_stderr(err), self.assertRaises(SystemExit) as cm:
            run_decode2file(Mock())
        self.assertEqual(err.getvalue(), "Provided output directory is invalid.\n")
        self.assertEqual(cm.exception.code, 1)

    @patch("builtins.open", new_callable=mock_open)
    @patch("sys.stdin", new_callable=StringIO)
    def test_run_decode2file(self, stdin: StringIO, file_open_mock: Mock) -> None:
        """test_run_decode2file checks the correctness of the
        decode functionality execution with files as output

        :param stdin: A StringIO object to simulate the
            standard input
        :param file_open_mock: A Mock object for the Python builtin open function
        """
        # Case 1: check correct create of a file based on the return decoded data
        temp_dir = mkdtemp()
        stdin.write("test message")
        stdin.seek(0)
        decode_obj = Mock()
        decode_obj.decode_msg.return_value = "test", "decoded test message"
        decode_obj.output_directory = Path(temp_dir)
        run_decode2file(decode_obj)
        decode_obj.decode_msg.assert_called_with("test message")
        file_path = Path(temp_dir) / "test.json"
        file_open_mock.assert_called_once_with(file_path, mode="w", encoding="utf-8")
        file_handle = file_open_mock()
        file_handle.write.assert_called_once_with("decoded test message")
        file_handle.close.assert_called()
        # Delete stream content
        stdin.truncate(0)
        stdin.seek(0)

        # Case 2: check multiple messages and the creation of multiple files
        stdin.write("test message\ntest message\ntest message\n")
        stdin.seek(0)
        decode_obj = Mock()
        decode_obj.decode_msg.side_effect = [
            ("test1", "decoded test message1"),
            ("test2", "decoded test message2"),
            ("test3", "decoded test message3"),
        ]
        decode_obj.output_directory = Path(temp_dir)
        file_open_mock.reset_mock()
        run_decode2file(decode_obj)
        self.assertEqual(
            decode_obj.decode_msg.call_args_list,
            [call("test message\n"), call("test message\n"), call("test message\n")],
        )
        calls = file_open_mock.call_args_list
        file_path = Path(temp_dir) / "test1.json"
        self.assertEqual(calls[0], call(file_path, mode="w", encoding="utf-8"))
        file_path = Path(temp_dir) / "test2.json"
        self.assertEqual(calls[1], call(file_path, mode="w", encoding="utf-8"))
        file_path = Path(temp_dir) / "test3.json"
        self.assertEqual(calls[2], call(file_path, mode="w", encoding="utf-8"))
        # Delete stream content
        stdin.truncate(0)
        stdin.seek(0)

        # Case 3: Write output with same msg_name to same file
        stdin.write("test message\ntest message\n")
        stdin.seek(0)
        decode_obj = Mock()
        decode_obj.decode_msg.side_effect = [
            ("test1", "decoded test message1"),
            ("test1", "decoded test message2"),
        ]
        decode_obj.output_directory = Path(temp_dir)
        run_decode2file(decode_obj)
        file_path = Path(temp_dir) / "test1.json"
        self.assertEqual(
            call().write("decoded test message1"), file_open_mock.mock_calls[-3]
        )
        self.assertEqual(
            call().write("decoded test message2"), file_open_mock.mock_calls[-2]
        )
        self.assertEqual(call().close(), file_open_mock.mock_calls[-1])
        # Delete stream content
        stdin.truncate(0)
        stdin.seek(0)

        # Case 4: Invalid CAN message
        stdin.write("test message")
        stdin.seek(0)
        decode_obj = Mock()
        decode_obj.decode_msg.return_value = None, None
        file_open_mock.reset_mock()
        decode_obj.output_directory = Path(temp_dir)
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            run_decode2file(decode_obj)
        err, out = _err.getvalue(), _out.getvalue()
        file_open_mock.assert_not_called()
        file_handle = file_open_mock()
        file_handle.write.assert_not_called()
        file_handle.close.assert_not_called()
        self.assertEqual(err, "No CAN message was decoded. Check configuration file.\n")
        self.assertEqual(out, "")
        os.rmdir(temp_dir)


if __name__ == "__main__":
    unittest.main()
