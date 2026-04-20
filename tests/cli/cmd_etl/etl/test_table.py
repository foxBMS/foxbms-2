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

"""Testing file 'cli/cmd_etl/etl/table.py'."""

import io
import logging  # noqa: TID251
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from datetime import UTC, datetime
from pathlib import Path
from tempfile import mkstemp
from unittest.mock import Mock, patch

import pyarrow as pa
import pyarrow.compute as pc
import pytz

try:
    from cli.cmd_etl.cmds.table_helper import (
        convert_start_date,
        get_output_format_enum,
    )
    from cli.cmd_etl.etl.table import Table
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.cmds.table_helper import (
        convert_start_date,
        get_output_format_enum,
    )
    from cli.cmd_etl.etl.table import Table


class TestTable(unittest.TestCase):
    """Tests the constructor of the Table class"""

    def test_table_init_correct_creation(self) -> None:
        """Tests the init of a Table object with correct object creation"""
        test_obj = Table(
            start_date="2024-01-01T00:00:00", output_format="csv", join_on="Current"
        )
        # Test via __dict__ comparison ist not possible, because
        # trying to print the start date causes an Arrow Timezone error
        self.assertTrue(hasattr(test_obj, "_start_date"))
        self.assertTrue(hasattr(test_obj, "_output_format"))
        self.assertTrue(hasattr(test_obj, "_join_on"))


class TestJoin(unittest.TestCase):
    """Tests the join method of the Table class"""

    def test_join_none(self) -> None:
        """Tests join method of Table class for join equals to None"""
        table_obj = Table(start_date="2024-01-01T00:00:00")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            table_obj.join(tables=[])
        self.assertTrue("is not possible" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    @patch("cli.cmd_etl.etl.table.Table._search_for_table")
    def test_join_correct_output(self, search_for_table_mock: Mock) -> None:
        """Tests join method of Table class with correct output

        :param search_for_table_mock: A mock for the _search_for_table method
        """
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="Current")
        timestamps_current = pa.array([0.01, 0.04, 0.06, 0.11, 0.15])
        current_values = pa.array([100, 200, 300, 400, 500])
        current_table = pa.Table.from_arrays(
            [timestamps_current, current_values], ["Timestamp", "Current"]
        )
        current_table = TestJoin.add_date(
            current_table, convert_start_date("2024-01-01T00:00:00")
        )
        current_table = current_table.drop("Timestamp")
        timestamps_voltage = pa.array([0.02, 0.03, 0.04, 0.08, 0.12])
        voltage_values = pa.array([4.2, 4.3, 4.4, 4.5, 4.6])
        voltage_table = pa.Table.from_arrays(
            [timestamps_voltage, voltage_values], ["Timestamp", "Voltage"]
        )
        voltage_table = TestJoin.add_date(
            voltage_table, convert_start_date("2024-01-01T00:00:00")
        )
        voltage_table = voltage_table.drop("Timestamp")
        search_for_table_mock.return_value = (current_table, [voltage_table])
        output_table = table_obj.join(tables=[current_table, voltage_table])
        expected_voltage_values = pa.array([None, 4.4, 4.4, 4.5, 4.6])
        self.assertEqual(
            expected_voltage_values.to_pylist(), output_table["Voltage"].to_pylist()
        )

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

    def test_outer_join(self) -> None:
        """Tests join method of Table class when join_on == 'ALL'"""
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="ALL")
        timestamps_current = pa.array([0.01, 0.04, 0.06, 0.11, 0.15])
        current_values = pa.array([100, 200, 300, 400, 500])
        current_table = pa.Table.from_arrays(
            [timestamps_current, current_values], ["Timestamp", "Current"]
        )
        current_table = TestJoin.add_date(
            current_table, convert_start_date("2024-01-01T00:00:00")
        )
        current_table = current_table.drop("Timestamp")
        timestamps_voltage = pa.array([0.02, 0.03, 0.04, 0.08, 0.12])
        voltage_values = pa.array([4.2, 4.3, 4.4, 4.5, 4.6])
        voltage_table = pa.Table.from_arrays(
            [timestamps_voltage, voltage_values], ["Timestamp", "Voltage"]
        )
        voltage_table = TestJoin.add_date(
            voltage_table, convert_start_date("2024-01-01T00:00:00")
        )
        voltage_table = voltage_table.drop("Timestamp")

        output_table = table_obj.join(tables=[current_table, voltage_table])
        expected_voltage_values = pa.array(
            [None, 4.2, 4.3, 4.4, 4.4, 4.5, 4.5, 4.6, 4.6]
        )
        self.assertEqual(
            expected_voltage_values.to_pylist(), output_table["Voltage"].to_pylist()
        )
        expected_current_values = pa.array(
            [100, 100, 100, 200, 300, 300, 400, 400, 500]
        )
        self.assertEqual(
            expected_current_values.to_pylist(), output_table["Current"].to_pylist()
        )


