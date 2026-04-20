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

"""Testing file 'cli/cmd_plot/data_handling/csv_handler.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

try:
    from cli.cmd_plot.data_handling.csv_handler import CSVHandler
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_plot.data_handling.csv_handler import CSVHandler

PATH_EXECUTION = Path(__file__).parent.parent / "test_execution"
PATH_DATA = Path(__file__).parent.parent / "test_data"


@patch("cli.cmd_plot.data_handling.csv_handler.pd.read_csv")
@patch("cli.cmd_plot.data_handling.csv_handler.CSVHandler.get_tmp_data")
@patch("cli.cmd_plot.data_handling.csv_handler.CSVHandler.write_tmp_file")
class TestGetData(unittest.TestCase):
    """Class to test the get_data method of the CSVHandler class"""

    def setUp(self) -> None:
        """Creates the needed CSVHandler object for the later tests"""
        self.csv_handler_obj = CSVHandler(
            {"current": "float", "date": "datetime"}, 0, 3
        )

    def test_get_data_no_tmp(
        self,
        write_tmp_file: Mock,
        get_tmp_data: Mock,
        mock_read_csv: Mock,
    ) -> None:
        """Tests the get_data with valid file"""
        file_path = Path("test_file")
        no_tmp = True
        get_tmp_data.return_value = None
        self.csv_handler_obj.get_data(file_path, no_tmp)
        mock_read_csv.assert_called_once_with(
            file_path,
            usecols=["current", "date"],
            dtype=self.csv_handler_obj.columns,
            skiprows=self.csv_handler_obj.skip,
            parse_dates=["date"],
            na_values="NULL",
        )
        mock_read_csv.return_value.round.assert_called_once_with(
            self.csv_handler_obj.precision
        )
        write_tmp_file.assert_called_once()

    def test_get_data_with_tmp_data_not_none(
        self,
        write_tmp_file: Mock,
        get_tmp_data: Mock,
        mock_read_csv: Mock,
    ) -> None:
        """Tests the get_data with valid file"""
        file_path = Path("test_file")
        no_tmp = True
        get_tmp_data.return_value = "TEST"
        data = self.csv_handler_obj.get_data(file_path, no_tmp)
        self.assertEqual(data, "TEST")
        mock_read_csv.assert_not_called()
        mock_read_csv.return_value.round.assert_not_called()
        write_tmp_file.assert_not_called()

    def test_get_data_permission_error(
        self,
        write_tmp_file: Mock,
        get_tmp_data: Mock,
        mock_read_csv: Mock,
    ) -> None:
        """Tests the get_data with valid file"""
        file_path = Path("test_file")
        no_tmp = True
        get_tmp_data.return_value = None
        mock_read_csv.side_effect = PermissionError
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            self.csv_handler_obj.get_data(file_path, no_tmp)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Can not access file", buf.getvalue())
        write_tmp_file.assert_not_called()

    def test_get_data_string_column(
        self,
        write_tmp_file: Mock,
        get_tmp_data: Mock,
        mock_read_csv: Mock,
    ) -> None:
        """Tests the get_data with valid file"""
        file_path = Path("test_file")
        no_tmp = True
        get_tmp_data.return_value = None
        data_mock = MagicMock()
        mock_read_csv.return_value = data_mock
        self.csv_handler_obj.columns = {"string_column": "string"}
        self.csv_handler_obj.get_data(file_path, no_tmp)
        # pylint: disable=C2801
        data_mock.__getitem__().fillna.assert_called_with("NULL")
        write_tmp_file.assert_called_once()

    def test_get_data_column_not_matched(
        self,
        write_tmp_file: Mock,
        get_tmp_data: Mock,
        mock_read_csv: Mock,
    ) -> None:
        """Tests the get_data column not matched"""
        file_path = Path("test_file")
        no_tmp = True
        get_tmp_data.return_value = None
        mock_read_csv.side_effect = ValueError("do not match columns: test")
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            self.csv_handler_obj.get_data(file_path, no_tmp)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("desired columns test not found", buf.getvalue())
        write_tmp_file.assert_not_called()

    def test_get_data_bad_column_type(
        self,
        write_tmp_file: Mock,
        get_tmp_data: Mock,
        mock_read_csv: Mock,
    ) -> None:
        """Tests the get_data with bad column type"""
        file_path = Path("test_file")
        no_tmp = True
        get_tmp_data.return_value = None
        mock_read_csv.side_effect = ValueError()
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            self.csv_handler_obj.get_data(file_path, no_tmp)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Error in data config file", buf.getvalue())
        write_tmp_file.assert_not_called()


class TestValidateConfig(unittest.TestCase):
    """Class to test the validate_config method of the CSVHandler class"""

    def setUp(self):
        self.config = {
            "general": {"skip": 0, "precision": 3},
            "columns": {"Date": "string"},
        }

    def test_validate_success(self):
        """Tests the validate_config method with valid config"""
        CSVHandler.validate_config(self.config)

    def test_validate_fail(self):
        """Tests the validate_config method with invalid config"""
        self.config["columns"]["Date"] = 2
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            CSVHandler.validate_config(self.config)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("CSV handler validation error", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
