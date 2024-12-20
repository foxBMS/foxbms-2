#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Command line interface definition for the 'bootloader' command"""

import logging
import sys
from pathlib import Path

import click

from ..cmd_bootloader.bootloader import CanBusConfig

# show can.pcan warnings only when it is relevant
if "-h" in sys.argv or "--help" in sys.argv or "bootloader" not in sys.argv:
    logging.getLogger("can.pcan").setLevel(logging.CRITICAL)


# pylint:disable=wrong-import-position
from ..cmd_bootloader import bootloader_impl
from ..helpers.misc import PROJECT_ROOT, set_logging_level_cb

# pylint:enable=wrong-import-position
CONTEXT_SETTINGS = {
    "help_option_names": ["-h", "--help"],
}


@click.group(context_settings=CONTEXT_SETTINGS)
def bootloader() -> None:
    """Command line tool to interact with the embedded bootloader."""


@bootloader.command("run-app")
@click.pass_context
@click.option(
    "-v",
    "--verbose",
    default=0,
    count=True,
    help="Verbose information",
    callback=set_logging_level_cb,
)
@click.option(
    "-i",
    "--interface",
    default="pcan",
    type=click.Choice(["pcan"]),
    help="CAN interface",
)
@click.option(
    "-c",
    "--channel",
    default="PCAN_USBBUS1",
    type=click.Choice(["PCAN_USBBUS1", "PCAN_USBBUS2"]),
    help="CAN channel",
)
@click.option(
    "-b",
    "--bitrate",
    default="500000",
    type=click.Choice(["500000"]),
    help="CAN Baudrate",
)
def cmd_run_app(
    ctx: click.Context,
    verbose: int,  # pylint: disable=unused-argument
    interface: str,
    channel: str,
    bitrate: str,
) -> None:
    """Run the BMS application."""
    try:
        bitrate_int = int(bitrate)
    except ValueError:
        ctx.exit("Could not convert provided bitrate to integer.")
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate_int)
    ctx.exit(bootloader_impl.run_app(bus_cfg))


@bootloader.command("load-app")
@click.pass_context
@click.option(
    "-v",
    "--verbose",
    default=0,
    count=True,
    help="Verbose information",
    callback=set_logging_level_cb,
)
@click.argument(
    "binary",
    default=PROJECT_ROOT / "build/app_embedded/src/app/main/foxbms.bin",
    is_eager=True,
    type=click.Path(exists=True, readable=True, dir_okay=False, path_type=Path),
)
@click.option(
    "-i",
    "--interface",
    default="pcan",
    type=click.Choice(["pcan"]),
    help="CAN interface",
)
@click.option(
    "-c",
    "--channel",
    default="PCAN_USBBUS1",
    type=click.Choice(["PCAN_USBBUS1", "PCAN_USBBUS2"]),
    help="CAN channel",
)
@click.option(
    "-b",
    "--bitrate",
    default="500000",
    type=click.Choice(["500000"]),
    help="CAN Baudrate",
)
# pylint: disable-next=too-many-arguments,too-many-positional-arguments
def cmd_load_app(
    ctx: click.Context,
    verbose: int,  # pylint: disable=unused-argument
    binary: Path,
    interface: str,
    channel: str,
    bitrate: str,
) -> None:
    """Load the application program on the BMS."""
    try:
        bitrate_int = int(bitrate)
    except ValueError:
        ctx.exit("Could not convert provided bitrate to integer.")
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate_int)
    ctx.exit(bootloader_impl.load_app(bus_cfg, binary))


@bootloader.command("reset")
@click.pass_context
@click.option(
    "-v",
    "--verbose",
    default=0,
    count=True,
    help="Verbose information",
    callback=set_logging_level_cb,
)
@click.option(
    "-i",
    "--interface",
    default="pcan",
    type=click.Choice(["pcan"]),
    help="CAN interface",
)
@click.option(
    "-c",
    "--channel",
    default="PCAN_USBBUS1",
    type=click.Choice(["PCAN_USBBUS1", "PCAN_USBBUS2"]),
    help="CAN channel",
)
@click.option(
    "-b",
    "--bitrate",
    default="500000",
    type=click.Choice(["500000"]),
    help="CAN Baudrate",
)
def cmd_reset(
    ctx: click.Context,
    verbose: int,  # pylint: disable=unused-argument
    interface: str,
    channel: str,
    bitrate: str,
) -> None:
    """Reset the bootloader."""
    try:
        bitrate_int = int(bitrate)
    except ValueError:
        ctx.exit("Could not convert provided bitrate to integer.")
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate_int)
    ctx.exit(bootloader_impl.reset_bootloader(bus_cfg))


@bootloader.command("check")
@click.pass_context
@click.option(
    "-v",
    "--verbose",
    default=0,
    count=True,
    help="Verbose information",
    callback=set_logging_level_cb,
)
@click.option(
    "-i",
    "--interface",
    default="pcan",
    type=click.Choice(["pcan"]),
    help="CAN interface",
)
@click.option(
    "-c",
    "--channel",
    default="PCAN_USBBUS1",
    type=click.Choice(["PCAN_USBBUS1", "PCAN_USBBUS2"]),
    help="CAN channel",
)
@click.option(
    "-b",
    "--bitrate",
    default="500000",
    type=click.Choice(["500000"]),
    help="CAN Baudrate",
)
def cmd_check(
    ctx: click.Context,
    verbose: int,  # pylint: disable=unused-argument
    interface: str,
    channel: str,
    bitrate: str,
) -> None:
    """Check the state of bootloader."""
    try:
        bitrate_int = int(bitrate)
    except ValueError:
        ctx.exit("Could not convert provided bitrate to integer.")
    bus_cfg = CanBusConfig(interface=interface, channel=channel, bitrate=bitrate_int)
    ctx.exit(bootloader_impl.check_bootloader(bus_cfg))
