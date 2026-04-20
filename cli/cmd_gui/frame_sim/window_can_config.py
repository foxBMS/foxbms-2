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

"""Implements the 'CAN Configuration' window"""

import tkinter as tk
from tkinter import ttk
from typing import TYPE_CHECKING

from ...helpers import fcan
from ...helpers.misc import APP_DBC_FILE

if TYPE_CHECKING:
    from .sim_gui import SimulateBmsFrame


# pylint: disable-next=too-many-ancestors
class CanConfigWindow(tk.Toplevel):
    """CAN Configuration Window"""

    def __init__(self, parent: "SimulateBmsFrame", root: tk.Misc) -> None:
        super().__init__(parent)
        self.parent = parent
        self.title("CAN Configuration")
        self.resizable(False, False)
        root_x, root_y = root.winfo_x(), root.winfo_y()
        padx, pady = int(0.1 * root.winfo_width()), int(0.1 * root.winfo_height())
        self.geometry(f"+{root_x + padx}+{root_y + pady}")

        can_bus_bms_config: dict = {"can_bus_name": "bms"}
        can_bus_unit_config: dict = {"can_bus_name": "unit"}

        ## Create Frame for Configuration of Can Bus Host
        can_bus_bms_frame = ttk.Labelframe(self, text="Can Bus BMS", padding=(10, 5))
        can_bus_bms_frame.pack(side=tk.LEFT, fill=tk.BOTH, pady=(10, 10), padx=(10, 5))
        can_bus_bms_frame.columnconfigure(1, weight=1)

        ttk.Label(can_bus_bms_frame, text="Interface", width=10).grid(
            column=0, row=0, pady=(0, 5), sticky="news"
        )
        ttk.Label(can_bus_bms_frame, text="Channel", width=10).grid(
            column=0, row=1, pady=5, sticky="news"
        )
        ttk.Label(can_bus_bms_frame, text="Bitrate", width=10).grid(
            column=0, row=2, pady=5, sticky="news"
        )

        # Configure Combobox Widgets
        can_bus_bms_config["interface"] = ttk.Combobox(
            self, width=20, values=fcan.SUPPORTED_INTERFACES
        )
        can_bus_bms_config["interface"].grid(
            in_=can_bus_bms_frame, column=1, row=0, pady=(0, 5), sticky="news"
        )
        can_bus_bms_config["interface"].current(0)
        can_bus_bms_config["interface"].bind(
            "<<ComboboxSelected>>",
            lambda e: self.change_interface_cb(e, can_bus_bms_config),
        )

        can_bus_bms_config["channel"] = ttk.Combobox(
            self,
            width=20,
            values=[
                str(i)
                for i in fcan.SUPPORTED_CHANNELS[can_bus_bms_config["interface"].get()]
            ],
        )
        can_bus_bms_config["channel"].grid(
            in_=can_bus_bms_frame, column=1, row=1, pady=5, sticky="news"
        )
        can_bus_bms_config["channel"].current(0)

        can_bus_bms_config["bitrate"] = ttk.Combobox(
            self, width=20, values=fcan.VALID_BIT_RATES
        )
        can_bus_bms_config["bitrate"].grid(
            in_=can_bus_bms_frame, column=1, row=2, pady=5, sticky="news"
        )
        can_bus_bms_config["bitrate"].current(0)

        # Add frame for "Add CAN Bus" button
        can_bus_bms_button_frame = ttk.Frame(can_bus_bms_frame, padding=(0, 5))
        can_bus_bms_button_frame.grid(
            column=0,
            columnspan=2,
            row=3,
            sticky="news",
            pady=(5, 0),
        )
        # Create a "Add CAN Bus" button
        add_can_bus_bms_button = ttk.Button(
            can_bus_bms_button_frame,
            text="Add CAN Bus",
            padding=(5, 0),
        )
        add_can_bus_bms_button.pack()
        add_can_bus_bms_button.bind(
            "<Button>", lambda e: self.add_can_bus_cb(e, can_bus_bms_config)
        )

        if hasattr(self.parent, "can_bus_bms"):
            can_bus_bms_config["interface"].set(self.parent.can_bus_bms.interface)
            self.change_interface_cb(tk.Event(), can_bus_bms_config)
            can_bus_bms_config["channel"].set(self.parent.can_bus_bms.channel)
            can_bus_bms_config["bitrate"].set(self.parent.can_bus_bms.bitrate)

        ## Create Frame for Configuration of Can Bus Unit
        can_bus_unit_frame = ttk.Labelframe(self, text="Can Bus Unit", padding=(10, 5))
        can_bus_unit_frame.pack(side=tk.LEFT, fill=tk.BOTH, pady=(10, 10), padx=(5, 10))
        can_bus_unit_frame.columnconfigure(1, weight=1)

        ttk.Label(self, text="Interface", width=10).grid(
            in_=can_bus_unit_frame, column=0, row=0, pady=(0, 5), sticky="news"
        )
        can_bus_unit_config["interface"] = ttk.Combobox(
            self, width=20, values=fcan.SUPPORTED_INTERFACES
        )
        can_bus_unit_config["interface"].grid(
            in_=can_bus_unit_frame, column=1, row=0, pady=(0, 5), sticky="news"
        )
        can_bus_unit_config["interface"].current(0)
        can_bus_unit_config["interface"].bind(
            "<<ComboboxSelected>>",
            lambda e: self.change_interface_cb(e, can_bus_unit_config),
        )

        ttk.Label(self, text="Channel", width=10).grid(
            in_=can_bus_unit_frame, column=0, row=1, pady=5, sticky="news"
        )
        can_bus_unit_config["channel"] = ttk.Combobox(
            self,
            width=20,
            values=[
                str(i)
                for i in fcan.SUPPORTED_CHANNELS[can_bus_unit_config["interface"].get()]
            ],
        )
        can_bus_unit_config["channel"].grid(
            in_=can_bus_unit_frame, column=1, row=1, pady=5, sticky="news"
        )
        can_bus_unit_config["channel"].current(0)

        ttk.Label(self, text="Bitrate", width=10).grid(
            in_=can_bus_unit_frame, column=0, row=2, pady=5, sticky="news"
        )
        can_bus_unit_config["bitrate"] = ttk.Combobox(
            self, width=20, values=fcan.VALID_BIT_RATES
        )
        can_bus_unit_config["bitrate"].grid(
            in_=can_bus_unit_frame, column=1, row=2, pady=5, sticky="news"
        )
        can_bus_unit_config["bitrate"].current(0)

        # Add frame for "Add CAN Bus" button
        can_bus_unit_button_frame = ttk.Frame(self, padding=(0, 5))
        can_bus_unit_button_frame.grid(
            in_=can_bus_unit_frame,
            column=0,
            columnspan=2,
            row=3,
            sticky="news",
            pady=(5, 0),
        )
        # Create a "Add CAN Bus" button
        add_can_bus_unit_button = ttk.Button(self, text="Add CAN Bus", padding=(5, 0))
        add_can_bus_unit_button.pack(in_=can_bus_unit_button_frame)

        add_can_bus_unit_button.bind(
            "<Button>", lambda e: self.add_can_bus_cb(e, can_bus_unit_config)
        )

        if hasattr(self.parent, "can_bus_unit"):
            can_bus_unit_config["interface"].set(self.parent.can_bus_unit.interface)
            self.change_interface_cb(tk.Event(), can_bus_unit_config)
            can_bus_unit_config["channel"].set(self.parent.can_bus_unit.channel)
            can_bus_unit_config["bitrate"].set(self.parent.can_bus_unit.bitrate)

    def change_interface_cb(self, event: tk.Event, can_bus_config: dict) -> None:
        """Select the corresponding channel to the interface if possible"""
        bus_interface = can_bus_config["interface"].get()
        if bus_interface in fcan.DEFAULT_CHANNELS:
            can_bus_config["channel"].set(fcan.SUPPORTED_CHANNELS[bus_interface][0])
            can_bus_config["channel"]["values"] = fcan.SUPPORTED_CHANNELS[bus_interface]

    def add_can_bus_cb(self, event: tk.Event, can_bus_config: dict) -> None:
        """Read the CAN configuration and pass it to the 'SimulateBMS' frame"""
        bus_channel = can_bus_config["channel"].get().strip()
        bus_interface = can_bus_config["interface"].get().strip()
        bus_bitrate = can_bus_config["bitrate"].get().strip()
        try:
            can_config = fcan.CanBusConfig(
                interface=bus_interface,
                bitrate=int(bus_bitrate),
                channel=bus_channel,
                dbc=APP_DBC_FILE,
            )
        except SystemExit as e:
            self.parent.write_text(f"Invalid input: {e}.\n")
            return
        if can_bus_config["can_bus_name"] == "bms":
            self.parent.can_bus_bms = can_config
        elif can_bus_config["can_bus_name"] == "unit":
            self.parent.can_bus_unit = can_config
        self.parent.write_text("Configuration has been added.\n")
