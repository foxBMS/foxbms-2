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
from can.typechecking import CanFilter
from cantools import database

from ..helpers.click_helpers import echo, recho, secho
from ..helpers.fcan import CanBusConfig
from ..helpers.misc import APP_DBC_FILE, BOOTLOADER_DBC_FILE
from .bootloader import Bootloader, BootloaderStatus
from .bootloader_can import BootloaderInterfaceCan

# Create the filter to prevent receiving the irrelevant can messages
db_bootloader = database.load_file(BOOTLOADER_DBC_FILE)
db_app = database.load_file(APP_DBC_FILE)
can_filters = []
if isinstance(db_bootloader, database.can.database.Database):  # pragma: no cover
    can_filters.extend(
        [
            CanFilter(can_id=message.frame_id, can_mask=0x7FF)
            for message in db_bootloader.messages
        ]
    )
if isinstance(db_app, database.can.database.Database):  # pragma: no cover
    can_filters.extend(
        [
            CanFilter(can_id=message.frame_id, can_mask=0x7FF)
            for message in db_app.messages
        ]
    )


class RootFilter(logging.Filter):  # pylint:disable=too-few-public-methods
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


class PcanFilter(logging.Filter):  # pylint:disable=too-few-public-methods
    """Filter to prevent certain messages from being displayed."""

    def filter(self, record):  # pragma: no cover
        messages_to_suppress = [
            "Bus error: an error counter reached the 'heavy'/'warning' limit",
        ]  # pragma: no cover
        return not any(
            msg in record.getMessage() for msg in messages_to_suppress
        )  # pragma: no cover


def _add_filters() -> None:
    # Get the logger
    root_logger = logging.getLogger()
    pcan_logger = logging.getLogger("can.pcan")
    # Add the custom filters to the loggers to disable specified messages
    root_logger.addFilter(RootFilter())
    pcan_logger.addFilter(PcanFilter())


def _instantiate_bootloader(can_bus: can.BusABC) -> Bootloader:
    can_bus.set_filters(can_filters)
    interface = BootloaderInterfaceCan(can_bus=can_bus)
    bl = Bootloader(interface=interface)
    return bl


def _check_bootloader(bl: Bootloader) -> int:
    echo("Checking if the bootloader is online...")
    retval_check_target, _ = bl.check_target()
    if retval_check_target == 0:
        echo("Bootloader is running.")
    elif retval_check_target == 1:
        recho("Bootloader is running, but some information is missing.")
    elif retval_check_target == 2:
        echo("foxBMS 2 application is running.")
    elif retval_check_target == 3:
        recho("Bootloader is not reachable.")
    else:
        recho("Unknown return value, something went wrong.")
    return retval_check_target


def _check_bootloader_status(
    bl: Bootloader, timeout: float = 20.0
) -> tuple[int, BootloaderStatus]:
    # Wait and check if the bootloader is online
    start_time = time.time()
    first_check = True
    bl_info: BootloaderStatus = BootloaderStatus(None, None, None)
    while (time.time() - start_time) <= float(timeout):
        retval_check_target, bl_info = bl.check_target()
        if retval_check_target == 3:
            if first_check:
                echo("Waiting for the bootloader to be powered on...")
                first_check = False
        elif retval_check_target == 1:
            # Can not retrieve all information from bootloader, try again.
            pass
        elif retval_check_target == 0:
            echo("Bootloader is online.")
            return 0, bl_info
        elif retval_check_target == 2:
            echo("The foxBMS 2 application is running, aborting.")
            return 2, bl_info
        else:
            recho("Unknown check value, aborting.")
            return 3, bl_info
    recho("Timeout, abort.")
    return 5, bl_info  # timeout


def catch_bus_initialization_failures(fun):
    """Guard functions that use can.Bus objects"""

    def wrap(*args, **kwargs):
        try:
            return fun(*args, **kwargs)
        except can.exceptions.CanInitializationError as e:
            recho(
                f"Could not initialize CAN bus "
                f"'{kwargs['bus_cfg'].interface}:{kwargs['bus_cfg'].channel}':{e}"
            )
            return 5
        except NameError as e:
            recho(
                f"Could not initialize CAN bus "
                f"'{kwargs['bus_cfg'].interface}:{kwargs['bus_cfg'].channel}':{e}\n"
                "Is the Kvaser canlib is installed?"
            )
            return 5

    return wrap


@catch_bus_initialization_failures
def _check_bootloader_wrapper(*, bus_cfg: CanBusConfig) -> int:
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bl = _instantiate_bootloader(can_bus)
        return _check_bootloader(bl)


@catch_bus_initialization_failures
def check_bootloader(cfg: CanBusConfig | Bootloader) -> int:
    """Check the status of the bootloader"""
    _add_filters()
    if isinstance(cfg, Bootloader):
        return _check_bootloader(cfg)

    if isinstance(cfg, CanBusConfig):
        return _check_bootloader_wrapper(bus_cfg=cfg)

    recho("Invalid bootloader configuration.")
    return 99


@catch_bus_initialization_failures
def run_app(*, bus_cfg: CanBusConfig) -> int:
    """Send command to the bootloader to run the application."""
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bd = _instantiate_bootloader(can_bus)
        if check_bootloader(bd):
            return 1
        secho("Starting the application...")
        if not bd.run_app():
            recho("Starting the application not successfully.")
            return 2
        secho("Application is running.")
    return 0


@catch_bus_initialization_failures
def reset_bootloader(*, bus_cfg: CanBusConfig, timeout: float = 20.0) -> int:
    """Reset the bootloader."""
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bl = _instantiate_bootloader(can_bus)
        _add_filters()

        bl_status, _ = _check_bootloader_status(bl, timeout)
        if bl_status != 0:
            return bl_status

        echo("Resetting bootloader...")
        if not bl.reset_bootloader():
            recho("Resetting bootloader was not successful.")
            return 4

        secho("Successfully resetted bootloader.", fg="green")
        return 0


@catch_bus_initialization_failures
def load_app(*, bus_cfg: CanBusConfig, timeout: float = 20.0) -> int:
    """Load a new binary on the target"""
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bl = _instantiate_bootloader(can_bus)
        _add_filters()

        bl_status, bl_info = _check_bootloader_status(bl, timeout)
        if bl_status != 0:
            return bl_status

        echo("Uploading application to target...")
        logging_level = logging.getLogger().getEffectiveLevel()
        show_progressbar = logging_level >= logging.WARNING
        if not bl.send_app_binary(bl_info, show_progressbar=show_progressbar):
            recho(
                "Sending the application binary to the bootloader was not successful."
            )
            return 4
        secho(
            "Successfully uploaded the application binary to the "
            "target, starting the foxBMS application!",
            fg="green",
        )

    return 0
