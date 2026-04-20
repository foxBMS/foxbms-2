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

"""Implements the 'bootloader' frame"""

import tkinter as tk
from pathlib import Path
from threading import Thread
from tkinter import filedialog as fd
from tkinter import ttk
from typing import TextIO

import click
from click import exceptions

from ...commands.c_bootloader import cmd_load_app
from ...helpers import fcan, io
from ...helpers.misc import (
    APP_DBC_FILE,
    BOOTLOADER_DBC_FILE,
    FOXBMS_APP_CRC_FILE,
    FOXBMS_APP_INFO_FILE,
    FOXBMS_BIN_FILE,
    PROJECT_BUILD_ROOT,
)


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class BootloaderFrame(ttk.Frame):
    """'Bootloader' Frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent)
        self.parent = parent
        self.text = text_widget
        self.text_index: int = 0
        self.bootloader_process: Thread
        self.file_path = PROJECT_BUILD_ROOT / "gui" / "output_gui_bootloader.txt"
        (PROJECT_BUILD_ROOT / "gui").mkdir(parents=True, exist_ok=True)
        self.file_path.touch()
        self.file_stream: TextIO

        self.columnconfigure(0, weight=1)
        self.rowconfigure(2, weight=1)

        # Create Frame for File Path input information
        file_frame = ttk.Labelframe(
            self, text="File Path Configuration", padding=(10, 5)
        )
        file_frame.grid(
            column=0, columnspan=2, row=0, pady=(10, 5), padx=10, sticky="news"
        )
        file_frame.columnconfigure(1, weight=1)

        bootloader_dbc_label = ttk.Label(self, text="Bootloader DBC file", width=25)
        bootloader_dbc_label.grid(
            in_=file_frame, column=0, row=0, pady=(0, 5), sticky="news"
        )

        self.bootloader_dbc_entry = ttk.Entry(self, width=30)
        self.bootloader_dbc_entry.grid(
            in_=file_frame, column=1, row=0, pady=(0, 5), sticky="news"
        )
        if BOOTLOADER_DBC_FILE.is_file():
            self.bootloader_dbc_entry.insert(tk.END, str(BOOTLOADER_DBC_FILE))

        bootloader_dbc_button = ttk.Button(
            self,
            text="Choose file",
            command=lambda: self.open_file_cb("dbc", self.bootloader_dbc_entry),
        )
        bootloader_dbc_button.grid(
            in_=file_frame, column=2, row=0, pady=(0, 5), sticky="news"
        )

        app_dbc_label = ttk.Label(self, text="App DBC file", width=25)
        app_dbc_label.grid(in_=file_frame, column=0, row=1, pady=5, sticky="news")

        self.app_dbc_entry = ttk.Entry(self, width=30)
        self.app_dbc_entry.grid(in_=file_frame, column=1, row=1, pady=5, sticky="news")
        if APP_DBC_FILE.is_file():
            self.app_dbc_entry.insert(tk.END, str(APP_DBC_FILE))

        app_dbc_button = ttk.Button(
            self,
            text="Choose file",
            command=lambda: self.open_file_cb("dbc", self.app_dbc_entry),
        )
        app_dbc_button.grid(in_=file_frame, column=2, row=1, pady=5, sticky="news")

        foxbms_bin_label = ttk.Label(self, text="foxBMS binary file", width=25)
        foxbms_bin_label.grid(in_=file_frame, column=0, row=2, pady=5, sticky="news")

        self.foxbms_bin_entry = ttk.Entry(self, width=30)
        self.foxbms_bin_entry.grid(
            in_=file_frame, column=1, row=2, pady=5, sticky="news"
        )
        if FOXBMS_BIN_FILE.is_file():
            self.foxbms_bin_entry.insert(tk.END, str(FOXBMS_BIN_FILE))

        foxbms_bin_button = ttk.Button(
            self,
            text="Choose file",
            command=lambda: self.open_file_cb("bin", self.foxbms_bin_entry),
        )
        foxbms_bin_button.grid(in_=file_frame, column=2, row=2, pady=5, sticky="news")

        foxbms_crc_csv_label = ttk.Label(self, text="foxBMS CRC CSV file", width=25)
        foxbms_crc_csv_label.grid(
            in_=file_frame, column=0, row=3, pady=5, sticky="news"
        )

        self.foxbms_crc_csv_entry = ttk.Entry(self, width=30)
        self.foxbms_crc_csv_entry.grid(
            in_=file_frame, column=1, row=3, pady=5, sticky="news"
        )
        if FOXBMS_APP_CRC_FILE.is_file():
            self.foxbms_crc_csv_entry.insert(tk.END, str(FOXBMS_APP_CRC_FILE))

        foxbms_crc_csv_button = ttk.Button(
            self,
            text="Choose file",
            command=lambda: self.open_file_cb("csv", self.foxbms_crc_csv_entry),
        )
        foxbms_crc_csv_button.grid(
            in_=file_frame, column=2, row=3, pady=5, sticky="news"
        )

        foxbms_crc_json_label = ttk.Label(self, text="foxBMS CRC JSON file", width=25)
        foxbms_crc_json_label.grid(
            in_=file_frame, column=0, row=4, pady=5, sticky="news"
        )

        self.foxbms_crc_json_entry = ttk.Entry(self, width=30)
        self.foxbms_crc_json_entry.grid(
            in_=file_frame, column=1, row=4, pady=5, sticky="news"
        )
        if FOXBMS_APP_INFO_FILE.is_file():
            self.foxbms_crc_json_entry.insert(tk.END, str(FOXBMS_APP_INFO_FILE))

        foxbms_crc_json_button = ttk.Button(
            self,
            text="Choose file",
            command=lambda: self.open_file_cb("json", self.foxbms_crc_json_entry),
        )
        foxbms_crc_json_button.grid(
            in_=file_frame, column=2, row=4, pady=5, sticky="news"
        )

        # Create Frame for Can Bus Configuration input information
        bus_frame = ttk.Labelframe(self, text="Can Bus Configuration", padding=(10, 5))
        bus_frame.grid(column=0, row=1, pady=(10, 5), padx=10)
        bus_frame.columnconfigure(1, weight=1)

        bus_interface_label = ttk.Label(self, text="Interface", width=15)
        bus_interface_label.grid(
            in_=bus_frame, column=0, row=0, pady=(0, 5), sticky="news"
        )

        self.bus_interface_combobox = ttk.Combobox(
            self, width=40, values=fcan.SUPPORTED_INTERFACES
        )
        self.bus_interface_combobox.grid(
            in_=bus_frame, column=1, row=0, pady=(0, 5), sticky="news"
        )
        self.bus_interface_combobox.bind(
            "<<ComboboxSelected>>", self.change_interface_cb
        )
        self.bus_interface_combobox.current(0)

        bus_channel_label = ttk.Label(self, text="Channel", width=15)
        bus_channel_label.grid(in_=bus_frame, column=0, row=1, pady=5, sticky="news")
        self.bus_channel_combobox = ttk.Combobox(
            self,
            width=40,
            values=[
                str(i)
                for i in fcan.SUPPORTED_CHANNELS[self.bus_interface_combobox.get()]
            ],
        )
        self.bus_channel_combobox.grid(
            in_=bus_frame, column=1, row=1, pady=5, sticky="news"
        )
        self.bus_channel_combobox.current(0)

        bus_bitrate_label = ttk.Label(self, text="Bitrate", width=15)
        bus_bitrate_label.grid(in_=bus_frame, column=0, row=2, pady=5, sticky="news")
        self.bus_bitrate_combobox = ttk.Combobox(
            self, width=40, values=fcan.VALID_BIT_RATES
        )
        self.bus_bitrate_combobox.grid(
            in_=bus_frame, column=1, row=2, pady=5, sticky="news"
        )
        self.bus_bitrate_combobox.current(0)

        # Create Frame for Buttons
        button_frame = ttk.Frame(self)
        button_frame.grid(column=0, row=2, pady=(5, 10), sticky="ns")
        button_frame.columnconfigure(0, weight=1)
        button_frame.rowconfigure(0, weight=1)
        # Create Button to run load-app
        self.load_app_button = ttk.Button(
            self, text="Load App", command=self.load_app_command_cb
        )
        self.load_app_button.grid(in_=button_frame, column=0, row=0)

    def open_file_cb(self, file_type: str, entry_object: ttk.Entry) -> None:
        """Open filedialog and print it in TextBox"""
        file_path = fd.askopenfilename(
            filetypes=[(f"{file_type.upper()} Files", f"*.{file_type}")]
        )
        entry_object.delete(0, tk.END)
        entry_object.insert(tk.END, file_path)

    def change_interface_cb(self, event: tk.Event) -> None:
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
        bootloader_dbc_file = self.bootloader_dbc_entry.get().strip()
        app_dbc_file = self.app_dbc_entry.get().strip()
        foxbms_bin_file = self.foxbms_bin_entry.get().strip()
        foxbms_crc_csv_file = self.foxbms_crc_csv_entry.get().strip()
        foxbms_crc_json_file = self.foxbms_crc_json_entry.get().strip()
        self.file_path.write_text("Started the load process\n")
        # handle is closed in 'check_thread'
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="a", encoding="utf-8")  # noqa: SIM115
        io.STDERR = self.file_stream
        io.STDOUT = self.file_stream
        self.bootloader_process = Thread(
            target=self.run_load_app,
            kwargs={
                "interface": bus_interface,
                "channel": bus_channel,
                "timeout": bus_timeout,
                "bitrate": bus_bitrate,
                "bootloader_dbc": bootloader_dbc_file,
                "app_dbc": app_dbc_file,
                "foxbms_bin": foxbms_bin_file,
                "foxbms_app_crc": foxbms_crc_csv_file,
                "foxbms_app_info": foxbms_crc_json_file,
            },
            daemon=True,
        )
        self.bootloader_process.start()
        self.check_thread()

    def run_load_app(  # noqa: PLR0913
        self,
        timeout: float,
        interface: str,
        channel: str,
        bitrate: int,
        bootloader_dbc: Path,
        app_dbc: Path,
        foxbms_bin: Path,
        foxbms_app_crc: Path,
        foxbms_app_info: Path,
    ) -> None:
        """Run load_app"""
        try:
            kwargs = {
                "interface": interface,
                "channel": channel,
                "bitrate": bitrate,
                "bootloader_dbc": bootloader_dbc,
                "app_dbc": app_dbc,
                "foxbms_bin": foxbms_bin,
                "foxbms_app_crc": foxbms_app_crc,
                "foxbms_app_info": foxbms_app_info,
            }
            if timeout is not None:
                kwargs["timeout"] = timeout
            context = click.Context(cmd_load_app)
            context.invoke(cmd_load_app, **kwargs)
        except exceptions.Exit as e:
            if e.exit_code == 0:
                pass

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
