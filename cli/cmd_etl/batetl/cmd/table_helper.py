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

"""Table subcommand implementation"""

import sys
from pathlib import Path

import click
import pyarrow as pa
import pyarrow.compute as pc

from ..etl.table import OutputFormats, Table
from . import read_config


def table_setup(config_path: Path) -> Table:
    """Reads config file and creates the Table object

    :param config: Path to the configuration file
    :return: Table object
    """
    config_dict = read_config(config_path)
    validate_table_config(config_dict)
    converted_start_date = convert_start_date(config_dict["start_date"])
    output_format = (
        None
        if config_dict.get("output_format", None) is None
        else get_output_format_enum(config_dict["output_format"])
    )
    join_on = config_dict.get("join_on", None)
    tolerance = config_dict.get("tolerance", -100000)
    timestamp_factor = config_dict.get("timestamp_factor", 1000000)
    return Table(
        converted_start_date,
        output_format,
        join_on,
        tolerance=tolerance,
        timestamp_factor=timestamp_factor,
    )


def convert_start_date(start_date: str) -> pa.TimestampScalar:
    """This method converts a string in UTC format to a pyarrow
    TimestampScalar

    :param start_date: Start date of the measurement as UTC string
    :return: Converted start date
    """
    try:
        return pc.strptime(start_date, format="%Y-%m-%dT%H:%M:%S", unit="us")
    except ValueError:
        click.secho(
            "Passed start_date is not in the correct UTC format.", fg="red", err=True
        )
        sys.exit(1)


def get_output_format_enum(output_format: str) -> OutputFormats:
    """Returns the enum value for the passed output format

    :param output_format: The output format as string (csv, parquet)
    :return: The OutputFormats enum value of the passed output format
    """
    try:
        return OutputFormats[output_format.upper()]
    except KeyError:
        click.secho("Output format %s is not valid", fg="red", err=True)
        sys.exit(1)


def validate_table_config(config: dict) -> None:
    """Validates the configuration file of the table subcommand

    :param config_dict: Dictionary with configurations to validate
    """
    if "start_date" not in config:
        click.secho(
            "Configuration file is missing 'start_date' parameter.", fg="red", err=True
        )
        sys.exit(1)
    if not isinstance(config["start_date"], str):
        click.secho(
            "'start_date' in the configuration file is not a string.",
            fg="red",
            err=True,
        )
        sys.exit(1)
    if "output_format" in config:
        if not isinstance(config["output_format"], str):
            click.secho(
                "'output_format' in the configuration file is not a string.",
                fg="red",
                err=True,
            )
            sys.exit(1)
    if "join_on" in config:
        if not isinstance(config["join_on"], str):
            click.secho(
                "'join_on' in the configuration file is not a string.",
                fg="red",
                err=True,
            )
            sys.exit(1)
    if "tolerance" in config:
        if not isinstance(config["tolerance"], int):
            click.secho(
                "'tolerance' in the configuration file is not an integer.",
                fg="red",
                err=True,
            )
            sys.exit(1)
    if "timestamp_factor" in config:
        if not isinstance(config["timestamp_factor"], int):
            click.secho(
                "'timestamp_factor' in the configuration file is not an integer.",
                fg="red",
                err=True,
            )
            sys.exit(1)


def run_table(table: Table, data: Path, output: Path) -> None:
    """Executes the table creation step

    :param table: Object which handles the creation
        of the table
    :param data: Path to the input data
    :param output: Path to the file/directory where the output should be stored.
    """
    if data.suffix:
        if output.suffix:
            tb = table.can_to_table(data)
            table.save_data({output: tb})
        else:
            click.secho(
                "Output as folder and input data as a file is not a valid configuration.",
                fg="red",
                err=True,
            )
            sys.exit(1)
    else:
        data_files = list(data.glob("**/*.*"))
        tables = [table.can_to_table(data=data_file) for data_file in data_files]
        if output.suffix:
            # Combines multiple tables with respect to join_on date column
            tb = table.join(tables=tables)
            table.save_data({output: tb})
        else:
            # Save each table in a separate file
            output.mkdir(parents=True, exist_ok=True)
            new_data_files = [output / data_file.name for data_file in data_files]
            table.save_data(dict(zip(new_data_files, tables)))
