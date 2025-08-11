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

"""Implements the 'Data Config' frame"""

import tkinter as tk
from dataclasses import dataclass
from pathlib import Path
from tkinter import filedialog as fd
from tkinter import ttk

from yaml import safe_dump, safe_load

from ...helpers.misc import PROJECT_BUILD_ROOT


@dataclass
class Column:
    """Container for a Column"""

    column_name: str
    column_type: str


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class DataConfigFrame(ttk.Frame):
    """PlotConfig Frame"""

    def __init__(self, parent, root) -> None:
        super().__init__(parent)
        self.root = root
        self.columns: list[Column] = []

        self.columnconfigure(0, weight=1)

        with open(
            Path(__file__).parent.parent.parent
            / "cmd_plot"
            / "data_handling"
            / "schemas"
            / "csv_handler.json",
            encoding="utf-8",
        ) as f:
            valid_column_types = safe_load(f)["properties"]["columns"][
                "additionalProperties"
            ]["enum"]

        # Set Styles
        font_heading = ("TkDefaultFont", 10, "bold")
        ttk.Style().configure("Multiline.TButton", justify="center")
        ttk.Style().configure("heading.TButton", font=font_heading, justify="center")

        # Create Frame for the File Information of the Data Configuration File
        file_frame = ttk.Frame(self, padding=(15, 5))
        file_frame.grid(column=0, row=0, sticky="news")
        file_frame.columnconfigure(1, weight=1)

        file_path_label = ttk.Label(self, text="Data-Config File Path", width=20)
        file_path_label.grid(
            in_=file_frame, column=0, row=0, pady=(10, 0), sticky="news"
        )
        self.file_path_entry = ttk.Entry(self, width=10)
        self.file_path_entry.insert(
            tk.END, str(PROJECT_BUILD_ROOT / "data_config.yaml")
        )
        self.file_path_entry.grid(
            in_=file_frame, column=1, columnspan=2, row=0, pady=(10, 0), sticky="we"
        )

        self.file_path_button = ttk.Button(
            self, text="Select File", command=self.open_file_cb
        )
        self.file_path_button.grid(
            in_=file_frame, column=3, row=0, pady=(10, 0), sticky="news"
        )

        # Create Labels and text widgets to input relevant data for the Data Configuration
        data_frame = ttk.Labelframe(self, text="Data Configuration", padding=(10, 5))
        data_frame.grid(column=0, row=1, padx=10, pady=(5, 0), sticky="news")
        data_frame.columnconfigure(1, weight=1)
        data_frame.columnconfigure(2, weight=1)

        skip_label = ttk.Label(self, text="Number of Lines to skip", width=23)
        skip_label.grid(in_=data_frame, column=0, row=2, pady=(0, 5), sticky="news")
        self.skip_entry = ttk.Entry(self, width=10)
        self.skip_entry.insert(tk.END, "0")
        self.skip_entry.grid(
            in_=data_frame, column=1, row=2, padx=(0, 5), pady=(0, 5), sticky="news"
        )

        precision_label = ttk.Label(self, text="Precision of Data", width=23)
        precision_label.grid(
            in_=data_frame, column=0, row=3, pady=(0, 5), sticky="news"
        )
        self.precision_entry = ttk.Entry(self, width=10)
        self.precision_entry.insert(tk.END, "2")
        self.precision_entry.grid(
            in_=data_frame, column=1, row=3, padx=(0, 5), pady=(0, 5), sticky="news"
        )

        # Adding Columns to Treeview
        columns_label = ttk.Label(self, text="Input Columns", width=23)
        columns_label.grid(in_=data_frame, column=0, row=4, pady=5, sticky="news")

        self.columns_header_entry = ttk.Entry(self, width=10)
        self.columns_header_entry.grid(
            in_=data_frame, column=1, row=4, padx=(0, 5), pady=5, sticky="news"
        )
        self.columns_type_entry = ttk.Combobox(
            self, width=10, values=valid_column_types
        )
        self.columns_type_entry.grid(
            in_=data_frame, column=2, row=4, pady=5, sticky="news"
        )
        self.columns_type_entry.current(0)
        self.columns_header_entry.insert(tk.END, "name")

        self.column_add_button = ttk.Button(
            self, text="Add Column", command=self.add_column_cb
        )
        self.column_add_button.grid(
            in_=data_frame, column=3, row=4, pady=5, sticky="news"
        )

        self.columns_treeview = ttk.Treeview(
            self, columns=("column", "type"), show="headings", height=6
        )
        self.columns_treeview.heading("column", text="Column")
        self.columns_treeview.heading("type", text="Type")
        self.columns_treeview.grid(
            in_=data_frame,
            column=1,
            columnspan=2,
            row=5,
            rowspan=6,
            pady=5,
            sticky="news",
        )

        # Create Button to remove added Columns
        button_frame_remove = ttk.Frame(self)
        button_frame_remove.grid(
            in_=data_frame,
            column=3,
            row=5,
            rowspan=6,
            padx=(5, 0),
            pady=5,
            sticky="news",
        )
        self.columns_remove_button = ttk.Button(
            self,
            text="Remove\nSelected\nColumn",
            style="Multiline.TButton",
            command=self.remove_column_cb,
        )
        self.columns_remove_button.pack(in_=button_frame_remove, side=tk.TOP)

        # Create Button to generate a Data Configuration File
        button_frame_generate = ttk.Frame(self)
        button_frame_generate.grid(column=0, row=3, sticky="news")
        self.data_config_generate = ttk.Button(
            self,
            text="Generate\nData Configuration",
            command=self.generate_data_config_cb,
            style="heading.TButton",
        )
        self.data_config_generate.pack(in_=button_frame_generate, pady=(2, 2))

    def add_column_cb(self) -> None:
        """Add column to List"""
        column_name = str(self.columns_header_entry.get().strip())
        column_type = str(self.columns_type_entry.get().strip())
        if column_name in ("", "name"):
            self.root.write_text("Column header has to be given\n")
            return
        if column_type == "":
            self.root.write_text("Column type has to be given\n")
            return

        self.columns.append(Column(column_name, column_type))
        self.columns_treeview.insert("", tk.END, values=(column_name, column_type))
        self.columns_header_entry.delete(0, tk.END)
        self.columns_type_entry.current(0)

    def remove_column_cb(self) -> None:
        """Remove column from List"""
        item = self.columns_treeview.focus()
        if item == "":
            self.root.write_text("Column has to be selected\n")
            return
        self.columns.pop(self.columns_treeview.index(item))
        self.columns_treeview.delete(item)

    def open_file_cb(self) -> None:
        """Open filedialog and print it in Entry widget"""
        output_directory = fd.asksaveasfilename(
            defaultextension=".yaml", filetypes=[("YAML File", "*.yaml")]
        )
        self.file_path_entry.delete(0, tk.END)
        self.file_path_entry.insert(tk.END, output_directory)

    def generate_data_config_cb(self) -> None:
        """Generate data configuration file"""
        data_file_path = self.file_path_entry.get().strip()
        if (data_file_path == "") or (" " in data_file_path):
            self.root.write_text(
                "Path of the Data-File has to be given as a valid path\n"
            )
            return
        if len(self.columns) == 0:
            self.root.write_text("Columns have to be given\n")
            return
        columns = {}
        for column in self.columns:
            columns[column.column_name] = column.column_type
        try:
            general = {
                "skip": int(self.skip_entry.get().strip()),
                "precision": int(self.precision_entry.get().strip()),
            }
        except ValueError:
            self.root.write_text(
                "Number of Lines to skip and Precision of Data have to be given as integers\n"
            )
            return
        data_config = {"general": general, "columns": columns}
        Path(data_file_path).parent.absolute().mkdir(parents=True, exist_ok=True)
        with open(data_file_path, mode="w", encoding="utf-8") as f:
            safe_dump(data_config, f)
        self.root.write_text(
            f"Data Configuration File has been saved in {data_file_path}\n"
        )
        self.root.tab_plot.data_config_entry.delete(0, tk.END)
        self.root.tab_plot.data_config_entry.insert(tk.END, str(data_file_path))
