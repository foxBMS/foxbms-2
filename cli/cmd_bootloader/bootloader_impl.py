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

"""Implements the functionalities behind the 'bootloader' command"""

import logging
import time
from dataclasses import asdict

import can
import click
from can import CanError

from ..helpers.click_helpers import recho
from ..helpers.fcan import CanBusConfig
from .bootloader import Bootloader
from .bootloader_can import BootloaderInterfaceCan


# pylint: disable-next=too-few-public-methods
class RootFilter(logging.Filter):
    """Filter to prevent certain messages from being displayed."""

    def filter(self, record):  # pragma: no cover
        messages_to_suppress = [
            "Can not get the state of bootloader.",
            "Can not get the current number of data transfer loops of bootloader.",
            "Cannot get any status of bootloader successfully, check "
            "the CAN connection and the power status of the hardware.",
            "Can not get all information from bootloader, something went wrong",
        ]  # pragma: no cover
        return not any(
            msg in record.getMessage() for msg in messages_to_suppress
        )  # pragma: no cover


# pylint: disable-next=too-few-public-methods
class PcanFilter(logging.Filter):
    """Filter to prevent certain messages from being displayed."""

    def filter(self, record):  # pragma: no cover
        messages_to_suppress = [
            "Bus error: an error counter reached the 'heavy'/'warning' limit",
        ]  # pragma: no cover
        return not any(
            msg in record.getMessage() for msg in messages_to_suppress
        )  # pragma: no cover


def check_bootloader(bus_cfg: CanBusConfig | Bootloader) -> int:
    """Check the status of bootloader"""
    # Get the logger
    root_logger = logging.getLogger()
    pcan_logger = logging.getLogger("can.pcan")
    # Add the custom filters to the loggers to disable specified messages
    root_logger.addFilter(RootFilter())
    pcan_logger.addFilter(PcanFilter())
    if isinstance(bus_cfg, Bootloader):
        click.echo("Checking if the bootloader is online...")
        retval_check_target = bus_cfg.check_target()
        if retval_check_target == 0:
            click.echo("Bootloader is running.")
        elif retval_check_target == 1:
            recho("Bootloader is running, but something went wrong.")
        elif retval_check_target == 2:
            click.echo("foxBMS 2 application is running.")
        elif retval_check_target == 3:
            recho("Bootloader is not reachable.")
        else:
            recho("Unknown return value, something went wrong.")
        return retval_check_target

    if isinstance(bus_cfg, CanBusConfig):
        try:
            with can.Bus(**asdict(bus_cfg)) as can_bus:
                interface = BootloaderInterfaceCan(can_bus=can_bus)
                bd = Bootloader(interface=interface)
                click.echo("Checking if the bootloader is online...")
                retval_check_target = bd.check_target()
                if retval_check_target == 0:
                    click.echo("Bootloader is running.")
                elif retval_check_target == 1:
                    recho("Bootloader is running, but something went wrong.")
                elif retval_check_target == 2:
                    click.echo("foxBMS 2 application is running.")
                elif retval_check_target == 3:
                    recho("Bootloader is not reachable.")
                else:
                    recho("Unknown return value, something went wrong.")
                return retval_check_target
        # Any instantiation error of the bus shall be caught and we do not care
        # what it specifically is
        except FileNotFoundError:
            recho(
                "There is no binary file or CRC file available, please run "
                "'waf build_app_embedded' command to build the project first, exit."
            )
            return 5
        except CanError:
            recho(
                "CAN interface error, check if the CAN adapter"
                " has been connected correctly, exit."
            )
            return 6
        # pylint: disable-next=broad-exception-caught
        except Exception:
            recho("Can not successfully init the CAN bus, exit.")
            return 7
    else:
        recho("Invalid bootloader configuration.")
        return 99


def run_app(bus_cfg: CanBusConfig) -> int:
    """Send command to bootloader to run the application."""
    try:
        with can.Bus(**asdict(bus_cfg)) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)
            if check_bootloader(bd):
                return 1
            click.echo("Starting the application...")
            if not bd.run_app():
                recho("Starting the application not successfully.")
                return 2
            click.echo("Application is running.")
    # Any instantiation error of the bus shall be caught and we do not care
    # what it specifically is
    except FileNotFoundError:
        click.echo(
            "There is no binary file or CRC file available, please run "
            "'waf build_app_embedded' command to build the project first, exit."
        )
        return 3
    except CanError:
        click.echo(
            "CAN interface error, check if the CAN adapter"
            " has been connected correctly, exit."
        )
        return 4
    # pylint: disable-next=broad-exception-caught
    except Exception:
        click.echo("Can not successfully init the CAN bus, exit.")
        return 5
    return 0


