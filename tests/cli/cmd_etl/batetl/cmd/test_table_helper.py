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

"""Testing file 'cli/cmd_etl/batetl/cmd/table_helper.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import Mock, patch

import pyarrow.compute as pc

try:
    from cli.cmd_etl.batetl.cmd.table_helper import (
        convert_start_date,
        get_output_format_enum,
        run_table,
        table_setup,
        validate_table_config,
    )
    from cli.cmd_etl.batetl.etl.table import OutputFormats
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[5]))
    from cli.cmd_etl.batetl.cmd.table_helper import (
        convert_start_date,
        get_output_format_enum,
        run_table,
        table_setup,
        validate_table_config,
    )
    from cli.cmd_etl.batetl.etl.table import OutputFormats


class TestTableHelper(unittest.TestCase):
    """Tests the helper functions for the table command"""

    @patch("cli.cmd_etl.batetl.cmd.table_helper.Table")
    @patch("cli.cmd_etl.batetl.cmd.table_helper.validate_table_config")
    @patch("cli.cmd_etl.batetl.cmd.table_helper.read_config")
    def test_table_setup(
        self, read_config_mock: Mock, validate_mock: Mock, table_mock: Mock
    ) -> None:
        """test_table_setup mocks all function calls in
        the table_setup function to ensure the correctness
        of the setup procedure.

        :param read_config_mock: Mock for the read_config function.
        :param validate_mock: Mock for validate_filter_config function.
        :param CANFilter_mock: Mock for the CANFilter class.
        """
        # Case 1: check whether all functions are called
        # once as intended with output format
        test_config = Path("test")
        test_config_dict = {"start_date": "2024-01-01T00:00:00", "output_format": "csv"}
        read_config_mock.return_value = test_config_dict
        table_setup(test_config)
        read_config_mock.assert_called_once_with(test_config)
        validate_mock.assert_called_once_with(test_config_dict)
        start_date_str = "2024-01-01T00:00:00"
        converted_start_date = convert_start_date(start_date_str)
        table_mock.assert_called_once_with(
            converted_start_date,
            get_output_format_enum("csv"),
            None,
            tolerance=-100000,
            timestamp_factor=1000000,
        )
        # Case 2: check whether all functions are called
        # once as intended without output format
        read_config_mock.reset_mock()
        validate_mock.reset_mock()
        table_mock.reset_mock()
        test_config = Path("test")
        test_config_dict = {"start_date": "2024-01-01T00:00:00"}
        read_config_mock.return_value = test_config_dict
        table_setup(test_config)
        read_config_mock.assert_called_once_with(test_config)
        validate_mock.assert_called_once_with(test_config_dict)
        table_mock.assert_called_once_with(
            converted_start_date,
            None,
            None,
            tolerance=-100000,
            timestamp_factor=1000000,
        )

    def test_convert_start_date(self) -> None:
        """Tests the convert_start_date which takes a date in the UTC
        format and converts it into a arrow TimestampScalar."""
        # Case 1: start date format is correct
        start_date_str = "2024-01-01T00:00:00"
        converted_start_date = convert_start_date(start_date_str)
        expected_start_date = pc.strptime(
            start_date_str, format="%Y-%m-%dT%H:%M:%S", unit="us"
        )
        self.assertEqual(converted_start_date, expected_start_date)
        # Case 2: start date format is not correct
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            start_date_str = "2024-01-01 00:00:00"
            convert_start_date(start_date_str)
        self.assertTrue("not in the correct UTC format" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_get_output_format_enum(self) -> None:
        """Tests the get_output_format_enum where an OutputFormat enum is
        return or not depending in the passed string."""
        # Case 1: Test with correct output format
        self.assertEqual(OutputFormats["CSV"], get_output_format_enum("csv"))
        # Case 2: Test with wrong output format
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            get_output_format_enum("txt")
        self.assertTrue("is not valid" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_validate_table_config(self) -> None:
        """test_validate_table_config checks that the validate_table_config
        validates the configuration correctly, especially wrong
        configuration.
        """
        # Case 1: config_dict as it could be
        test_config_dict = {
            "start_date": "2024-01-01T00:00:00",
            "join_on": "current",
            "output_format": "csv",
            "tolerance": -100000,
            "timestamp_factor": 1000000,
        }
        self.assertIsNone(validate_table_config(test_config_dict))
        # Case 2: config_dict without start_date
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {}
            validate_table_config(test_config_dict)
        self.assertTrue("missing 'start_date'" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 3: config_dict as it should be without join_on and output_format
        test_config_dict = {"start_date": "2024-01-01T00:00:00"}
        self.assertIsNone(validate_table_config(test_config_dict))
        # Case 4: start_date is not a string
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {"start_date": 10}
            validate_table_config(test_config_dict)
        self.assertTrue("start_date" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 5: join_on is not a string
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {"start_date": "2024-01-01T00:00:00", "join_on": 10}
            self.assertIsNone(validate_table_config(test_config_dict))
        self.assertTrue("join_on" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 6: output_format is not a string
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {
                "start_date": "2024-01-01T00:00:00",
                "output_format": 10,
            }
            self.assertIsNone(validate_table_config(test_config_dict))
            # check if key word output_format is part of the error message
        self.assertTrue("output_format" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 6: tolerance is not an integer
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {
                "start_date": "2024-01-01T00:00:00",
                "tolerance": "test",
            }
            self.assertIsNone(validate_table_config(test_config_dict))
            # check if key word tolerance is part of the error message
        self.assertTrue("tolerance" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 7: timestamp_factor is not an integer
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {
                "start_date": "2024-01-01T00:00:00",
                "timestamp_factor": "test",
            }
            self.assertIsNone(validate_table_config(test_config_dict))
            # check if key word timestamp_factor is part of the error message
        self.assertTrue("timestamp_factor" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    @patch("cli.cmd_etl.batetl.cmd.table_helper.Path.mkdir")
    def test_run_table(self, mkdir_mock: Mock) -> None:
        """test_run_table checks the correctness of the
        table functionality execution via the command line

        :param mkdir_mock: A mock for the pathlib.Path.mkdir function

        """
        # Case 1: check case with data suffix and output suffix
        table_obj = Mock()
        table_obj.can_to_table = Mock(return_value="test table")
        table_obj.save_data = Mock()
        data = Path("data.txt")
        output = Path("output.txt")
        run_table(table_obj, data, output)
        table_obj.can_to_table.assert_called_once_with(data)
        table_obj.save_data.assert_called_once_with({output: "test table"})
        # Case 2: check case with data suffix and output without suffix
        output = Path("output")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            run_table(table_obj, data, output)
        self.assertTrue("not a valid configuration" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 3: check case with data without suffix and output with suffix
        table_obj.join = Mock(return_value="join table")
        data = Mock()
        data.suffix = None
        output = Path("output.txt")
        data.glob = Mock(return_value=["test_file_1.txt", "test_file_2.txt"])
        table_obj.save_data = Mock()
        run_table(table_obj, data, output)
        table_obj.join.assert_called_once_with(tables=["test table", "test table"])
        table_obj.save_data.assert_called_once_with({output: "join table"})
        # Case 4: check case with data without suffix and output without suffix
        output = Path("etl_test_table_helper_output")
        data.glob = Mock(
            return_value=[Path("test_file_1.txt"), Path("test_file_2.txt")]
        )
        table_obj.save_data = Mock()
        run_table(table_obj, data, output)
        new_data_files = [
            Path("etl_test_table_helper_output/test_file_1.txt"),
            Path("etl_test_table_helper_output/test_file_2.txt"),
        ]
        tables = ["test table", "test table"]
        mkdir_mock.assert_called_once()
        table_obj.save_data.assert_called_once_with(dict(zip(new_data_files, tables)))


if __name__ == "__main__":
    unittest.main()
