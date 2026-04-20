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

"""Click commands for interacting with the embedded bootloader."""

from pathlib import Path

import click

from ..cmd_bootloader import bootloader_impl
from ..helpers.click_helpers import HELP_NAMES, verbosity_option
from ..helpers.fcan import CanBusConfig, common_can_options
from ..helpers.path_options import (
    FoxbmsFiles,
    app_dbc_file_option,
    bootloader_dbc_file_option,
    foxbms_files_option,
)


@click.group(context_settings=HELP_NAMES)
def bootloader() -> None:
    """Bootloader command group entry point."""


@bootloader.command("run-app")
@common_can_options
@bootloader_dbc_file_option
@app_dbc_file_option
@foxbms_files_option
@verbosity_option
@click.pass_context
def cmd_run_app(
    ctx: click.Context,
    interface: str,
    channel: str,
    bitrate: int,
    bootloader_dbc: Path,
    app_dbc: Path,
    foxbms_bin: Path,
    foxbms_app_crc: Path,
    foxbms_app_info: Path,
    verbose: int,
) -> None:
    """Start the BMS application via bootloader communication."""
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate)
    foxbms_files = FoxbmsFiles(
        foxbms_app_crc_file=foxbms_app_crc,
        foxbms_app_info_file=foxbms_app_info,
        foxbms_bin_file=foxbms_bin,
    )
    ctx.exit(
        bootloader_impl.run_app(
            bus_cfg=bus_cfg,
            app_dbc=app_dbc,
            bootloader_dbc=bootloader_dbc,
            foxbms_files=foxbms_files,
        )
    )


@bootloader.command("load-app")
@click.option(
    "-t",
    "--timeout",
    default=20.0,
    type=float,
    help="Timeout in seconds",
)
@common_can_options
@bootloader_dbc_file_option
@app_dbc_file_option
@foxbms_files_option
@verbosity_option
@click.pass_context
def cmd_load_app(
    ctx: click.Context,
    timeout: float,
    interface: str,
    channel: str,
    bitrate: int,
    bootloader_dbc: Path,
    app_dbc: Path,
    foxbms_bin: Path,
    foxbms_app_crc: Path,
    foxbms_app_info: Path,
    verbose: int,
) -> None:
    """Load the application program onto the BMS target."""
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate)
    foxbms_files = FoxbmsFiles(
        foxbms_app_crc_file=foxbms_app_crc,
        foxbms_app_info_file=foxbms_app_info,
        foxbms_bin_file=foxbms_bin,
    )
    ctx.exit(
        bootloader_impl.load_app(
            bus_cfg=bus_cfg,
            timeout=timeout,
            app_dbc=app_dbc,
            bootloader_dbc=bootloader_dbc,
            foxbms_files=foxbms_files,
        )
    )


@bootloader.command("reset")
@click.option(
    "-t",
    "--timeout",
    default=20.0,
    type=float,
    help="Timeout in seconds",
)
@common_can_options
@bootloader_dbc_file_option
@app_dbc_file_option
@foxbms_files_option
@verbosity_option
@click.pass_context
def cmd_reset(
    ctx: click.Context,
    timeout: float,
    interface: str,
    channel: str,
    bitrate: int,
    bootloader_dbc: Path,
    app_dbc: Path,
    foxbms_bin: Path,
    foxbms_app_crc: Path,
    foxbms_app_info: Path,
    verbose: int,
) -> None:
    """Reset the bootloader and verify expected state transition."""
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate)
    foxbms_files = FoxbmsFiles(
        foxbms_app_crc_file=foxbms_app_crc,
        foxbms_app_info_file=foxbms_app_info,
        foxbms_bin_file=foxbms_bin,
    )
    ctx.exit(
        bootloader_impl.reset_bootloader(
            bus_cfg=bus_cfg,
            timeout=timeout,
            app_dbc=app_dbc,
            bootloader_dbc=bootloader_dbc,
            foxbms_files=foxbms_files,
        )
    )


@bootloader.command("check")
@common_can_options
@bootloader_dbc_file_option
@app_dbc_file_option
@foxbms_files_option
@verbosity_option
@click.pass_context
def cmd_check(
    ctx: click.Context,
    interface: str,
    channel: str,
    bitrate: int,
    bootloader_dbc: Path,
    app_dbc: Path,
    foxbms_bin: Path,
    foxbms_app_crc: Path,
    foxbms_app_info: Path,
    verbose: int,
) -> None:
    """Check whether the bootloader is reachable and in expected state."""
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate)
    foxbms_files = FoxbmsFiles(
        foxbms_app_crc_file=foxbms_app_crc,
        foxbms_app_info_file=foxbms_app_info,
        foxbms_bin_file=foxbms_bin,
    )
    ctx.exit(
        bootloader_impl.check_bootloader(
            cfg=bus_cfg,
            app_dbc=app_dbc,
            bootloader_dbc=bootloader_dbc,
            foxbms_files=foxbms_files,
        )
    )
