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

"""Testing file 'cli/cmd_etl/batetl/cmd/can_filter_helper.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
from pathlib import Path
from unittest.mock import MagicMock, Mock, mock_open, patch

try:
    from cli.cmd_etl.batetl.cmd.can_filter_helper import (
        _sanitize_args,
        can_filter_setup,
        run_filter,
        validate_filter_config,
    )
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[5]))
    from cli.cmd_etl.batetl.cmd.can_filter_helper import (
        _sanitize_args,
        can_filter_setup,
        run_filter,
        validate_filter_config,
    )
    from cli.helpers.misc import PROJECT_BUILD_ROOT


class TestCANFilterHelper(unittest.TestCase):
    """Test definitions for the CANFilter helper"""

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "tmp_file"):
            if cls.tmp_file.is_file():
                cls.tmp_file.unlink()

        return super().tearDownClass()

    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper.CANFilter")
    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper.validate_filter_config")
    def test_can_filter_setup(self, validate_mock: Mock, can_filter_mock: Mock) -> None:
        """test_can_filter_setup mocks all function calls in
        the can_filter_setup function to ensure the correctness
        of the setup procedure.

        :param validate_mock: Mock for validate_filter_config function.
        :param can_filter_mock: Mock for the CANFilter class.
        """
        # Case 1: check whether all functions are called
        # once as intended
        test_config = {"test": True}
        validate_mock.return_value = test_config
        can_filter_setup(test_config)
        validate_mock.assert_called_once_with(test_config)
        can_filter_mock.assert_called_once_with(**test_config)

    def test_validate_filter_config(self) -> None:
        """test_validate_filter_config checks that the validate_filter_config
        validates the configuration correctly, especially wrong
        configuration.
        """
        # Case 1: config as it should be
        test_config = {"id_pos": 2, "ids": ["130", "140"], "sampling": {"130": 2}}
        self.assertIsNone(validate_filter_config(test_config))

        # Case 2: config as it should be without sampling
        test_config = {"id_pos": 2, "ids": ["130", "140"]}
        self.assertIsNone(validate_filter_config(test_config))

        # Case 3: check all possible wrong configurations
        test_cases = [
            {
                "config": {
                    "id_pos": "a",
                    "ids": ["130", "140"],
                    "sampling": {"130": 2},
                },
                "echo": "'id_pos' in the configuration file is not an integer.",
            },
            {
                "config": {"id_pos": 2, "ids": 130, "sampling": {"130": 2}},
                "echo": "'ids' is not a list.",
            },
            {
                "config": {
                    "id_pos": 2,
                    "ids": ["130", 140],
                    "sampling": {"130": 2},
                },
                "echo": "Not all ids are defined as string. Missing quotes ?",
            },
            {
                "config": {
                    "id_pos": 2,
                    "ids": ["130", "14G"],
                    "sampling": {"130": 2},
                },
                "echo": "'ids' are not defined as hexadecimal values!",
            },
            {
                "config": {
                    "id_pos": 2,
                    "ids": ["130", "140-14G"],
                    "sampling": {"130": 2},
                },
                "echo": "'ids' are not defined as hexadecimal values!",
            },
            {
                "config": {"id_pos": 2, "ids": ["130", "140"], "sampling": 2},
                "echo": "'sampling' is not a dictionary.",
            },
            {
                "config": {
                    "id_pos": 2,
                    "ids": ["130", "140"],
                    "sampling": {130: 2},
                },
                "echo": "Not all ids in sampling are defined as string. Missing quotes?",
            },
            {
                "config": {
                    "id_pos": 2,
                    "ids": ["130", "140"],
                    "sampling": {"150": 2},
                },
                "echo": "Defined sampling is not a subset of the ids.",
            },
        ]
        for i in test_cases:
            with self.subTest(f"Case: {i['echo']}"):
                buf = io.StringIO()
                with (
                    redirect_stderr(buf),
                    self.assertRaises(SystemExit) as cm,
                ):
                    validate_filter_config(i["config"])
                # check if key word id_pos is part of the error message
                self.assertTrue(i["echo"] in buf.getvalue())
                self.assertEqual(cm.exception.code, 1)

    def test__sanitize_args_use_std_streams(self):
        """Exit with error message on invalid input file."""
        _err, _out = io.StringIO(), io.StringIO()
        filter_obj = Mock()
        filter_obj.input = None
        filter_obj.output = None
        with redirect_stderr(_err), redirect_stdout(_out):
            _sanitize_args(filter_obj)
        # nothing shall happen

    def test__sanitize_args_output_directory_not_writeable(self):
        """Output directory not writeable."""
        _err, _out = io.StringIO(), io.StringIO()
        filter_obj = Mock()
        filter_obj.input = None
        filter_obj.output = MagicMock(return_value="foo.txt")
        filter_obj.output.parent.mkdir.side_effect = [PermissionError]
        filter_obj.output.parent.resolve.return_value = "./foxbms-2"
        with (
            redirect_stderr(_err),
            redirect_stdout(_out),
            self.assertRaises(SystemExit) as cm,
        ):
            _sanitize_args(filter_obj)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(cm.exception.code, 1)
        self.assertRegex(err, r"Directory '.*[\\\/]foxbms-2' is not writeable.\n")
        self.assertEqual(out, "")

    def test__sanitize_args_output_file_not_writeable(self):
        """Output file not writeable."""
        _err, _out = io.StringIO(), io.StringIO()
        filter_obj = Mock()
        filter_obj.input = None
        filter_obj.output = MagicMock(return_value="foo.txt")
        filter_obj.output.mkdir.return_value = None
        filter_obj.output.touch.side_effect = [PermissionError]
        filter_obj.output.resolve.return_value = "./foxbms-2/foo.txt"
        with (
            redirect_stderr(_err),
            redirect_stdout(_out),
            self.assertRaises(SystemExit) as cm,
        ):
            _sanitize_args(filter_obj)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(cm.exception.code, 1)
        self.assertRegex(err, r"'.*[\\\/]foxbms-2[\\\/]foo.txt' is not writeable.")
        self.assertEqual(out, "")

    def test__sanitize_args_input_file_does_not_exist(self):
        """Input files does not exist."""
        _err, _out = io.StringIO(), io.StringIO()
        filter_obj = Mock()
        filter_obj.input = MagicMock(return_value="foo.txt")
        filter_obj.input.is_file.return_value = False
        filter_obj.input.resolve.return_value = "./foxbms-2/foo.txt"
        filter_obj.output = None
        with (
            redirect_stderr(_err),
            redirect_stdout(_out),
            self.assertRaises(SystemExit) as cm,
        ):
            _sanitize_args(filter_obj)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(cm.exception.code, 1)
        self.assertRegex(err, r"'.*[\\\/]foxbms-2[\\\/]foo.txt' does not exist.\n")
        self.assertEqual(out, "")

    @patch("builtins.open", side_effect=[IOError])
    def test__sanitize_args_input_file_not_readable(self, *_):
        """Input file not readable."""
        _err, _out = io.StringIO(), io.StringIO()
        filter_obj = Mock()
        filter_obj.input = MagicMock(return_value="foo.txt")
        filter_obj.input.is_file.return_value = True
        filter_obj.input.resolve.return_value = "./foxbms-2/foo.txt"
        filter_obj.output = None
        with (
            redirect_stderr(_err),
            redirect_stdout(_out),
            self.assertRaises(SystemExit) as cm,
        ):
            _sanitize_args(filter_obj)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(cm.exception.code, 1)
        self.assertRegex(err, r"'.*[\\\/]foxbms-2[\\\/]foo.txt' is not readable.\n")
        self.assertEqual(out, "")

    # @patch("cli.cmd_etl.batetl.cmd.can_filter_helper.Path.is_file", return_value=True)
    @patch("builtins.open", new_callable=mock_open, read_data="foo")
    def test__sanitize_args_input_file_readable(self, *_):
        """Input file readble."""
        _err, _out = io.StringIO(), io.StringIO()
        filter_obj = Mock()
        filter_obj.input = MagicMock(return_value="foo.txt")
        filter_obj.input.is_file.return_value = True
        filter_obj.output = None
        with redirect_stderr(_err), redirect_stdout(_out):
            _sanitize_args(filter_obj)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(err, "")
        self.assertEqual(out, "")

    @patch("sys.stdout", new_callable=StringIO)
    @patch("sys.stdin", new_callable=StringIO)
    def test_run_filter_success_using_stdin_and_stdout(
        self, stdin: StringIO, stdout: StringIO
    ) -> None:
        """Read from stdin and write to stdout"""
        # case 1/4 in source
        stdin.write("line1\nline2\nline3\n")
        stdin.seek(0)
        filter_obj = Mock()
        filter_obj.input = None
        filter_obj.output = None
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None
        run_filter(filter_obj)
        stdout.seek(0)
        self.assertListEqual(["line1\n", "line3\n"], stdout.readlines())

    @patch("sys.stdout", new_callable=StringIO)
    @patch("sys.stdin", new_callable=StringIO)
    def test_run_filter_failure_using_stdin_and_stdout(
        self, stdin: StringIO, stdout: StringIO
    ) -> None:
        """Read from stdin but can not write to stdout"""
        # case 1/4 in source
        stdin.write("line1\nline2\nline3\n")
        stdin.seek(0)
        filter_obj = Mock()
        filter_obj.input = None
        filter_obj.output = None
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None
        stdout.write = Mock(side_effect=OSError())
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            run_filter(filter_obj)
        self.assertTrue("Could not write to stdout" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    @patch("sys.stdin", new_callable=StringIO)
    def test_run_filter_using_stdin_and_file(self, stdin: StringIO) -> None:
        """Read from stdin and write to file"""
        # case 2/4 in source
        stdin.write("line1\nline2\nline3\n")
        stdin.seek(0)
        filter_obj = Mock()
        filter_obj.input = None
        PROJECT_BUILD_ROOT.mkdir(exist_ok=True, parents=True)
        self.tmp_file = PROJECT_BUILD_ROOT / "foo.log"  # pylint: disable=attribute-defined-outside-init
        self.tmp_file.touch()
        filter_obj.output = self.tmp_file
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None
        run_filter(filter_obj)
        self.assertListEqual(
            ["line1", "line3"],
            self.tmp_file.read_text(encoding="utf-8").strip().splitlines(),
        )

    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper._sanitize_args", return_value=None)
    @patch("sys.stdout", new_callable=StringIO)
    @patch("builtins.open", new_callable=mock_open, read_data="line1\nline2\nline3\n")
    def test_run_filter_using_file_and_stdout(
        self, _, stdout: StringIO, is_file: Mock
    ) -> None:
        """Read from file and write to stdout"""
        # case 3/4 in source
        is_file.return_value = True
        filter_obj = Mock()
        filter_obj.input = Path(".")
        filter_obj.output = None
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None
        run_filter(filter_obj)
        stdout.seek(0)
        self.assertListEqual(["line1\n", "line3\n"], stdout.readlines())

    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper._sanitize_args", return_value=None)
    @patch("sys.stdout", new_callable=StringIO)
    @patch("builtins.open", new_callable=mock_open, read_data="line1\nline2\nline3\n")
    def test_run_filter_using_file_but_writing_to_stdout_fails(
        self, _open, stdout: StringIO, _: Mock
    ) -> None:
        """Read from file and write to stdout"""
        # case 3/4 in source
        filter_obj = Mock()
        filter_obj.input = Path(".")
        filter_obj.output = None
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None

        stdout.write = Mock(side_effect=OSError())
        err = io.StringIO()
        with redirect_stderr(err), self.assertRaises(SystemExit) as cm:
            run_filter(filter_obj)
        self.assertTrue("Could not write to stdout" in err.getvalue())
        self.assertEqual(cm.exception.code, 1)

    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper._sanitize_args", return_value=None)
    @patch("builtins.open")
    def test_run_filter_using_file_and_file(self, m_open: Mock, _: Mock) -> None:
        """Read from file and write to file"""
        # case 4/4 in source
        m_read = mock_open(read_data="line1\nline2\nline3\n").return_value
        m_write = mock_open().return_value
        m_open.side_effect = [m_read, m_write]

        filter_obj = Mock()
        filter_obj._input = Path(".")  # pylint: disable=protected-access
        filter_obj.output = Path(".")
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None

        run_filter(filter_obj)


if __name__ == "__main__":
    unittest.main()
