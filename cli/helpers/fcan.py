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

"""CAN bus configuration helpers and Click option decorators."""

import re
from dataclasses import dataclass
from pathlib import Path

import click
from click.decorators import FC

from .click_helpers import recho

SUPPORTED_CHANNELS: dict[str, list[str | int]] = {
    "pcan": [
        "PCAN_USBBUS1",
        "PCAN_USBBUS2",
        "PCAN_USBBUS3",
        "PCAN_USBBUS4",
        "PCAN_USBBUS5",
        "PCAN_USBBUS6",
        "PCAN_USBBUS7",
        "PCAN_USBBUS8",
        "PCAN_USBBUS9",
        "PCAN_USBBUS10",
        "PCAN_USBBUS11",
        "PCAN_USBBUS12",
        "PCAN_USBBUS13",
        "PCAN_USBBUS14",
        "PCAN_USBBUS15",
        "PCAN_USBBUS16",
    ],
    "kvaser": [0, 1, 2, 3, 4, 5, 6, 7],
}

DEFAULT_CHANNELS = {
    "pcan": SUPPORTED_CHANNELS["pcan"][0],
    "kvaser": SUPPORTED_CHANNELS["kvaser"][0],
}
DEFAULT_INTERFACE = "pcan"
SUPPORTED_INTERFACES = list(DEFAULT_CHANNELS.keys())

VALID_BIT_RATES = ["500000"]
DEFAULT_BIT_RATE = "500000"
SOCKETCAN_CHANNEL_RE = re.compile(r"^(v?can)\d+$")


@dataclass
class CanBusConfig:
    """Represent and validate a CAN bus connection configuration.

    Attributes:
        interface: CAN backend identifier (for example ``pcan`` or ``socketcan``).
        channel: Interface-specific channel identifier.
        bitrate: Bus bitrate in bit/s.
        dbc: Optional path to a DBC file.
    """

    interface: str
    channel: str | int | None = None
    bitrate: int = 500000
    dbc: Path | None = None

    def __post_init__(self) -> None:
        """Validate interface-specific channel constraints.

        Raises:
            SystemExit: If interface/channel combination is invalid.
        """
        err_msg = "Invalid channel choice for interface '{interface}'."
        if self.interface == "pcan":
            if not self.channel:
                self.channel = "PCAN_USBBUS1"
            if not isinstance(self.channel, str):
                recho(err_msg.format(interface="pcan"))
                raise SystemExit(1)
            if self.channel not in SUPPORTED_CHANNELS["pcan"]:
                recho(err_msg.format(interface="pcan"))
                raise SystemExit(1)
        elif self.interface == "kvaser":
            if not self.channel:
                self.channel = 0
            try:
                self.channel = int(self.channel)
            except (ValueError, TypeError) as exc:
                recho(err_msg.format(interface="kvaser"))
                raise SystemExit(1) from exc
        elif self.interface == "socketcan":
            if not self.channel:
                self.channel = "can0"
            if not isinstance(self.channel, str):
                recho(err_msg.format(interface="socketcan"))
                raise SystemExit(1)
            # socketcan channels are canX or vcanX with X as a number.
            if not SOCKETCAN_CHANNEL_RE.match(self.channel):
                recho(err_msg.format(interface="socketcan"))
                raise SystemExit(1)
        elif self.interface == "virtual":
            pass
        else:
            recho(f"Unsupported interface '{self.interface}'.")
            raise SystemExit(2)
        if self.dbc:
            self.dbc = Path(self.dbc)


def common_can_options(fun: FC) -> FC:
    """Add common CAN-related Click options to a command function.

    Args:
        fun: Click command function to decorate.

    Returns:
        Decorated command function.
    """
    fun = click.option(
        "-i",
        "--interface",
        default=DEFAULT_INTERFACE,
        type=click.Choice(SUPPORTED_INTERFACES),
        help="CAN interface.",
    )(fun)
    tmp = "Usable are:\n\n"
    for k, v in SUPPORTED_CHANNELS.items():
        tmp += f"  {k}: {', '.join(str(i) for i in v)}\n\n"
    fun = click.option(
        "-c",
        "--channel",
        default=None,
        help="CAN channel (must be appropiate for the selected interface; "
        f"defaults are {DEFAULT_CHANNELS}).\n\n{tmp}",
    )(fun)
    return click.option(
        "-b",
        "--bitrate",
        default=DEFAULT_BIT_RATE,
        type=click.Choice(VALID_BIT_RATES),
        help="CAN Baudrate.",
    )(fun)
