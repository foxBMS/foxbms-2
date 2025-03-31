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

"""Testing file 'cli/cmd_etl/batetl/__main__.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import Mock, call, patch

from click.testing import CliRunner

try:
    from cli.cmd_etl.batetl import cmd_decode, cmd_filter, cmd_table, main
    from cli.cmd_etl.batetl.__main__ import entry_point
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.batetl import cmd_decode, cmd_filter, cmd_table, main
    from cli.cmd_etl.batetl.__main__ import entry_point


class TestbatetlMain(unittest.TestCase):
    """Test of batetl package entry point and the subcommands"""

    def test_entry_point(self) -> None:
        """Tests the batetl package command line entry point"""
        # Case 1: check help message output
        runner = CliRunner()
        result = runner.invoke(entry_point, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.run_decode2file")
    @patch("cli.cmd_etl.batetl.run_decode2stdout")
    @patch("cli.cmd_etl.batetl.can_decode_setup")
    def test_cmd_decode(
        self, setup_mock: Mock, run_decode2stdout_mock: Mock, run_decode2file_mock: Mock
    ) -> None:
        """Tests the decode command line interface

        :param setup_mock: A mock for the CANDecode class setup
        :param run_decode_mock: A Mock for the run_decode function, which
            executes the decoding
        """
        # Case 1: check execution with stdout as output stream
        decode_obj_mock = Mock(name="decode")
        setup_mock.return_value = decode_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("test.yml", mode="w", encoding="utf-8") as _:
                result = runner.invoke(cmd_decode, ["--config", "test.yml"])
                setup_mock.assert_called_once_with(Path("test.yml"))
                run_decode2stdout_mock.assert_called_once_with(decode_obj_mock)
                run_decode2file_mock.assert_not_called()
                self.assertEqual(result.exit_code, 0)
        setup_mock.reset_mock()
        run_decode2stdout_mock.reset_mock()
        run_decode2file_mock.reset_mock()
        # Case 2: # check execution with files as output
        with runner.isolated_filesystem():
            temp = Path("temp")
            temp.mkdir(parents=True, exist_ok=True)
            with open("test.yml", mode="w", encoding="utf-8") as _:
                result = runner.invoke(
                    cmd_decode, ["--config", "test.yml", "--output", "temp"]
                )
                setup_mock.assert_called_once_with(Path("test.yml"))
                run_decode2file_mock.assert_called_once_with(decode_obj_mock, temp)
                run_decode2stdout_mock.assert_not_called()
                self.assertEqual(result.exit_code, 0)
        # Case 3: check help message output
        result = runner.invoke(cmd_decode, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.run_filter")
    @patch("cli.cmd_etl.batetl.can_filter_setup")
    def test_cmd_filter(self, setup_mock: Mock, run_filter_mock: Mock) -> None:
        """Tests the filter command line interface

        :param setup_mock: A mock for the CANFilter class setup
        :param run_filter_mock: A Mock for the run_filter function, which
            executes the filtering
        """
        # Case 1: check execution as expected
        filter_obj_mock = Mock(name="filter")
        setup_mock.return_value = filter_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("test.txt", mode="w", encoding="utf-8") as _:
                result = runner.invoke(cmd_filter, ["--config", "test.txt"])
                setup_mock.assert_called_once_with(Path("test.txt"))
                run_filter_mock.assert_called_once_with(filter_obj_mock)
                self.assertEqual(result.exit_code, 0)
        # Case 2: check help message output
        result = runner.invoke(cmd_filter, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.run_table")
    @patch("cli.cmd_etl.batetl.table_setup")
    def test_cmd_table(self, setup_mock: Mock, run_table_mock: Mock) -> None:
        """Tests the table command line interface

        :param setup_mock: A mock for the Table class setup
        :param run_table_mock: A Mock for the run_table function, which
            executes the create of a table out of the CAN data
        """
        # Case 1: check execution as expected
        table_obj_mock = Mock(name="table")
        setup_mock.return_value = table_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("test_config.txt", mode="w", encoding="utf-8") as _:
                with open("test_output.txt", mode="w", encoding="utf-8") as _:
                    with open("test_data.txt", mode="w", encoding="utf-8") as _:
                        result = runner.invoke(
                            cmd_table,
                            [
                                "--config",
                                "test_config.txt",
                                "--output",
                                "test_output.txt",
                                "test_data.txt",
                            ],
                        )
                        setup_mock.assert_called_once_with(Path("test_config.txt"))
                        run_table_mock.assert_called_once_with(
                            table_obj_mock,
                            Path("test_data.txt"),
                            Path("test_output.txt"),
                        )
                        self.assertEqual(result.exit_code, 0)
        # Case 2: check help message output
        result = runner.invoke(cmd_table, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.entry_point")
    def test_main(self, entry_point_mock: Mock) -> None:
        """Tests the main function in command line interface

        :param setup_mock: A mock for the entry point function
        """
        entry_point_mock.add_command = Mock()
        main()
        entry_point_mock.add_command.assert_has_calls(
            [call(cmd_filter), call(cmd_decode), call(cmd_table)]
        )


if __name__ == "__main__":
    unittest.main()
