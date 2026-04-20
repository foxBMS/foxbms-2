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

"""Implements the functionalities behind the 'bootloader' command"""

import time

# not used to actually log, therefore import is fine
from collections.abc import Callable
from dataclasses import asdict
from logging import WARNING, Filter, LogRecord, getLogger  # noqa: TID251
from pathlib import Path
from typing import Any

import can
from can.typechecking import CanFilter
from cantools import database

from ..helpers import TOOL
from ..helpers.click_helpers import echo, recho, secho
from ..helpers.fcan import CanBusConfig
from ..helpers.logger import logger
from ..helpers.path_options import FoxbmsFiles
from .bootloader import Bootloader, BootloaderStatus
from .bootloader_can import BootloaderInterfaceCan

# Filter for received can messages
can_filters = []


class FoxPyFilter(Filter):  # pylint:disable=too-few-public-methods
    """Filter to prevent certain messages from being displayed."""

    def filter(self, record: LogRecord) -> bool:  # pragma: no cover
        """Filer ignorable messages"""
        messages_to_suppress = [
            "Can not get the state of bootloader.",
            "Can not get the current number of data transfer loops of bootloader.",
            "Can not retrieve any bootloader information.",
            "Can not retrieve all bootloader information.",
        ]  # pragma: no cover
        return not any(
            msg in record.getMessage() for msg in messages_to_suppress
        )  # pragma: no cover


class PcanFilter(Filter):  # pylint:disable=too-few-public-methods
    """Filter to prevent certain messages from being displayed."""

    def filter(self, record: LogRecord) -> bool:  # pragma: no cover
        """Filer ignorable messages"""
        messages_to_suppress = [
            "Bus error: an error counter reached the 'heavy'/'warning' limit",
        ]  # pragma: no cover
        return not any(
            msg in record.getMessage() for msg in messages_to_suppress
        )  # pragma: no cover


def _create_filter(app_dbc: Path, bootloader_dbc: Path) -> None:
    """Create the filter to prevent receiving the irrelevant can messages"""
    db_bootloader = database.load_file(bootloader_dbc)
    db_app = database.load_file(app_dbc)
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


def _add_filters() -> None:
    fox_py_logger = getLogger(TOOL)
    fox_py_logger.addFilter(FoxPyFilter())
    pcan_logger = getLogger("can.pcan")
    pcan_logger.addFilter(PcanFilter())


def _instantiate_bootloader(
    can_bus: can.BusABC,
    app_dbc: Path,
    bootloader_dbc: Path,
    foxbms_files: FoxbmsFiles,
) -> Bootloader:
    can_bus.set_filters(can_filters)
    interface = BootloaderInterfaceCan(
        can_bus=can_bus, app_dbc=app_dbc, bootloader_dbc=bootloader_dbc
    )
    return Bootloader(
        interface=interface,
        app=foxbms_files.foxbms_bin_file,
        crc_table=foxbms_files.foxbms_app_crc_file,
        program_info=foxbms_files.foxbms_app_info_file,
    )


def _check_bootloader(bl: Bootloader) -> int:
    echo("Checking if the bootloader is online...")
    retval_check_target, _ = bl.check_target()
    if retval_check_target == 0:
        secho("Bootloader is running.", fg="green")
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
                secho("Waiting for the bootloader to be powered on...", fg="green")
                first_check = False
        elif retval_check_target == 1:
            # Can not retrieve all information from bootloader, try again.
            pass
        elif retval_check_target == 0:
            secho("Bootloader is online.", fg="green")
            return 0, bl_info
        elif retval_check_target == 2:
            secho("The foxBMS 2 application is running, aborting.", fg="yellow")
            return 2, bl_info
        else:
            recho("Unknown check value, aborting.")
            return 3, bl_info
    recho("Timeout, abort.")
    return 5, bl_info  # timeout


def catch_bus_initialization_failures[R](
    fun: Callable[..., R],
) -> Callable[..., int | R]:
    """Guard functions that use can.Bus objects"""

    def wrap(*args: Any, **kwargs: Any) -> int | R:  # noqa: ANN401
        if not can_filters:
            app_dbc = str(kwargs.get("app_dbc"))
            bootloader_dbc = str(kwargs.get("bootloader_dbc"))
            _create_filter(Path(app_dbc), Path(bootloader_dbc))
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
def _check_bootloader_wrapper(
    *,
    bus_cfg: CanBusConfig,
    app_dbc: Path,
    bootloader_dbc: Path,
    foxbms_files: FoxbmsFiles,
) -> int:
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bl = _instantiate_bootloader(can_bus, app_dbc, bootloader_dbc, foxbms_files)
        return _check_bootloader(bl)


@catch_bus_initialization_failures
def check_bootloader(
    cfg: CanBusConfig | Bootloader,
    app_dbc: Path,
    bootloader_dbc: Path,
    foxbms_files: FoxbmsFiles,
) -> int:
    """Check the status of the bootloader"""
    _add_filters()
    if isinstance(cfg, Bootloader):
        return _check_bootloader(cfg)

    if isinstance(cfg, CanBusConfig):
        return _check_bootloader_wrapper(
            bus_cfg=cfg,
            app_dbc=app_dbc,
            bootloader_dbc=bootloader_dbc,
            foxbms_files=foxbms_files,
        )

    recho("Invalid bootloader configuration.")
    return 99


@catch_bus_initialization_failures
def run_app(
    *,
    bus_cfg: CanBusConfig,
    app_dbc: Path,
    bootloader_dbc: Path,
    foxbms_files: FoxbmsFiles,
) -> int:
    """Send command to the bootloader to run the application."""
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bd = _instantiate_bootloader(can_bus, app_dbc, bootloader_dbc, foxbms_files)
        if check_bootloader(bd, app_dbc, bootloader_dbc, foxbms_files):
            return 1
        secho("Starting the application...")
        if not bd.run_app():
            recho("Starting the application not successfully.")
            return 2
        secho("Application is running.")
    return 0


@catch_bus_initialization_failures
def reset_bootloader(
    *,
    bus_cfg: CanBusConfig,
    timeout: float = 20.0,
    app_dbc: Path,
    bootloader_dbc: Path,
    foxbms_files: FoxbmsFiles,
) -> int:
    """Reset the bootloader."""
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bl = _instantiate_bootloader(can_bus, app_dbc, bootloader_dbc, foxbms_files)
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
def load_app(
    *,
    bus_cfg: CanBusConfig,
    timeout: float = 20.0,
    app_dbc: Path,
    bootloader_dbc: Path,
    foxbms_files: FoxbmsFiles,
) -> int:
    """Load a new binary on the target"""
    with can.Bus(**asdict(bus_cfg)) as can_bus:
        bl = _instantiate_bootloader(can_bus, app_dbc, bootloader_dbc, foxbms_files)
        _add_filters()

        bl_status, bl_info = _check_bootloader_status(bl, timeout)
        if bl_status != 0:
            return bl_status

        echo("Uploading application to target...")
        logging_level = logger.getEffectiveLevel()
        show_progressbar = logging_level >= WARNING
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
