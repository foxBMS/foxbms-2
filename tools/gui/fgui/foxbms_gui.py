#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""TODO"""
import logging
from pathlib import Path

import click
import wx
from fgui.entry.entry_frame import EntryFrame
from fgui.misc.can.can_helpers import try_to_select_can_adapter
from fgui.misc.misc import LOG_LEVELS
from fgui.misc.program_arguments import (
    C_SETUP_BAUD_RATE,
    C_SETUP_CAN_ADAPTERS,
    C_SETUP_LOGGING,
    C_SETUP_VERBOSITY,
    chose_baud_rate,
    chose_can_adapter,
    chose_logging_dir,
    validate_argument_combination,
)
from fgui.workers.can_node_worker import CanAdapterProcess


@click.command()
@click.help_option("--help", "-h")
@click.option("-v", "--verbose", **C_SETUP_VERBOSITY)
@click.option("-c", "--can-adapter", **C_SETUP_CAN_ADAPTERS)
@click.option("-b", "--baud-rate", **C_SETUP_BAUD_RATE)
@click.option("-l", "--logging", "logging_dir", **C_SETUP_LOGGING)
def main(verbose: int, can_adapter: str, baud_rate: str, logging_dir: click.Path):
    """main entry point for the wrapper GUI application"""
    log_level = LOG_LEVELS[min(verbose, max(LOG_LEVELS.keys()))]
    logging.basicConfig(level=log_level)
    logging.debug(f"Logging level: {logging.getLevelName(log_level)}")

    app = wx.App(redirect=True)
    wx.Locale(wx.LANGUAGE_ENGLISH)

    if not can_adapter:
        can_adapter = chose_can_adapter()
    else:
        can_adapter = try_to_select_can_adapter(can_adapter)
    logging.debug(f"CAN adapter: {can_adapter}")

    if can_adapter and not baud_rate:
        baud_rate = chose_baud_rate()
    logging.debug(f"BAUD rate: {baud_rate}")

    if can_adapter and baud_rate and not logging_dir:
        logging_dir = chose_logging_dir(logging_dir)
    if logging_dir:
        logging_dir = Path(logging_dir)
    logging.debug(f"CAN Logging: {logging_dir}")

    validate_argument_combination(can_adapter, baud_rate, logging_dir)

    can_process = None
    if can_adapter and baud_rate:
        can_process = CanAdapterProcess(can_adapter, baud_rate, logging_dir)
        can_process.daemon = True
        can_process.start()

    EntryFrame(can_process=can_process)
    app.MainLoop()

    if can_process:
        if not can_process.g_canceled.is_set():
            logging.debug("(1/2) start canceling CanAdapterProcess")
            can_process.g_cancel()
            logging.debug("(2/2) done canceling CanAdapterProcess")
        can_process.join()
    logging.debug("exit")


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter
