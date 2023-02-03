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

"""Implements the command line argument the module must understand."""

import logging
import os

import click
import wx

from .can.can_constants import (
    BAUD_RATES,
    DEFAULT_BAUD_RATE_INDEX,
    SUPPORTED_CAN_ADAPTERS,
)
from .can.can_helpers import detect_can_adapters
from .gui_helpers import get_icon

C_SETUP_VERBOSITY = {
    "count": True,
    "help": "Sets the verbosity level.",
}
C_SETUP_CAN_ADAPTERS = {
    "type": click.Choice(SUPPORTED_CAN_ADAPTERS, case_sensitive=True),
    "help": "CAN adapter to be used.",
}
C_SETUP_BAUD_RATE = {
    "type": click.Choice([str(i) for i in BAUD_RATES]),
    "help": "BAUD rate for the chosen CAN adapter.",
}
C_SETUP_LOGGING = {
    "type": click.Path(
        exists=True,
        file_okay=False,
        dir_okay=True,
        writable=True,
        readable=True,
        resolve_path=True,
        allow_dash=False,
    ),
    "is_eager": True,
    "help": "DIRECTORY path to store CAN log files.",
}


def chose_can_adapter():
    """Helper to select a CAN adapter"""
    available_can_adapters = detect_can_adapters()
    if not available_can_adapters:
        logging.debug("Could not find any CAN adapter.")
        return None
    if len(available_can_adapters) > 1:
        user_abort = False
        choices = [str(i) for i in available_can_adapters]
        with wx.SingleChoiceDialog(
            None,
            "Select CAN adapter to be used",
            "CAN Adapter Selection",
            choices,
            wx.CHOICEDLG_STYLE,
        ) as can_adapter_selection_dialog:
            can_adapter_selection_dialog.SetFocus()
            can_adapter_selection_dialog.SetIcon(get_icon())
            if can_adapter_selection_dialog.ShowModal() != wx.ID_OK:
                logging.error("No CAN adapter selected.")
                user_abort = True
            else:
                selected_can_adapter_str = (
                    can_adapter_selection_dialog.GetStringSelection()
                )
        if user_abort:
            for i in available_can_adapters:
                i.shutdown()
            return None
        selected_can_adapter = None
        for i in available_can_adapters:
            if str(i) == selected_can_adapter_str:
                selected_can_adapter = i
            else:
                i.shutdown()  # they are not selected, shutdown immediately
    else:
        selected_can_adapter = available_can_adapters[0]
        selected_can_adapter_str = str(selected_can_adapter)
    selected_can_adapter.shutdown()
    return selected_can_adapter.channel_info


def chose_baud_rate():
    """Helper to select the BAUD rate, if a CAN adapter was specified but no
    BAUD rate."""
    user_abort = False
    choices = [f"{i} 1/s" for i in BAUD_RATES]
    with wx.SingleChoiceDialog(
        None, "Select baud rate to be used", "baud rate", choices, wx.CHOICEDLG_STYLE
    ) as baud_rate_dialog:
        baud_rate_dialog.SetIcon(get_icon())
        selected_baud_rate_str = baud_rate_dialog.SetSelection(DEFAULT_BAUD_RATE_INDEX)
        if baud_rate_dialog.ShowModal() != wx.ID_OK:
            user_abort = True
        else:
            selected_baud_rate_str = baud_rate_dialog.GetStringSelection()
    if user_abort:
        return None
    baud_rate = int(selected_baud_rate_str.split(" ")[0])
    return baud_rate


def chose_logging_dir(logging_dir):
    """Helper to select the logging directory for the CAN messages."""
    if not logging_dir:
        with wx.DirDialog(
            None,
            "Select logging directory",
            defaultPath=str(os.getcwd()),
        ) as dir_dialog:
            if dir_dialog.ShowModal() == wx.ID_CANCEL:
                pass
            else:
                logging_dir = dir_dialog.GetPath()
    return logging_dir


def validate_argument_combination(can_adapter, baud_rate, logging_dir):
    """Validates that the combination of provided arguments on the command line
    is meaningful."""
    if not can_adapter and baud_rate:
        logging.error(
            (
                "BAUD rate supplied without supplying a CAN adapter. "
                "The option is ignored."
            )
        )
    if not can_adapter and logging_dir:
        logging.error(
            (
                "Logging directory supplied without supplying a CAN adapter. "
                "The option is ignored."
            )
        )
