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
from collections.abc import Generator
from contextlib import contextmanager
from pathlib import Path
from threading import Thread
from tkinter import filedialog as fd
from tkinter import ttk
from typing import TextIO

import click
from click import exceptions

from ...commands.c_bootloader import cmd_load_app
from ...helpers import fcan, io
from ...helpers.project_context import (
    APP_DBC_FILE,
    BOOTLOADER_DBC_FILE,
    FOXBMS_APP_CRC_FILE,
    FOXBMS_APP_INFO_FILE,
    FOXBMS_BIN_FILE,
)
from ..frame_base import BaseFrame


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class BootloaderFrame(BaseFrame):
    """'Bootloader' Frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent, text_widget, "output_gui_bootloader.txt")
        self.bootloader_process: Thread

        self.columnconfigure(0, weight=1)
        self.rowconfigure((0, 1), weight=1)

        # Create Frame for File Path input information
        file_frame = ttk.Labelframe(
            self, text="File Path Configuration", padding=(10, 5)
        )
        file_frame.grid(
            column=0, columnspan=2, row=0, pady=(15, 5), padx=15, sticky="nwe"
        )
        file_frame.columnconfigure(1, weight=1)
        file_frame.rowconfigure((0, 1, 2, 3, 4), weight=1)

        ttk.Label(file_frame, text="Bootloader DBC file").grid(
            column=0, row=0, padx=(0, 10), pady=(0, 5), sticky="we"
        )

        self.bootloader_dbc_entry = ttk.Entry(file_frame)
        self.bootloader_dbc_entry.grid(column=1, row=0, pady=(0, 5), sticky="we")
        if BOOTLOADER_DBC_FILE.is_file():
            self.bootloader_dbc_entry.insert(tk.END, str(BOOTLOADER_DBC_FILE))

        ttk.Button(
            file_frame,
            text="Select File",
            command=lambda: self.select_file_cb("dbc", self.bootloader_dbc_entry),
        ).grid(column=2, row=0, pady=(0, 5), sticky="we")

        ttk.Label(file_frame, text="App DBC file").grid(
            column=0, row=1, padx=(0, 10), pady=5, sticky="we"
        )

        self.app_dbc_entry = ttk.Entry(file_frame)
        self.app_dbc_entry.grid(column=1, row=1, pady=5, sticky="we")
        if APP_DBC_FILE.is_file():
            self.app_dbc_entry.insert(tk.END, str(APP_DBC_FILE))

        ttk.Button(
            file_frame,
            text="Select File",
            command=lambda: self.select_file_cb("dbc", self.app_dbc_entry),
        ).grid(column=2, row=1, pady=5, sticky="we")

        ttk.Label(self, text="foxBMS binary file").grid(
            in_=file_frame, column=0, row=2, padx=(0, 10), pady=5, sticky="we"
        )

        self.foxbms_bin_entry = ttk.Entry(file_frame)
        self.foxbms_bin_entry.grid(column=1, row=2, pady=5, sticky="we")
        if FOXBMS_BIN_FILE.is_file():
            self.foxbms_bin_entry.insert(tk.END, str(FOXBMS_BIN_FILE))

        ttk.Button(
            file_frame,
            text="Select File",
            command=lambda: self.select_file_cb("bin", self.foxbms_bin_entry),
        ).grid(column=2, row=2, pady=5, sticky="we")

        ttk.Label(file_frame, text="foxBMS CRC CSV file").grid(
            column=0, row=3, padx=(0, 10), pady=5, sticky="we"
        )

        self.foxbms_crc_csv_entry = ttk.Entry(file_frame)
        self.foxbms_crc_csv_entry.grid(column=1, row=3, pady=5, sticky="we")
        if FOXBMS_APP_CRC_FILE.is_file():
            self.foxbms_crc_csv_entry.insert(tk.END, str(FOXBMS_APP_CRC_FILE))

        ttk.Button(
            file_frame,
            text="Select File",
            command=lambda: self.select_file_cb("csv", self.foxbms_crc_csv_entry),
        ).grid(column=2, row=3, pady=5, sticky="we")

        ttk.Label(file_frame, text="foxBMS CRC JSON file").grid(
            column=0, row=4, padx=(0, 10), pady=5, sticky="we"
        )

        self.foxbms_crc_json_entry = ttk.Entry(file_frame)
        self.foxbms_crc_json_entry.grid(column=1, row=4, pady=5, sticky="we")
        if FOXBMS_APP_INFO_FILE.is_file():
            self.foxbms_crc_json_entry.insert(tk.END, str(FOXBMS_APP_INFO_FILE))

        ttk.Button(
            file_frame,
            text="Select File",
            command=lambda: self.select_file_cb("json", self.foxbms_crc_json_entry),
        ).grid(column=2, row=4, pady=5, sticky="we")

        # Create Frame for Can Bus Configuration input information
        bus_frame = ttk.Labelframe(self, text="Can Bus Configuration", padding=(10, 5))
        bus_frame.grid(column=0, row=1, pady=(10, 5), padx=10, sticky="n")
        bus_frame.columnconfigure(1, weight=1)
        bus_frame.rowconfigure((0, 1, 2), weight=1)

        ttk.Label(bus_frame, text="Interface").grid(
            column=0, row=0, padx=(0, 10), pady=(0, 5), sticky="we"
        )

        self.bus_interface_combobox = ttk.Combobox(
            bus_frame, width=30, values=fcan.SUPPORTED_INTERFACES
        )
        self.bus_interface_combobox.grid(column=1, row=0, pady=(0, 5), sticky="we")
        self.bus_interface_combobox.bind(
            "<<ComboboxSelected>>", self.change_interface_cb
        )
        self.bus_interface_combobox.current(0)

        ttk.Label(bus_frame, text="Channel").grid(
            column=0, row=1, padx=(0, 10), pady=5, sticky="we"
        )
        self.bus_channel_combobox = ttk.Combobox(
            bus_frame,
            width=30,
            values=[
                str(i)
                for i in fcan.SUPPORTED_CHANNELS[self.bus_interface_combobox.get()]
            ],
        )
        self.bus_channel_combobox.grid(column=1, row=1, pady=5, sticky="we")
        self.bus_channel_combobox.current(0)

        ttk.Label(bus_frame, text="Bitrate").grid(
            column=0, row=2, padx=(0, 10), pady=5, sticky="we"
        )
        self.bus_bitrate_combobox = ttk.Combobox(
            bus_frame, width=30, values=fcan.VALID_BIT_RATES
        )
        self.bus_bitrate_combobox.grid(column=1, row=2, pady=5, sticky="we")
        self.bus_bitrate_combobox.current(0)

        # Create Button to run load-app
        self.load_app_button = ttk.Button(
            self, text="Load App", command=self.load_app_command_cb
        )
        self.load_app_button.grid(column=0, row=2, pady=5, sticky="s")

    def select_file_cb(self, file_type: str, entry_object: ttk.Entry) -> None:
        """Open filedialog and write selected item into Entry widget"""
        file_path = fd.askopenfilename(
            filetypes=[(f"{file_type.upper()} Files", f"*.{file_type}")]
        )
        if file_path:
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
        self.reset_text()
        bus_channel = self.bus_channel_combobox.get().strip()
        bus_interface = self.bus_interface_combobox.get().strip()
        bus_bitrate = self.bus_bitrate_combobox.get().strip()
        bus_timeout = None
        bootloader_dbc_file = Path(self.bootloader_dbc_entry.get().strip())
        app_dbc_file = Path(self.app_dbc_entry.get().strip())
        foxbms_bin_file = Path(self.foxbms_bin_entry.get().strip())
        foxbms_crc_csv_file = Path(self.foxbms_crc_csv_entry.get().strip())
        foxbms_crc_json_file = Path(self.foxbms_crc_json_entry.get().strip())
        required_files = {
            "Bootloader DBC": bootloader_dbc_file,
            "App DBC": app_dbc_file,
            "foxBMS binary": foxbms_bin_file,
            "foxBMS CRC CSV": foxbms_crc_csv_file,
            "foxBMS CRC JSON": foxbms_crc_json_file,
        }
        invalid_inputs = [
            name for name, path in required_files.items() if not path.is_file()
        ]
        if invalid_inputs:
            self.write_text(
                "Invalid input files: "
                + ", ".join(invalid_inputs)
                + ". \nPlease select existing files.\n"
            )
            return
        open(self.file_path, mode="w", encoding="utf-8").close()  # pylint: disable=consider-using-with
        self.load_app_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        kwargs = {
            "interface": bus_interface,
            "channel": bus_channel,
            "bitrate": bus_bitrate,
            "bootloader_dbc": Path(bootloader_dbc_file),
            "app_dbc": Path(app_dbc_file),
            "foxbms_bin": Path(foxbms_bin_file),
            "foxbms_app_crc": Path(foxbms_crc_csv_file),
            "foxbms_app_info": Path(foxbms_crc_json_file),
        }
        self.write_text(
            f"Running load-app with interface={bus_interface}, "
            f"channel={bus_channel}, bitrate={bus_bitrate}.\n"
        )
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="a", encoding="utf-8")  # noqa: SIM115
        self.bootloader_process = Thread(
            target=self.run_load_app,
            kwargs={
                "kwargs": kwargs,
                "timeout": bus_timeout,
            },
            daemon=True,
        )
        self.bootloader_process.start()
        self.check_thread()

    def run_load_app(
        self,
        kwargs: dict[str, str | Path | int | float],
        timeout: float | None,
    ) -> None:
        """Run load_app"""
        if timeout is not None:
            kwargs["timeout"] = timeout
        with self._redirect_io(self.file_stream):
            try:
                context = click.Context(cmd_load_app)
                context.invoke(cmd_load_app, **kwargs)
            except exceptions.Exit as e:
                self.write_text(f"load-app exited with code {e.exit_code}.\n")

    @staticmethod
    @contextmanager
    def _redirect_io(file_stream: TextIO) -> Generator[None, None, None]:
        """Temporarily redirect click output streams during load-app run."""
        old_stderr = io.STDERR
        old_stdout = io.STDOUT
        io.STDERR = file_stream
        io.STDOUT = file_stream
        try:
            yield
        finally:
            io.STDERR = old_stderr
            io.STDOUT = old_stdout

    def check_thread(self) -> None:
        """Disable the 'Load App' button and reset the output streams
        if the provided thread is dead.
        """
        self.write_text()
        if self.bootloader_process.is_alive():
            self.after(50, self.check_thread)
            return
        self.file_stream.close()
        self.load_app_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        if self.file_stream == io.STDERR:
            io.STDERR = None
        if self.file_stream == io.STDOUT:
            io.STDOUT = None
