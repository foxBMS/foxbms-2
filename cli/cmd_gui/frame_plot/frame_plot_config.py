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

"""Implements the 'Plot Config' frame"""

import json
import tkinter as tk
from pathlib import Path
from tkinter import filedialog as fd
from tkinter import ttk
from typing import TYPE_CHECKING

from ...cmd_plot.drawer.graph_types import GraphTypes
from ...helpers.project_context import PROJECT_BUILD_ROOT
from ..style_config import get_italic_font, get_text_font

if TYPE_CHECKING:  # pragma: no cover
    from .plot_gui import PlotFrame

# spell:ignore Segoe


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class PlotConfigFrame(ttk.Frame):
    """'Plot Config' Frame"""

    # pylint: disable-next=too-many-statements
    def __init__(self, parent: ttk.Notebook, root: "PlotFrame") -> None:
        super().__init__(parent)
        self.plots: list[dict] = []
        self.root = root
        self.columnconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)

        self.italic_font = get_italic_font()
        self.text_font = get_text_font()

        # Create Frame for the File Information of the Plot Configuration File
        file_path_frame = ttk.Frame(self)
        file_path_frame.grid(column=0, row=0, padx=15, pady=(10, 5), sticky="news")
        file_path_frame.columnconfigure(1, weight=1)

        ttk.Label(file_path_frame, text="Plot-Config File Path").grid(
            column=0, row=0, padx=(0, 10), sticky="news"
        )
        self.file_path_entry = ttk.Entry(file_path_frame)
        self.file_path_entry.insert(
            tk.END, str(PROJECT_BUILD_ROOT / "gui" / Path("plot_config.yaml"))
        )
        self.file_path_entry.grid(column=1, row=0, sticky="we")
        ttk.Button(
            file_path_frame, text="Select File", command=self.select_file_cb
        ).grid(column=2, row=0, sticky="news")

        # Create Frame for Input of the Plot Configuration File
        plot_config_frame = ttk.Frame(self)
        plot_config_frame.grid(column=0, row=1, sticky="news")
        plot_config_frame.columnconfigure(0, weight=1)
        plot_config_frame.rowconfigure((0, 1), weight=1)

        # Create Frame for Plot Input
        plot_data_frame = ttk.Labelframe(
            plot_config_frame, text="Plot Data", padding=(10, 3)
        )
        plot_data_frame.grid(column=0, row=0, padx=(15, 0), sticky="news")
        plot_data_frame.columnconfigure(1, weight=1)
        plot_data_frame.rowconfigure((0, 1, 2, 3, 4, 5, 6), weight=1)

        ttk.Label(plot_data_frame, text="Plot-File Name").grid(
            column=0, row=0, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.plot_file_name_entry = ttk.Entry(plot_data_frame)
        self.plot_file_name_entry.grid(
            column=1,
            row=0,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )

        ttk.Label(plot_data_frame, text="Plot Type").grid(
            column=0, row=1, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.plot_type_combobox = ttk.Combobox(
            plot_data_frame,
            values=GraphTypes._member_names_,  # pylint: disable=no-member
        )
        self.plot_type_combobox.grid(
            column=1, row=1, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.plot_type_combobox.current(0)

        ttk.Label(plot_data_frame, text="Plot Title").grid(
            column=0, row=2, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.plot_title_entry = ttk.Entry(plot_data_frame)
        self.plot_title_entry.grid(
            column=1,
            row=2,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )

        ttk.Label(plot_data_frame, text="Input Column x-Axis").grid(
            column=0, row=3, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.x_axis_column_entry = ttk.Entry(plot_data_frame)
        self.x_axis_column_entry.grid(
            column=1,
            row=3,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )

        ttk.Label(plot_data_frame, text="Label for x-Axis").grid(
            column=0, row=4, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.x_axis_name_entry = ttk.Entry(plot_data_frame)
        self.x_axis_name_entry.grid(
            column=1,
            row=4,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )

        ttk.Label(plot_data_frame, text="Labels for y-Axes").grid(
            column=0, row=5, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.y_axes_names_entry = ttk.Entry(plot_data_frame, font=self.italic_font)
        self.y_axes_names_entry.grid(
            column=1,
            row=5,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )
        self.y_axes_names_entry.bind("<KeyRelease>", self.change_font_cb)
        self.y_axes_names_entry.insert(tk.END, "separate labels with a comma")

        # Add Variables for Checkbutton
        self.save_plot_value = tk.BooleanVar(value=False)
        self.show_plot_value = tk.BooleanVar(value=False)

        ttk.Checkbutton(
            plot_data_frame,
            text="Save Plot",
            variable=self.save_plot_value,
            state="!alternate",
        ).grid(column=2, row=0, padx=(5, 0), pady=(0, 5), sticky="we")
        ttk.Checkbutton(
            plot_data_frame,
            text="Show Plot",
            variable=self.show_plot_value,
            state="!alternate",
        ).grid(column=2, row=1, padx=(5, 0), pady=(0, 5), sticky="we")

        # Create Frame for 'Add Plot' Button
        add_plot_button_frame = ttk.Frame(plot_data_frame)
        add_plot_button_frame.grid(column=2, row=2, rowspan=4, sticky="news")
        add_plot_button_frame.columnconfigure(0, weight=1)
        add_plot_button_frame.rowconfigure(0, weight=1)

        ttk.Button(
            add_plot_button_frame,
            text="Add\nPlot",
            command=self.add_plot_cb,
            style="Multiline.TButton",
        ).grid(column=0, row=0)

        # Create Frame for Line Input
        line_data_frame = ttk.Labelframe(
            plot_config_frame, text="Line Data", padding=(10, 3)
        )
        line_data_frame.grid(column=0, row=1, padx=(15, 0), pady=5, sticky="news")
        line_data_frame.columnconfigure(1, weight=1)
        line_data_frame.rowconfigure((0, 1, 2, 3), weight=1)

        ttk.Label(line_data_frame, text="Input Column y-Axis").grid(
            column=0, row=0, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.y_axis_column_entry = ttk.Entry(line_data_frame)
        self.y_axis_column_entry.grid(
            column=1, row=0, padx=(0, 10), pady=(0, 5), sticky="we"
        )

        ttk.Label(line_data_frame, text="Label for the Line").grid(
            column=0, row=1, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.line_name_entry = ttk.Entry(line_data_frame, font=self.italic_font)
        self.line_name_entry.grid(
            column=1,
            row=1,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )
        self.line_name_entry.bind("<KeyRelease>", self.change_font_cb)
        self.line_name_entry.insert(tk.END, "optional")

        ttk.Label(line_data_frame, text="min y-value").grid(
            column=0, row=2, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.min_value_entry = ttk.Entry(line_data_frame, font=self.italic_font)
        self.min_value_entry.grid(
            column=1,
            row=2,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )
        self.min_value_entry.bind("<KeyRelease>", self.change_font_cb)
        self.min_value_entry.insert(tk.END, "optional")

        ttk.Label(line_data_frame, text="max y-value").grid(
            column=0, row=3, padx=(0, 10), pady=(0, 5), sticky="we"
        )
        self.max_value_entry = ttk.Entry(line_data_frame, font=self.italic_font)
        self.max_value_entry.grid(
            column=1,
            row=3,
            padx=(0, 10),
            pady=(0, 5),
            sticky="we",
        )
        self.max_value_entry.bind("<KeyRelease>", self.change_font_cb)
        self.max_value_entry.insert(tk.END, "optional")

        ttk.Button(
            line_data_frame,
            text="Add\nLine",
            command=self.add_line_cb,
            style="Multiline.TButton",
        ).grid(column=2, row=0, rowspan=4)

        # Create Frame for Treeview
        treeview_frame = ttk.Frame(plot_config_frame)
        treeview_frame.grid(
            column=1, row=0, rowspan=2, padx=10, pady=(3, 5), sticky="news"
        )
        treeview_frame.rowconfigure(0, weight=1)
        treeview_frame.columnconfigure(0, weight=1)

        self.plots_treeview = ttk.Treeview(treeview_frame, show="tree")
        self.plots_treeview.grid(
            column=0,
            row=0,
            pady=(5, 0),
            sticky="news",
        )

        item_button_frame = ttk.Frame(treeview_frame)
        item_button_frame.grid(column=0, row=1, pady=5)
        item_button_frame.rowconfigure(0, weight=1)
        item_button_frame.columnconfigure(0, weight=1)
        item_button_frame.columnconfigure(1, weight=1)
        ttk.Button(
            item_button_frame,
            text="Open\nSelected Item",
            style="Multiline.TButton",
            command=self.open_selected_item_cb,
        ).grid(column=0, row=0, padx=(0, 2), sticky="ns")
        ttk.Button(
            item_button_frame,
            text="Remove\nSelected Item",
            style="Multiline.TButton",
            command=self.remove_selected_item_cb,
        ).grid(column=1, row=0, padx=(2, 0), sticky="ns")
        # Create Button to generate a Plot Configuration File
        ttk.Button(
            treeview_frame,
            text="Generate\nPlot Configuration",
            command=self.generate_plot_config_cb,
            style="Heading.TButton",
        ).grid(column=0, row=2)

    def change_font_cb(self, event: tk.Event) -> None:
        """Change the font of the widget to default font"""
        # ensure that the widget supports setting the font
        if isinstance(event.widget, ttk.Entry):
            event.widget.configure(font=self.text_font)

    def select_file_cb(self) -> None:
        """Open filedialog and write selected item into Entry widget"""
        file_path = fd.asksaveasfilename(
            defaultextension=".yaml", filetypes=[("YAML File", "*.yaml")]
        )
        if file_path:
            self.file_path_entry.delete(0, tk.END)
            self.file_path_entry.insert(tk.END, file_path)

    def add_plot_cb(self) -> None:
        """Add the Input-Data for the Plot to the Table"""
        plot_file_name = self.plot_file_name_entry.get().strip()
        plot_type = self.plot_type_combobox.get().strip()
        plot_title = self.plot_title_entry.get().strip()
        x_axis_column = self.x_axis_column_entry.get().strip()
        x_axis_name = self.x_axis_name_entry.get().strip()
        y_axes_names = self.y_axes_names_entry.get().strip()

        err = 0

        if (plot_file_name == "") or (" " in plot_file_name):
            self.root.write_text(
                "Name of the Plot-File has to be given as a valid path.\n"
            )
            err += 1
        if x_axis_column == "":
            self.root.write_text("Please provide an input column for the x-axis.\n")
            err += 1
        if plot_title == "":
            self.root.write_text("Please provide a Title for the plot.\n")
            err += 1
        if x_axis_name == "":
            self.root.write_text("Please provide a Label for the x-axis.\n")
            err += 1
        if ("separate labels with a comma" in y_axes_names) or (y_axes_names == ""):
            self.root.write_text("Please provide Labels for the y-axes.\n")
            err += 1
        if plot_type not in GraphTypes._member_names_:  # pylint: disable=protected-access, no-member
            self.root.write_text("Please select a Plot Type.\n")
            err += 1

        for plot in self.plots:
            if plot_file_name == plot["name"]:
                self.root.write_text(
                    "Name of the Plot-File has to be unique for each Plot.\n"
                )
                err += 1
                break
        y_labels = [label.strip() for label in y_axes_names.split(",")]
        if len(y_labels) > 3:
            self.root.write_text("One Plot cannot contain more than 3 lines.\n")
            err += 1

        if err > 0:
            return

        description = {
            "title": plot_title,
            "x_axis": x_axis_name,
            "y_axes": y_labels,
        }

        graph_keys = ["show", "save"]
        graph_values: list[bool] = [
            self.show_plot_value.get(),
            self.save_plot_value.get(),
        ]
        graph = dict(zip(graph_keys, graph_values, strict=True))

        self.plots.append(
            {
                "name": plot_file_name,
                "type": plot_type,
                "mapping": {"x": x_axis_column},
                "description": description,
                "graph": graph,
            }
        )
        self._update_treeview()

        self._insert_text(self.plot_file_name_entry, "")
        self._insert_text(self.plot_title_entry, "")
        self._insert_text(self.x_axis_column_entry, "")
        self._insert_text(self.x_axis_name_entry, "")
        self._insert_text(self.y_axes_names_entry, "separate labels with a comma")
        self.y_axes_names_entry.configure(font=self.italic_font)
        self.show_plot_value.set(False)
        self.save_plot_value.set(False)
        self.plot_type_combobox.current(0)

    def add_line_cb(self) -> None:
        """Add the Input-Data for the Line to the selected Plot from the Table"""
        try:
            items = self.get_selected_item()
            if items is None:
                return
            parent_plot, selected_item, index = items
        except ValueError as e:
            self.root.write_text(str(e))
            return
        if parent_plot != selected_item:
            self.root.write_text("Please select a Plot.\n")
            return

        line_key = ""
        for line in ("y1", "y2", "y3"):
            if line not in self.plots[index]["mapping"]:
                line_key = line
                break
        if line_key == "":
            self.root.write_text("A Plot cannot contain more than 3 lines.\n")
            return
        input_column = self.y_axis_column_entry.get().strip()
        label = self.line_name_entry.get().strip()
        min_value = self.min_value_entry.get().strip()
        max_value = self.max_value_entry.get().strip()
        err = 0
        if input_column == "":
            self.root.write_text("Please provide an input column for the y-axis.\n")
            err += 1
        line_dict: dict[str, list | str | float] = {"input": [input_column]}
        if label not in ("optional", ""):
            line_dict["labels"] = [label]
        if min_value not in ("optional", ""):
            try:
                line_dict["min"] = float(min_value)
            except ValueError:
                self.root.write_text("Minimum y-value has to be a number.\n")
                err += 1
        if max_value not in ("optional", ""):
            try:
                line_dict["max"] = float(max_value)
            except ValueError:
                self.root.write_text("Maximum y-value has to be a number.\n")
                err += 1
        if err > 0:
            return
        self.plots[index]["mapping"][line_key] = line_dict

        try:
            self._update_treeview()
        except tk.TclError as e:
            self.root.write_text(str(e))
            del self.plots[index]["mapping"][line_key]
            return

        self._insert_text(self.y_axis_column_entry, "")
        self._insert_text(self.line_name_entry, "optional")
        self._insert_text(self.min_value_entry, "optional")
        self._insert_text(self.max_value_entry, "optional")
        self.line_name_entry.configure(font=self.italic_font)
        self.min_value_entry.configure(font=self.italic_font)
        self.max_value_entry.configure(font=self.italic_font)

    def open_plot(self, plot: dict) -> None:
        """Open plot in the 'Plot Data' Frame"""
        self._insert_text(self.plot_file_name_entry, plot["name"])
        self._insert_text(self.plot_type_combobox, plot["type"])
        self._insert_text(self.x_axis_column_entry, plot["mapping"]["x"])
        self._insert_text(self.plot_title_entry, plot["description"]["title"])
        self._insert_text(self.x_axis_name_entry, plot["description"]["x_axis"])
        self._insert_text(
            self.y_axes_names_entry, ", ".join(plot["description"]["y_axes"])
        )
        self.y_axes_names_entry.configure(font=self.text_font)
        self.show_plot_value.set(plot["graph"]["show"])
        self.save_plot_value.set(plot["graph"]["save"])

    def open_line(self, line: dict) -> None:
        """Open line in the 'Line Data' Frame"""
        self._insert_text(self.y_axis_column_entry, line["input"][0])
        if "min" in line:
            self._insert_text(self.min_value_entry, line["min"])
            self.min_value_entry.configure(font=self.text_font)
        if "max" in line:
            self._insert_text(self.max_value_entry, line["max"])
            self.max_value_entry.configure(font=self.text_font)
        if "labels" in line:
            self._insert_text(self.line_name_entry, line["labels"][0])
            self.line_name_entry.configure(font=self.text_font)

    def get_selected_item(self) -> tuple[str, str, int] | None:
        """Extract the selected item from the treeview"""
        selected_item = self.plots_treeview.focus()
        if selected_item == "":
            self.root.write_text("Please select an item.\n")
            return None

        if ("_" in selected_item) and selected_item.split("_")[-1] in (
            "y1",
            "y2",
            "y3",
        ):
            parent_plot = self.plots_treeview.parent(selected_item)
            selected_item = selected_item.split("_")[-1]
        else:
            parent_plot = selected_item

        for index, plot in enumerate(self.plots):
            if parent_plot == plot["name"]:
                return parent_plot, selected_item, index
        err = "Item could not be found.\n"
        raise ValueError(err)

    def open_selected_item_cb(self) -> None:
        """Get the selected item and executes the corresponding function"""
        try:
            items = self.get_selected_item()
            if items is None:
                return
            parent_plot, selected_item, index = items
        except ValueError as e:
            self.root.write_text(str(e))
            return
        if parent_plot == selected_item:
            self.open_plot(self.plots[index])
        else:
            self.open_line(self.plots[index]["mapping"][selected_item])

    def remove_selected_item_cb(self) -> None:
        """Get the selected item and remove it"""
        try:
            items = self.get_selected_item()
            if items is None:
                return
            parent_plot, selected_item, index = items
        except ValueError as e:
            self.root.write_text(str(e))
            return
        if parent_plot == selected_item:
            self.plots.pop(index)
        else:
            self._remove_line_from_plot(self.plots[index]["mapping"], selected_item)
        self._update_treeview()

    def generate_plot_config_cb(self) -> None:
        """Generate plot configuration file"""
        if len(self.plots) == 0:
            self.root.write_text("Please add Plots.\n")
            return
        for plot in self.plots:
            if (
                ("y1" not in plot["mapping"])
                and ("y2" not in plot["mapping"])
                and ("y3" not in plot["mapping"])
            ):
                self.root.write_text("Every Plot has to contain at least one line.\n")
                return
        plot_file_path = self.file_path_entry.get().strip()
        if (plot_file_path == "") or (" " in plot_file_path):
            self.root.write_text(
                "Path of the Plot Configuration File has to be given as a valid path.\n"
            )
            return
        Path(plot_file_path).parent.absolute().mkdir(parents=True, exist_ok=True)
        with open(plot_file_path, mode="w", encoding="utf-8") as file:
            json.dump(self.plots, file)
        self.root.write_text(
            f"Plot Configuration File has been saved in '{plot_file_path}'.\n"
        )
        self.root.run_plot_tab.plot_config_entry.delete(0, tk.END)
        self.root.run_plot_tab.plot_config_entry.insert(tk.END, str(plot_file_path))

    def _update_treeview(self) -> None:
        """Remove all elements from the treeview and insert all elements from self.plots"""
        for item in self.plots_treeview.get_children():
            self.plots_treeview.delete(item)

        for plot in self.plots:
            self.plots_treeview.insert("", tk.END, plot["name"], text=plot["name"])
            for key, line in plot["mapping"].items():
                if key in ("y1", "y2", "y3"):
                    line_input = line["input"][0]
                    self.plots_treeview.insert(
                        plot["name"], tk.END, line_input + "_" + key, text=line_input
                    )
                    self.plots_treeview.item(plot["name"], open=True)

    @staticmethod
    def _remove_line_from_plot(mapping: dict[str, str | dict], key: str) -> None:
        """Remove the given Line and adjust the keys if necessary"""
        if key == "y3":
            del mapping[key]
        if key == "y2":
            if "y3" in mapping:
                mapping[key] = mapping["y3"]
                del mapping["y3"]
            else:
                del mapping[key]
        if key == "y1":
            if "y3" in mapping:
                mapping[key] = mapping["y3"]
                del mapping["y3"]
            elif "y2" in mapping:
                mapping[key] = mapping["y2"]
                del mapping["y2"]
            else:
                del mapping["y1"]

    @staticmethod
    def _insert_text(entry_obj: ttk.Entry, input_str: str) -> None:
        """Delete the content and insert the input"""
        entry_obj.delete(0, tk.END)
        entry_obj.insert(tk.END, input_str)
