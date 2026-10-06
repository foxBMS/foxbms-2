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

"""Testing file 'cli/cmd_gui/frame_plot/frame_plot_config.py'."""

# pylint: disable=too-many-lines

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from tkinter import font
from unittest.mock import MagicMock, call, mock_open, patch

try:
    from cli.cmd_gui.frame_plot.frame_plot_config import PlotConfigFrame
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot.frame_plot_config import PlotConfigFrame

try:
    from tests.cli.cmd_gui.tk_helpers import destroy_tk_root
except ModuleNotFoundError:
    from tests.cli.cmd_gui.tk_helpers import destroy_tk_root

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestPlotConfigFrame(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Test of the PlotConfigFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.root = tk.Tk()
        self.root.withdraw()
        self.frame = PlotConfigFrame(self.root, self.root)

    def tearDown(self) -> None:  # noqa: D102
        self.frame.destroy()
        del self.frame
        destroy_tk_root(self.root)
        del self.root

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function"""
        mock_filename.return_value = "Plot Configuration File"
        self.frame.select_file_cb()
        self.assertEqual(self.frame.file_path_entry.get(), "Plot Configuration File")

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file_no_selection(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function when no file is selected"""
        mock_filename.return_value = ""
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.file_path_entry.insert(tk.END, "file/path")
        self.frame.select_file_cb()
        self.assertEqual(self.frame.file_path_entry.get(), "file/path")

    def test_add_plot_invalid_input(self) -> None:
        """Test 'add_plot_cb' function for invalid input"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_type_combobox.delete(0, tk.END)
        self.frame.plot_title_entry.delete(0, tk.END)
        self.frame.x_axis_column_entry.delete(0, tk.END)
        self.frame.x_axis_name_entry.delete(0, tk.END)
        self.frame.y_axes_names_entry.delete(0, tk.END)
        self.frame.add_plot_cb()
        mock_write_text.assert_has_calls(
            [
                call("Name of the Plot-File has to be given as a valid path.\n"),
                call("Please provide an input column for the x-axis.\n"),
                call("Please provide a Title for the plot.\n"),
                call("Please provide a Label for the x-axis.\n"),
                call("Please provide Labels for the y-axes.\n"),
                call("Please select a Plot Type.\n"),
            ]
        )

    def test_add_plot_invalid_lines(self) -> None:
        """Test 'add_plot_cb' function if there are more than 3 labels for the y-axes"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "file")
        self.frame.x_axis_column_entry.delete(0, tk.END)
        self.frame.x_axis_column_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.x_axis_name_entry.insert(0, "Label")
        self.frame.y_axes_names_entry.delete(0, tk.END)
        self.frame.y_axes_names_entry.insert(0, "Label 1, Label 2, Label 3, Label 4")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "One Plot cannot contain more than 3 lines.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._insert_text")
    def test_add_plot_identical_names(
        self, mock_insert_text: MagicMock, mock_update_treeview: MagicMock
    ) -> None:
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots.append(
            {
                "name": "file",
                "type": "LINE",
                "mapping": {"x": "Column"},
                "description": {
                    "title": "Title",
                    "x_axis": "Label",
                    "y_axes": "Label 1, Label 2, Label 3",
                },
                "graph": {"show": False, "save": False},
            }
        )

        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "file")
        self.frame.x_axis_column_entry.delete(0, tk.END)
        self.frame.x_axis_column_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.x_axis_name_entry.insert(0, "Label")
        self.frame.y_axes_names_entry.delete(0, tk.END)
        self.frame.y_axes_names_entry.insert(0, "Label 1, Label 2, Label 3")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "Name of the Plot-File has to be unique for each Plot.\n"
        )
        mock_update_treeview.assert_not_called()
        mock_insert_text.assert_not_called()
        self.assertEqual(1, len(self.frame.plots))

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._insert_text")
    def test_add_plot_valid(
        self, mock_insert_text: MagicMock, mock_update_treeview: MagicMock
    ) -> None:
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots.append(
            {
                "name": "file_1",
                "type": "LINE",
                "mapping": {"x": "Column"},
                "description": {
                    "title": "Title",
                    "x_axis": "Label",
                    "y_axes": "Label 1, Label 2, Label 3",
                },
                "graph": {"show": False, "save": False},
            }
        )
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "file_2")
        self.frame.x_axis_column_entry.delete(0, tk.END)
        self.frame.x_axis_column_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.x_axis_name_entry.insert(0, "Label")
        self.frame.y_axes_names_entry.delete(0, tk.END)
        self.frame.y_axes_names_entry.insert(0, "Label 1, Label 2, Label 3")
        self.frame.plot_type_combobox.current(0)
        self.frame.add_plot_cb()
        mock_write_text.assert_not_called()
        mock_update_treeview.assert_called_once()
        mock_insert_text.assert_called()
        self.assertFalse(self.frame.show_plot_value.get())
        self.assertFalse(self.frame.save_plot_value.get())
        self.assertEqual(0, self.frame.plot_type_combobox.current())
        self.assertEqual(2, len(self.frame.plots))

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_empty(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when get_selected_item returns None"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = None
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_not_called()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_error(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when get_selected_item raises an Error"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.side_effect = ValueError("Item could not be found.\n")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Item could not be found.\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_line(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when a line is selected"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_y1", 0)
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Please select a Plot.\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_full_plot(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when selected plot has too many lines"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append(
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}, "y3": {}}}
        )
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "A Plot cannot contain more than 3 lines.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_no_column(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when no input column is specified"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append({"name": "plot_1", "mapping": {"y1": {}, "y2": {}}})
        self.frame.y_axis_column_entry.delete(0, tk.END)
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Please provide an input column for the y-axis.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_min_error(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when value for min is not a number"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append({"name": "plot_1", "mapping": {"y1": {}, "y2": {}}})
        self.frame.y_axis_column_entry.delete(0, tk.END)
        self.frame.y_axis_column_entry.insert(tk.END, "column")
        self.frame.min_value_entry.delete(0, tk.END)
        self.frame.min_value_entry.insert(tk.END, "number")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Minimum y-value has to be a number.\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_max_error(self, mock_get_item: MagicMock) -> None:
        """Test 'add_line_cb' function when value for max is not a number"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append({"name": "plot_1", "mapping": {"y1": {}, "y2": {}}})
        self.frame.y_axis_column_entry.delete(0, tk.END)
        self.frame.y_axis_column_entry.insert(tk.END, "column")
        self.frame.max_value_entry.delete(0, tk.END)
        self.frame.max_value_entry.insert(tk.END, "number")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Maximum y-value has to be a number.\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._update_treeview")
    def test_add_line_update_treeview_error(
        self, mock_update_treeview: MagicMock, mock_get_item: MagicMock
    ) -> None:
        """Test 'add_line_cb' function when the treeview cannot be updated"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_update_treeview.side_effect = tk.TclError("Could not update treeview.\n")
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        self.frame.plots.append(plot)
        self.frame.y_axis_column_entry.delete(0, tk.END)
        self.frame.y_axis_column_entry.insert(tk.END, "column")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        mock_write_text.assert_called_once_with("Could not update treeview.\n")
        self.assertEqual(self.frame.plots[0], plot)

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._insert_text")
    def test_add_line(
        self,
        mock_insert_text: MagicMock,
        mock_update_treeview: MagicMock,
        mock_get_item: MagicMock,
    ) -> None:
        """Test 'add_line_cb' function"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        self.frame.plots.append(plot)
        self.frame.y_axis_column_entry.delete(0, tk.END)
        self.frame.y_axis_column_entry.insert(tk.END, "column")
        self.frame.max_value_entry.delete(0, tk.END)
        self.frame.max_value_entry.insert(tk.END, "0")
        self.frame.min_value_entry.delete(0, tk.END)
        self.frame.min_value_entry.insert(tk.END, "0")
        self.frame.line_name_entry.delete(0, tk.END)
        self.frame.line_name_entry.insert(tk.END, "Label")
        self.frame.add_line_cb()
        plot["mapping"]["y3"] = {
            "input": ["column"],
            "labels": ["Label"],
            "max": 0,
            "min": 0,
        }
        calls = [
            call(self.frame.y_axis_column_entry, ""),
            call(self.frame.line_name_entry, "optional"),
            call(self.frame.min_value_entry, "optional"),
            call(self.frame.max_value_entry, "optional"),
        ]
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        self.assertEqual(self.frame.plots[0], plot)
        mock_write_text.assert_not_called()
        mock_insert_text.assert_has_calls(calls)

    def test_update_treeview_no_plots(self) -> None:
        """Test '_update_treeview' function without plots"""
        self.frame.plots_treeview.insert("", tk.END, "Plot 1")
        self.frame.plots_treeview.insert("", tk.END, "Plot 2")
        self.assertEqual(len(self.frame.plots_treeview.get_children()), 2)
        # pylint: disable-next=protected-access
        self.frame._update_treeview()
        self.assertEqual(len(self.frame.plots_treeview.get_children()), 0)

    def test_update_treeview_no_lines(self) -> None:
        """Test '_update_treeview' function when the plots have no lines"""
        plot_1 = {"name": "Plot 1", "mapping": {}}
        plot_2 = {"name": "Plot 2", "mapping": {}}
        self.frame.plots.append(plot_1)
        self.frame.plots.append(plot_2)
        self.assertEqual(len(self.frame.plots_treeview.get_children()), 0)
        # pylint: disable-next=protected-access
        self.frame._update_treeview()
        children = self.frame.plots_treeview.get_children()
        self.assertEqual(len(children), 2)
        for child in children:
            self.assertEqual(len(self.frame.plots_treeview.item(child)["values"]), 0)

    def test_update_treeview_lines(self) -> None:
        """Test '_update_treeview' function with valid lines"""
        plot_1 = {
            "name": "Plot 1",
            "mapping": {"y1": {"input": ["Line 1"]}, "y2": {"input": ["Line 2"]}},
        }
        plot_2 = {
            "name": "Plot 2",
            "mapping": {"y1": {"input": ["Line 3"]}, "y2": {"input": ["Line 4"]}},
        }
        self.frame.plots.append(plot_1)
        self.frame.plots.append(plot_2)
        self.assertEqual(len(self.frame.plots_treeview.get_children()), 0)
        # pylint: disable-next=protected-access
        self.frame._update_treeview()
        children = self.frame.plots_treeview.get_children()
        self.assertEqual(len(children), 2)
        for child in children:
            lines = self.frame.plots_treeview.get_children(child)
            self.assertEqual(len(lines), 2)
            for line in lines:
                self.assertIn(
                    line, ("Line 1_y1", "Line 2_y2", "Line 3_y1", "Line 4_y2")
                )

    def test_update_treeview_invalid_lines(self) -> None:
        """Test '_update_treeview' function with invalid lines"""
        self.frame.plots_treeview.insert("", tk.END, "Plot 1")
        self.frame.plots_treeview.insert("", tk.END, "Plot 2")
        plot_1 = {"name": "Plot 1", "mapping": {"invalid_1": {}, "Invalid_2": {}}}
        plot_2 = {"name": "Plot 2", "mapping": {"not_valid_1": {}, "Not_Valid_2": {}}}
        self.frame.plots.append(plot_1)
        self.frame.plots.append(plot_2)
        self.assertEqual(len(self.frame.plots_treeview.get_children()), 2)
        # pylint: disable-next=protected-access
        self.frame._update_treeview()
        children = self.frame.plots_treeview.get_children()
        self.assertEqual(len(children), 2)
        for child in children:
            self.assertEqual(len(self.frame.plots_treeview.get_children(child)), 0)

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_open_selected_item_error(self, mock_get_item: MagicMock) -> None:
        """Test 'open_selected_item_cb' function when get_selected_item raises an Error"""
        mock_get_item.side_effect = ValueError("Item could not be found.\n")
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.open_selected_item_cb()
        mock_write_text.assert_called_once_with("Item could not be found.\n")
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_open_selected_item_empty(self, mock_get_item: MagicMock) -> None:
        """Test 'open_selected_item_cb' function when get_selected_item returns None"""
        mock_get_item.return_value = None
        self.frame.open_selected_item_cb()
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.open_plot")
    def test_open_selected_item_plot(
        self, mock_open_plot: MagicMock, mock_get_item: MagicMock
    ) -> None:
        """Test 'open_selected_item_cb' function when the item is a plot"""
        mock_get_item.return_value = ("Plot", "Plot", 0)
        plot = {}
        self.frame.plots.append(plot)
        self.frame.open_selected_item_cb()
        mock_open_plot.assert_called_once_with(plot)
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.open_line")
    def test_open_selected_item_line(
        self, mock_open_line: MagicMock, mock_get_item: MagicMock
    ) -> None:
        """Test 'open_selected_item_cb' function when the item is a line"""
        mock_get_item.return_value = ("Plot", "y1", 0)
        plot = {"mapping": {"y1": {}}}
        self.frame.plots.append(plot)
        self.frame.open_selected_item_cb()
        mock_open_line.assert_called_once_with(plot["mapping"]["y1"])
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_remove_selected_item_error(self, mock_get_item: MagicMock) -> None:
        """Test 'remove_selected_item_cb' function when get_selected_item raises an Error"""
        mock_get_item.side_effect = ValueError("Item could not be found.\n")
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.remove_selected_item_cb()
        mock_write_text.assert_called_once_with("Item could not be found.\n")
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_remove_selected_item_empty(self, mock_get_item: MagicMock) -> None:
        """Test 'remove_selected_item_cb' function when get_selected_item returns None"""
        mock_get_item.return_value = None
        self.frame.remove_selected_item_cb()
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._update_treeview")
    def test_remove_selected_item_plot(
        self, mock_update_treeview: MagicMock, mock_get_item: MagicMock
    ) -> None:
        """Test 'remove_selected_item_cb' function when the item is a plot"""
        mock_get_item.return_value = ("Plot", "Plot", 0)
        plot = {}
        self.frame.plots.append(plot)
        self.frame.remove_selected_item_cb()
        self.assertEqual(0, len(self.frame.plots))
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch(
        "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._remove_line_from_plot"
    )
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame._update_treeview")
    def test_remove_selected_item_line(
        self,
        mock_update_treeview: MagicMock,
        mock_remove_line: MagicMock,
        mock_get_item: MagicMock,
    ) -> None:
        """Test 'remove_selected_item_cb' function when the item is a line"""
        mock_get_item.return_value = ("Plot", "y1", 0)
        plot = {"mapping": {"y1": {}}}
        self.frame.plots.append(plot)
        self.frame.remove_selected_item_cb()
        mock_remove_line.assert_called_once_with(plot["mapping"], "y1")
        mock_update_treeview.assert_called_once()
        mock_get_item.assert_called_once()

    def test_get_selected_item_empty(self) -> None:
        """Test 'get_selected_item' function when no item is selected"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = ""
        self.frame.plots_treeview.focus = mock_treeview_focus
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        result = self.frame.get_selected_item()
        mock_write_text.assert_called_once_with("Please select an item.\n")
        mock_treeview_focus.assert_called_once()
        self.assertIsNone(result)

    def test_get_selected_item_plot(self) -> None:
        """Test 'get_selected_item' function when a plot is selected"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_1"
        mock_write_text = MagicMock()
        self.frame.plots_treeview.focus = mock_treeview_focus
        self.frame.root.write_text = mock_write_text
        self.frame.plots = [{"name": "plot_2"}, {"name": "plot_1"}]
        result = self.frame.get_selected_item()
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual(("plot_1", "plot_1", 1), result)

    def test_get_selected_item_line(self) -> None:
        """Test 'get_selected_item' function when a line is selected"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_y1"
        self.frame.plots_treeview.focus = mock_treeview_focus
        mock_treeview_parent = MagicMock()
        mock_treeview_parent.return_value = "plot_1"
        self.frame.plots_treeview.parent = mock_treeview_parent
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots = [{"name": "plot_2"}, {"name": "plot_3"}, {"name": "plot_1"}]
        result = self.frame.get_selected_item()
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        mock_treeview_parent.assert_called_once_with(mock_treeview_focus.return_value)
        self.assertEqual(("plot_1", "y1", 2), result)

    def test_get_selected_item_error(self) -> None:
        """Test 'get_selected_item' function when the selected item not in list"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_1"
        mock_write_text = MagicMock()
        self.frame.plots_treeview.focus = mock_treeview_focus
        self.frame.root.write_text = mock_write_text
        self.frame.plots = [{"name": "plot_2"}, {"name": "plot_3"}]
        with self.assertRaises(ValueError) as e:
            self.frame.get_selected_item()
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual("Item could not be found.\n", str(e.exception))

    def test_generate_plot_config_no_plots(self) -> None:
        """Test 'generate_plot_config_cb' function without plots"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.assertEqual(0, len(self.frame.plots))
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with("Please add Plots.\n")

    def test_generate_plot_config_no_lines(self) -> None:
        """Test 'generate_plot_config_cb' function without lines"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots.append({"mapping": {}})
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with(
            "Every Plot has to contain at least one line.\n"
        )

    def test_generate_plot_config_invalid_path(self) -> None:
        """Test 'generate_plot_config_cb' function with invalid file path"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots.append({"mapping": {"y1": {}}})
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with(
            "Path of the Plot Configuration File has to be given as a valid path.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.json")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.Path.mkdir")
    def test_generate_plot_config_valid_input(
        self, mock_mkdir: MagicMock, mock_json: MagicMock
    ) -> None:
        """Test 'generate_plot_config_cb' function with valid input"""
        mock_open_file = mock_open()
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_tab_plot = MagicMock()
        self.frame.root.run_plot_tab = mock_tab_plot
        self.frame.plots.append({"mapping": {"y1": {}}})
        plot_file_path = "test.yaml"
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.file_path_entry.insert(tk.END, plot_file_path)
        with patch("builtins.open", mock_open_file):
            self.frame.generate_plot_config_cb()
        mock_mkdir.assert_called_once_with(parents=True, exist_ok=True)
        mock_open_file.assert_called_once_with(
            plot_file_path, mode="w", encoding="utf-8"
        )
        mock_json.dump.assert_called_once_with(self.frame.plots, mock_open_file())
        mock_tab_plot.plot_config_entry.delete.assert_called_once_with(0, tk.END)
        mock_tab_plot.plot_config_entry.insert.assert_called_once_with(
            tk.END, plot_file_path
        )
        mock_write_text.assert_called_once_with(
            f"Plot Configuration File has been saved in '{plot_file_path}'.\n"
        )

    def test_change_font_entry(self) -> None:
        """Test 'change_font_cb' function with an entry widget"""
        event = tk.Event()
        event.widget = self.frame.y_axes_names_entry
        self.assertEqual(
            font.nametofont(str(event.widget.cget("font"))).actual("slant"),
            "italic",
        )
        self.frame.change_font_cb(event)
        self.assertEqual(
            font.nametofont(str(event.widget.cget("font"))).actual("slant"),
            "roman",
        )

    def test_change_font_no_entry(self) -> None:
        """Test 'change_font_cb' function with a widget that is not an entry widget"""
        event = tk.Event()
        event.widget = tk.Button()
        self.frame.change_font_cb(event)


# pylint: disable=too-many-public-methods
class TestPlotConfigFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the PlotConfigFrame class"""

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function"""
        mock_plot_config_file = MagicMock()
        mock_filename.return_value = "Plot Configuration File"
        PlotConfigFrame.select_file_cb(mock_plot_config_file)
        mock_plot_config_file.file_path_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_config_file.file_path_entry.insert.assert_called_once_with(
            tk.END, "Plot Configuration File"
        )

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file_no_selection(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function when no file is selected"""
        mock_plot_config_file = MagicMock()
        mock_filename.return_value = ""
        PlotConfigFrame.select_file_cb(mock_plot_config_file)
        mock_plot_config_file.file_path_entry.delete.assert_not_called()
        mock_plot_config_file.file_path_entry.insert.assert_not_called()

    def test_add_plot_name_invalid_input(self) -> None:
        """Test 'add_plot_cb' function if the file name is invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = ""
        mock_plot_config_file.plot_type_combobox.get.return_value = ""
        mock_plot_config_file.plot_title_entry.get.return_value = ""
        mock_plot_config_file.x_axis_column_entry.get.return_value = ""
        mock_plot_config_file.x_axis_name_entry.get.return_value = ""
        mock_plot_config_file.y_axes_names_entry.get.return_value = ""
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_has_calls(
            [
                call("Name of the Plot-File has to be given as a valid path.\n"),
                call("Please provide an input column for the x-axis.\n"),
                call("Please provide a Title for the plot.\n"),
                call("Please provide a Label for the x-axis.\n"),
                call("Please provide Labels for the y-axes.\n"),
                call("Please select a Plot Type.\n"),
            ]
        )

    def test_add_plot_invalid_lines(self) -> None:
        """Test 'add_plot_cb' function if there are more than 3 labels for the y-axes"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_column_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.x_axis_name_entry.get.return_value = "Label"
        mock_plot_config_file.y_axes_names_entry.get.return_value = (
            "Label 1, Label 2, Label 3, Label 4"
        )
        mock_plot_config_file.plot_type_combobox.get.return_value = "LINE"
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "One Plot cannot contain more than 3 lines.\n"
        )

    def test_add_plot_identical_names(self) -> None:
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_insert_text = MagicMock()
        # pylint: disable-next=protected-access
        mock_plot_config_frame._insert_text = mock_insert_text
        mock_plot_config_frame.plot_file_name_entry.get.return_value = "file"
        mock_plot_config_frame.x_axis_column_entry.get.return_value = "Column"
        mock_plot_config_frame.plot_title_entry.get.return_value = "Title"
        mock_plot_config_frame.x_axis_name_entry.get.return_value = "Label"
        mock_plot_config_frame.y_axes_names_entry.get.return_value = (
            "Label 1, Label 2, Label 3"
        )
        mock_plot_config_frame.plot_type_combobox.get.return_value = "LINE"
        mock_plot_config_frame.show_plot_value.get.return_value = False
        mock_plot_config_frame.save_plot_value.get.return_value = False
        mock_plot_config_frame.plots = [
            {
                "name": "file",
                "type": "LINE",
                "mapping": {"x": "Column"},
                "description": {
                    "title": "Title",
                    "x_axis": "Label",
                    "y_axes": "Label 1, Label 2, Label 3",
                },
                "graph": {"show": False, "save": False},
            }
        ]
        PlotConfigFrame.add_plot_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            "Name of the Plot-File has to be unique for each Plot.\n"
        )
        # pylint: disable-next=protected-access
        mock_plot_config_frame._update_treeview.assert_not_called()
        mock_insert_text.assert_not_called()
        mock_plot_config_frame.show_plot_value.set.assert_not_called()
        mock_plot_config_frame.save_plot_value.set.assert_not_called()
        mock_plot_config_frame.plot_type_combobox.current.assert_not_called()
        self.assertEqual(1, len(mock_plot_config_frame.plots))

    def test_add_plot_valid(self) -> None:
        """Test 'add_plot_cb' function if two plots have different names"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_insert_text = MagicMock()
        # pylint: disable-next=protected-access
        mock_plot_config_file._insert_text = mock_insert_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "file_2"
        mock_plot_config_file.x_axis_column_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.x_axis_name_entry.get.return_value = "Label"
        mock_plot_config_file.y_axes_names_entry.get.return_value = (
            "Label 1, Label 2, Label 3"
        )
        mock_plot_config_file.plot_type_combobox.get.return_value = "LINE"
        mock_plot_config_file.show_plot_value.get.return_value = False
        mock_plot_config_file.save_plot_value.get.return_value = False
        mock_plot_config_file.plots = [
            {
                "name": "file_1",
                "type": "LINE",
                "mapping": {"x": "Column"},
                "description": {
                    "title": "Title",
                    "x_axis": "Label",
                    "y_axes": "Label 1, Label 2, Label 3",
                },
                "graph": {"show": False, "save": False},
            }
        ]
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_not_called()
        # pylint: disable-next=protected-access
        mock_plot_config_file._update_treeview.assert_called_once()
        mock_insert_text.assert_called()
        mock_plot_config_file.show_plot_value.set.assert_called_once_with(False)
        mock_plot_config_file.save_plot_value.set.assert_called_once_with(False)
        mock_plot_config_file.plot_type_combobox.current.assert_called_once_with(0)
        self.assertEqual(2, len(mock_plot_config_file.plots))

    def test_update_treeview_no_plots(self) -> None:
        """Test '_update_treeview' function when plots is empty"""
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plots_treeview = MagicMock()
        mock_plot_config_frame.plots_treeview.get_children.return_value = [
            "Child 1",
            "Child 2",
        ]
        mock_plot_config_frame.plots = []
        # pylint: disable-next=protected-access
        PlotConfigFrame._update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plots_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plots_treeview.delete.assert_has_calls(
            [call("Child 1"), call("Child 2")]
        )

    def test_update_treeview_plots_no_lines(self) -> None:
        """Test '_update_treeview' function when the plots have no lines"""
        plot_1 = {"name": "Plot 1", "mapping": {}}
        plot_2 = {"name": "Plot 2", "mapping": {}}
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plots_treeview = MagicMock()
        mock_plot_config_frame.plots_treeview.get_children.return_value = []
        mock_plot_config_frame.plots = [plot_1, plot_2]
        # pylint: disable-next=protected-access
        PlotConfigFrame._update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plots_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plots_treeview.delete.assert_not_called()
        mock_plot_config_frame.plots_treeview.insert.assert_has_calls(
            [
                call("", tk.END, "Plot 1", text="Plot 1"),
                call("", tk.END, "Plot 2", text="Plot 2"),
            ]
        )

    def test_update_treeview_plots_lines(self) -> None:
        """Test '_update_treeview' function with plots and valid lines"""
        plot_1 = {
            "name": "Plot 1",
            "mapping": {"y1": {"input": ["Line 1"]}, "y2": {"input": ["Line 2"]}},
        }
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plots_treeview = MagicMock()
        mock_plot_config_frame.plots_treeview.get_children.return_value = []
        mock_plot_config_frame.plots = [plot_1]
        # pylint: disable-next=protected-access
        PlotConfigFrame._update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plots_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plots_treeview.delete.assert_not_called()
        mock_plot_config_frame.plots_treeview.insert.assert_has_calls(
            [
                call("", tk.END, "Plot 1", text="Plot 1"),
                call("Plot 1", tk.END, "Line 1_y1", text="Line 1"),
                call("Plot 1", tk.END, "Line 2_y2", text="Line 2"),
            ]
        )

    def test_update_treeview_plots_invalid_lines(self) -> None:
        """Test '_update_treeview' function with plots and invalid lines"""
        plot_1 = {"name": "Plot 1", "mapping": {"invalid_1": {}, "Invalid_2": {}}}
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plots_treeview = MagicMock()
        mock_plot_config_frame.plots_treeview.get_children.return_value = []
        mock_plot_config_frame.plots = [plot_1]
        # pylint: disable-next=protected-access
        PlotConfigFrame._update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plots_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plots_treeview.delete.assert_not_called()
        mock_plot_config_frame.plots_treeview.insert.assert_has_calls(
            [call("", tk.END, "Plot 1", text="Plot 1")]
        )

    def test_open_selected_item_error(self) -> None:
        """Test 'open_selected_item_cb' function when get_selected_item raises an Error"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.side_effect = ValueError("Item could not be found.\n")
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.open_selected_item_cb(mock_plot_config_frame)
        mock_plot_config_frame.root.write_text.assert_called_once_with(
            "Item could not be found.\n"
        )
        mock_get_item.assert_called_once()

    def test_open_selected_item_empty(self) -> None:
        """Test 'open_selected_item_cb' function when get_selected_item returns None"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = None
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.open_selected_item_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()

    def test_open_selected_item_plot(self) -> None:
        """Test 'open_selected_item_cb' function when the item is a plot"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("Plot", "Plot", 0)
        mock_open_plot = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.open_plot = mock_open_plot
        plot = {}
        mock_plot_config_frame.plots = [plot]
        PlotConfigFrame.open_selected_item_cb(mock_plot_config_frame)
        mock_open_plot.assert_called_once_with(plot)
        mock_get_item.assert_called_once()

    def test_open_selected_item_line(self) -> None:
        """Test 'open_selected_item_cb' function when the item is a line"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("Plot", "y1", 0)
        mock_open_line = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.open_line = mock_open_line
        plot = {"mapping": {"y1": {}}}
        mock_plot_config_frame.plots = [plot]
        PlotConfigFrame.open_selected_item_cb(mock_plot_config_frame)
        mock_open_line.assert_called_once_with(plot["mapping"]["y1"])
        mock_get_item.assert_called_once()

    def test_remove_selected_item_error(self) -> None:
        """Test 'remove_selected_item_cb' function when get_selected_item raises an Error"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.side_effect = ValueError("Item could not be found.\n")
        mock_write_text = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.root.write_text = mock_write_text
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with("Item could not be found.\n")
        mock_get_item.assert_called_once()

    def test_remove_selected_item_empty(self) -> None:
        """Test 'remove_selected_item_cb' function when get_selected_item returns None"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_get_item.return_value = None
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()

    def test_remove_selected_item_plot(self) -> None:
        """Test 'remove_selected_item_cb' function when the item is a plot"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_update_treeview = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        # pylint: disable-next=protected-access
        mock_plot_config_frame._update_treeview = mock_update_treeview
        mock_plot_config_frame.plots = [{}]
        mock_get_item.return_value = ("Plot", "Plot", 0)
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        self.assertEqual(0, len(mock_plot_config_frame.plots))
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()

    def test_remove_selected_item_line(self) -> None:
        """Test 'remove_selected_item_cb' function when the item is a line"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_update_treeview = MagicMock()
        # pylint: disable-next=protected-access
        mock_plot_config_frame._update_treeview = mock_update_treeview
        mock_remove_line = MagicMock()
        # pylint: disable-next=protected-access
        mock_plot_config_frame._remove_line_from_plot = mock_remove_line
        plot = {"mapping": {"y1": {}}}
        mock_plot_config_frame.plots = [plot]
        mock_get_item.return_value = ("Plot", "y1", 0)
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        mock_remove_line.assert_called_once_with(plot["mapping"], "y1")
        mock_update_treeview.assert_called_once()
        mock_get_item.assert_called_once()

    def test_get_selected_item_empty(self) -> None:
        """Test 'get_selected_item' function when no item is selected"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = ""
        mock_write_text = MagicMock()
        mock_plot_config_frame.plots_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.root.write_text = mock_write_text
        result = PlotConfigFrame.get_selected_item(mock_plot_config_frame)
        mock_write_text.assert_called_once_with("Please select an item.\n")
        mock_treeview_focus.assert_called_once()
        self.assertIsNone(result)

    def test_get_selected_item_plot(self) -> None:
        """Test 'get_selected_item' function when a plot is selected"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_1"
        mock_write_text = MagicMock()
        mock_plot_config_frame.plots_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [{"name": "plot_2"}, {"name": "plot_1"}]
        result = PlotConfigFrame.get_selected_item(mock_plot_config_frame)
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual(("plot_1", "plot_1", 1), result)

    def test_get_selected_item_line(self) -> None:
        """Test 'get_selected_item' function when a line is selected"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_y1"
        mock_treeview_parent = MagicMock()
        mock_treeview_parent.return_value = "plot_1"
        mock_write_text = MagicMock()
        mock_plot_config_frame.plots_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.plots_treeview.parent = mock_treeview_parent
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [
            {"name": "plot_2"},
            {"name": "plot_3"},
            {"name": "plot_1"},
        ]
        result = PlotConfigFrame.get_selected_item(mock_plot_config_frame)
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        mock_treeview_parent.assert_called_once_with(mock_treeview_focus.return_value)
        self.assertEqual(("plot_1", "y1", 2), result)

    def test_get_selected_item_error(self) -> None:
        """Test 'get_selected_item' function when the selected item not in list"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_not_existing"
        mock_write_text = MagicMock()
        mock_plot_config_frame.plots_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [
            {"name": "plot_2"},
            {"name": "plot_3"},
            {"name": "plot_1"},
        ]
        with self.assertRaises(ValueError) as e:
            PlotConfigFrame.get_selected_item(mock_plot_config_frame)
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual("Item could not be found.\n", str(e.exception))

    def test_generate_plot_config_no_plots(self) -> None:
        """Test 'generate_plot_config_cb' function when there are no plots"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = []
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with("Please add Plots.\n")

    def test_generate_plot_config_no_lines(self) -> None:
        """Test 'generate_plot_config_cb' function when there are no lines"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [{"mapping": {}}]
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            "Every Plot has to contain at least one line.\n"
        )

    def test_generate_plot_config_invalid_path(self) -> None:
        """Test 'generate_plot_config_cb' function when the file path is not valid"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_file_path_entry = MagicMock()
        mock_file_path_entry.get.return_value = ""
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [{"mapping": {"y1": {}}}]
        mock_plot_config_frame.file_path_entry = mock_file_path_entry
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            "Path of the Plot Configuration File has to be given as a valid path.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.json")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.Path.mkdir")
    def test_generate_plot_config(
        self, mock_mkdir: MagicMock, mock_json: MagicMock
    ) -> None:
        """Test 'generate_plot_config_cb' function"""
        mock_open_file = mock_open()
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_file_path_entry = MagicMock()
        plot_file_path = "test.yaml"
        mock_file_path_entry.get.return_value = plot_file_path
        mock_plot_config_frame.file_path_entry = mock_file_path_entry
        mock_tab_plot = MagicMock()
        mock_plot_config_frame.root.run_plot_tab = mock_tab_plot
        mock_plot_config_frame.plots = [{"mapping": {"y1": {}}}]
        with patch("builtins.open", mock_open_file):
            PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_mkdir.assert_called_once_with(parents=True, exist_ok=True)
        mock_open_file.assert_called_once_with(
            plot_file_path, mode="w", encoding="utf-8"
        )
        mock_json.dump.assert_called_once_with(
            mock_plot_config_frame.plots, mock_open_file()
        )
        mock_tab_plot.plot_config_entry.delete.assert_called_once_with(0, tk.END)
        mock_tab_plot.plot_config_entry.insert.assert_called_once_with(
            tk.END, plot_file_path
        )
        mock_write_text.assert_called_once_with(
            f"Plot Configuration File has been saved in '{plot_file_path}'.\n"
        )

    def test_add_line_empty(self) -> None:
        """Test 'add_line_cb' function when get_selected_item returns None"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = None
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_not_called()

    def test_add_line_error(self) -> None:
        """Test 'add_line_cb' function when get_selected_item raises an Error"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.side_effect = ValueError("Item could not be found.\n")
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Item could not be found.\n")

    def test_add_line_line(self) -> None:
        """Test 'add_line_cb' function when a line is selected"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_y1", 0)
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Please select a Plot.\n")

    def test_add_line_full_plot(self) -> None:
        """Test 'add_line_cb' function when selected plot has too many lines"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.plots = [
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}, "y3": {}}}
        ]
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "A Plot cannot contain more than 3 lines.\n"
        )

    def test_add_line_no_column(self) -> None:
        """Test 'add_line_cb' function when no input column is specified"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = ""
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.plots = [
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        ]
        mock_plot_config_frame.y_axis_column_entry = mock_y_axis_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Please provide an input column for the y-axis.\n"
        )

    def test_add_line_min_error(self) -> None:
        """Test 'add_line_cb' function when value for min is not a number"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_min_value_entry = MagicMock()
        mock_min_value_entry.get.return_value = "number"
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.plots = [
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        ]
        mock_plot_config_frame.y_axis_column_entry = mock_y_axis_entry
        mock_plot_config_frame.min_value_entry = mock_min_value_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Minimum y-value has to be a number.\n")

    def test_add_line_max_error(self) -> None:
        """Test 'add_line_cb' function when value for max is not a number"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_max_value_entry = MagicMock()
        mock_max_value_entry.get.return_value = "number"
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.plots = [
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        ]
        mock_plot_config_frame.y_axis_column_entry = mock_y_axis_entry
        mock_plot_config_frame.max_value_entry = mock_max_value_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Maximum y-value has to be a number.\n")

    def test_add_line_update_treeview_error(self) -> None:
        """Test 'add_line_cb' function when the treeview cannot be updated"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_update_treeview = MagicMock()
        mock_update_treeview.side_effect = tk.TclError("Could not update treeview.\n")
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        # pylint: disable-next=protected-access
        mock_plot_config_frame._update_treeview = mock_update_treeview
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        mock_plot_config_frame.plots = [plot]
        mock_plot_config_frame.y_axis_column_entry = mock_y_axis_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        mock_write_text.assert_called_once_with("Could not update treeview.\n")
        self.assertEqual(mock_plot_config_frame.plots[0], plot)

    def test_add_line(self) -> None:
        """Test 'add_line_cb' function"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_update_treeview = MagicMock()
        mock_insert_text = MagicMock()
        # pylint: disable-next=protected-access
        mock_plot_config_frame._insert_text = mock_insert_text
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_max_value_entry = MagicMock()
        mock_max_value_entry.get.return_value = "0"
        mock_min_value_entry = MagicMock()
        mock_min_value_entry.get.return_value = "0"
        mock_label_line_entry = MagicMock()
        mock_label_line_entry.get.return_value = "Label"
        # pylint: disable-next=protected-access
        mock_plot_config_frame._update_treeview = mock_update_treeview
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        mock_plot_config_frame.plots = [plot]
        mock_plot_config_frame.y_axis_column_entry = mock_y_axis_entry
        mock_plot_config_frame.max_value_entry = mock_max_value_entry
        mock_plot_config_frame.min_value_entry = mock_min_value_entry
        mock_plot_config_frame.line_name_entry = mock_label_line_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        plot["mapping"]["y3"] = {
            "input": ["column"],
            "labels": ["Label"],
            "max": 0,
            "min": 0,
        }
        calls = [
            call(mock_plot_config_frame.y_axis_column_entry, ""),
            call(mock_plot_config_frame.line_name_entry, "optional"),
            call(mock_plot_config_frame.min_value_entry, "optional"),
            call(mock_plot_config_frame.max_value_entry, "optional"),
        ]
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        self.assertEqual(mock_plot_config_frame.plots[0], plot)
        mock_write_text.assert_not_called()
        mock_insert_text.assert_has_calls(calls)


