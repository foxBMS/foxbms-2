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

"""Testing file 'cli/cmd_plot/data_handling/csv_handler.py'."""

import io
import shutil
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import Mock, patch

import numpy as np
import pandas

try:
    from cli.cmd_plot.data_handling.csv_handler import CSVHandler
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_plot.data_handling.csv_handler import CSVHandler

PATH_EXECUTION = Path(__file__).parent.parent / "test_execution"
PATH_DATA = Path(__file__).parent.parent / "test_data"


@patch("cli.cmd_plot.data_handling.csv_handler.Path.glob")
@patch("cli.cmd_plot.data_handling.csv_handler.CSVHandler._check_for_tmp_directory")
class TestCheckForTmpData(unittest.TestCase):
    """Class to test the check_for_tmp_data method from the CSVHandlers"""

    def test_if_tmp_true(self, _: Mock, mock_glob: Mock) -> None:
        """Tests the check_for_tmp_data if tmp_dir is true"""
        mock_glob.return_value = [Path("test.parquet")]
        result = CSVHandler._check_for_tmp_data(Path("test.csv"))  # pylint: disable=protected-access
        self.assertEqual(result, Path("test.parquet"))

    def test_if_tmp_false(self, mock_check: Mock, _: Mock) -> None:
        """Tests the check_for_tmp_data if tmp_dir is false"""
        mock_check.return_value = False
        result = CSVHandler._check_for_tmp_data(Path("test.csv"))  # pylint: disable=protected-access
        self.assertEqual(result, None)

    def test_if_not_the_same_name(self, _: Mock, mock_glob: Mock) -> None:
        """Tests the check_for_tmp_data with input file and parquet not
        having the same name"""
        mock_glob.return_value = [Path("test2.parquet")]
        result = CSVHandler._check_for_tmp_data(Path("test.csv"))  # pylint: disable=protected-access
        self.assertEqual(result, None)


class TestCheckTmpDirectory(unittest.TestCase):
    """Class to test the check_for_tmp_directory method of the
    CSVHandler class"""

    def setUp(self) -> None:
        """Setups file_path and file mock"""
        self.mock_file_path = Mock()
        self.mock_file = Mock(name="mock_file")
        self.mock_file_path.parent.iterdir.return_value = [self.mock_file]
        self.mock_file.is_dir.return_value = True

    def test_with_temp_dir(self) -> None:
        """Tests check_for_tmp_directory with available tmp folder"""
        self.mock_file.name = "temp_data_foxplot_test"
        result = CSVHandler._check_for_tmp_directory(self.mock_file_path)  # pylint: disable=protected-access
        self.assertEqual(result, self.mock_file)

    def test_with_without_temp_dir(self) -> None:
        """Tests check_for_tmp_directory without available tmp folder"""
        self.mock_file.name = "temp_data_test"
        result = CSVHandler._check_for_tmp_directory(self.mock_file_path)  # pylint: disable=protected-access
        self.assertEqual(result, None)


class TestCreateTmpDirectory(unittest.TestCase):
    """Class to test the create_tmp_directory method of the CSVHandler class"""

    @patch("cli.cmd_plot.data_handling.csv_handler.tempfile.mkdtemp")
    def test_create(self, mock_mkdtemp: Mock) -> None:
        """Teste the creation of a temporary directory"""
        test_path = Path(__file__)
        CSVHandler._create_tmp_directory(test_path)  # pylint: disable=protected-access
        mock_mkdtemp.assert_called_once_with(
            prefix="temp_data_foxplot_", dir=test_path.parent
        )


