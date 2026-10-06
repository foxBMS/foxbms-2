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

"""Implements the 'Run Plot' frame"""

import tkinter as tk
from pathlib import Path
from tkinter import filedialog as fd
from tkinter import ttk
from typing import TYPE_CHECKING

from ...cmd_plot.data_handling.data_source_types import DataSourceTypes
from ...helpers.project_context import PROJECT_BUILD_ROOT, PROJECT_ROOT

if TYPE_CHECKING:  # pragma: no cover
    from .plot_gui import PlotFrame


# pylint: disable-next=too-many-ancestors
class RunPlotFrame(ttk.Frame):
    """'Run Plot' Frame"""

    def __init__(self, parent: ttk.Notebook, root: "PlotFrame") -> None:
        super().__init__(parent)
        self.root = root

        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        example_data_source_file = PROJECT_ROOT / Path(
            "docs/tools/fox/plot/img/example_data.csv"
        )
        example_data_config_file = PROJECT_ROOT / Path(
            "docs/tools/fox/plot/img/csv_config.yaml"
        )
        example_plot_config_file = PROJECT_ROOT / Path(
            "docs/tools/fox/plot/img/plot_config.yaml"
        )

        input_frame = ttk.Frame(self)
        input_frame.grid(column=0, row=0, padx=20, pady=(10, 0), sticky="nwe")
        input_frame.columnconfigure(1, weight=1)
        input_frame.rowconfigure((0, 1, 2, 3, 4), weight=1, uniform="a")

        # Data Source
        ttk.Label(input_frame, text="Data-Source File").grid(
            column=0, row=0, padx=(0, 10), pady=5, sticky="we"
        )
        self.data_source_entry = ttk.Entry(input_frame)
        self.data_source_entry.grid(column=1, row=0, pady=5, sticky="we")
        if example_data_source_file.is_file():
            self.data_source_entry.insert(tk.END, str(example_data_source_file))
        ttk.Button(
            input_frame,
            text="Select File",
            command=lambda: self.select_file_cb(self.data_source_entry),
        ).grid(column=2, row=0, pady=5, sticky="we")

        # Output Directory
        ttk.Label(input_frame, text="Output Directory").grid(
            column=0, row=1, padx=(0, 10), pady=5, sticky="we"
        )
        self.output_directory_entry = ttk.Entry(input_frame)
        self.output_directory_entry.grid(column=1, row=1, pady=5, sticky="we")
        self.output_directory_entry.insert(tk.END, str(PROJECT_BUILD_ROOT / "gui"))

        ttk.Button(
            input_frame,
            text="Select\nDirectory",
            command=self.select_directory_cb,
            style="Multiline.TButton",
        ).grid(column=2, row=1, pady=2, sticky="we")

        # Data Type
        ttk.Label(input_frame, text="Data Type").grid(
            column=0, row=2, padx=(0, 10), pady=5, sticky="we"
        )
        self.data_type_entry = ttk.Combobox(
            input_frame,
            values=DataSourceTypes._member_names_,  # pylint: disable=no-member
        )
        self.data_type_entry.grid(column=1, row=2, pady=5, sticky="we")
        self.data_type_entry.current(0)

        # Data Configuration
        ttk.Label(input_frame, text="Data-Configuration File").grid(
            column=0, row=3, padx=(0, 10), pady=5, sticky="we"
        )
        self.data_config_entry = ttk.Entry(input_frame)
        self.data_config_entry.grid(column=1, row=3, pady=5, sticky="we")
        if example_data_config_file.is_file():
            self.data_config_entry.insert(tk.END, str(example_data_config_file))

        ttk.Button(
            input_frame,
            text="Select File",
            command=lambda: self.select_file_cb(self.data_config_entry),
        ).grid(column=2, row=3, pady=5, sticky="we")

        # Plot Configuration
        ttk.Label(input_frame, text="Plot-Configuration File").grid(
            column=0, row=4, padx=(0, 10), pady=5, sticky="we"
        )
        self.plot_config_entry = ttk.Entry(input_frame)
        self.plot_config_entry.grid(column=1, row=4, pady=5, sticky="we")
        if example_plot_config_file.is_file():
            self.plot_config_entry.insert(tk.END, str(example_plot_config_file))

        ttk.Button(
            input_frame,
            text="Select File",
            command=lambda: self.select_file_cb(self.plot_config_entry),
        ).grid(column=2, row=4, pady=5, sticky="we")

    def select_file_cb(self, entry_object: ttk.Entry) -> None:
        """Open filedialog and write selected item into Entry widget"""
        file_path = fd.askopenfilename()
        if file_path:
            entry_object.delete(0, tk.END)
            entry_object.insert(tk.END, file_path)

    def select_directory_cb(self) -> None:
        """Open directory dialog and write selected item into Entry widget"""
        directory = fd.askdirectory()
        if directory:
            self.output_directory_entry.delete(0, tk.END)
            self.output_directory_entry.insert(tk.END, directory)
