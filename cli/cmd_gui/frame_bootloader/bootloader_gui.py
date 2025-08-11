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

"""Implements the 'bootloader' frame"""

import tkinter as tk
from threading import Thread
from tkinter import ttk
from typing import TextIO

import click
from click import exceptions

from ...commands.c_bootloader import cmd_load_app
from ...helpers import fcan, io
from ...helpers.misc import PROJECT_BUILD_ROOT


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class BootloaderFrame(ttk.Frame):
    """'Bootloader' Frame"""

    def __init__(self, parent, text_widget: tk.Text) -> None:
        super().__init__(parent)
        self.parent: ttk.Notebook = parent
        self.text = text_widget
        self.text_index: int = 0
        self.bootloader_process: Thread
        self.file_path = PROJECT_BUILD_ROOT / "output_gui_bootloader.txt"
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)
        self.file_path.touch()
        self.file_stream: TextIO

        self.columnconfigure(0, weight=1)

        # Create Frame for input information
        bus_frame = ttk.Labelframe(
            self, text="Can Bus Configuration", padding=(10, 7, 10, 7)
        )
        bus_frame.grid(column=0, row=0, pady=(15, 5))
        bus_frame.columnconfigure(1, weight=1)

        bus_interface_label = ttk.Label(self, text="Interface", width=15)
        bus_interface_label.grid(
            in_=bus_frame, column=0, row=0, pady=(5, 5), sticky="news"
        )

        self.bus_interface_combobox = ttk.Combobox(
            self, width=30, values=fcan.SUPPORTED_INTERFACES
        )
        self.bus_interface_combobox.grid(
            in_=bus_frame, column=1, row=0, pady=(5, 5), sticky="news"
        )
        self.bus_interface_combobox.bind(
            "<<ComboboxSelected>>", self.change_interface_cb
        )
        self.bus_interface_combobox.current(0)

        bus_channel_label = ttk.Label(self, text="Channel", width=15)
        bus_channel_label.grid(
            in_=bus_frame, column=0, row=1, pady=(5, 5), sticky="news"
        )
        self.bus_channel_combobox = ttk.Combobox(
            self,
            width=30,
            values=[
                str(i)
                for i in fcan.SUPPORTED_CHANNELS[self.bus_interface_combobox.get()]
            ],
        )
        self.bus_channel_combobox.grid(
            in_=bus_frame, column=1, row=1, pady=(5, 5), sticky="news"
        )
        self.bus_channel_combobox.current(0)

        bus_bitrate_label = ttk.Label(self, text="Bitrate", width=15)
        bus_bitrate_label.grid(
            in_=bus_frame, column=0, row=2, pady=(5, 5), sticky="news"
        )
        self.bus_bitrate_combobox = ttk.Combobox(
            self, width=30, values=fcan.VALID_BIT_RATES
        )
        self.bus_bitrate_combobox.grid(
            in_=bus_frame, column=1, row=2, pady=(5, 5), sticky="news"
        )
        self.bus_bitrate_combobox.current(0)

        # Create Frame for Buttons
        button_frame = ttk.Frame(self)
        button_frame.grid(column=0, row=1, pady=(10, 0))
        button_frame.columnconfigure(0, weight=1)
        button_frame.rowconfigure(0, weight=1)
        # Create Button to run load-app
        self.load_app_button = ttk.Button(
            self, text="Load App", command=self.load_app_command_cb
        )
        self.load_app_button.grid(in_=button_frame, column=0, row=0)

    def change_interface_cb(self, event) -> None:
        """Select the corresponding channel to the interface if possible"""
        bus_interface = self.bus_interface_combobox.get()
        if bus_interface in fcan.DEFAULT_CHANNELS:
            self.bus_channel_combobox.set(
                fcan.SUPPORTED_CHANNELS[self.bus_interface_combobox.get()][0]
            )
            self.bus_channel_combobox["values"] = fcan.SUPPORTED_CHANNELS[
                self.bus_interface_combobox.get()
            ]

    def load_app_command_cb(self) -> None:
        """Start the bootloader-process to run load_app"""
        self.text_index = 0
        self.text.config(state="normal")
        self.text.delete("1.0", tk.END)
        self.text.config(state="disabled")
        self.load_app_button.state(["disabled"])
        bus_channel = self.bus_channel_combobox.get().strip()
        bus_interface = self.bus_interface_combobox.get().strip()
        bus_bitrate = self.bus_bitrate_combobox.get().strip()
        bus_timeout = None
        self.file_path.write_text("Started the load process\n")
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="a", encoding="utf-8")
        io.STDERR = self.file_stream
        io.STDOUT = self.file_stream
        self.bootloader_process = Thread(
            target=self.run_load_app,
            kwargs={
                "interface": bus_interface,
                "channel": bus_channel,
                "timeout": bus_timeout,
                "bitrate": bus_bitrate,
            },
            daemon=True,
        )
        self.bootloader_process.start()
        self.check_thread()

    # pylint: disable-next=useless-return
    def run_load_app(self, timeout, interface, channel, bitrate) -> None:
        """Run load_app"""
        try:
            kwargs = {"interface": interface, "channel": channel, "bitrate": bitrate}
            if timeout is not None:
                kwargs["timeout"] = timeout
            context = click.Context(cmd_load_app)
            context.invoke(cmd_load_app, **kwargs)
        except exceptions.Exit as e:
            if e.exit_code == 0:
                return

    def check_thread(self) -> None:
        """If the provided thread is not alive the button is activated"""
        self.write_text()
        if self.bootloader_process.is_alive():
            self.after(50, self.check_thread)
        else:
            self.load_app_button.state(["!disabled"])
            self.file_stream.close()
            io.STDERR = None
            io.STDOUT = None

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
            if text_length > 0:
                self.text_index = text_length
            self.text.see(tk.END)
        self.text.config(state="disabled")