class TestOpenPlotLine(unittest.TestCase):
    """Test of the 'open_plot' and 'open_line' functions of the PlotConfigFrame class"""

    def test_open_plot(self) -> None:
        """Test of 'open_plot' function"""
        plot = {
            "name": "Name",
            "type": "Type",
            "mapping": {"x": "Test"},
            "description": {
                "title": "Title",
                "x_axis": "X-Axis",
                "y_axes": ["Y-Axis 1", "Y-Axis 2"],
            },
            "graph": {"show": True, "save": False},
        }
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.show_plot_value.set = MagicMock()
        mock_plot_config_frame.save_plot_value.set = MagicMock()
        PlotConfigFrame.open_plot(mock_plot_config_frame, plot)
        mock_plot_config_frame.show_plot_value.set.assert_called_once_with(
            plot["graph"]["show"]
        )
        mock_plot_config_frame.save_plot_value.set.assert_called_once_with(
            plot["graph"]["save"]
        )
        calls = [
            call(mock_plot_config_frame.plot_file_name_entry, plot["name"]),
            call(mock_plot_config_frame.plot_type_combobox, plot["type"]),
            call(mock_plot_config_frame.x_axis_column_entry, plot["mapping"]["x"]),
            call(mock_plot_config_frame.plot_title_entry, plot["description"]["title"]),
            call(
                mock_plot_config_frame.x_axis_name_entry, plot["description"]["x_axis"]
            ),
            call(
                mock_plot_config_frame.y_axes_names_entry,
                "Y-Axis 1, Y-Axis 2",
            ),
        ]
        # pylint: disable-next=protected-access
        mock_plot_config_frame._insert_text.assert_has_calls(calls)

    def test_open_line(self) -> None:
        """Test 'open_line' function without additional input"""
        mock_plot_config_frame = MagicMock()
        line = {"input": ["Line 1"]}
        PlotConfigFrame.open_line(mock_plot_config_frame, line)
        # pylint: disable-next=protected-access
        mock_plot_config_frame._insert_text.assert_called_once_with(
            mock_plot_config_frame.y_axis_column_entry, "Line 1"
        )

    def test_open_line_additional_input(self) -> None:
        """Test 'open_line' function with additional input"""
        mock_plot_config_frame = MagicMock()
        line = {"input": ["Line 1"], "min": 10, "max": 20, "labels": ["Label 1"]}
        PlotConfigFrame.open_line(mock_plot_config_frame, line)
        calls = [
            call(mock_plot_config_frame.y_axis_column_entry, "Line 1"),
            call(mock_plot_config_frame.min_value_entry, 10),
            call(mock_plot_config_frame.max_value_entry, 20),
            call(mock_plot_config_frame.line_name_entry, "Label 1"),
        ]
        # pylint: disable-next=protected-access
        mock_plot_config_frame._insert_text.assert_has_calls(calls)


