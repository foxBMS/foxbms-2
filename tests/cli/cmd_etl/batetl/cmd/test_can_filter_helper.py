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
from contextlib import redirect_stderr
from io import StringIO
from pathlib import Path
from unittest.mock import Mock, patch

try:
    from cli.cmd_etl.batetl.cmd.can_filter_helper import (
        can_filter_setup,
        run_filter,
        validate_filter_config,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[5]))
    from cli.cmd_etl.batetl.cmd.can_filter_helper import (
        can_filter_setup,
        run_filter,
        validate_filter_config,
    )


class TestCANFilterHelper(unittest.TestCase):
    """Test definitions for the CANFilter helper"""

    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper.CANFilter")
    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper.validate_filter_config")
    @patch("cli.cmd_etl.batetl.cmd.can_filter_helper.read_config")
    def test_can_filter_setup(
        self, read_config_mock: Mock, validate_mock: Mock, can_filter_mock: Mock
    ) -> None:
        """test_can_filter_setup mocks all function calls in
        the can_filter_setup function to ensure the correctness
        of the setup procedure.

        :param read_config_mock: Mock for the read_config function.
        :param validate_mock: Mock for validate_filter_config function.
        :param can_filter_mock: Mock for the CANFilter class.
        """
        # Case 1: check whether all functions are called
        # once as intended
        test_config = Path("test")
        test_config_dict = {"test": True}
        read_config_mock.return_value = test_config_dict
        validate_mock.return_value = test_config_dict
        can_filter_setup(test_config)
        read_config_mock.assert_called_once_with(test_config)
        validate_mock.assert_called_once_with(test_config_dict)
        can_filter_mock.assert_called_once_with(**test_config_dict)

    def test_validate_filter_config(self) -> None:
        """test_validate_filter_config checks that the validate_filter_config
        validates the configuration correctly, especially wrong
        configuration.
        """
        # Case 1: config_dict as it should be
        test_config_dict = {"id_pos": 2, "ids": ["130", "140"], "sampling": {"130": 2}}
        self.assertIsNone(validate_filter_config(test_config_dict))
        # Case 2: config_dict as it should be without sampling
        test_config_dict = {"id_pos": 2, "ids": ["130", "140"]}
        self.assertIsNone(validate_filter_config(test_config_dict))
        # Case 3: check all possible wrong configurations
        list_of_test_cases = [
            {
                "config_dict": {"ids": ["130", "140"], "sampling": {"130": 2}},
                "echo": "Configuration file is missing 'id_pos' parameter.",
            },
            {
                "config_dict": {
                    "id_pos": "a",
                    "ids": ["130", "140"],
                    "sampling": {"130": 2},
                },
                "echo": "'id_pos' in the configuration file is not an integer.",
            },
            {
                "config_dict": {"id_pos": 2, "sampling": {"130": 2}},
                "echo": "'Configuration file is missing 'ids' parameter'.",
            },
            {
                "config_dict": {"id_pos": 2, "ids": 130, "sampling": {"130": 2}},
                "echo": "'ids' is not a list.",
            },
            {
                "config_dict": {
                    "id_pos": 2,
                    "ids": ["130", 140],
                    "sampling": {"130": 2},
                },
                "echo": "Not all ids are defined as string. Missing quotes ?",
            },
            {
                "config_dict": {
                    "id_pos": 2,
                    "ids": ["130", "14G"],
                    "sampling": {"130": 2},
                },
                "echo": "'ids' are not defined as hexadecimal values!",
            },
            {
                "config_dict": {
                    "id_pos": 2,
                    "ids": ["130", "140-14G"],
                    "sampling": {"130": 2},
                },
                "echo": "'ids' are not defined as hexadecimal values!",
            },
            {
                "config_dict": {"id_pos": 2, "ids": ["130", "140"], "sampling": 2},
                "echo": "'sampling' is not a dictionary.",
            },
            {
                "config_dict": {
                    "id_pos": 2,
                    "ids": ["130", "140"],
                    "sampling": {130: 2},
                },
                "echo": "Not all ids in sampling are defined as string. Missing quotes ?",
            },
            {
                "config_dict": {
                    "id_pos": 2,
                    "ids": ["130", "140"],
                    "sampling": {"150": 2},
                },
                "echo": "Defined sampling is not a subset of the ids.",
            },
        ]
        for i in list_of_test_cases:
            with self.subTest(f"Case: {i['echo']}"):
                buf = io.StringIO()
                with (
                    redirect_stderr(buf),
                    self.assertRaises(SystemExit) as cm,
                ):
                    validate_filter_config(i["config_dict"])
                # check if key word id_pos is part of the error message
                self.assertTrue(i["echo"] in buf.getvalue())
                self.assertEqual(cm.exception.code, 1)

    @patch("sys.stdout", new_callable=StringIO)
    @patch("sys.stdin", new_callable=StringIO)
    def test_run_filter(self, stdin: StringIO, stdout: StringIO) -> None:
        """test_run_filter checks the correctness of the
        filter functionality execution via the command line

        :param stdin: a StringIO object to simulate the
            standard input
        :param stdout: a StringIO object to simulate the
            standard output
        """
        # Case 1: check correct behaviour with text as input
        stdin.write("line1\nline2\nline3\n")
        stdin.seek(0)
        filter_obj = Mock()
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None
        run_filter(filter_obj)
        stdout.seek(0)
        self.assertListEqual(["line1\n", "line3\n"], stdout.readlines())
        # Case 2: Can not write to stdout
        stdin.write("line1\nline2\nline3\n")
        stdin.seek(0)
        filter_obj = Mock()
        filter_obj.filter_msg.side_effect = lambda x: x if "2" not in x else None
        stdout.write = Mock(side_effect=OSError())
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            run_filter(filter_obj)
        self.assertTrue("Could not write to stdout" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)


if __name__ == "__main__":
    unittest.main()
