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

"""Implementation of the CSVHandler class which enables reading data from a
.csv file"""

import sys
from pathlib import Path

import pandas
from jsonschema import exceptions, validate
from pyarrow.lib import ArrowInvalid  # pylint: disable=no-name-in-module
from yaml import safe_load

from ...helpers.click_helpers import recho
from ...helpers.file_tracker import FileTracker
from ...helpers.tmp_handler import TmpHandler
from .csv_handler_interface import CSVHandlerInterface


class CSVHandler(CSVHandlerInterface):  # pylint: disable=too-few-public-methods
    """Class that implements the interface CSVHandler"""

    def __init__(self, columns: dict[str, str], skip: int, precision: int) -> None:
        """Creates the CSVHandler object"""
        self.columns = columns
        self.skip = skip
        self.precision = precision

    def get_data(self, file_path: Path, no_tmp: bool = True) -> pandas.DataFrame:
        """Read the given file and returns the contained data."""
        try:
            return self._get_data(file_path, no_tmp)
        except ArrowInvalid as e:
            recho(f"Parquet Error: {e}")
            sys.exit(1)
        except (ValueError, TypeError) as e:
            if "do not match columns" in str(e):
                not_found_columns = str(e).split(":")[1].strip()
                recho(
                    "Error in data config file: Skip value removed header or "
                    f"desired columns {not_found_columns} not found in CSV file."
                )
            else:
                recho(f"Error in data config file: {str(e)}")
            sys.exit(1)

    def _get_data(self, file_path: Path, no_tmp: bool) -> pandas.DataFrame:
        """Private function to read the given file and return the
        contained data without logical error handling"""
        tmp_handler = TmpHandler(file_path.parent)
        file_tracker = FileTracker(tmp_handler.tmp_dir)
        # Check whether the data has already been saved as a parquet file
        parquet_file_path = tmp_handler.check_for_tmp_file(file_path, "parquet")
        data_file_changed = file_tracker.check_file_changed(file_path)
        if parquet_file_path is not None and not data_file_changed and not no_tmp:
            data = pandas.read_parquet(parquet_file_path, engine="pyarrow")
        else:
            # get specific column with datetime type and set values to string
            # for later dict merge
            datetime_columns = {
                x: "string" for x in self.columns if self.columns[x] == "datetime"
            }
            string_columns = {
                x: "string" for x in self.columns if self.columns[x] == "string"
            }
            self.columns = self.columns | datetime_columns
            # Int64 data type supports NaN values in contrary to numpys int64
            self.columns = {
                k: (v if v != "int" else "Int64") for (k, v) in self.columns.items()
            }
            data = pandas.read_csv(
                file_path,
                usecols=list(self.columns.keys()),
                dtype=self.columns,
                skiprows=self.skip,
                parse_dates=list(datetime_columns.keys()),
            )
            # Emptfy field are read as NaN values, which cause problems with
            # string columns during plotting
            for string_column in string_columns:
                data[string_column] = data[string_column].fillna("NULL")
            data = data.round(self.precision)
            parquet_file_name = tmp_handler.get_hash_name(file_path, "parquet")
            data.to_parquet((tmp_handler.tmp_dir / parquet_file_name), engine="pyarrow")
        return data

    @staticmethod
    def validate_config(config: dict) -> None:
        """Validates the CSVHandler configuration"""
        schema_path = Path(__file__).parent / "schemas" / "csv_handler.json"
        with open(schema_path, encoding="utf-8") as f:
            schema = safe_load(f)
        try:
            validate(config, schema=schema)
        except exceptions.ValidationError as e:
            error_text = str(e).splitlines()[0]
            recho(f"CSV handler validation error: {error_text}")
            sys.exit(1)