# pylint: disable=too-many-return-statements
def reset_bootloader(bus_cfg: CanBusConfig, timeout: int = 20) -> int:
    """Send command to reset bootloader."""
    try:
        with can.Bus(**asdict(bus_cfg)) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)

            # Get the logger
            root_logger = logging.getLogger()
            pcan_logger = logging.getLogger("can.pcan")

            # Add the custom filters to the loggers to disable specified messages
            root_logger.addFilter(RootFilter())
            pcan_logger.addFilter(PcanFilter())

            # Wait and check if the bootloader is online
            start_time = time.time()
            first_check = True
            start_reset = False
            while (time.time() - start_time) <= float(timeout):
                retval_check_target = bd.check_target()
                if retval_check_target in (1, 3):
                    if first_check:
                        click.echo("Waiting bootloader to be powered on ...")
                        first_check = False
                elif retval_check_target == 0:
                    click.echo("Bootloader is online, resetting bootloader.")
                    start_reset = True
                    break
                elif retval_check_target == 2:
                    click.echo("The foxBMS 2 application is running, abort.")
                    return 2
                else:
                    recho("Unknown check value, abort.")
                    return 3

            if start_reset:
                click.echo("Resetting bootloader ...")
                if not bd.reset_bootloader():
                    recho("Resetting bootloader not successfully.")
                    return 4
                click.echo("Successfully reset bootloader.")
                return 0

            recho("Timeout, abort.")
            return 5
    # Any instantiation error of the bus shall be caught and we do not care
    # what it specifically is
    except FileNotFoundError:
        recho(
            "There is no binary file or CRC file available, please run "
            "'waf build_app_embedded' command to build the project first, exit."
        )
        return 6
    except CanError:
        recho(
            "CAN interface error, check if the CAN adapter"
            " has been connected correctly, exit."
        )
        return 7
    # pylint: disable-next=broad-exception-caught
    except Exception:
        click.echo("Can not successfully init the CAN bus, exit.")
        return 8


# pylint: disable=too-many-return-statements
def load_app(bus_cfg: CanBusConfig, timeout: int = 20) -> int:
    """Load a new binary on the target"""
    try:
        with can.Bus(**asdict(bus_cfg)) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)

            # Get the logger
            root_logger = logging.getLogger()
            pcan_logger = logging.getLogger("can.pcan")

            # Add the custom filters to the loggers to disable specified messages
            root_logger.addFilter(RootFilter())
            pcan_logger.addFilter(PcanFilter())

            # Wait and check if the bootloader is online
            start_time = time.time()
            first_check = True
            start_transfer = False
            while (time.time() - start_time) <= float(timeout):
                retval_check_target = bd.check_target()
                if retval_check_target in (1, 3):
                    if first_check:
                        click.echo("Waiting for the bootloader to be powered on ...")
                        first_check = False
                elif retval_check_target == 0:
                    click.echo(
                        "Bootloader is online. Uploading the application to target ..."
                    )
                    start_transfer = True
                    break
                elif retval_check_target == 2:
                    recho("The foxBMS 2 application is running, abort.")
                    return 2
                else:
                    recho("Unknown check value, abort.")
                    return 3

            # Start transferring the application
            if start_transfer:
                click.echo("Uploading application to target...")
                logging_level = logging.getLogger().getEffectiveLevel()
                show_progressbar = logging_level >= logging.WARNING
                if not bd.send_app_binary(show_progressbar=show_progressbar):
                    recho(
                        "Sending the application binary to the bootloader was "
                        "not successfully."
                    )
                    return 4
                recho(
                    "Successfully uploaded the application binary to the "
                    "target, start the foxBMS application!"
                )
                return 0

            recho("Timeout, abort.")
            return 5
    # Any instantiation error of the bus shall be caught and we do not care
    # what it specifically is
    except FileNotFoundError as e:
        recho(
            f"File {e.filename} not found, please run 'waf build_app_embedded'"
            " command to build the project first/again, exit."
        )
        return 6
    except CanError:
        recho(
            "CAN interface error, check if the CAN adapter "
            "has been connected correctly, exit."
        )
        return 7
    # pylint: disable-next=broad-exception-caught
    except Exception:
        recho("Can not successfully init the CAN bus, exit.")
        return 8
