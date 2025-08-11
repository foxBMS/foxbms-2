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

"""Implements the 'plot_config' frame"""

import json
import tkinter as tk
from pathlib import Path
from tkinter import filedialog as fd
from tkinter import font, ttk

from ...cmd_plot.drawer.graph_types import GraphTypes
from ...helpers.misc import PROJECT_BUILD_ROOT


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class PlotConfigFrame(ttk.Frame):
    """PlotConfig Frame"""

    # pylint: disable-next=too-many-statements, too-many-locals
    def __init__(self, parent, root) -> None:
        super().__init__(parent)
        self.plots: list[dict] = []
        self.root = root
        self.columnconfigure(0, weight=1)

        # Set Styles for Headings and Buttons
        font_heading = font.Font(family="TkDefaultFont", size=10, weight="bold")
        self.font_italics = font.Font(
            name="Italic.TEntry", family="Segoe UI", size=9, slant="italic"
        )
        self.font_default = font.Font(name="Default.TEntry", family="Segoe UI", size=9)
        ttk.Style().configure("heading.TButton", font=font_heading, justify="center")
        ttk.Style().configure("Multiline.TButton", justify="center")

        # Create Frame for the File Information of the Plot Configuration File
        file_frame = ttk.Frame(self, padding=(15, 5))
        file_frame.grid(column=0, row=0, sticky="news")
        file_frame.columnconfigure(1, weight=1)

        file_path_label = ttk.Label(self, text="Plot-Config File Path", width=20)
        file_path_label.grid(
            in_=file_frame, column=0, row=0, sticky="news", pady=(10, 0)
        )
        self.file_path_entry = ttk.Entry(self, width=10)
        self.file_path_entry.insert(
            tk.END, str(PROJECT_BUILD_ROOT / Path("plot_config.yaml"))
        )
        self.file_path_entry.grid(
            in_=file_frame, column=1, row=0, sticky="we", pady=(10, 0)
        )
        self.file_path_button = ttk.Button(
            self, text="Select File", command=self.open_file_cb
        )
        self.file_path_button.grid(
            in_=file_frame, column=2, row=0, sticky="news", pady=(10, 0)
        )

        # Create Frame for Input of the Plot Configuration File
        config_frame = ttk.Frame(self)
        config_frame.grid(column=0, row=1, sticky="news")

        config_frame.columnconfigure(0, weight=5)

        # Create Frame for Plot Input
        plot_config_frame = ttk.Labelframe(
            self, text="Plot Data", padding=(10, 3, 10, 3)
        )
        plot_config_frame.grid(
            in_=config_frame, column=0, row=0, padx=(10, 0), pady=(5, 0), sticky="news"
        )
        plot_config_frame.columnconfigure(0, weight=1)
        plot_config_frame.rowconfigure(0, weight=1)

        plot_config_entries_frame = ttk.Frame(self, padding=(0, 0, 10, 0))
        plot_config_entries_frame.grid(
            in_=plot_config_frame, column=0, row=0, sticky="news"
        )
        plot_config_entries_frame.columnconfigure(1, weight=1)

        plot_file_name_label = ttk.Label(self, text="Plot-File Name", width=20)
        plot_file_name_label.grid(
            in_=plot_config_entries_frame, column=0, row=0, pady=(0, 5), sticky="news"
        )
        self.plot_file_name_entry = ttk.Entry(self, width=10)
        self.plot_file_name_entry.grid(
            in_=plot_config_entries_frame,
            column=1,
            row=0,
            pady=(0, 5),
            sticky="news",
        )

        plot_type_label = ttk.Label(self, text="Plot Type", width=20)
        plot_type_label.grid(
            in_=plot_config_entries_frame, column=0, row=1, pady=(0, 5), sticky="news"
        )
        self.plot_type_entry = ttk.Combobox(
            self,
            width=5,
            values=GraphTypes._member_names_,  # pylint: disable=no-member
        )
        self.plot_type_entry.grid(
            in_=plot_config_entries_frame, column=1, row=1, pady=(0, 5), sticky="news"
        )
        self.plot_type_entry.current(0)

        plot_title_label = ttk.Label(self, text="Plot Title", width=20)
        plot_title_label.grid(
            in_=plot_config_entries_frame, column=0, row=2, pady=(0, 5), sticky="news"
        )
        self.plot_title_entry = ttk.Entry(self, width=10)
        self.plot_title_entry.grid(
            in_=plot_config_entries_frame,
            column=1,
            row=2,
            pady=(0, 5),
            sticky="news",
        )

        x_axis_label = ttk.Label(self, text="Input Column x-Axis", width=20)
        x_axis_label.grid(
            in_=plot_config_entries_frame, column=0, row=3, pady=(0, 5), sticky="news"
        )
        self.x_axis_entry = ttk.Entry(self, width=10)
        self.x_axis_entry.grid(
            in_=plot_config_entries_frame,
            column=1,
            row=3,
            pady=(0, 5),
            sticky="we",
        )

        label_x_axis_label = ttk.Label(self, text="Label for x-Axis", width=20)
        label_x_axis_label.grid(
            in_=plot_config_entries_frame, column=0, row=4, pady=(0, 5), sticky="news"
        )
        self.label_x_axis_entry = ttk.Entry(self, width=10)
        self.label_x_axis_entry.grid(
            in_=plot_config_entries_frame,
            column=1,
            row=4,
            pady=(0, 5),
            sticky="we",
        )

        label_y_axes_label = ttk.Label(self, text="Labels for y-Axes", width=20)
        label_y_axes_label.grid(
            in_=plot_config_entries_frame, column=0, row=5, pady=(0, 5), sticky="news"
        )
        self.label_y_axes_entry = ttk.Entry(self, width=10, font=self.font_italics)
        self.label_y_axes_entry.grid(
            in_=plot_config_entries_frame,
            column=1,
            row=5,
            pady=(0, 5),
            sticky="we",
        )
        self.label_y_axes_entry.bind("<KeyRelease>", self.change_font_cb)
        self.label_y_axes_entry.insert(tk.END, "separate labels with a comma")

        plot_config_buttons_frame = ttk.Frame(self)
        plot_config_buttons_frame.grid(
            in_=plot_config_frame, column=1, row=0, sticky="news"
        )
        plot_config_buttons_frame.rowconfigure(1, weight=1)

        checkbox_frame = ttk.Frame(self)
        checkbox_frame.grid(
            in_=plot_config_buttons_frame, column=0, row=0, sticky="news"
        )

        self.save_checkbutton_value = tk.BooleanVar()
        self.save_checkbutton_value.set(False)
        save_plot_label = ttk.Label(self, text="Save Plot", width=12)
        save_plot_label.grid(in_=checkbox_frame, column=0, row=0, pady=(0, 5))
        save_plot_checkbutton = ttk.Checkbutton(
            self, text="Yes", variable=self.save_checkbutton_value
        )
        save_plot_checkbutton.grid(in_=checkbox_frame, column=1, row=0, pady=(0, 5))
        save_plot_checkbutton.state(["!alternate"])

        self.show_checkbutton_value = tk.BooleanVar()
        self.show_checkbutton_value.set(False)
        show_plot_label = ttk.Label(self, text="Show Plot", width=12)
        show_plot_label.grid(in_=checkbox_frame, column=0, row=1, pady=(0, 5))
        show_plot_checkbutton = ttk.Checkbutton(
            self, text="Yes", variable=self.show_checkbutton_value
        )
        show_plot_checkbutton.grid(in_=checkbox_frame, column=1, row=1, pady=(0, 5))
        show_plot_checkbutton.state(["!alternate"])

        plot_add_button_frame = ttk.Frame(self)
        plot_add_button_frame.grid(
            in_=plot_config_buttons_frame, column=0, row=1, sticky="news"
        )
        plot_add_button_frame.columnconfigure(0, weight=1)
        plot_add_button_frame.rowconfigure(0, weight=1)

        self.plot_add_button = ttk.Button(
            self, text="Add Plot", command=self.add_plot_cb
        )
        self.plot_add_button.grid(in_=plot_add_button_frame, column=0, row=0)

        # Create Frame for Line Input
        line_frame = ttk.Labelframe(self, text="Line Data", padding=(10, 3, 10, 3))
        line_frame.grid(
            in_=config_frame, column=0, row=1, padx=(10, 0), pady=(5, 0), sticky="news"
        )
        line_frame.columnconfigure(0, weight=1)
        line_frame.rowconfigure(0, weight=1)

        line_input_frame = ttk.Frame(self, padding=(0, 0, 10, 0))
        line_input_frame.grid(in_=line_frame, column=0, row=0, sticky="news")
        line_input_frame.columnconfigure(1, weight=1)

        y_axis_label = ttk.Label(self, text="Input Column y-Axis", width=20)
        y_axis_label.grid(
            in_=line_input_frame, column=0, row=0, pady=(0, 5), sticky="news"
        )
        self.y_axis_entry = ttk.Entry(self, width=10)
        self.y_axis_entry.grid(
            in_=line_input_frame, column=1, row=0, pady=(0, 5), sticky="news"
        )

        label_line_label = ttk.Label(self, text="Label for the Line", width=20)
        label_line_label.grid(
            in_=line_input_frame, column=0, row=10, pady=(0, 5), sticky="news"
        )
        self.label_line_entry = ttk.Entry(self, width=10, font=self.font_italics)
        self.label_line_entry.grid(
            in_=line_input_frame,
            column=1,
            row=10,
            pady=(0, 5),
            sticky="news",
        )
        self.label_line_entry.bind("<KeyRelease>", self.change_font_cb)
        self.label_line_entry.insert(tk.END, "optional")

        min_value_label = ttk.Label(self, text="min y-value", width=20)
        min_value_label.grid(
            in_=line_input_frame, column=0, row=11, pady=(0, 5), sticky="news"
        )
        self.min_value_entry = ttk.Entry(self, width=10, font=self.font_italics)
        self.min_value_entry.grid(
            in_=line_input_frame,
            column=1,
            row=11,
            pady=(0, 5),
            sticky="news",
        )
        self.min_value_entry.bind("<KeyRelease>", self.change_font_cb)
        self.min_value_entry.insert(tk.END, "optional")

        max_value_label = ttk.Label(self, text="max y-value", width=20)
        max_value_label.grid(
            in_=line_input_frame, column=0, row=12, pady=(0, 5), sticky="news"
        )
        self.max_value_entry = ttk.Entry(self, width=10, font=self.font_italics)
        self.max_value_entry.grid(
            in_=line_input_frame,
            column=1,
            row=12,
            pady=(0, 5),
            sticky="news",
        )
        self.max_value_entry.bind("<KeyRelease>", self.change_font_cb)
        self.max_value_entry.insert(tk.END, "optional")

        self.line_add_button = ttk.Button(
            self,
            text="Add\nLine",
            command=self.add_line_cb,
            style="Multiline.TButton",
            width=8,
        )
        self.line_add_button.grid(in_=line_frame, column=1, row=0, pady=3)

        # Create Frame for a Treeview
        treeview_frame = ttk.Frame(self, padding=(10, 0))
        treeview_frame.grid(
            in_=config_frame, column=1, row=0, rowspan=2, pady=(8, 0), sticky="news"
        )

        treeview_frame.rowconfigure(0, weight=1)
        treeview_frame.columnconfigure(0, weight=1)
        treeview_frame.columnconfigure(2, weight=1)

        self.plot_treeview = ttk.Treeview(self, show="tree")
        self.plot_treeview.grid(
            in_=treeview_frame,
            column=0,
            columnspan=3,
            row=0,
            pady=(5, 0),
            sticky="news",
        )

        item_button_frame = ttk.Frame(self, padding=(0, 5))
        item_button_frame.grid(
            in_=treeview_frame, column=0, columnspan=3, row=1, sticky="ns"
        )
        item_button_frame.rowconfigure(0, weight=1)
        item_button_frame.columnconfigure(0, weight=1)
        item_button_frame.columnconfigure(1, weight=1)

        self.open_item_button = ttk.Button(
            self,
            text="Open\nSelected Item",
            style="Multiline.TButton",
            command=self.open_selected_item_cb,
        )
        self.remove_item_button = ttk.Button(
            self,
            text="Remove\nSelected Item",
            style="Multiline.TButton",
            command=self.remove_selected_item_cb,
        )

        self.open_item_button.grid(in_=item_button_frame, column=0, row=0, padx=(0, 2))
        self.remove_item_button.grid(
            in_=item_button_frame, column=1, row=0, padx=(2, 0)
        )

        # Create Button to generate a Plot Configuration File
        self.plot_config_generate = ttk.Button(
            self,
            text="Generate\nPlot Configuration",
            command=self.generate_plot_config_cb,
            style="heading.TButton",
        )
        self.plot_config_generate.grid(in_=treeview_frame, column=1, row=2)

    def change_font_cb(self, event: tk.Event) -> None:
        """Change the font of the widget to the default font"""
        event.widget.configure(font=self.font_default)

    def open_file_cb(self) -> None:
        """Open filedialog and print it in Entry widget"""
        output_directory = fd.asksaveasfilename(
            defaultextension=".yaml", filetypes=[("YAML File", "*.yaml")]
        )
        self.file_path_entry.delete(0, tk.END)
        self.file_path_entry.insert(tk.END, output_directory)

    def add_plot_cb(self) -> None:
        """Add the Input-Data for the Plot to the Table"""
        plot_file_name = self.plot_file_name_entry.get().strip()
        plot_type = self.plot_type_entry.get().strip()
        plot_title = self.plot_title_entry.get().strip()
        input_x_axis = self.x_axis_entry.get().strip()
        label_x_axis = self.label_x_axis_entry.get().strip()
        label_y_axes = self.label_y_axes_entry.get().strip()

        err = 0

        if (plot_file_name == "") or (" " in plot_file_name):
            self.root.write_text(
                "Name of the Plot-File has to be given as a valid path\n"
            )
            err += 1
        if input_x_axis == "":
            self.root.write_text("Input column for x-axis has to be given\n")
            err += 1
        if plot_title == "":
            self.root.write_text("Title of the plot has to be given\n")
            err += 1
        if label_x_axis == "":
            self.root.write_text("Label for the x-axis has to be given\n")
            err += 1
        if ("separate labels with a comma" in label_y_axes) or (label_y_axes == ""):
            self.root.write_text("Labels for the y-axes have to be given\n")
            err += 1
        if plot_type == "":
            self.root.write_text("Plot Type has to be given\n")
            err += 1

        for plot in self.plots:
            if plot_file_name == plot["name"]:
                self.root.write_text(
                    "Name of the Plot-File has to be unique for each Plot\n"
                )
                err += 1
        y_labels = [label.strip() for label in label_y_axes.split(",")]
        if len(y_labels) > 3:
            self.root.write_text("One Plot can not contain more than 3 lines\n")
            err += 1

        if err > 0:
            return

        description = {
            "title": plot_title,
            "x_axis": label_x_axis,
            "y_axes": y_labels,
        }

        graph_keys = ["show", "save"]
        graph_values: list = [
            self.show_checkbutton_value.get(),
            self.save_checkbutton_value.get(),
        ]
        graph = dict(zip(graph_keys, graph_values))

        self.plots.append(
            {
                "name": plot_file_name,
                "type": plot_type,
                "mapping": {"x": input_x_axis},
                "description": description,
                "graph": graph,
            }
        )
        self.update_treeview()

        PlotConfigFrame.insert_text(self.plot_file_name_entry, "")
        PlotConfigFrame.insert_text(self.plot_title_entry, "")
        PlotConfigFrame.insert_text(self.x_axis_entry, "")
        PlotConfigFrame.insert_text(self.label_x_axis_entry, "")
        PlotConfigFrame.insert_text(
            self.label_y_axes_entry, "separate labels with a comma"
        )
        self.label_y_axes_entry.configure(font=self.font_italics)
        self.show_checkbutton_value.set(False)
        self.save_checkbutton_value.set(False)
        self.plot_type_entry.current(0)

    def open_plot(self, plot) -> None:
        """Open the given Plot in the 'Plot Data' Frame"""
        PlotConfigFrame.insert_text(self.plot_file_name_entry, plot["name"])
        PlotConfigFrame.insert_text(self.plot_type_entry, plot["type"])
        PlotConfigFrame.insert_text(self.x_axis_entry, plot["mapping"]["x"])
        PlotConfigFrame.insert_text(self.plot_title_entry, plot["description"]["title"])
        PlotConfigFrame.insert_text(
            self.label_x_axis_entry, plot["description"]["x_axis"]
        )
        PlotConfigFrame.insert_text(
            self.label_y_axes_entry, ", ".join(plot["description"]["y_axes"])
        )
        self.label_y_axes_entry.configure(font=self.font_default)
        self.show_checkbutton_value.set(plot["graph"]["show"])
        self.save_checkbutton_value.set(plot["graph"]["save"])

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
            self.root.write_text("The selected item has to be a Plot\n")
            return

        line_key = ""
        for line in ("y1", "y2", "y3"):
            if line not in self.plots[index]["mapping"]:
                line_key = line
                break
        if line_key == "":
            self.root.write_text("A Plot can only contain 3 lines\n")
            return
        input_column = self.y_axis_entry.get().strip()
        label = self.label_line_entry.get().strip()
        min_value = self.min_value_entry.get().strip()
        max_value = self.max_value_entry.get().strip()
        err = 0
        if input_column == "":
            self.root.write_text("Input columns for y-axis has to be given\n")
            err += 1
        line_dict: dict[str, list | str | float] = {"input": [input_column]}
        if label not in ("optional", ""):
            line_dict["labels"] = [label]
        if min_value not in ("optional", ""):
            try:
                line_dict["min"] = float(min_value)
            except ValueError:
                self.root.write_text("Minimum y-value has to be given as a number\n")
                err += 1
        if max_value not in ("optional", ""):
            try:
                line_dict["max"] = float(max_value)
            except ValueError:
                self.root.write_text("Maximum y-value has to be given as a number\n")
                err += 1
        if err > 0:
            return
        self.plots[index]["mapping"][line_key] = line_dict

        try:
            self.update_treeview()
        except tk.TclError as e:
            self.root.write_text(str(e))
            del self.plots[index]["mapping"][line_key]
            return

        PlotConfigFrame.insert_text(self.y_axis_entry, "")
        PlotConfigFrame.insert_text(self.label_line_entry, "optional")
        PlotConfigFrame.insert_text(self.min_value_entry, "optional")
        PlotConfigFrame.insert_text(self.max_value_entry, "optional")
        self.label_line_entry.configure(font=self.font_italics)
        self.min_value_entry.configure(font=self.font_italics)
        self.max_value_entry.configure(font=self.font_italics)

    def open_line(self, line) -> None:
        """Open the given Line in the 'Line Data' Frame"""
        PlotConfigFrame.insert_text(self.y_axis_entry, line["input"][0])
        labels = ""
        min_value = ""
        max_value = ""
        if "min" in line:
            min_value = line["min"]
        if "max" in line:
            max_value = line["max"]
        if "labels" in line:
            labels = line["labels"][0]
        PlotConfigFrame.insert_text(self.min_value_entry, min_value)
        PlotConfigFrame.insert_text(self.max_value_entry, max_value)
        PlotConfigFrame.insert_text(self.label_line_entry, labels)
        self.min_value_entry.configure(font=self.font_default)
        self.max_value_entry.configure(font=self.font_default)
        self.label_line_entry.configure(font=self.font_default)

    def get_selected_item(self) -> tuple[str, str, int] | None:
        """Gets the selected item"""
        selected_item = self.plot_treeview.focus()
        if selected_item == "":
            self.root.write_text("Select an item from the table\n")
            return None

        if ("_" in selected_item) and selected_item.split("_")[-1] in (
            "y1",
            "y2",
            "y3",
        ):
            parent_plot = self.plot_treeview.parent(selected_item)
            selected_item = selected_item.split("_")[-1]
        else:
            parent_plot = selected_item

        for index, plot in enumerate(self.plots):
            if parent_plot == plot["name"]:
                return parent_plot, selected_item, index
        raise ValueError("Item couldn't be found\n")

    def open_selected_item_cb(self) -> None:
        """Gets the selected item and executes the corresponding function"""
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
        """Gets the selected item and removes it"""
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
            PlotConfigFrame.remove_line_from_plot(
                self.plots[index]["mapping"], selected_item
            )
        self.update_treeview()

    def generate_plot_config_cb(self) -> None:
        """Generate plot configuration file"""
        if len(self.plots) == 0:
            self.root.write_text("Add Plots to generate a Plot Configuration File\n")
            return
        for plot in self.plots:
            if (
                ("y1" not in plot["mapping"])
                and ("y2" not in plot["mapping"])
                and ("y3" not in plot["mapping"])
            ):
                self.root.write_text("Every Plot has to contain at least one line\n")
                return
        plot_file_path = self.file_path_entry.get().strip()
        if (plot_file_path == "") or (" " in plot_file_path):
            self.root.write_text(
                "Path of the Plot Configuration File has to be given as a valid path\n"
            )
            return
        Path(plot_file_path).parent.absolute().mkdir(parents=True, exist_ok=True)
        with open(plot_file_path, mode="w", encoding="utf-8") as f:
            json.dump(self.plots, f)
        self.root.write_text(
            f"Plot Configuration File has been saved in {plot_file_path}\n"
        )
        self.root.tab_plot.plot_config_entry.delete(0, tk.END)
        self.root.tab_plot.plot_config_entry.insert(tk.END, str(plot_file_path))

    def update_treeview(self) -> None:
        """Remove all elements from the treeview and read from self.plots"""
        for item in self.plot_treeview.get_children():
            self.plot_treeview.delete(item)

        for plot in self.plots:
            self.plot_treeview.insert("", tk.END, plot["name"], text=plot["name"])
            for key, line in plot["mapping"].items():
                if key in ("y1", "y2", "y3"):
                    line_input = line["input"][0]
                    self.plot_treeview.insert(
                        plot["name"], tk.END, line_input + "_" + key, text=line_input
                    )
                    self.plot_treeview.item(plot["name"], open=True)

    @staticmethod
    def remove_line_from_plot(mapping: dict[str, str | dict], key: str) -> None:
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
    def insert_text(tkinter_obj, input_str: str) -> None:
        """Delete the content of the object and insert the input"""
        tkinter_obj.delete(0, tk.END)
        tkinter_obj.insert(tk.END, input_str)
