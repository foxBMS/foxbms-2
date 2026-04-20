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

"""Implements the 'Simulate BMS' frame"""

import tkinter as tk
from threading import Thread
from tkinter import ttk
from typing import TYPE_CHECKING

from ...com.can_com import CAN
from ...helpers.misc import PROJECT_BUILD_ROOT
from .sim_bms_impl import sim_bms
from .sim_unit_impl import sim_unit
from .window_can_config import CanConfigWindow

if TYPE_CHECKING:
    from ...helpers.fcan import CanBusConfig


BMS_RX_MSGS: list[str] = ["f_Debug", "f_BmsStateRequest"]


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class SimulateBmsFrame(ttk.Frame):
    """'Simulate BMS' frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent)
        self.parent = parent
        self.text = text_widget
        self.text_index: int = 0
        self.bms_process: Thread
        self.unit_process: Thread
        self.file_path = PROJECT_BUILD_ROOT / "gui" / "output_gui_sim.txt"
        (PROJECT_BUILD_ROOT / "gui").mkdir(parents=True, exist_ok=True)
        self.file_path.touch()

        self.can_bus_bms: CanBusConfig
        self.can_bus_unit: CanBusConfig

        self.can_com_bms: CAN
        self.can_com_unit: CAN

        self.sim_active: bool = False

        # Set Styles for Headings and for Notebook
        font_heading = ("TkDefaultFont", 10, "bold")
        ttk.Style().configure("heading.TButton", font=font_heading)
        ttk.Style().configure("Multiline.TButton", justify="center")

        ## Create a Button Frame
        config_button_frame = ttk.Frame(self, padding=(0, 5))
        config_button_frame.pack(side=tk.TOP, pady=(5, 0))

        # Create a "Add CAN Configuration" button
        self.open_can_window_button = ttk.Button(
            self,
            text="Add CAN\nConfiguration",
            style="Multiline.TButton",
            padding=5,
            command=lambda: CanConfigWindow(self, self.parent.master),
        )
        self.open_can_window_button.pack(
            in_=config_button_frame, side=tk.LEFT, padx=(0, 5)
        )

        # Create a "Start/Stop Simulation" button
        self.start_stop_button = ttk.Button(
            self,
            text="Start/Stop\nSimulation",
            command=self.start_stop_sim_cb,
            style="Multiline.TButton",
            padding=5,
        )
        self.start_stop_button.pack(in_=config_button_frame, side=tk.LEFT, padx=5)

        # Create a "Send" button
        self.send_msg_button = ttk.Button(
            self,
            text="Send\nMessage",
            command=self.send_msg_cb,
            style="Multiline.TButton",
            padding=5,
        )
        self.send_msg_button.pack(in_=config_button_frame, side=tk.LEFT, padx=(5, 0))

    def start_stop_sim_cb(self) -> None:
        """Start or stop the simulation"""
        if not hasattr(self, "can_bus_bms") or not hasattr(self, "can_bus_unit"):
            self.write_text("Add CAN Configurations before starting the Simulation.\n")
            return
        if self.sim_active:
            self.write_text("Stopping the Simulation...\n")
            if hasattr(self, "can_com_bms"):
                self.can_com_bms.shutdown(block=True, timeout=1)
            if hasattr(self, "can_com_unit"):
                self.can_com_unit.shutdown(block=True, timeout=1)
        else:
            self.bms_process = Thread(target=self.run_sim_bms, daemon=True)
            self.unit_process = Thread(target=self.run_sim_unit, daemon=True)
            self.bms_process.start()
            self.unit_process.start()
            self.sim_active = True
            self.write_text("Starting the Simulation...\n")
            self.check_threads()

    def send_msg_cb(self) -> None:
        """Configure and send the selected message"""
        if not self.sim_active:
            self.write_text("Simulation has to be started first.\n")
            return
        try:
            msg_data = {
                "id": 768,
                "data": {
                    "f_Debug_Mux": 0x04,
                    "RequestRtcTime": 1,
                    "RequestBootTimestamp": 0,
                },
            }
            self.write_text("Sending message.\n")
            self.can_com_unit.write(msg_data)
        except Exception as e:  # noqa: BLE001
            self.write_text(str(e))

    def run_sim_bms(self) -> None:
        """Run the bms simulation"""
        self.can_com_bms = CAN("CAN Bus BMS", self.can_bus_bms)
        sim_bms(self.can_com_bms, self.write_text)
        self.can_com_unit.shutdown(block=True, timeout=1)

    def run_sim_unit(self) -> None:
        """Run the unit simulation"""
        self.can_com_unit = CAN("CAN Bus Unit", self.can_bus_unit)
        sim_unit(self.can_com_unit, self.write_text)
        self.can_com_bms.shutdown(block=True, timeout=1)

    def check_threads(self) -> None:
        """Stop simulation if both threads are not alive"""
        self.write_text()
        if self.bms_process.is_alive() or self.unit_process.is_alive():
            self.after(50, self.check_threads)
        else:
            self.sim_active = False
            if hasattr(self, "can_com_bms"):
                del self.can_com_bms
            if hasattr(self, "can_com_unit"):
                del self.can_com_unit
            self.write_text("Simulation terminated.\n")

    def write_text(self, file_input: None | str = None) -> None:
        """Writes the file content in the text box"""
        if file_input is not None:
            with open(self.file_path, mode="a", encoding="utf-8", errors="ignore") as f:
                f.write(file_input)
        if self != self.parent.nametowidget(self.parent.select()):
            return
        self.text.config(state="normal")
        with open(self.file_path, encoding="utf-8", errors="ignore") as f:
            file_content = f.read()
            text_length = len(file_content)
            self.text.insert(tk.END, file_content[self.text_index :])
            if text_length > self.text_index:
                self.text_index = text_length
                self.text.see(tk.END)
        self.text.config(state="disabled")
