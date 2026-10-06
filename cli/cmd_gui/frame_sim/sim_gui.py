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

import threading
import tkinter as tk
from queue import Queue
from threading import Thread
from tkinter import ttk
from typing import TYPE_CHECKING

from ...com.can_com import CAN
from ..frame_base import BaseFrame
from .sim_bms_impl import sim_bms
from .sim_unit_impl import sim_unit
from .window_can_config import CanConfigWindow

if TYPE_CHECKING:
    from ...helpers.fcan import CanBusConfig


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class SimulateBmsFrame(BaseFrame):
    """'Simulate BMS' frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent, text_widget, "output_gui_sim.txt")
        self.bms_process: Thread
        self.unit_process: Thread
        self.log_queue: Queue[str] = Queue()

        self.can_bus_bms: CanBusConfig
        self.can_bus_unit: CanBusConfig

        self.can_com_bms: CAN
        self.can_com_unit: CAN

        self.sim_active: bool = False

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
        """Start and stop the simulation"""
        if not hasattr(self, "can_bus_bms") or not hasattr(self, "can_bus_unit"):
            self.update_text("Add CAN Configurations before starting the Simulation.\n")
            return
        if self.sim_active:
            self.update_text("Stopping the Simulation...\n")
            if hasattr(self, "can_com_bms"):
                self.can_com_bms.shutdown(block=True, timeout=1)
            if hasattr(self, "can_com_unit"):
                self.can_com_unit.shutdown(block=True, timeout=1)
        else:
            self.bms_process = Thread(target=self.run_bms_sim, daemon=True)
            self.unit_process = Thread(target=self.run_unit_sim, daemon=True)
            self.update_text("Starting the Simulation...\n")
            self.bms_process.start()
            self.unit_process.start()
            self.sim_active = True
            self.check_threads()

    def send_msg_cb(self) -> None:
        """Configure and send the selected message"""
        if not self.sim_active:
            self.update_text("Start the Simulation before sending messages.\n")
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
            self.update_text("Sending message.\n")
            self.can_com_unit.write(msg_data)
        except Exception as e:  # noqa: BLE001
            self.update_text(str(e) + "\n")

    def run_bms_sim(self) -> None:
        """Run the bms simulation"""
        self.can_com_bms = CAN("CAN Bus BMS", self.can_bus_bms)
        sim_bms(self.can_com_bms, self.log_queue)
        self.can_com_unit.shutdown(block=True, timeout=1)

    def run_unit_sim(self) -> None:
        """Run the higher-level control unit simulation"""
        self.can_com_unit = CAN("CAN Bus Unit", self.can_bus_unit)
        sim_unit(self.can_com_unit, self.log_queue)
        self.can_com_bms.shutdown(block=True, timeout=1)

    def check_threads(self) -> None:
        """Stop simulation if both threads are not alive"""
        self.update_text()
        if self.bms_process.is_alive() or self.unit_process.is_alive():
            self.after(50, self.check_threads)
        else:
            self.sim_active = False
            if hasattr(self, "can_com_bms"):
                del self.can_com_bms
            if hasattr(self, "can_com_unit"):
                del self.can_com_unit
            self.update_text("Simulation has terminated.\n")

    def write_text_from_queue(self) -> None:
        """Flush text generated by worker threads into the log file from the UI thread."""
        while not self.log_queue.empty():
            self.write_text(self.log_queue.get_nowait())

    def update_text(self, file_input: str | None = None) -> None:
        """Writes the file content in the text box"""
        if file_input is not None:
            if threading.current_thread() is threading.main_thread():
                self.write_text(file_input)
            else:
                self.log_queue.put(file_input)
                return
        self.write_text_from_queue()
        self.write_text()

    def on_close(self) -> None:
        """Stop the simulation if it is active"""
        if self.sim_active:
            self.start_stop_sim_cb()
        super().on_close()
