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


"""Path definition decorators for foxBMS commands."""

from dataclasses import dataclass
from pathlib import Path

import click
from click.decorators import FC

from .misc import (
    APP_DBC_FILE,
    BOOTLOADER_DBC_FILE,
    FOXBMS_APP_CRC_FILE,
    FOXBMS_APP_INFO_FILE,
    FOXBMS_BIN_FILE,
)


@dataclass
class FoxbmsFiles:
    """Container for commonly used foxBMS artifact paths."""

    foxbms_app_crc_file: Path
    foxbms_app_info_file: Path
    foxbms_bin_file: Path


def bootloader_dbc_file_option(fun: FC) -> FC:
    """Add a Click option for overriding the bootloader DBC path.

    Args:
        fun: Click command function to decorate.

    Returns:
        Decorated command function.
    """
    return click.option(
        "--bootloader-dbc",
        default=BOOTLOADER_DBC_FILE,
        type=click.Path(exists=True, path_type=Path),
        help="Path of the Bootloader DBC File",
    )(fun)


def app_dbc_file_option(fun: FC) -> FC:
    """Add a Click option for overriding the application DBC path.

    Args:
        fun: Click command function to decorate.

    Returns:
        Decorated command function.
    """
    return click.option(
        "--app-dbc",
        default=APP_DBC_FILE,
        type=click.Path(exists=True, path_type=Path),
        help="Path of the App DBC File",
    )(fun)


def foxbms_files_option(fun: FC) -> FC:
    """Add Click options for key foxBMS output artifact paths.

    Args:
        fun: Click command function to decorate.

    Returns:
        Decorated command function.
    """
    fun = click.option(
        "--foxbms-app-crc",
        default=FOXBMS_APP_CRC_FILE,
        type=click.Path(exists=True, path_type=Path),
        help="Path of the foxBMS App CRC File",
    )(fun)
    fun = click.option(
        "--foxbms-app-info",
        default=FOXBMS_APP_INFO_FILE,
        type=click.Path(exists=True, path_type=Path),
        help="Path of the foxBMS App Info File",
    )(fun)
    return click.option(
        "--foxbms-bin",
        default=FOXBMS_BIN_FILE,
        type=click.Path(exists=True, path_type=Path),
        help="Path of the foxBMS BIN File",
    )(fun)
