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

"""Implements the functionalities behind the 'bootloader' command"""

import logging
from pathlib import Path
from typing import Optional

import can

from ..helpers.misc import FOXBMS_BIN_FILE
from .bootloader import Bootloader, CanBusConfig
from .bootloader_can import BootloaderInterfaceCan


def check_bootloader(bus_cfg: CanBusConfig | Bootloader) -> int:
    """Check the status of bootloader"""
    if isinstance(bus_cfg, Bootloader):
        logging.debug("Checking if the bootloader is online...")
        if not bus_cfg.check_target():
            return 2
        logging.info("Bootloader is running.")
        return 0
    if isinstance(bus_cfg, CanBusConfig):
        with can.Bus(**bus_cfg.as_dict()) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface, path_app_binary=None)
            logging.debug("Checking if the bootloader is online...")
            if not bd.check_target():
                logging.error("Checking the status of bootloader not successful.")
                return 1
            logging.info("Bootloader is running.")
            return 0
    else:
        logging.error("Invalid bootloader configuration.")
        return 99


def run_app(bus_cfg: CanBusConfig) -> int:
    """Send command to bootloader to run the application."""
    with can.Bus(**bus_cfg.as_dict()) as can_bus:
        interface = BootloaderInterfaceCan(can_bus=can_bus)
        bd = Bootloader(interface=interface, path_app_binary=None)
        if check_bootloader(bd):
            return 1
        logging.debug("Running the application...")
        if not bd.run_app():
            logging.error("Running the application was not successful.")
            return 2
        logging.info("Application is running.")
    return 0


def reset_bootloader(bus_cfg: CanBusConfig) -> int:
    """Send command to reset bootloader."""
    with can.Bus(**bus_cfg.as_dict()) as can_bus:
        interface = BootloaderInterfaceCan(can_bus=can_bus)
        bd = Bootloader(interface=interface, path_app_binary=None)
        if check_bootloader(bd):
            return 1
        logging.debug("Resetting bootloader ...")
        if not bd.reset_bootloader():
            logging.error("Resetting bootloader was not successful.")
            return 2
        logging.info("Successfully reset bootloader.")
    return 0


def load_app(bus_cfg: CanBusConfig, binary: Optional[Path]) -> int:
    """Load a new binary on the target"""
    with can.Bus(**bus_cfg.as_dict()) as can_bus:
        if not binary:
            binary = FOXBMS_BIN_FILE
        interface = BootloaderInterfaceCan(can_bus=can_bus)
        bd = Bootloader(interface=interface, path_app_binary=binary)
        if check_bootloader(bd):
            return 1
        logging.debug("Uploading application to target...")
        if not bd.send_app_binary():
            logging.error(
                "Sending the application binary to the bootloader was not successful."
            )
            return 2
    logging.info("Successfully uploaded application to the target.")
    return 0
