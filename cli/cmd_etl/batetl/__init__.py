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

"""Implements the functionalities behind the 'etl' commands"""

from pathlib import Path

import click

from ...helpers.click_helpers import echo
from .cmd import read_config
from .cmd.can_decode_helper import can_decode_setup, run_decode2file, run_decode2stdout
from .cmd.can_filter_helper import can_filter_setup, run_filter
from .cmd.table_helper import run_table, table_setup

DEFAULT_CONFIG_FILE_FILTER = Path("filter.yml")
DEFAULT_CONFIG_FILE_DECODE = Path("decode.yml")


def get_config_file_options(
    ctx: click.Context, _param: click.Parameter | None, config_file: Path
):
    """Read configuration options from a configuration file, if present."""
    options: dict = {}
    if config_file.is_file():
        if config_file in (DEFAULT_CONFIG_FILE_DECODE, DEFAULT_CONFIG_FILE_FILTER):
            echo(f"Default configuration file {config_file} is used.")
        options = read_config(config_file)

    # we need to fix parsing the 'filter:sampling' setup
    if sampling := options.get("sampling", None):
        options["sampling"] = sampling.items()

    ctx.default_map = options


@click.command("filter")
@click.option(
    "-c",
    "--config",
    type=click.Path(file_okay=True, dir_okay=False, path_type=Path),
    default=DEFAULT_CONFIG_FILE_FILTER,
    callback=get_config_file_options,
    is_eager=True,
    expose_value=False,
    help="A configuration file (YML) to define the filter setup",
)
@click.argument(
    "_input",
    metavar="INPUT",
    required=False,
    type=click.Path(file_okay=True, dir_okay=False, path_type=Path, exists=True),
    default=None,
)
@click.option(
    "-o",
    "--output",
    required=False,
    type=click.Path(file_okay=True, dir_okay=False, path_type=Path, exists=False),
    default=None,
    help="Stores the filter output to a file; otherwise stdout is used",
)
@click.option(
    "-ip",
    "--id-pos",
    type=int,
    default=-1,
    help="Index of the CAN ID column in the log file",
)
@click.option("-i", "--ids", multiple=True, help="IDs to be filtered")
@click.option(
    "-s",
    "--sampling",
    required=False,
    nargs=2,
    type=click.Tuple([str, int]),
    multiple=True,
)
@click.pass_context
def cmd_filter(  # pylint: disable=too-many-arguments,too-many-positional-arguments
    ctx: click.Context,
    config_file: Path | None = None,  # pylint: disable=unused-argument; used in the callback
    _input: Path | None = None,
    output: Path | None = None,
    id_pos: int = -1,
    ids: list[str] | None = None,
    sampling: list[tuple[str, int]] | None = None,
) -> None:
    """Filter out unwanted CAN messages read from either standard input or the provided file.
    The subcommand writes the filtered CAN messages to standard output or to a
    file.
    """
    if not ids:
        ids = []
    if not sampling:
        sampling = []
    config = {
        "_input": _input,
        "output": output,
        "id_pos": id_pos,
        "ids": list(ids),
        "sampling": dict(sampling),
    }
    # we need to catch th case that no argument has been provided, i.e., there
    # is not configuration file and no options have been provided on the
    # command line
    if config == {
        "id_pos": -1,
        "ids": [],
        "sampling": {},
        "_input": None,
        "output": None,
    }:
        print(ctx.get_help())
        ctx.exit(0)
    filter_obj = can_filter_setup(config)
    run_filter(filter_obj)
    ctx.exit(0)


@click.command("decode")
@click.option(
    "-c",
    "--config",
    type=click.Path(file_okay=True, dir_okay=False, path_type=Path),
    default=DEFAULT_CONFIG_FILE_DECODE,
    callback=get_config_file_options,
    is_eager=True,
    expose_value=False,
    help="A configuration file (YML) to define the decoding parameters",
)
@click.option(
    "-d",
    "--dbc",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    help="Path to the DBC file",
)
@click.option(
    "-tp",
    "--timestamp-pos",
    type=int,
    default=-1,
    help="Index of the timestamp column in the log file",
)
@click.option(
    "-ip",
    "--id-pos",
    type=int,
    default=-1,
    help="Index of the CAN ID column in the log file",
)
@click.option(
    "-dp",
    "--data-pos",
    type=int,
    default=-1,
    help="Index of the data column in the log file",
)
@click.option(
    "-o",
    "--output",
    type=click.Path(exists=False, file_okay=False, dir_okay=True, path_type=Path),
    required=False,
    help="Directory in which the files with decoded CAN messages are saved",
)
@click.pass_context
# pylint: disable-next=too-many-arguments,too-many-positional-arguments
def cmd_decode(
    ctx: click.Context,
    config_file: Path | None = None,  # pylint: disable=unused-argument; used in the callback
    dbc: Path | None = None,
    timestamp_pos: int = -1,
    id_pos: int = -1,
    data_pos: int = -1,
    output: Path | None = None,
) -> None:
    """Decode CAN message from the standard input.
    Decoded CAN messages are saved in separate files (JSON) in the output
    directory.
    """
    config = {
        "dbc": dbc,
        "timestamp_pos": timestamp_pos,
        "id_pos": id_pos,
        "data_pos": data_pos,
        "output": output,
    }
    # we need to catch th case that no argument has been provided, i.e., there
    # is not configuration file and no options have been provided on the
    # command line
    if config == {
        "dbc": None,
        "timestamp_pos": -1,
        "id_pos": -1,
        "data_pos": -1,
        "output": None,
    }:
        print(ctx.get_help())
        ctx.exit(0)

    decode_obj = can_decode_setup(config)
    if output:
        run_decode2file(decode_obj)
    else:
        run_decode2stdout(decode_obj)
    ctx.exit(0)


@click.command("table")
@click.argument(
    "data",
    nargs=1,
    type=click.Path(exists=True, file_okay=True, dir_okay=True, path_type=Path),
)
@click.option(
    "-c",
    "--config",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="A configuration file to define the conversion of "
    "decoded data (JSON) to a table",
)
@click.option(
    "-o",
    "--output",
    type=click.Path(exists=False, file_okay=True, dir_okay=True, path_type=Path),
    required=True,
    help="Directory or file in which the table(s) should be saved",
)
@click.pass_context
def cmd_table(ctx: click.Context, data: Path, config: Path, output: Path) -> None:
    """Convert files with decoded CAN message (JSON) to one or multiple tables.
    The input is either one file with decoded CAN messages or a directory
    containing files with decoded CAN messages. The output of the table
    command depends on the use-case, which is defined by the configuration
    file and whether the input or output is a file or folder. The use-cases are

    One to One: etl table -c config.yml -o output_file.csv input_file.json

    Many to One: etl table -c config.yml -o output_file.csv input_folder

    Many to Many: etl table -c config.yml -o output_folder input_folder
    """
    table_obj = table_setup(config)
    run_table(table_obj, data, output)
    ctx.exit(0)


@click.group(context_settings={"help_option_names": ["-h", "--help"]})
def entry_point() -> None:
    """Extract Transform Load (ETL) functionalities via command line.

    These scripts and tools will simplify the collection of
    foxBMS 2 data and their analysis.
    """


def main() -> None:
    """Initiate the entry point of batetl"""
    entry_point.add_command(cmd_filter)
    entry_point.add_command(cmd_decode)
    entry_point.add_command(cmd_table)
    entry_point()


if __name__ == "__main__":
    main()
