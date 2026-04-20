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

"""Testing file 'cli/commands/c_etl.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

import yaml
from click.testing import CliRunner

try:
    from cli.cli import main
    from cli.cmd_etl.etl import OutputFormats
    from cli.cmd_etl.etl.convert import InputFormats
    from cli.commands.c_etl import (
        cmd_convert,
        cmd_convert_gamry,
        cmd_convert_graphtec,
        cmd_decode,
        cmd_filter,
        cmd_table,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cli import main
    from cli.cmd_etl.etl import OutputFormats
    from cli.cmd_etl.etl.convert import InputFormats
    from cli.commands.c_etl import (
        cmd_convert,
        cmd_convert_gamry,
        cmd_convert_graphtec,
        cmd_decode,
        cmd_filter,
        cmd_table,
    )


class TestConvertGroup(unittest.TestCase):
    """Tests for the 'convert' group."""

    def test_convert_help(self) -> None:
        """Ensure help output is shown when calling '--help'."""
        runner = CliRunner()
        result = runner.invoke(cmd_convert, ["--help"])
        self.assertEqual(result.exit_code, 0)
        self.assertIn("Usage", result.output)


class TestCmdConvertGamry(unittest.TestCase):
    """Tests for the 'cmd_convert_gamry' command."""

    def test_cmd_convert_gamry_help(self) -> None:
        """Ensure help output is shown when calling '--help' on the GAMRY subcommand."""
        runner = CliRunner()
        result = runner.invoke(cmd_convert_gamry, ["--help"])
        self.assertEqual(result.exit_code, 0)
        self.assertIn("Usage", result.output)

    @patch("cli.commands.c_etl.run_converter")
    @patch("cli.commands.c_etl.converter_setup")
    def test_cmd_convert_dta_defaults_csv(
        self, converter_setup_mock: Mock, run_converter_mock: Mock
    ) -> None:
        """Verify default behavior: output-format=CSV, recursive=False, skip_footer=0."""
        converter_obj_mock = Mock(name="converter")
        converter_setup_mock.return_value = converter_obj_mock

        runner = CliRunner()
        with runner.isolated_filesystem():
            data_file = Path("data.dta")
            data_file.write_text("", encoding="utf-8")

            # Call the group with defaults and subcommand with data only
            result = runner.invoke(cmd_convert, ["gamry", str(data_file)])

            self.assertEqual(result.exit_code, 0)
            converter_setup_mock.assert_called_once()
            config_arg = (
                converter_setup_mock.call_args.kwargs
                or converter_setup_mock.call_args.args[0]
            )
            self.assertEqual(config_arg["data_path"], Path("data.dta"))
            self.assertFalse(config_arg["recursive"])
            self.assertEqual(
                config_arg["conversion"]["input_format"], InputFormats.GAMRY
            )
            self.assertEqual(
                config_arg["conversion"]["output_format"], OutputFormats.CSV
            )
            self.assertEqual(config_arg["conversion"]["additional"], {"skip_footer": 0})
            run_converter_mock.assert_called_once_with(converter_obj_mock)

    @patch("cli.commands.c_etl.run_converter")
    @patch("cli.commands.c_etl.converter_setup")
    def test_cmd_convert_dta_with_skip_footer_and_parquet(
        self, converter_setup_mock: Mock, run_converter_mock: Mock
    ) -> None:
        """Verify that '-s/--skip_footer' and '--output-format PARQUET' are forwarded correctly."""
        converter_obj_mock = Mock(name="converter")
        converter_setup_mock.return_value = converter_obj_mock

        runner = CliRunner()
        with runner.isolated_filesystem():
            data_file = Path("measurements.dta")
            data_file.write_text("", encoding="utf-8")

            # Provide group output-format and subcommand skip_footer
            result = runner.invoke(
                cmd_convert,
                [
                    "--output-format",
                    "PARQUET",
                    "gamry",
                    str(data_file),
                    "--skip_footer",
                    "7",
                ],
            )

            self.assertEqual(result.exit_code, 0)
            converter_setup_mock.assert_called_once()
            config_arg = (
                converter_setup_mock.call_args.kwargs
                or converter_setup_mock.call_args.args[0]
            )
            self.assertEqual(config_arg["data_path"], Path("measurements.dta"))
            self.assertFalse(config_arg["recursive"])
            self.assertEqual(
                config_arg["conversion"]["input_format"], InputFormats.GAMRY
            )
            self.assertEqual(
                config_arg["conversion"]["output_format"], OutputFormats.PARQUET
            )
            self.assertEqual(config_arg["conversion"]["additional"], {"skip_footer": 7})
            run_converter_mock.assert_called_once_with(converter_obj_mock)


class TestCmdConvertGraphtec(unittest.TestCase):
    """Tests for the 'cmd_convert_graphtec' command."""

    def test_cmd_convert_graphtec_help(self) -> None:
        """Ensure help output is shown when calling '--help' on the GRAPHTEC subcommand."""
        runner = CliRunner()
        result = runner.invoke(cmd_convert_graphtec, ["--help"])
        self.assertEqual(result.exit_code, 0)
        self.assertIn("Usage", result.output)

    @patch("cli.commands.c_etl.run_converter")
    @patch("cli.commands.c_etl.converter_setup")
    def test_cmd_convert_graphtec(
        self, converter_setup_mock: Mock, run_converter_mock: Mock
    ) -> None:
        """Verify that convert graphtec works as intended."""
        converter_obj_mock = Mock(name="converter")
        converter_setup_mock.return_value = converter_obj_mock

        runner = CliRunner()
        with runner.isolated_filesystem():
            data_file = Path("measurements.csv")
            data_file.write_text("", encoding="utf-8")

            result = runner.invoke(
                cmd_convert,
                ["--output-format", "parquet", "graphtec", str(data_file)],
            )

            self.assertEqual(result.exit_code, 0)
            converter_setup_mock.assert_called_once()
            config_arg = (
                converter_setup_mock.call_args.kwargs
                or converter_setup_mock.call_args.args[0]
            )
            self.assertEqual(config_arg["data_path"], Path("measurements.csv"))
            self.assertFalse(config_arg["recursive"])
            self.assertEqual(
                config_arg["conversion"]["input_format"], InputFormats.GRAPHTEC
            )
            self.assertEqual(
                config_arg["conversion"]["output_format"], OutputFormats.PARQUET
            )
            self.assertEqual(config_arg["conversion"]["additional"], {"skip": 32})
            run_converter_mock.assert_called_once_with(converter_obj_mock)


class TestDecode(unittest.TestCase):
    """Test 'decode' command"""

    def test_cmd_decode_help(self) -> None:
        """Test help option

        Note: If you try to use this test locally, make sure no default
        config file is exists in your repository root.
        """
        runner = CliRunner()
        result = runner.invoke(cmd_decode, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.commands.c_etl.run_decode2file")
    @patch("cli.commands.c_etl.run_decode2stdout")
    @patch("cli.commands.c_etl.can_decode_setup")
    def test_cmd_decode_to_stdout(
        self,
        m_can_decode_setup: Mock,
        run_decode2stdout_mock: Mock,
        run_decode2file_mock: Mock,
    ) -> None:
        """Tests the decode command line interface with output to stdout

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

    @patch("cli.commands.c_etl.run_decode2file")
    @patch("cli.commands.c_etl.run_decode2stdout")
    @patch("cli.commands.c_etl.can_decode_setup")
    def test_cmd_decode_to_files(
        self,
        m_can_decode_setup: Mock,
        run_decode2stdout_mock: Mock,
        run_decode2file_mock: Mock,
    ) -> None:
        """Tests the decode command line interface when no config file is given"""
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


class TestFilter(unittest.TestCase):
    """Test 'filter' command"""

    def test_cmd_filter_help(self) -> None:
        """Test help option"""
        runner = CliRunner()
        result = runner.invoke(cmd_filter, ["-c", "test"])
        self.assertTrue("Usage" in result.output)

    @patch("cli.commands.c_etl.run_filter")
    @patch("cli.commands.c_etl.can_filter_setup")
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

    @patch("cli.commands.c_etl.run_filter")
    @patch("cli.commands.c_etl.can_filter_setup")
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


class TestTable(unittest.TestCase):
    """Test 'table' command"""

    def test_cmd_table_help(self) -> None:
        """Test help option for the 'table' command when no argument or option are provided"""
        runner = CliRunner()
        result = runner.invoke(cmd_table, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.commands.c_etl.run_table")
    @patch("cli.commands.c_etl.table_setup")
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


class TestFoxCliMainCommandEtl(unittest.TestCase):
    """Test of the 'etl' commands and options."""

    def test_help(self):
        """Test 'fox.py etl --help' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["etl", "--help"])
        self.assertEqual(result.exit_code, 0)


if __name__ == "__main__":
    unittest.main()