class TestGetData(unittest.TestCase):
    """Class to test the get_data method of the CSVHandler class"""

    def tearDown(self) -> None:
        """Delets the temporary folder"""
        tmp_folder = CSVHandler._check_for_tmp_directory(PATH_DATA / "input_data.csv")  # pylint: disable=protected-access
        if tmp_folder:
            shutil.rmtree(tmp_folder)

    def test_get_data_with_tmp(self) -> None:
        """Tests the get_data with valid file."""
        expected_time = np.array(
            [
                "53",
                "54",
                "55",
                "56",
                "57",
                "58",
                "59",
                "00",
                "01",
                "02",
                "03",
                "04",
                "05",
                "06",
                "07",
                "08",
                "09",
                "10",
                "11",
                "12",
                "13",
                "14",
                "15",
                "16",
                "17",
                "18",
                "19",
                "20",
                "21",
                "22",
                "23",
                "24",
                "25",
                "26",
                "27",
                "28",
                "29",
                "30",
                "31",
                "32",
            ],
            dtype=str,
        )
        expected_current = np.array(
            [
                3.0,
                5.0,
                7.0,
                9.0,
                11.0,
                13.0,
                15.0,
                17.0,
                19.0,
                21.0,
                23.0,
                25.0,
                27.0,
                29.0,
                31.0,
                33.0,
                35.0,
                37.0,
                39.0,
                41.0,
                43.0,
                45.0,
                47.0,
                49.0,
                51.0,
                53.0,
                55.0,
                57.0,
                59.0,
                61.0,
                63.0,
                67.0,
                69.0,
                71.0,
                73.0,
                75.0,
                77.0,
                79.0,
                81.0,
                83.0,
            ]
        )
        index = np.arange(40)
        data_dict = {"Time": expected_time, "Current(A)": expected_current}
        expected_data = pandas.DataFrame(data=data_dict, index=index)
        # Get DataFrame from CSVHandler
        columns = {
            "Time": "str",
            "Current(A)": "float",
        }
        csv_handler = CSVHandler(columns=columns, skip=4, precision=3)
        actual_data = csv_handler.get_data(PATH_DATA / "input_data.csv")
        pandas.testing.assert_frame_equal(actual_data, expected_data)

    def test_get_data_tmp_available(self) -> None:
        """Tests the get_data method with tmp_dir true"""
        creat_tmp_directory = CSVHandler._create_tmp_directory  # pylint: disable=protected-access
        check_for_tmp_directory = CSVHandler._check_for_tmp_directory  # pylint: disable=protected-access
        columns = {
            "Time": "str",
            "Current(A)": "float",
        }
        csv_handler = CSVHandler(columns=columns, skip=4, precision=3)
        file_path = PATH_DATA / "input_data.csv"
        tmp_dir = CSVHandler._create_tmp_directory(file_path)  # pylint: disable=protected-access
        CSVHandler._create_tmp_directory = Mock()  # pylint: disable=protected-access
        CSVHandler._check_for_tmp_directory = Mock(return_value=tmp_dir)  # pylint: disable=protected-access
        csv_handler.get_data(file_path)  # pylint: disable=protected-access
        CSVHandler._create_tmp_directory.assert_not_called()  # pylint: disable=protected-access
        # Reset of class methods is needed otherwise other unittests are affected
        CSVHandler._create_tmp_directory = creat_tmp_directory  # pylint: disable=protected-access
        CSVHandler._check_for_tmp_directory = check_for_tmp_directory  # pylint: disable=protected-access

    @patch("cli.cmd_plot.data_handling.csv_handler.pandas.read_parquet")
    def test_get_data_available_parquet(self, mock_read: Mock) -> None:
        """Tests the get_data with available parquet"""
        columns = {
            "Time": "str",
            "Current(A)": "float",
        }
        csv_handler = CSVHandler(columns=columns, skip=4, precision=3)
        csv_handler._check_for_tmp_data = Mock(return_value="test")  # pylint: disable=protected-access
        csv_handler.get_data(Path("test.csv"))
        mock_read.assert_called_once_with("test", engine="pyarrow")

    @patch("cli.cmd_plot.data_handling.csv_handler.CSVHandler._check_for_tmp_data")
    def test_get_data_invalid_parquet(self, mock_tmp: Mock) -> None:
        """Tests the get_data with invalid parquet file."""
        columns = {
            "Time": "str",
            "Current(A)": "float",
        }
        csv_handler = CSVHandler(columns=columns, skip=4, precision=3)
        mock_tmp.return_value = PATH_DATA / "input_data.csv"
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            csv_handler.get_data(PATH_DATA / "input_data.csv")
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("Parquet Error" in buf.getvalue())

    @patch("cli.cmd_plot.data_handling.csv_handler.CSVHandler._check_for_tmp_data")
    def test_get_data_column_not_matched(self, mock_tmp: Mock) -> None:
        """Tests the get_data column not matched"""
        columns = {
            "Time2": "str",
            "Current(A)": "float",
        }
        csv_handler = CSVHandler(columns=columns, skip=4, precision=3)
        buf = io.StringIO()
        mock_tmp.return_value = None
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            csv_handler.get_data(PATH_DATA / "input_data.csv")
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("Skip value removed header or" in buf.getvalue())

    @patch("cli.cmd_plot.data_handling.csv_handler.CSVHandler._check_for_tmp_data")
    def test_get_data_bad_column_type(self, mock_tmp: Mock) -> None:
        """Tests the get_data with bad column type"""
        columns = {
            "Time": "test",
            "Current(A)": "float",
        }
        csv_handler = CSVHandler(columns=columns, skip=4, precision=3)
        buf = io.StringIO()
        mock_tmp.return_value = None
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            csv_handler.get_data(PATH_DATA / "input_data.csv")
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("data type" in buf.getvalue())


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
