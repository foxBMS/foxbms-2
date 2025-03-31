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

from .cmd.can_decode_helper import can_decode_setup, run_decode2file, run_decode2stdout
from .cmd.can_filter_helper import can_filter_setup, run_filter
from .cmd.table_helper import run_table, table_setup


@click.command("filter")
@click.option(
    "-c",
    "--config",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="A configuration file (YML) to define the filter",
)
@click.pass_context
def cmd_filter(
    ctx: click.Context,
    config: Path,
) -> None:
    """This subcommand is used to filter out unwanted CAN messages read from
    the standard input. The subcommand writes the filtered CAN messages to the
    standard output.
    """
    filter_obj = can_filter_setup(config)
    run_filter(filter_obj)
    ctx.exit(0)


@click.command("decode")
@click.option(
    "-c",
    "--config",
    type=click.Path(exists=True, file_okay=True, dir_okay=False, path_type=Path),
    required=True,
    help="A configuration file (YML) to define the decoding",
)
@click.option(
    "-o",
    "--output",
    type=click.Path(exists=False, file_okay=False, dir_okay=True, path_type=Path),
    required=False,
    help="Folder in which the files with decoded CAN messages are saved",
)
@click.pass_context
def cmd_decode(ctx: click.Context, config: Path, output: Path | None = None) -> None:
    """This subcommand is used to decode CAN message from the standard input.
    Decoded CAN messages are saved in separate file (JSON) in the output
    folder.
    """
    decode_obj = can_decode_setup(config)
    if output is not None:
        output.mkdir(parents=True, exist_ok=True)
        run_decode2file(decode_obj, output)
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
    help="Folder or file in which the table|s should be saved",
)
@click.pass_context
def cmd_table(ctx: click.Context, data: Path, config: Path, output: Path) -> None:
    """This subcommand converts files with decoded CAN message (JSON) to one or
    multiple tables. The input is either one file with decoded CAN messages
    or a folder containing files with decoded CAN messages. The output is
    either one table or multiple tables depending on the configuration file
    and the provided input."""
    table_obj = table_setup(config)
    run_table(table_obj, data, output)
    ctx.exit(0)


@click.group(context_settings={"help_option_names": ["-h", "--help"]})
def entry_point() -> None:
    """Extract Transform Load (ETL) functionalities via command line.

    These scripts and tools will simplify the collection of
    foxBMS 2 data and their analysis.
    """


def main():
    """Initiate the entry point of batetl"""
    entry_point.add_command(cmd_filter)
    entry_point.add_command(cmd_decode)
    entry_point.add_command(cmd_table)
    entry_point()


if __name__ == "__main__":
    main()
