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

import io
import sys
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, Mock, call, patch

import yaml
from click.testing import CliRunner

try:
    from cli.cmd_etl.batetl import (
        DEFAULT_CONFIG_FILE_FILTER,
        cmd_decode,
        cmd_filter,
        cmd_table,
        get_config_file_options,
        main,
    )
    from cli.cmd_etl.batetl.__main__ import entry_point
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.batetl import (
        DEFAULT_CONFIG_FILE_FILTER,
        cmd_decode,
        cmd_filter,
        cmd_table,
        get_config_file_options,
        main,
    )
    from cli.cmd_etl.batetl.__main__ import entry_point


class TestBatetlMain(unittest.TestCase):
    """Test batetl package entry point"""

    def test_entry_point(self) -> None:
        """check help message output of the main command"""
        runner = CliRunner()
        result = runner.invoke(entry_point, [])
        self.assertTrue("Usage" in result.output)


@patch("cli.cmd_etl.batetl.read_config")
class TestBatetlGetConfigFileOptions(unittest.TestCase):
    """Test batetl configuration parser"""

    def test_get_config_file_options_no_file(self, _: MagicMock) -> None:
        """Test when the configuration file does not exist."""
        ctx = MagicMock()
        config_file = MagicMock()
        config_file.is_file.return_value = False
        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertDictEqual(ctx.default_map, {})

    def test_get_config_file_options_file_exists_no_sampling(
        self, m_read_config: MagicMock
    ) -> None:
        """Test when the configuration file exists."""
        ctx = MagicMock()
        config_file = MagicMock()
        config_file.is_file.return_value = True
        m_read_config.return_value = {"foo": "bar"}

        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertDictEqual(ctx.default_map, {"foo": "bar"})

    def test_get_config_file_options_file_exists_sampling(
        self, m_read_config: MagicMock
    ) -> None:
        """Test when the configuration file exists and sampling is provided."""
        ctx = MagicMock()
        config_file = MagicMock()
        config_file.is_file.return_value = True
        m_read_config.return_value = {"foo": "bar", "sampling": {"a": "b"}}

        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertDictEqual(
            ctx.default_map, {"foo": "bar", "sampling": {"a": "b"}.items()}
        )

    @patch("cli.cmd_etl.batetl.Path.is_file")
    def test_get_config_file_options_default_file(
        self, m_is_file: MagicMock, m_read_config: MagicMock
    ) -> None:
        """Test when the configuration file exists."""
        ctx = MagicMock()
        config_file = DEFAULT_CONFIG_FILE_FILTER
        m_is_file.return_value = True
        m_read_config.return_value = {"foo": "bar"}
        buf = io.StringIO()
        with redirect_stdout(buf):
            get_config_file_options(ctx, MagicMock(), config_file)
        self.assertIn("Default configuration file filter.yml is used.", buf.getvalue())


