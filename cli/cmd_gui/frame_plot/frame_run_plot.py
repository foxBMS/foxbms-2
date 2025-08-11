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

"""Implements the 'Run Plot' frame"""

import tkinter as tk
from pathlib import Path
from tkinter import filedialog as fd
from tkinter import ttk

from ...cmd_plot.data_handling.data_source_types import DataSourceTypes
from ...helpers.misc import PROJECT_BUILD_ROOT, PROJECT_ROOT


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class RunPlotFrame(ttk.Frame):
    """RunPlot Frame"""

    def __init__(self, parent, root) -> None:
        super().__init__(parent)
        self.root = root

        self.columnconfigure(1, weight=1)
        self.columnconfigure(2, weight=1)
        self.columnconfigure(3, weight=1)
        self.columnconfigure(4, weight=1)

        # Set Styles
        ttk.Style().configure("Multiline.TButton", justify="center")

        # Create Label, Button and text widget for selecting the Data Source File
        data_source_label = ttk.Label(self, text="Data Source", width=17)
        data_source_label.grid(
            column=0, row=0, padx=(20, 0), pady=(20, 5), sticky="news"
        )
        self.data_source_entry = ttk.Entry(self, width=50)
        self.data_source_entry.grid(
            column=1, columnspan=4, row=0, padx=(30, 0), pady=(20, 5), sticky="news"
        )
        self.data_source_entry.insert(
            tk.END, str(PROJECT_ROOT / Path("docs/tools/fox/plot/img/example_data.csv"))
        )
        self.data_source_button = ttk.Button(
            self, text="Choose file", command=self.open_data_source_cb
        )
        self.data_source_button.grid(
            column=5, row=0, padx=(0, 20), pady=(20, 5), sticky="news"
        )

        # Create Label, Button and text widget for selecting directory for Output
        output_label = ttk.Label(
            self, text="Output Directory", justify="left", width=17
        )
        output_label.grid(column=0, row=1, padx=(20, 0), pady=(0, 5), sticky="news")
        self.output_entry = ttk.Entry(self, width=50)
        self.output_entry.grid(
            column=1, columnspan=4, row=1, padx=(30, 0), pady=(0, 5), sticky="we"
        )
        self.output_entry.insert(tk.END, str(PROJECT_BUILD_ROOT))
        self.output_button = ttk.Button(
            self,
            text="Choose\ndirectory",
            command=self.open_output_directory_cb,
            style="Multiline.TButton",
        )
        self.output_button.grid(
            column=5, row=1, padx=(0, 20), pady=(0, 5), sticky="news"
        )

        # Create Label and text widget for the datatype
        data_type_label = ttk.Label(self, text="Data Type", width=17)
        data_type_label.grid(column=0, row=2, padx=(20, 0), pady=(0, 10), sticky="news")
        self.data_type_entry = ttk.Combobox(
            self,
            width=50,
            values=DataSourceTypes._member_names_,  # pylint: disable=no-member
        )
        self.data_type_entry.grid(
            column=1, columnspan=4, row=2, padx=(30, 0), pady=(0, 10), sticky="news"
        )
        self.data_type_entry.current(0)

        # Data Configuration
        # Create text widget and Button to select file
        data_config_label = ttk.Label(self, text="Data Configuration", width=17)
        data_config_label.grid(
            column=0, row=3, padx=(20, 0), pady=(5, 10), sticky="news"
        )
        self.data_config_entry = ttk.Entry(self, width=50)
        self.data_config_entry.insert(
            tk.END, str(PROJECT_ROOT / Path("docs/tools/fox/plot/img/csv_config.yaml"))
        )
        self.data_config_entry.grid(
            column=1, columnspan=4, row=3, padx=(30, 0), pady=(5, 10), sticky="news"
        )
        self.data_config_button = ttk.Button(
            self, text="Choose file", command=self.open_data_config_cb
        )
        self.data_config_button.grid(
            column=5, row=3, padx=(0, 20), pady=(5, 10), sticky="news"
        )

        # Plot Configuration
        # Create text widget and Button to select file
        plot_config_label = ttk.Label(self, text="Plot Configuration", width=17)
        plot_config_label.grid(
            column=0, row=4, padx=(20, 0), pady=(5, 5), sticky="news"
        )
        self.plot_config_entry = ttk.Entry(self, width=50)
        self.plot_config_entry.grid(
            column=1, columnspan=4, row=4, padx=(30, 0), pady=(5, 5), sticky="news"
        )
        self.plot_config_entry.insert(
            tk.END, str(PROJECT_ROOT / Path("docs/tools/fox/plot/img/plot_config.yaml"))
        )
        self.plot_config_button = ttk.Button(
            self, text="Choose file", command=self.open_plot_config_cb
        )
        self.plot_config_button.grid(
            column=5, row=4, padx=(0, 20), pady=(5, 5), sticky="news"
        )

    def open_data_config_cb(self) -> None:
        """Open filedialog and print it in TextBox"""
        data_config_path = fd.askopenfilename()
        self.data_config_entry.delete(0, tk.END)
        self.data_config_entry.insert(tk.END, data_config_path)

    def open_plot_config_cb(self) -> None:
        """Open filedialog and print it in TextBox"""
        plot_config_path = fd.askopenfilename()
        self.plot_config_entry.delete(0, tk.END)
        self.plot_config_entry.insert(tk.END, plot_config_path)

    def open_data_source_cb(self) -> None:
        """Open filedialog and print it in TextBox"""
        data_source_path = fd.askopenfilename()
        self.data_source_entry.delete(0, tk.END)
        self.data_source_entry.insert(tk.END, data_source_path)

    def open_output_directory_cb(self) -> None:
        """Open filedialog and print it in TextBox"""
        output_directory = fd.askdirectory()
        self.output_entry.delete(0, tk.END)
        self.output_entry.insert(tk.END, output_directory)
