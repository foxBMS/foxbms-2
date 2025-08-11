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

"""Testing file 'cli/cmd_etl/batetl/etl/table.py'."""

import datetime
import io
import logging
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from tempfile import mkstemp
from unittest.mock import Mock, patch

import pyarrow as pa
import pyarrow.compute as pc

try:
    from cli.cmd_etl.batetl.cmd.table_helper import (
        convert_start_date,
        get_output_format_enum,
    )
    from cli.cmd_etl.batetl.etl.table import Table
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[5]))
    from cli.cmd_etl.batetl.cmd.table_helper import (
        convert_start_date,
        get_output_format_enum,
    )
    from cli.cmd_etl.batetl.etl.table import Table


class TestTable(unittest.TestCase):
    """The test case definition for the Table class"""

    def test_table_init(self) -> None:
        """Tests the creation of a Table object"""
        # Case 1: Test that the Table object is
        # created correctly with all parameters passed
        test_obj = Table(
            start_date="2024-01-01T00:00:00", output_format="csv", join_on="Current"
        )
        # Test via __dict__ comparison ist not possible, because
        # trying to print the start date causes an Arrow Timezone error
        self.assertTrue(hasattr(test_obj, "_start_date"))
        self.assertTrue(hasattr(test_obj, "_output_format"))
        self.assertTrue(hasattr(test_obj, "_join_on"))

    @patch("cli.cmd_etl.batetl.etl.table.Table._search_for_table")
    def test_join(self, search_for_table_mock: Mock) -> None:
        """Tests join method of Table class

        :param search_for_table_mock: A mock for the _search_for_table method
        """
        # Case 1: Test the check for join equals to None
        table_obj = Table(start_date="2024-01-01T00:00:00")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            table_obj.join(tables=[])
        self.assertTrue("is not possible" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        # Case 2: Test correct output
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="Current")
        timestamps_current = pa.array([0.01, 0.04, 0.06, 0.11, 0.15])
        current_values = pa.array([100, 200, 300, 400, 500])
        current_table = pa.Table.from_arrays(
            [timestamps_current, current_values], ["Timestamp", "Current"]
        )
        current_table = TestTable.add_date(
            current_table, convert_start_date("2024-01-01T00:00:00")
        )
        current_table = current_table.drop("Timestamp")
        timestamps_voltage = pa.array([0.02, 0.03, 0.04, 0.08, 0.12])
        voltage_values = pa.array([4.2, 4.3, 4.4, 4.5, 4.6])
        voltage_table = pa.Table.from_arrays(
            [timestamps_voltage, voltage_values], ["Timestamp", "Voltage"]
        )
        voltage_table = TestTable.add_date(
            voltage_table, convert_start_date("2024-01-01T00:00:00")
        )
        voltage_table = voltage_table.drop("Timestamp")
        search_for_table_mock.return_value = (current_table, [voltage_table])
        output_table = table_obj.join(tables=[current_table, voltage_table])
        expected_voltage_values = pa.array([None, 4.4, 4.4, 4.5, 4.6])
        self.assertEqual(
            expected_voltage_values.to_pylist(), output_table["Voltage"].to_pylist()
        )

    @patch("cli.cmd_etl.batetl.etl.table.Table._cast_columns")
    @patch("cli.cmd_etl.batetl.etl.table.Table._add_date")
    def test_can_to_table(self, add_date_mock: Mock, cast_columns_mock: Mock):
        """Tests the can_to_table method

        :param add_date_mock: A mock for the _add_date method
        :param cast_columns_mock: A mock for the _cast_columns_method
        """
        # Case 1: Detection of not valid json objects as input
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        temp_f, temp_path = mkstemp()
        with open(temp_f, mode="w", encoding="utf-8") as f:
            f.write('{"test","error"}')
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            table_obj.can_to_table(Path(temp_path))
        self.assertTrue("Input data are not valid" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        Path(temp_path).unlink()
        # Case 2: Correct execution
        table_mock = Mock()
        drop_table_mock = Mock()
        table_mock.drop.return_value = drop_table_mock
        with patch(
            "cli.cmd_etl.batetl.etl.table.read_json", return_value=table_mock
        ) as read_json_mock:
            add_date_mock.return_value = table_mock
            table_obj.can_to_table(Path("test.txt"))
            read_json_mock.assert_called_once_with(Path("test.txt"))
            add_date_mock.assert_called_once_with(table_mock)
            table_mock.drop.assert_called_once_with("Timestamp")
            cast_columns_mock.assert_called_once_with(drop_table_mock)

    @patch("cli.cmd_etl.batetl.etl.table.write_table")
    @patch("cli.cmd_etl.batetl.etl.table.write_csv")
    def test_save_data(self, write_csv_mock: Mock, write_table_mock: Mock) -> None:
        """Tests the save_data method

        :param write_csv_mock: A mock for the write_csv function
        :param write_table_mock: A mock for the wirte_table function
        """
        # Case 1: csv as Output format
        with io.StringIO() as buf, redirect_stdout(buf):
            tables = {Path("test.txt"): "test_table"}
            table_obj = Table(
                start_date=convert_start_date("2024-01-01T00:00:00"),
                output_format=get_output_format_enum("csv"),
            )
            table_obj.save_data(tables)
            write_csv_mock.assert_called_once_with("test_table", Path("test.csv"))
        # Case 2: parquet as Output format
        with io.StringIO() as buf, redirect_stdout(buf):
            table_obj = Table(
                start_date=convert_start_date("2024-01-01T00:00:00"),
                output_format=get_output_format_enum("parquet"),
            )
            table_obj.save_data(tables)
            write_table_mock.assert_called_once_with("test_table", Path("test.parquet"))
        # Case 3: Output format defined by output file suffix (csv)
        tables = {Path("test.csv"): "test_table"}
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        write_csv_mock.reset_mock()
        table_obj.save_data(tables)
        write_csv_mock.assert_called_once_with("test_table", Path("test.csv"))
        # Case 4: Output format defined by output file suffix (parquet)
        tables = {Path("test.parquet"): "test_table"}
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        write_table_mock.reset_mock()
        table_obj.save_data(tables)
        write_table_mock.assert_called_once_with("test_table", Path("test.parquet"))
        # Case 5: Check exception handling if more than
        # one table should saved in one file
        tables = {
            Path("test1.parquet"): "test_table",
            Path("test2.parquet"): "test_table",
        }
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            table_obj.save_data(tables)
        self.assertTrue(
            "Save more than one table without an output format is not "
            "possible or did you forget to specify a file name for the "
            "output parameter." in buf.getvalue()
        )
        self.assertEqual(cm.exception.code, 1)
        # Case 6: Permission exception
        tables = {
            Path("test1.csv"): "test_table",
        }
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        write_csv_mock.side_effect = PermissionError
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            table_obj.save_data(tables)
        self.assertIn(
            "Could not write output file, because permission is denied.", buf.getvalue()
        )
        self.assertEqual(cm.exception.code, 1)

    def test_search_for_table(self) -> None:
        """Tests the _search_for_table method"""
        # Case 1: join_on table is found
        timestamps_current = pa.array([0.01, 0.04, 0.06, 0.11, 0.15])
        current_values = pa.array([100, 200, 300, 400, 500])
        current_table = pa.Table.from_arrays(
            [timestamps_current, current_values], ["Timestamp", "Current"]
        )
        timestamps_voltage = pa.array([0.02, 0.03, 0.04, 0.08, 0.12])
        voltage_values = pa.array([4.2, 4.3, 4.4, 4.5, 4.6])
        voltage_table = pa.Table.from_arrays(
            [timestamps_voltage, voltage_values], ["Timestamp", "Voltage"]
        )
        tables = [current_table, voltage_table]
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="Current")
        join_table, remaining_tables = table_obj._search_for_table(tables)  # pylint: disable=protected-access
        self.assertTrue("Current" in join_table.column_names)
        self.assertTrue("Voltage" in remaining_tables[0].column_names)
        # Case 2: join_on table not found
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="Date")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            join_table, remaining_tables = table_obj._search_for_table(tables)  # pylint: disable=protected-access
        self.assertTrue("Column 'Date' not found in any table." in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_add_date(self) -> None:
        """Tests the _add_date method"""
        timestamps = pa.array([0.01, 0.04, 0.06])
        test_table = pa.Table.from_arrays([timestamps], ["Timestamp"])
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        result_table = table_obj._add_date(test_table)  # pylint: disable=protected-access
        expected_date_column = [
            datetime.datetime(2024, 1, 1, 0, 0, 0, 10000),
            datetime.datetime(2024, 1, 1, 0, 0, 0, 40000),
            datetime.datetime(2024, 1, 1, 0, 0, 0, 60000),
        ]
        self.assertEqual(expected_date_column, result_table["Date"].to_pylist())

    def test_cast_columns(self) -> None:
        """Tests the _cast_columns method"""
        # Case 1: Date column
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        dates = pa.array([0.01, 0.04, 0.06])
        test_table = pa.Table.from_arrays([dates], ["Date"])
        casted_table = table_obj._cast_columns(test_table)  # pylint: disable=protected-access
        self.assertEqual(casted_table, test_table)
        # Case 2: Column with unit
        timestamps = pa.array([0.01, 0.04, 0.06])
        test_table = pa.Table.from_arrays([timestamps], ["Timestamp_s"])
        casted_table = table_obj._cast_columns(test_table)  # pylint: disable=protected-access
        self.assertTrue(casted_table["Timestamp_s"].type == pa.float32())
        # Case 3: Column with unit can not be casted
        timestamps = pa.array([20, 30, 42949672940])
        test_table = pa.Table.from_arrays([timestamps], ["Timestamp_s"])
        with self.assertLogs(level=logging.INFO) as al:
            table_obj._cast_columns(test_table)  # pylint: disable=protected-access
        self.assertTrue("can't be converted to float32" in al.output[0])

    @staticmethod
    def add_date(table: pa.Table, start_date: pa.TimestampScalar) -> pa.Table:
        """Used to add a date column to test tables

        :param table: A pyarrow table where a Date column should be added
        :param start_date: The start date which should be added
        """
        duration = (
            # The timestamps in a CAN log are converted to a duration in
            # microseconds, therefore the factor 1000000.
            pc.multiply(table.column("Timestamp"), 1000000)
            .cast(
                options=pc.CastOptions(
                    target_type=pa.int64(), allow_float_truncate=True
                )
            )
            .cast(pa.duration("us"))
        )
        date_column = pc.add(start_date, duration)
        return table.add_column(0, "Date", date_column)


if __name__ == "__main__":
    unittest.main()