class TestCanToTable(unittest.TestCase):
    """Tests the can_to_table method of the Table class"""

    def setUp(self):
        """Creates the table_obj for the tests"""
        self.table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))

    def test_can_to_table(self):
        """Tests the can_to_table method with not a valid input"""
        temp_f, temp_path = mkstemp()
        with open(temp_f, mode="w", encoding="utf-8") as f:
            f.write('{"test","error"}')
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            self.table_obj.can_to_table(Path(temp_path))
        self.assertTrue("Input data are not valid" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)
        Path(temp_path).unlink()

    @patch("cli.cmd_etl.etl.table.Table._cast_columns")
    @patch("cli.cmd_etl.etl.table.Table._add_date")
    def test_can_to_table_valid_input(
        self, add_date_mock: Mock, cast_columns_mock: Mock
    ):
        """Tests the can_to_table method with valid input

        :param add_date_mock: A mock for the _add_date method
        :param cast_columns_mock: A mock for the _cast_columns_method
        """
        table_mock = Mock()
        drop_table_mock = Mock()
        table_mock.drop.return_value = drop_table_mock
        with patch(
            "cli.cmd_etl.etl.table.read_json", return_value=table_mock
        ) as read_json_mock:
            add_date_mock.return_value = table_mock
            self.table_obj.can_to_table(Path("test.txt"))
            read_json_mock.assert_called_once_with(Path("test.txt"))
            add_date_mock.assert_called_once_with(table_mock)
            table_mock.drop.assert_called_once_with("Timestamp")
            cast_columns_mock.assert_called_once_with(drop_table_mock)


class TestSaveData(unittest.TestCase):
    """Tests the save_data method of the Table class

    :param write_csv_mock: A mock for the write_csv function
    :param write_table_mock: A mock for the write_table function
    """

    def test_save_data_csv_output_format(self) -> None:
        """Tests the save_data method with csv as output format"""
        with io.StringIO() as buf, redirect_stdout(buf):
            table_mock = Mock()
            tables = {Path("test.txt"): table_mock}
            table_obj = Table(
                start_date=convert_start_date("2024-01-01T00:00:00"),
                output_format=get_output_format_enum("csv"),
            )
            table_obj.save_data(tables)
            table_mock.to_pandas().to_csv.assert_called_once_with(
                Path("test.csv"),
                sep=table_obj._separator,  # pylint: disable=protected-access
                index=False,
                date_format="%Y-%m-%d %H:%M:%S.%f%z",
            )

    @patch("cli.cmd_etl.etl.table.write_table")
    def test_save_data_parquet_output_format(
        self,
        write_table_mock: Mock,
    ) -> None:
        """Tests the save_data method with parquet as output format"""
        tables = {Path("test.txt"): "test_table"}
        with io.StringIO() as buf, redirect_stdout(buf):
            table_obj = Table(
                start_date=convert_start_date("2024-01-01T00:00:00"),
                output_format=get_output_format_enum("parquet"),
            )
            table_obj.save_data(tables)
            write_table_mock.assert_called_once_with("test_table", Path("test.parquet"))

    def test_save_data_suffix_csv(self) -> None:
        """Tests the save_data method with csv as output format defined by suffix"""
        table_mock = Mock()
        tables = {Path("test.csv"): table_mock}
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        table_obj.save_data(tables)
        table_mock.to_pandas().to_csv.assert_called_once_with(
            Path("test.csv"),
            sep=table_obj._separator,  # pylint: disable=protected-access
            index=False,
            date_format="%Y-%m-%d %H:%M:%S.%f%z",
        )

    @patch("cli.cmd_etl.etl.table.write_table")
    def test_save_data_suffix_parquet(self, write_table_mock: Mock) -> None:
        """Tests the save_data method with parquet as output format defined by suffix"""
        tables = {Path("test.parquet"): "test_table"}
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        table_obj.save_data(tables)
        write_table_mock.assert_called_once_with("test_table", Path("test.parquet"))

    def test_save_data_more_than_one_table(self, *_: list[Mock]) -> None:
        """Tests the save_data method with exception handling in case that
        more than one table should be saved in one file
        """
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

    def test_save_data_permission_denied(self) -> None:
        """Tests the save_data method with exception handling in case the
        permission to write a file is denied
        """
        table_mock = Mock()
        table_mock.to_pandas().to_csv.side_effect = PermissionError
        tables = {Path("test.csv"): table_mock}
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
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


