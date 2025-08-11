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

"""Combine decoded CAN messages to a table"""

import logging
import sys
from enum import Enum
from pathlib import Path

import pyarrow as pa  # type: ignore
import pyarrow.compute as pc  # type: ignore
from pyarrow.csv import write_csv  # type: ignore
from pyarrow.json import read_json  # type: ignore
from pyarrow.lib import ArrowInvalid  # type: ignore # pylint: disable=no-name-in-module
from pyarrow.parquet import write_table  # type: ignore

from cli.helpers.click_helpers import echo, recho


class OutputFormats(Enum):
    """Definition of all supported output formats"""

    CSV = 0
    PARQUET = 1


class Table:
    """Implements the functionalities to create tables out of the
    decoded CAN messages.

    :param _start_date: The start date of the CAN measurement.
    :param _output_format: Only support output formats are CSV and
        PARQUET
    :param _join_on: All CAN message are join based on the
        Date column of this specified column
    """

    def __init__(
        self,
        start_date: pa.TimestampScalar,
        output_format: OutputFormats | None = None,
        join_on: str | None = None,
        **kwargs,
    ) -> None:
        self._start_date = start_date
        self._output_format = output_format
        self._join_on = join_on
        self._tolerance = kwargs.get("tolerance", -100000)
        self._timestamp_factor = kwargs.get("timestamp_factor", 1000000)

    def join(self, tables: list[pa.Table]) -> pa.Table:
        """The method join takes a list of tables and joins these tables based
        on the Date column of the table containing the specified join_on
        column.

        :param tables: A list of tables containing each a Date column.
        :return: A table containing only one Date column and all other columns
            of the passed tables.
        """
        if self._join_on is not None:
            left_table, right_tables = self._search_for_table(tables)
            for right_table in right_tables:
                left_table = left_table.join_asof(
                    right_table,
                    on="Date",
                    by=[],
                    right_on="Date",
                    tolerance=self._tolerance,
                )
            return left_table
        recho("Combining several tables without 'join_on' parameter is not possible.")
        sys.exit(1)

    def can_to_table(self, data: Path) -> pa.Table:
        """The can_to_table method converts CAN messages stored as json object
        to a pyarrow table

        :param data: Path to the .json file with the CAN messages.
        :return: Pyarrow table created from the CAN messages.
        """
        try:
            tb = read_json(data)
            tb = self._add_date(tb)
            # Column Timestamp has to be dropped otherwise asof_join will raise
            # an exception that both tables contain a Timestamp column
            tb = tb.drop("Timestamp")
            return Table._cast_columns(tb)
        except ArrowInvalid as e:
            recho(f"Input data are not valid, because of '{e}'")
            sys.exit(1)

    def save_data(self, tables: dict[Path, pa.table]) -> None:
        """The save_data method saves all passed tables with respect
        to the configuration.

        :param tables: A dictionary with the key as save path for the table
            with name but without suffix and the value as the table object.
        """
        try:
            if self._output_format:
                match self._output_format:
                    case OutputFormats.CSV:
                        method = write_csv
                        suffix = ".csv"
                    case _:  # parquet format as default
                        method = write_table
                        suffix = ".parquet"
                for file_path, table in tables.items():
                    echo(
                        f"Converted file {file_path} to table with output "
                        f"format {self._output_format.name}.",
                    )
                    file_path.parent.mkdir(parents=True, exist_ok=True)
                    new_file_path = file_path.parent / Path(file_path.stem + suffix)
                    method(table, new_file_path)
            else:
                if len(tables) > 1:
                    recho(
                        "Save more than one table without an output format is not "
                        "possible or did you forget to specify a file name for the "
                        "output parameter."
                    )
                    sys.exit(1)
                file_path = list(tables.keys())[0]
                match OutputFormats[file_path.suffix[1:].upper()]:
                    case OutputFormats.CSV:
                        method = write_csv
                    case _:  # parquet format as default
                        method = write_table
                file_path.parent.mkdir(parents=True, exist_ok=True)
                method(tables[file_path], file_path)
        except PermissionError:
            recho("Could not write output file, because permission is denied.")
            sys.exit(1)

    def _search_for_table(
        self, tables: list[pa.table]
    ) -> tuple[pa.table, list[pa.table]]:
        """The method _search_for_table searches in a list of tables the first
        table containing the join_on column.

        :param tables: A list of tables where should be searched
        :return: A tuple with the join_on column and the remaining tables
        """
        for table in tables:
            if self._join_on in table.column_names:
                tables.remove(table)
                return table, tables
        recho(f"Column '{self._join_on}' not found in any table.")
        sys.exit(1)

    def _add_date(self, table: pa.Table) -> pa.Table:
        """The method _add_date adds a Date column based
        on the Timestamp column and the passed start date.

        :param table: The table where the Date column should be added.
        :return: New table with Date column

        """
        duration = (
            # The timestamps in a CAN log are converted to a duration in
            # microseconds, therefore the factor 1000000.
            pc.multiply(table.column("Timestamp"), self._timestamp_factor)
            .cast(
                options=pc.CastOptions(
                    target_type=pa.int64(), allow_float_truncate=True
                )
            )
            .cast(pa.duration("us"))
        )
        date_column = pc.add(self._start_date, duration)
        return table.add_column(0, "Date", date_column)

    @staticmethod
    def _cast_columns(table: pa.Table) -> pa.Table:
        """The _cast_columns method converts all columns with units at the end of the column name
        to float32.

        :param table: The table with columns to be casted
        :return: The new table with teh casted columns
        """
        for i, column in enumerate(table.column_names):
            if "Date" in column or "None" in column:
                continue
            try:
                table = table.set_column(
                    i, column, table.column(column).cast(pa.float32())
                )
            except ArrowInvalid as e:
                # replace measurement errors with null values
                table = table.set_column(
                    i,
                    column,
                    pc.if_else(
                        pc.greater_equal(pc.abs(table.column(column)), 2e24),
                        None,
                        table.column(column),
                    ),
                )
                logging.info(
                    "Values in column %s can't be converted to float32, because of '%s'",
                    column,
                    e,
                )
        return table
