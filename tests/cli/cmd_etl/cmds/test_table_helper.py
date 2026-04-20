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

"""Testing file 'cli/cmd_etl/cmds/table_helper.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import Mock, patch

import pyarrow as pa
import pyarrow.compute as pc

try:
    from cli.cmd_etl.cmds.table_helper import (
        convert_start_date,
        get_output_format_enum,
        run_table,
        table_setup,
        validate_table_config,
    )
    from cli.cmd_etl.etl.table import OutputFormats
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.cmds.table_helper import (
        convert_start_date,
        get_output_format_enum,
        run_table,
        table_setup,
        validate_table_config,
    )
    from cli.cmd_etl.etl.table import OutputFormats


@patch("cli.cmd_etl.cmds.table_helper.Table")
@patch("cli.cmd_etl.cmds.table_helper.validate_table_config")
@patch("cli.cmd_etl.cmds.table_helper.read_config")
class TestTableSetup(unittest.TestCase):
    """Tests table_setup method

    :param read_config_mock: Mock for the read_config function.
    :param validate_mock: Mock for validate_filter_config function.
    :param CANFilter_mock: Mock for the CANFilter class.
    """

    def setUp(self):
        """Creates the converted start date used in the tests"""
        start_date_str = "2024-01-01T00:00:00"
        self.converted_start_date = convert_start_date(start_date_str)

    def test_table_setup_with_output_format(
        self, read_config_mock: Mock, validate_mock: Mock, table_mock: Mock
    ) -> None:
        """Tests the table_setup method with output format"""
        test_config = Path("test")
        test_config_dict = {
            "start_date": "2024-01-01T00:00:00",
            "output_format": "csv",
            "separator": ",",
        }
        read_config_mock.return_value = test_config_dict
        table_setup(test_config)
        read_config_mock.assert_called_once_with(test_config)
        validate_mock.assert_called_once_with(test_config_dict)
        table_mock.assert_called_once_with(
            self.converted_start_date,
            get_output_format_enum("csv"),
            None,
            tolerance=-100000,
            timestamp_factor=1000000,
            separator=",",
        )

    def test_table_setup_without_output_format(
        self, read_config_mock: Mock, validate_mock: Mock, table_mock: Mock
    ) -> None:
        """Tests the table_setup method without output format"""
        read_config_mock.reset_mock()
        validate_mock.reset_mock()
        table_mock.reset_mock()
        test_config = Path("test")
        test_config_dict = {"start_date": "2024-01-01T00:00:00", "separator": ","}
        read_config_mock.return_value = test_config_dict
        table_setup(test_config)
        read_config_mock.assert_called_once_with(test_config)
        validate_mock.assert_called_once_with(test_config_dict)
        table_mock.assert_called_once_with(
            self.converted_start_date,
            None,
            None,
            tolerance=-100000,
            timestamp_factor=1000000,
            separator=",",
        )


class TestConvertStartDate(unittest.TestCase):
    """Tests convert_start_date method"""

    def test_convert_start_date_valid_format(self) -> None:
        """Tests the convert_start_date which takes a valid date in the UTC
        format and converts it into a arrow TimestampScalar.
        """
        start_date_str = "2024-01-01T00:00:00"
        converted_start_date = convert_start_date(start_date_str)
        expected_start_date = pc.strptime(
            start_date_str, format="%Y-%m-%dT%H:%M:%S", unit="us"
        ).cast(options=pc.CastOptions(target_type=pa.timestamp(unit="us", tz="+00:00")))
        self.assertEqual(converted_start_date, expected_start_date)

    def test_convert_start_date_invalid_format(self) -> None:
        """Tests the convert_start_date which takes a invalid date in the UTC
        format and converts it into a arrow TimestampScalar.
        """
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            start_date_str = "2024-01-01 00:00:00"
            convert_start_date(start_date_str)
        self.assertTrue("not in the correct UTC format" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)


class TestGetOutputFormatEnum(unittest.TestCase):
    """Tests get_output_format_enum method"""

    def test_get_output_format_enum_valid_format(self) -> None:
        """Tests the get_output_format_enum where an OutputFormat enum is
        returned.
        """
        self.assertEqual(OutputFormats["CSV"], get_output_format_enum("csv"))

    def test_get_output_format_enum_invalid_format(self) -> None:
        """Tests the get_output_format_enum where an OutputFormat enum is not
        returned.
        """
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            get_output_format_enum("txt")
        self.assertTrue("is not valid" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)


class TestValidateTableConfig(unittest.TestCase):
    """Tests validate_table_config method"""

    def test_validate_table_config_valid_config(self) -> None:
        """Tests the validate_table_config with valid configuration"""
        test_config_dict = {
            "start_date": "2024-01-01T00:00:00",
            "join_on": "current",
            "output_format": "csv",
            "tolerance": -100000,
            "timestamp_factor": 1000000,
        }
        self.assertIsNone(validate_table_config(test_config_dict))

    def test_validate_table_config_without_start_date(self) -> None:
        """Tests the validate_table_config with config without start date"""
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {}
            validate_table_config(test_config_dict)
        self.assertTrue("missing 'start_date'" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_validate_table_config_without_join_on_and_output_format(self) -> None:
        """Tests the validate_table_config with config without start date"""
        test_config_dict = {"start_date": "2024-01-01T00:00:00"}
        self.assertIsNone(validate_table_config(test_config_dict))

    def test_validate_table_config_start_date_not_a_string(self) -> None:
        """Tests the validate_table_config with the start date not as string"""
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {"start_date": 10}
            validate_table_config(test_config_dict)
        self.assertTrue("start_date" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_validate_table_config_join_on_not_a_string(self) -> None:
        """Tests the validate_table_config with join_on not as string"""
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            test_config_dict = {"start_date": "2024-01-01T00:00:00", "join_on": 10}
            self.assertIsNone(validate_table_config(test_config_dict))
        self.assertTrue("join_on" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_validate_table_config_output_format_not_a_string(self) -> None:
        """Tests the validate_table_config with output_format not as string"""
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

    def test_validate_table_config_with_tolerance_not_as_int(self) -> None:
        """Tests the validate_table_config with tolerance not as int"""
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

    def test_validate_table_config_with_timestamp_factor_not_as_int(self) -> None:
        """Tests the validate_table_config with tolerance not as int"""
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


class TestRunTable(unittest.TestCase):
    """Tests run_table method"""

    def setUp(self):
        """Creates needed table_obj Mock"""
        self.table_obj = Mock()
        self.table_obj.save_data = Mock()
        self.table_obj.can_to_table = Mock(return_value="test table")

    def test_run_table_data_and_output_suffix(self) -> None:
        """Tests the run_table method with data suffix and output suffix"""
        data = Path("data.txt")
        output = Path("output.txt")
        run_table(self.table_obj, data, output)
        self.table_obj.can_to_table.assert_called_once_with(data)
        self.table_obj.save_data.assert_called_once_with({output: "test table"})

    def test_run_table_data_suffix_and_no_output_suffix(self) -> None:
        """Tests the run_table method with data suffix and no output suffix"""
        data = Path("data.txt")
        output = Path("output")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            run_table(self.table_obj, data, output)
        self.assertTrue("not a valid configuration" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_run_table_no_data_suffix_and_output_suffix(self) -> None:
        """Tests the run_table method with no data suffix and output suffix"""
        self.table_obj.join = Mock(return_value="join table")
        data = Mock()
        data.suffix = None
        output = Path("output.txt")
        data.glob = Mock(return_value=["test_file_1.txt", "test_file_2.txt"])
        run_table(self.table_obj, data, output)
        self.table_obj.join.assert_called_once_with(tables=["test table", "test table"])
        self.table_obj.save_data.assert_called_once_with({output: "join table"})

    @patch("cli.cmd_etl.cmds.table_helper.Path.mkdir")
    def test_run_table_no_data_suffix_and_no_output_suffix(
        self, mkdir_mock: Mock
    ) -> None:
        """Tests the run_table method with no data suffix and output suffix

        :param mkdir_mock: A mock for the pathlib.Path.mkdir function
        """
        data = Mock()
        data.suffix = None
        data.glob = Mock(
            return_value=[Path("test_file_1.txt"), Path("test_file_2.txt")]
        )
        output = Path("etl_test_table_helper_output")
        self.table_obj.save_data = Mock()
        run_table(self.table_obj, data, output)
        new_data_files = [
            Path("etl_test_table_helper_output/test_file_1.txt"),
            Path("etl_test_table_helper_output/test_file_2.txt"),
        ]
        tables = ["test table", "test table"]
        mkdir_mock.assert_called_once()
        self.table_obj.save_data.assert_called_once_with(
            dict(zip(new_data_files, tables, strict=False))
        )


if __name__ == "__main__":
    unittest.main()
