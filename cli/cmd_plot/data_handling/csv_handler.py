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

"""Implementation of the CSVHandler class to take given data and return it in a different format."""

import sys
import tempfile
from pathlib import Path

import pandas
from jsonschema import exceptions, validate
from pyarrow.lib import ArrowInvalid  # pylint: disable=no-name-in-module
from yaml import safe_load

from ...helpers.click_helpers import recho
from .csv_handler_interface import CSVHandlerInterface


class CSVHandler(CSVHandlerInterface):  # pylint: disable=too-few-public-methods
    """Class that implements the interface CSVHandler"""

    def __init__(self, columns: dict[str, str], skip: int, precision: int) -> None:
        self.skip = skip
        self.precision = precision
        self.columns = columns

    def get_data(self, file_path: Path) -> pandas.DataFrame:
        """Read the given file and returns the contained data."""
        try:
            return self._get_data(file_path)
        except ArrowInvalid as e:
            recho(f"Parquet Error: {e}")
            sys.exit(1)
        except (ValueError, TypeError) as e:
            if "do not match columns" in str(e):
                not_found_columns = str(e).split(":")[1]
                recho(
                    "Error in data config file: Skip value removed header or "
                    f"desired columns {not_found_columns} not found in CSV file."
                )
            else:
                recho(f"Error in data config file: {str(e)}")
            sys.exit(1)

    def _get_data(self, file_path: Path) -> pandas.DataFrame:
        """Private function to read the given file and return the
        contained data without logical error handling"""
        # Check whether the data has already been saved as a parquet file
        parquet_file_path = self._check_for_tmp_data(file_path)
        if parquet_file_path:
            data = pandas.read_parquet(parquet_file_path, engine="pyarrow")
        else:
            # get specific column with datetime type and set values to string
            # for later dict merge
            datetime_columns = {
                x: "string" for x in self.columns if self.columns[x] == "datetime"
            }
            self.columns = self.columns | datetime_columns
            data = pandas.read_csv(
                file_path,
                usecols=list(self.columns.keys()),
                dtype=self.columns,
                skiprows=self.skip,
                parse_dates=list(datetime_columns.keys()),
            )
            data = data.round(self.precision)
            tmp_dir = CSVHandler._check_for_tmp_directory(file_path=file_path)
            if tmp_dir is None:
                tmp_dir = CSVHandler._create_tmp_directory(file_path)
            data.to_parquet(
                tmp_dir / Path(file_path.stem + ".parquet"), engine="pyarrow"
            )
        return data

    @staticmethod
    def validate_config(config: dict):
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

    @staticmethod
    def _check_for_tmp_data(file_path: Path) -> Path | None:
        """Check whether there the data is already saved in the temporary directory."""
        tmp_dir = CSVHandler._check_for_tmp_directory(file_path=file_path)
        if tmp_dir:
            files = sorted(Path(tmp_dir).glob("*.parquet"))
            for parquet in files:
                if parquet.stem == file_path.stem:
                    return parquet
        return None

    @staticmethod
    def _check_for_tmp_directory(file_path: Path):
        """Check whether there is a temporary directory to save the data in."""
        dirs = [d for d in file_path.parent.iterdir() if d.is_dir()]
        for d in dirs:
            if "temp_data_foxplot_" in d.name:
                return d
        return None

    @staticmethod
    def _create_tmp_directory(file_path: Path):
        """Create a temporary directory to save the data in."""
        temp_dir = tempfile.mkdtemp(prefix="temp_data_foxplot_", dir=file_path.parent)
        return temp_dir