class TestBatetlDecode(unittest.TestCase):
    """Test 'decode' command"""

    def test_cmd_decode_help(self) -> None:
        "Test help option"
        runner = CliRunner()
        result = runner.invoke(cmd_decode, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.run_decode2file")
    @patch("cli.cmd_etl.batetl.run_decode2stdout")
    @patch("cli.cmd_etl.batetl.can_decode_setup")
    def test_cmd_decode_to_stdout(
        self,
        m_can_decode_setup: Mock,
        run_decode2stdout_mock: Mock,
        run_decode2file_mock: Mock,
    ) -> None:
        """Tests the decode command line interface

        :param setup_mock: A mock for the CANDecode class setup
        :param run_decode_mock: A Mock for the run_decode function, which
            executes the decoding
        """
        # Case 1: check execution with stdout as output stream
        decode_obj_mock = Mock(name="decode")
        m_can_decode_setup.return_value = decode_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            p = Path("test.yml")

            p.write_text(
                yaml.dump(
                    {
                        "dbc": str(Path(__file__)),
                        "timestamp_pos": 0,
                        "id_pos": 1,
                        "data_pos": 2,
                    }
                ),
                encoding="utf-8",
            )
            result = runner.invoke(cmd_decode, ["--config", str(p.resolve())])
            m_can_decode_setup.assert_called_once_with(
                {
                    "dbc": Path(__file__),
                    "timestamp_pos": 0,
                    "id_pos": 1,
                    "data_pos": 2,
                    "output": None,
                }
            )
            run_decode2stdout_mock.assert_called_once_with(decode_obj_mock)
            run_decode2file_mock.assert_not_called()
            self.assertEqual(result.exit_code, 0)
        m_can_decode_setup.reset_mock()
        run_decode2stdout_mock.reset_mock()
        run_decode2file_mock.reset_mock()

    @patch("cli.cmd_etl.batetl.run_decode2file")
    @patch("cli.cmd_etl.batetl.run_decode2stdout")
    @patch("cli.cmd_etl.batetl.can_decode_setup")
    def test_cmd_decode_to_files(
        self,
        m_can_decode_setup: Mock,
        run_decode2stdout_mock: Mock,
        run_decode2file_mock: Mock,
    ) -> None:
        """Tests the decode command line interface

        :param setup_mock: A mock for the CANDecode class setup
        :param run_decode_mock: A Mock for the run_decode function, which
            executes the decoding
        """
        # Case 1: check execution with stdout as output stream
        decode_obj_mock = Mock(name="decode")
        m_can_decode_setup.return_value = decode_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            p = Path("test.yml")

            p.write_text(
                yaml.dump(
                    {
                        "dbc": str(Path(__file__)),
                        "timestamp_pos": 0,
                        "id_pos": 1,
                        "data_pos": 2,
                        "output": str(Path(__file__).parent),
                    }
                ),
                encoding="utf-8",
            )
            result = runner.invoke(cmd_decode, ["--config", str(p.resolve())])
            m_can_decode_setup.assert_called_once_with(
                {
                    "dbc": Path(__file__),
                    "timestamp_pos": 0,
                    "id_pos": 1,
                    "data_pos": 2,
                    "output": Path(__file__).parent,
                }
            )
            run_decode2stdout_mock.assert_not_called()
            run_decode2file_mock.assert_called_once_with(decode_obj_mock)
            self.assertEqual(result.exit_code, 0)
        m_can_decode_setup.reset_mock()
        run_decode2stdout_mock.reset_mock()
        run_decode2file_mock.reset_mock()


class TestBatetlFilter(unittest.TestCase):
    """Test 'filter' command"""

    def test_cmd_filter_help(self) -> None:
        "Test help option"
        runner = CliRunner()
        result = runner.invoke(cmd_filter, ["-c", "test"])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.run_filter")
    @patch("cli.cmd_etl.batetl.can_filter_setup")
    def test_cmd_filter(self, setup_mock: Mock, run_filter_mock: Mock) -> None:
        """Tests the filter command line interface

        :param setup_mock: A mock for the CANFilter class setup
        :param run_filter_mock: A Mock for the run_filter function, which
            executes the filtering
        """
        filter_obj_mock = Mock(name="filter")
        setup_mock.return_value = filter_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("filter.yml", mode="w", encoding="utf-8") as f:
                f.write('id_pos: 4\nids: ["0241", "035C"]')
            result = runner.invoke(cmd_filter, ["--config", "filter.yml"])
            setup_mock.assert_called_once_with(
                {
                    "id_pos": 4,
                    "ids": ["0241", "035C"],
                    "sampling": {},
                    "_input": None,
                    "output": None,
                }
            )
            run_filter_mock.assert_called_once_with(filter_obj_mock)
            self.assertEqual(result.exit_code, 0)

    @patch("cli.cmd_etl.batetl.run_filter")
    @patch("cli.cmd_etl.batetl.can_filter_setup")
    def test_cmd_filter_using_sampling(
        self, setup_mock: Mock, run_filter_mock: Mock
    ) -> None:
        """Tests the filter command line interface

        :param setup_mock: A mock for the CANFilter class setup
        :param run_filter_mock: A Mock for the run_filter function, which
            executes the filtering
        """
        filter_obj_mock = Mock(name="filter")
        setup_mock.return_value = filter_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("filter.yml", mode="w", encoding="utf-8") as f:
                f.write('id_pos: 4\nids: ["0241", "035C"]\n"sampling":\n  "240": 10')
            result = runner.invoke(cmd_filter, ["--config", "filter.yml"])
            setup_mock.assert_called_once_with(
                {
                    "id_pos": 4,
                    "ids": ["0241", "035C"],
                    "sampling": {"240": 10},
                    "_input": None,
                    "output": None,
                }
            )
            run_filter_mock.assert_called_once_with(filter_obj_mock)
            self.assertEqual(result.exit_code, 0)


class TestBatetlTable(unittest.TestCase):
    """Test 'table' command"""

    def test_cmd_table_help(self) -> None:
        "Test help option for the 'table' command when no argument or option are provided"
        runner = CliRunner()
        result = runner.invoke(cmd_table, [])
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


class TestBatetlMisc(unittest.TestCase):
    """Test the wrapper for the cli integration"""

    @patch("cli.cmd_etl.batetl.entry_point")
    def test_main(self, m_entry_point: MagicMock) -> None:
        """Tests the main function in command line interface

        :param setup_mock: A mock for the entry point function
        """
        m_entry_point.add_command = MagicMock()
        main()
        m_entry_point.add_command.assert_has_calls(
            [call(cmd_filter), call(cmd_decode), call(cmd_table)]
        )


if __name__ == "__main__":
    unittest.main()
