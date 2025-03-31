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

"""Command line interface definition for the 'log' command"""

from pathlib import Path

import click

from ..cmd_log import log_impl
from ..helpers.click_helpers import verbosity_option
from ..helpers.fcan import CanBusConfig, common_can_options
from ..helpers.misc import PROJECT_BUILD_ROOT


@click.command("log")
@click.option(
    "-o",
    "--output",
    type=click.Path(exists=False, file_okay=False, dir_okay=True, path_type=Path),
    default=PROJECT_BUILD_ROOT / "logs",
    help="Output directory.",
)
@click.option(
    "-s", "--log-file-size", type=int, default=200000, help="Size of a log file."
)
@common_can_options
@verbosity_option
@click.pass_context
def log(  # pylint: disable=too-many-arguments,too-many-positional-arguments
    ctx: click.Context,
    output: Path,
    log_file_size: int,
    interface: str,
    channel: str,
    bitrate: str,
    verbose: int = 0,
) -> None:
    """Log CAN traffic."""
    bitrate_int = int(bitrate)  # this is guaranteed to work due to the choice list
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate_int)
    ctx.exit(log_impl.log(bus_cfg, output, log_file_size))
