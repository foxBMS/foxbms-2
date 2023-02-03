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

"""Miscellaneous helpers that are related to to CAN (speaking with CAN
adapters, reading DBC files etc.)"""

import logging
from pathlib import Path
from typing import Tuple, Union

import can
import cantools
import wx
from can.interface import Bus
from cantools.database.can import Database
from fgui import (
    DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT,
    DEFAULT_DBC_FILENAME_IF_IN_PROJECT_LAYOUT,
)
from fgui.misc.can.can_constants import DEFAULT_BAUD_RATE

# we need to pylint's disable=abstract-class-instantiated it is used since the
# 'python-can' documentation tells that this is the way to use it. See
# https://python-can.readthedocs.io/en/master/configuration.html


def get_dbc_dialog() -> Tuple[Union[Database, None], Tuple[Path, None]]:
    """Returns a tuple dbc database and the file it has been read from."""
    dbc = None
    dbc_file = None
    dlg_setup = {
        "message": "Open dbc file",
        "defaultDir": str(DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT),
        "defaultFile": DEFAULT_DBC_FILENAME_IF_IN_PROJECT_LAYOUT,
        "wildcard": "dbc files (*.dbc)|*.dbc",
        "style": wx.FD_OPEN | wx.FD_FILE_MUST_EXIST,
    }
    with wx.FileDialog(None, **dlg_setup) as dlg:
        if dlg.ShowModal() == wx.ID_CANCEL:
            pass  # user abort
        else:
            dbc_file = dlg.GetPath()
    if dbc_file:
        dbc = dbc_file_to_database(dbc_file)
        dbc_file = Path(dbc_file)
    return (dbc, dbc_file)


def dbc_file_to_database(dbc_file) -> Database:
    """returns the database representation of a dbc file."""
    return cantools.database.load_file(dbc_file)


def detect_can_adapters() -> list[can.interface.Bus]:
    """Detect possible CAN adapters. Currently only PCAN USB is supported"""
    connected_adapters: list[can.interface.Bus] = []
    for k, _ in can.interfaces.BACKENDS.items():
        if k == "pcan":
            try_channels = [f"PCAN_USBBUS{i}" for i in range(1, 17)]
            logging.info(f"Checking for {k} in {try_channels}")
            style = (
                wx.PD_CAN_ABORT
                | wx.PD_APP_MODAL
                | wx.PD_ELAPSED_TIME
                | wx.PD_REMAINING_TIME
            )
            dlg_setup = {
                "title": "Searching CAN adapters",
                "message": "Checking for available PCAN devices...",
                "maximum": len(try_channels) + 1,
                "style": style,
            }
            with wx.ProgressDialog(**dlg_setup) as dlg:
                for i, channel in enumerate(try_channels):
                    dlg.Update(i, f"Checking {channel}")
                    logging.debug(f"Checking {channel} ({i})")
                    adapter = None
                    try:
                        # pylint: disable=abstract-class-instantiated
                        adapter: can.interface = can.interface.Bus(
                            bustype=k,
                            channel=channel,
                            state=can.bus.BusState.ACTIVE,
                        )
                    except can.interfaces.pcan.pcan.PcanError:
                        pass
                    if adapter:
                        connected_adapters.append(adapter)
                    if dlg.WasCancelled():
                        break
    logging.info(f"Connected CAN adapters: {connected_adapters}")
    return connected_adapters


def adapter_initialize(
    can_adapter: Union[str, None], baud_rate: Union[int, None]
) -> can.interface:
    """wrapper for CAN adapter initialization. Currently only PCAN USB is supported"""
    if not can_adapter:
        raise IOError("No CAN adapter supplied.")

    if can_adapter.lower().startswith("pcan"):
        cfg: str = can_adapter.split("_")
        bustype = cfg[0].lower()
        channel = can_adapter
        # pylint: disable=abstract-class-instantiated
        adapter: can.interface = can.interface.Bus(
            bustype=bustype, channel=channel, bitrate=baud_rate
        )
    else:
        raise IOError("CAN adapter not supported.")
    return adapter


def adapter_uninitialize(adapter: Bus) -> None:
    """wrapper for CAN adapter uninitialization."""
    adapter.flush_tx_buffer()
    logging.debug("flush tx buffer")
    adapter.shutdown()
    logging.debug(f"uninitialize {adapter}")


def try_to_select_can_adapter(can_adapter: str):
    """Try to select a specific CAN adapter"""
    adapter = None
    try:
        adapter = adapter_initialize(can_adapter, DEFAULT_BAUD_RATE)
    except:  # pylint: disable=bare-except
        pass  # we don't care why the provided CAN adapter can't be used
    if adapter:
        adapter_uninitialize(adapter)
    else:
        raise IOError(f"The provided CAN adapter '{can_adapter}' does not exist.")
    return can_adapter
