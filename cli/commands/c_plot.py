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

"""Command line interface definition for plotting."""

from pathlib import Path

import click

from ..cmd_plot.execution import Executor
from ..helpers.click_helpers import HELP_NAMES
from ..helpers.file_tracker import FileTracker


@click.command(context_settings=HELP_NAMES)
@click.argument(
    "input_data", nargs=-1, type=click.Path(exists=True, path_type=Path), required=True
)
@click.option(
    "-d",
    "--data-config",
    required=True,
    help="Path of the configuration file for the data.",
    type=click.Path(exists=True, path_type=Path),
)
@click.option(
    "-p",
    "--plot-config",
    required=True,
    help="Path of the configuration file for the plots.",
    type=click.Path(exists=True, path_type=Path),
)
@click.option(
    "-o",
    "--output",
    help="Path of the directory in which the plot-images will be saved.",
    type=click.Path(exists=True, path_type=Path),
)
@click.option(
    "-t",
    "--data-type",
    type=click.Choice(["CSV"], case_sensitive=True),
    help="Type of the data-files to be used as input.",
)
@click.pass_context
def plot(  # pylint: disable=too-many-arguments,too-many-positional-arguments
    ctx: click.Context,
    input_data: list[Path],
    data_config: Path,
    plot_config: Path,
    output: Path | None,
    data_type: str | None,
) -> None:
    """Run the 'plot' tool with the given data as input."""
    tmp_handler = FileTracker(Path.cwd())
    no_tmp = tmp_handler.check_file_changed(data_config)
    Executor(
        input_data,
        data_config,
        plot_config,
        output,
        data_source_type=data_type,
        no_tmp=no_tmp,
    ).create_plots()
    ctx.exit(0)
