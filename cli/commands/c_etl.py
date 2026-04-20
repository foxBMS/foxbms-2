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

"""Click commands for ETL conversion, filtering, decoding, and tabulation."""

from pathlib import Path

import click

from ..cmd_etl.cmds import get_config_file_options
from ..cmd_etl.cmds.can_decode_helper import (
    can_decode_setup,
    run_decode2file,
    run_decode2stdout,
)
from ..cmd_etl.cmds.can_filter_helper import can_filter_setup, run_filter
from ..cmd_etl.cmds.convert_helper import converter_setup, run_converter
from ..cmd_etl.cmds.table_helper import run_table, table_setup
from ..cmd_etl.etl.convert import InputFormats, OutputFormats
from ..helpers.click_helpers import HELP_NAMES, echo, verbosity_option

DEFAULT_CONFIG_FILE_FILTER = Path("filter.yml")
DEFAULT_CONFIG_FILE_DECODE = Path("decode.yml")


def _output_format_cb(
    _ctx: click.Context, _param: click.Parameter | None, value: str
) -> OutputFormats:
    """Convert Click option string values to :class:`OutputFormats`."""
    return OutputFormats[value.upper()]


@click.command("gamry")
@click.argument(
    "data",
    nargs=1,
    type=click.Path(exists=True, file_okay=True, dir_okay=True, path_type=Path),
)
@click.option(
    "-s",
    "--skip_footer",
    type=int,
    default=0,
    required=False,
    help="Skips a number of lines at the end (footer) of a .dta file.",
)
@click.pass_context
def cmd_convert_gamry(ctx: click.Context, data: Path, skip_footer: int) -> None:
    """Convert Gamry DTA files to CSV or PARQUET output."""
    parent_config = ctx.parent.config  # type: ignore[union-attr]
    config = {
        "data_path": data,
        "recursive": parent_config["recursive"],
        "conversion": {
            "input_format": InputFormats.GAMRY,
            "output_format": parent_config["output_format"],
            "additional": {"skip_footer": skip_footer},
        },
    }
    converter_obj = converter_setup(config)
    run_converter(converter_obj)
    ctx.exit(0)


@click.command("graphtec")
@click.argument(
    "data",
    nargs=1,
    type=click.Path(exists=True, file_okay=True, dir_okay=True, path_type=Path),
)
@click.option(
    "-s",
    "--skip",
    type=int,
    default=32,
    required=False,
    help="Skips a specific number of lines at the beginning of a file.",
)
@click.pass_context
def cmd_convert_graphtec(ctx: click.Context, data: Path, skip: int) -> None:
    """Convert GRAPHTEC CSV files to normalized output formats."""
    parent_config = ctx.parent.config  # type: ignore[union-attr]
    config = {
        "data_path": data,
        "recursive": parent_config["recursive"],
        "conversion": {
            "input_format": InputFormats.GRAPHTEC,
            "output_format": parent_config["output_format"],
            "additional": {"skip": skip},
        },
    }
    converter_obj = converter_setup(config)
    run_converter(converter_obj)
    ctx.exit(0)


@click.group(name="convert", context_settings=HELP_NAMES)
@click.option(
    "-r",
    "--recursive",
    type=bool,
    is_flag=True,
    default=False,
    required=False,
    help="Enables recursive file search in a directory.",
)
@click.option(
    "-o",
    "--output-format",
    type=click.Choice([fmt.name for fmt in OutputFormats], case_sensitive=False),
    default=OutputFormats.CSV.name,
    callback=_output_format_cb,
    required=False,
    help="Defines the target file format.",
)
@verbosity_option
@click.pass_context
def cmd_convert(
    ctx: click.Context, recursive: bool, output_format: OutputFormats, verbose: int = 0
) -> None:
    """Configure conversion defaults for ETL convert subcommands."""
    ctx.config = {  # type: ignore[attr-defined]
        "recursive": recursive,
        "output_format": output_format,
    }


cmd_convert.add_command(cmd_convert_gamry)
cmd_convert.add_command(cmd_convert_graphtec)


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
@verbosity_option
@click.pass_context
def cmd_filter(  # pylint: disable=too-many-arguments,too-many-positional-arguments
    ctx: click.Context,
    config_file: Path | None = None,  # pylint: disable=unused-argument; used in the callback
    _input: Path | None = None,
    output: Path | None = None,
    id_pos: int = -1,
    ids: list[str] | None = None,
    sampling: list[tuple[str, int]] | None = None,
    verbose: int = 0,
) -> None:
    """Filter out unwanted CAN messages from input stream or file.

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
    # is no configuration file and no options have been provided on the
    # command line
    if config == {
        "id_pos": -1,
        "ids": [],
        "sampling": {},
        "_input": None,
        "output": None,
    }:
        echo(ctx.get_help())
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
@verbosity_option
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
    verbose: int = 0,
) -> None:
    """Decode CAN messages from standard input.

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
    if (
        config["dbc"] is None
        and config["timestamp_pos"] == -1
        and config["id_pos"] == -1
        and config["data_pos"] == -1
    ):
        echo(ctx.get_help())
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
@verbosity_option
@click.pass_context
def cmd_table(
    ctx: click.Context, data: Path, config: Path, output: Path, verbose: int = 0
) -> None:
    """Convert files with decoded CAN message (JSON) to one or multiple tables.

    The input is either one file with decoded CAN messages or a directory
    containing files with decoded CAN messages. The output of the table
    command depends on the use-case, which is defined by the configuration
    file and whether the input or output is a file or directory. The use-cases are

    One to One: etl table -c config.yml -o output_file.csv input_file.json

    Many to One: etl table -c config.yml -o output_file.csv input_directory

    Many to Many: etl table -c config.yml -o output_directory input_directory
    """
    table_obj = table_setup(config)
    run_table(table_obj, data, output)
    ctx.exit(0)


@click.group(context_settings=HELP_NAMES)
def etl() -> None:
    """ETL command group entry point.

    These scripts and tools will simplify the collection of
    foxBMS 2 data and their analysis.
    """


etl.add_command(cmd_filter)
etl.add_command(cmd_decode)
etl.add_command(cmd_table)
etl.add_command(cmd_convert)
