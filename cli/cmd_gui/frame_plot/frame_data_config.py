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

"""Implements the 'Data Config' frame"""

import tkinter as tk
from dataclasses import dataclass
from pathlib import Path
from tkinter import filedialog as fd
from tkinter import ttk
from typing import TYPE_CHECKING

from yaml import safe_dump, safe_load

from ...helpers.project_context import PROJECT_BUILD_ROOT

if TYPE_CHECKING:  # pragma: no cover
    from .plot_gui import PlotFrame


@dataclass
class Column:
    """Container for a Column"""

    column_name: str
    column_type: str


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class DataConfigFrame(ttk.Frame):
    """'Data Config' Frame"""

    def __init__(self, parent: ttk.Notebook, root: "PlotFrame") -> None:
        super().__init__(parent)
        self.root = root
        self.columns: list[Column] = []

        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=0)
        self.rowconfigure(1, weight=1)

        with open(
            Path(__file__).parent.parent.parent
            / "cmd_plot"
            / "data_handling"
            / "schemas"
            / "csv_handler.json",
            encoding="utf-8",
        ) as file:
            self.valid_column_types: list[str] = safe_load(file)["properties"][
                "columns"
            ]["additionalProperties"]["enum"]

        # Create Frame for the File Information of the Data Configuration File
        file_path_frame = ttk.Frame(self)
        file_path_frame.grid(column=0, row=0, padx=15, pady=(10, 5), sticky="nwe")
        file_path_frame.columnconfigure(1, weight=1)
        file_path_frame.rowconfigure(0, weight=0)

        ttk.Label(file_path_frame, text="Data-Config File Path").grid(
            column=0, row=0, padx=(0, 10), sticky="we"
        )
        self.file_path_entry = ttk.Entry(file_path_frame)
        self.file_path_entry.grid(column=1, columnspan=2, row=0, sticky="we")
        self.file_path_entry.insert(
            tk.END, str(PROJECT_BUILD_ROOT / "gui" / "data_config.yaml")
        )
        ttk.Button(
            file_path_frame, text="Select File", command=self.select_file_cb
        ).grid(column=3, row=0, sticky="news")

        # Create Frame for Data Configuration
        data_config_frame = ttk.Labelframe(
            self, text="Data Configuration", padding=(10, 5)
        )
        data_config_frame.grid(column=0, row=1, padx=15, sticky="news")
        data_config_frame.columnconfigure((1, 2), weight=1)
        data_config_frame.rowconfigure(3, weight=1)

        ttk.Label(data_config_frame, text="Number of Lines to skip").grid(
            column=0, row=0, pady=(0, 5), padx=(0, 10), sticky="we"
        )
        self.skip_entry = ttk.Entry(data_config_frame)
        self.skip_entry.grid(column=1, row=0, padx=(0, 5), pady=(0, 5), sticky="we")
        self.skip_entry.insert(tk.END, "0")
        ttk.Label(data_config_frame, text="Precision of Data").grid(
            column=0, row=1, pady=(0, 5), padx=(0, 10), sticky="we"
        )
        self.precision_entry = ttk.Entry(data_config_frame)
        self.precision_entry.grid(
            column=1, row=1, padx=(0, 5), pady=(0, 5), sticky="we"
        )
        self.precision_entry.insert(tk.END, "2")

        # Widgets for adding Columns to Treeview
        ttk.Label(data_config_frame, text="Input Columns").grid(
            column=0, row=2, pady=(0, 5), padx=(0, 10), sticky="we"
        )

        self.columns_header_entry = ttk.Entry(data_config_frame)
        self.columns_header_entry.grid(
            column=1, row=2, padx=(0, 5), pady=(0, 5), sticky="we"
        )
        self.columns_type_entry = ttk.Combobox(
            data_config_frame, values=self.valid_column_types
        )
        self.columns_type_entry.grid(column=2, row=2, pady=(0, 5), sticky="we")
        self.columns_type_entry.current(0)
        self.columns_header_entry.insert(tk.END, "name")

        ttk.Button(
            data_config_frame, text="Add Column", command=self.add_column_cb
        ).grid(column=3, row=2, pady=(0, 5), sticky="news")

        self.columns_treeview = ttk.Treeview(
            data_config_frame, columns=("column", "type"), show="headings", height=5
        )
        self.columns_treeview.heading("column", text="Column")
        self.columns_treeview.heading("type", text="Type")
        self.columns_treeview.grid(
            column=1,
            columnspan=2,
            row=3,
            pady=(5, 2),
            sticky="news",
        )

        # Create Button to remove added Columns
        remove_button_frame = ttk.Frame(data_config_frame)
        remove_button_frame.grid(
            column=3,
            row=3,
            padx=(5, 0),
            pady=5,
            sticky="news",
        )
        ttk.Button(
            remove_button_frame,
            text="Remove\nSelected\nColumn",
            style="Multiline.TButton",
            command=self.remove_column_cb,
        ).pack(side=tk.TOP)

        # Create "Generate Data Configuration" Button
        generate_config_button_frame = ttk.Frame(self)
        generate_config_button_frame.grid(column=0, row=2, sticky="news")
        ttk.Button(
            generate_config_button_frame,
            text="Generate\nData Configuration",
            command=self.generate_data_config_cb,
            style="Heading.TButton",
        ).pack(pady=(2, 2))

    def select_file_cb(self) -> None:
        """Open filedialog and write selected item into Entry widget"""
        file_path = fd.asksaveasfilename(
            defaultextension=".yaml", filetypes=[("YAML File", "*.yaml")]
        )
        if file_path:
            self.file_path_entry.delete(0, tk.END)
            self.file_path_entry.insert(tk.END, file_path)

    def add_column_cb(self) -> None:
        """Add column to List"""
        column_name = str(self.columns_header_entry.get().strip())
        column_type = str(self.columns_type_entry.get().strip())
        if column_name in ("", "name"):
            self.root.write_text("Column header is missing.\n")
            return
        if column_type == "":
            self.root.write_text("Column type is missing.\n")
            return
        if column_type not in self.valid_column_types:
            self.root.write_text("Column type is not valid.\n")
            return
        self.columns.append(Column(column_name, column_type))
        self.columns_treeview.insert("", tk.END, values=(column_name, column_type))
        self.columns_header_entry.delete(0, tk.END)
        self.columns_type_entry.current(0)

    def remove_column_cb(self) -> None:
        """Remove selected column from List"""
        item = self.columns_treeview.focus()
        if item == "":
            self.root.write_text("Please select a Column from the list.\n")
            return
        self.columns.pop(self.columns_treeview.index(item))
        self.columns_treeview.delete(item)

    def generate_data_config_cb(self) -> None:
        """Generate a data configuration file"""
        data_file_path = self.file_path_entry.get().strip()
        if (data_file_path == "") or (" " in data_file_path):
            self.root.write_text(
                "Path of the Data Configuration File has to be given as a valid path.\n"
            )
            return
        if len(self.columns) == 0:
            self.root.write_text("Please add Columns.\n")
            return
        column_dict = {}
        for column in self.columns:
            column_dict[column.column_name] = column.column_type
        try:
            general = {
                "skip": int(self.skip_entry.get().strip()),
                "precision": int(self.precision_entry.get().strip()),
            }
        except ValueError:
            self.root.write_text(
                "Number of Lines to skip and Precision of Data have to be given as integers.\n"
            )
            return
        data_config = {"general": general, "columns": column_dict}
        Path(data_file_path).parent.absolute().mkdir(parents=True, exist_ok=True)
        with open(data_file_path, mode="w", encoding="utf-8") as file:
            safe_dump(data_config, file)
        self.root.write_text(
            f"Data Configuration File has been saved in '{data_file_path}'.\n"
        )
        self.root.run_plot_tab.data_config_entry.delete(0, tk.END)
        self.root.run_plot_tab.data_config_entry.insert(tk.END, str(data_file_path))
