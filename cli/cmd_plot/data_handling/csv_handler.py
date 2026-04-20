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

"""Implementation of the CSVHandler class which enables reading data from a
.csv file
"""

import sys
from collections.abc import Hashable, Mapping
from pathlib import Path

import pandas as pd
from jsonschema import exceptions, validate
from yaml import safe_load

from ...helpers.click_helpers import recho
from .handler_interface import DataHandlerInterface


class CSVHandler(DataHandlerInterface):
    """Implementation of the CSVHandler"""

    def __init__(
        self, columns: dict[str, str], skip: int, precision: int, na_value: str = "NULL"
    ) -> None:
        """Creates the CSVHandler object"""
        self.columns = columns
        self.skip = skip
        self.precision = precision
        self.na_value = na_value

    def get_data(self, file_path: Path, no_tmp: bool = True) -> pd.DataFrame:
        """Read the given file and returns the contained data."""
        try:
            return self._get_data(file_path, no_tmp)
        except (ValueError, TypeError) as e:
            if "do not match columns" in str(e):
                not_found_columns = str(e).split(":")[1].strip()
                recho(
                    "Error in data config file: Skip value removed header or "
                    f"desired columns {not_found_columns} not found in CSV file."
                )
            else:
                recho(f"Error in data config file: {e!s}")
        except OSError as e:
            recho(f"Can not access file: {e}")
        sys.exit(1)

    def _get_data(self, file_path: Path, no_tmp: bool) -> pd.DataFrame:
        """Private function to read the given file and return the
        contained data without logical error handling
        """
        data = CSVHandler.get_tmp_data(file_path, no_tmp)
        if data is not None:
            return data
        # get specific column with datetime type and set values to string
        # for later dict merge
        datetime_columns = {
            x: "string" for x in self.columns if self.columns[x] == "datetime"
        }
        string_columns = {
            x: "string" for x in self.columns if self.columns[x] == "string"
        }
        self.columns = self.columns | datetime_columns
        # Int64 data type supports NaN values in contrary to numpy's int64
        corrected_columns: Mapping[Hashable, str] = {
            k: (v if v != "int" else "Int64") for (k, v) in self.columns.items()
        }
        data = pd.read_csv(
            file_path,
            usecols=list(self.columns.keys()),
            dtype=corrected_columns,
            skiprows=self.skip,
            parse_dates=list(datetime_columns.keys()),
            na_values=self.na_value,
        )
        # Empty field are read as NaN values, which cause problems with
        # string columns during plotting
        for string_column in string_columns:
            data[string_column] = data[string_column].fillna(self.na_value)
        data = data.round(self.precision)
        CSVHandler.write_tmp_file(data, file_path)
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