class TestRemoveLine(unittest.TestCase):
    """Test of the '_remove_line_from_plot' function of the PlotConfigFrame class"""

    def test_y3(self) -> None:
        """Tests removing line y3"""
        mapping = {"y1": {}, "y2": {}, "y3": {}}
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y3")
        self.assertNotIn("y3", mapping)

    def test_y2_with_y3(self) -> None:
        """Tests removing line y2 when line y3 exists"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
            "y3": {"former_key": "y3"},
        }
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y2")
        self.assertNotIn("y3", mapping)
        self.assertIn("y2", mapping)
        self.assertEqual(mapping["y2"]["former_key"], "y3")

    def test_y2_without_y3(self) -> None:
        """Tests removing line y2 without line y3"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
        }
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y2")
        self.assertNotIn("y3", mapping)
        self.assertNotIn("y2", mapping)

    def test_y1_with_y3(self) -> None:
        """Tests removing line y1 when line y3 exists"""
        mapping = {"y1": {"former_key": "y1"}, "y3": {"former_key": "y3"}}
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y3", mapping)
        self.assertIn("y1", mapping)
        self.assertEqual(mapping["y1"]["former_key"], "y3")

    def test_y1_with_y2(self) -> None:
        """Tests removing line y1 when line y2 exists"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
        }
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y2", mapping)
        self.assertIn("y1", mapping)
        self.assertEqual(mapping["y1"]["former_key"], "y2")

    def test_y1_with_y2_y3(self) -> None:
        """Tests removing line y1 when lines y2 and y3 exist"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
            "y3": {"former_key": "y3"},
        }
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y3", mapping)
        self.assertIn("y2", mapping)
        self.assertIn("y1", mapping)
        self.assertEqual(mapping["y1"]["former_key"], "y3")

    def test_y1_without_y2_y3(self) -> None:
        """Tests removing line y1 without lines y2 and y3"""
        mapping = {
            "y1": {"former_key": "y1"},
        }
        # pylint: disable-next=protected-access
        PlotConfigFrame._remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y3", mapping)
        self.assertNotIn("y2", mapping)
        self.assertNotIn("y1", mapping)


class TestInsertText(unittest.TestCase):
    """Test of the 'insert_text' function of the PlotConfigFrame class"""

    def test_insert_text(self) -> None:
        """Test 'insert_text' function"""
        entry_widget = MagicMock()
        # pylint: disable-next=protected-access
        PlotConfigFrame._insert_text(entry_widget, "New Entry")
        entry_widget.delete.assert_called_once_with(0, tk.END)
        entry_widget.insert(tk.END, "New Entry")


if __name__ == "__main__":
    unittest.main()