class TestSearchForTable(unittest.TestCase):
    """Tests the search_for_table method of the Table class"""

    def setUp(self):
        """Creates the tables for the tests"""
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
        self.tables = [current_table, voltage_table]

    def test_search_for_table_table_found(self) -> None:
        """Tests the search_for_table method with table found"""
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="Current")
        join_table, remaining_tables = table_obj._search_for_table(self.tables)  # pylint: disable=protected-access
        self.assertTrue("Current" in join_table.column_names)
        self.assertTrue("Voltage" in remaining_tables[0].column_names)

    def test_search_for_table_table_not_found(self) -> None:
        """Tests the search_for_table method with table not found"""
        table_obj = Table(start_date="2024-01-01T00:00:00", join_on="Date")
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            _ = table_obj._search_for_table(self.tables)  # pylint: disable=protected-access
        self.assertTrue("Column 'Date' not found in any table." in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)


class TestAddDate(unittest.TestCase):
    """Tests the add_date method of the Table class"""

    def test_add_date(self) -> None:
        """Tests the add_date method with valid input"""
        timestamps = pa.array([0.01, 0.04, 0.06])
        test_table = pa.Table.from_arrays([timestamps], ["Timestamp"])
        table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))
        result_table = table_obj._add_date(test_table)  # pylint: disable=protected-access
        expected_date_column = [
            datetime(2024, 1, 1, 0, 0, 0, 10000, tzinfo=UTC),
            datetime(2024, 1, 1, 0, 0, 0, 40000, tzinfo=UTC),
            datetime(2024, 1, 1, 0, 0, 0, 60000, tzinfo=UTC),
        ]
        result_as_list = result_table["Date"].to_pylist()
        # The time zone must be tested with an additional assert statement,
        # as assertEqual does not check for different time zones when
        # comparing two dates.
        self.assertEqual(result_as_list[0].tzinfo, pytz.UTC)
        self.assertEqual(expected_date_column, result_as_list)


class TestCastColumns(unittest.TestCase):
    """Tests the cast_columns method of the Table class"""

    def setUp(self):
        """Creates the table object for the tests"""
        self.table_obj = Table(start_date=convert_start_date("2024-01-01T00:00:00"))

    def test_cast_columns_date(self) -> None:
        """Tests the cast_columns method with a date column"""
        dates = pa.array([0.01, 0.04, 0.06])
        test_table = pa.Table.from_arrays([dates], ["Date"])
        casted_table = self.table_obj._cast_columns(test_table)  # pylint: disable=protected-access
        self.assertEqual(casted_table, test_table)

    def test_cast_columns_unit(self) -> None:
        """Tests the cast_columns method with a column containing values with a unit"""
        timestamps = pa.array([0.01, 0.04, 0.06])
        test_table = pa.Table.from_arrays([timestamps], ["Timestamp_s"])
        casted_table = self.table_obj._cast_columns(test_table)  # pylint: disable=protected-access
        self.assertTrue(casted_table["Timestamp_s"].type == pa.float32())

    def test_cast_columns_cast_not_possible(self) -> None:
        """Tests the cast_columns method with a column containing values that
        cannot be casted
        """
        timestamps = pa.array([20, 30, 42949672940])
        test_table = pa.Table.from_arrays([timestamps], ["Timestamp_s"])
        with self.assertLogs(level=logging.INFO) as al:
            self.table_obj._cast_columns(test_table)  # pylint: disable=protected-access
        self.assertTrue("cannot be converted to float32" in al.output[0])


if __name__ == "__main__":
    unittest.main()
