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

"""Testing file 'cli/cmd_gui/frame_plot/frame_plot_config.py'."""

# pylint: disable=too-many-lines

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.cmd_gui.frame_plot.frame_plot_config import PlotConfigFrame, font
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot.frame_plot_config import PlotConfigFrame, font

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestPlotConfigFrame(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Test of the PlotConfigFrame class"""

    def setUp(self):
        parent = tk.Tk()
        parent.withdraw()
        self.frame = PlotConfigFrame(parent, parent)

    def tearDown(self):
        self.frame.root.update()
        self.frame.root.destroy()
        if hasattr(self.frame, "file_stream"):
            self.frame.file_stream.close()

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_open_file_cb(self, mock_filename: MagicMock):
        """Test 'open_file_cb' function"""
        mock_filename.return_value = "Plot Configuration File"
        self.frame.open_file_cb()
        self.assertEqual(self.frame.file_path_entry.get(), "Plot Configuration File")

    def test_add_plot_cb_name_invalid(self):
        """Test 'add_plot_cb' function if the file name is invalid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.label_x_axis_entry.delete(0, tk.END)
        self.frame.label_x_axis_entry.insert(tk.END, "Label")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "Name of the Plot-File has to be given as a valid path\n"
        )

    def test_add_plot_cb_no_x_input(self):
        """Test 'add_plot_cb' function if input for x-axis is invalid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "Input column for x-axis has to be given\n"
        )

    def test_add_plot_cb_title_invalid(self):
        """Test 'add_plot_cb' function if title is invalid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.plot_title_entry.delete(0, tk.END)
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with("Title of the plot has to be given\n")

    def test_add_plot_cb_label_x_invalid(self):
        """Test 'add_plot_cb' function if label for x-axis is invalid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.label_x_axis_entry.delete(0, tk.END)
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "Label for the x-axis has to be given\n"
        )

    def test_add_plot_cb_label_y_invalid(self):
        """Test 'add_plot_cb' function if labels for y-axes are invalid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "Labels for the y-axes have to be given\n"
        )

    def test_add_plot_cb_type_invalid(self):
        """Test 'add_plot_cb' function if type is invalid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label")
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with("Plot Type has to be given\n")

    def test_add_plot_cb_lines(self):
        """Test 'add_plot_cb' function if there are more than 3 labels for the y-axes"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label 1, Label 2, Label 3, Label 4")
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.plot_type_entry.insert(0, "type")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "One Plot can not contain more than 3 lines\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_add_plot_cb(
        self, mock_insert_text: MagicMock, mock_update_treeview: MagicMock
    ):
        """Test 'add_plot_cb' function"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label 1, Label 2, Label 3")
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.plot_type_entry.insert(0, "type")
        self.frame.add_plot_cb()
        plot = {
            "name": "test_file",
            "type": "type",
            "mapping": {"x": "Column"},
            "description": {
                "title": "Title",
                "x_axis": "Label",
                "y_axes": ["Label 1", "Label 2", "Label 3"],
            },
            "graph": {"show": False, "save": False},
        }
        mock_write_text.assert_not_called()
        mock_update_treeview.assert_called_once()
        mock_insert_text.assert_has_calls(
            [
                call(self.frame.plot_file_name_entry, ""),
                call(self.frame.plot_title_entry, ""),
                call(self.frame.x_axis_entry, ""),
                call(self.frame.label_x_axis_entry, ""),
                call(self.frame.label_y_axes_entry, "separate labels with a comma"),
            ]
        )
        self.assertFalse(self.frame.show_checkbutton_value.get())
        self.assertFalse(self.frame.save_checkbutton_value.get())
        self.assertEqual(0, self.frame.plot_type_entry.current())
        self.assertEqual(1, len(self.frame.plots))
        self.assertEqual(plot, self.frame.plots[0])

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_add_plot_cb_identical_names(
        self, mock_insert_text: MagicMock, mock_update_treeview: MagicMock
    ):
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        # Plot 1
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label 1, Label 2, Label 3")
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.plot_type_entry.insert(0, "type")
        self.frame.add_plot_cb()
        # Plot 2
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.plot_type_entry.insert(0, "type")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label 1")
        self.frame.add_plot_cb()
        mock_write_text.assert_called_once_with(
            "Name of the Plot-File has to be unique for each Plot\n"
        )
        mock_update_treeview.assert_called_once()
        mock_insert_text.assert_has_calls(
            [
                call(self.frame.plot_file_name_entry, ""),
                call(self.frame.plot_title_entry, ""),
                call(self.frame.x_axis_entry, ""),
                call(self.frame.label_x_axis_entry, ""),
                call(self.frame.label_y_axes_entry, "separate labels with a comma"),
            ]
        )
        self.assertFalse(self.frame.show_checkbutton_value.get())
        self.assertFalse(self.frame.save_checkbutton_value.get())
        self.assertEqual("type", self.frame.plot_type_entry.get())
        self.assertEqual(1, len(self.frame.plots))

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_add_plot_cb_various_names(
        self, mock_insert_text: MagicMock, mock_update_treeview: MagicMock
    ):
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        # Plot 1
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file")
        self.frame.x_axis_entry.delete(0, tk.END)
        self.frame.x_axis_entry.insert(0, "Column")
        self.frame.plot_title_entry.insert(0, "Title")
        self.frame.label_x_axis_entry.insert(0, "Label")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label 1, Label 2, Label 3")
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.plot_type_entry.insert(0, "type")
        self.frame.add_plot_cb()
        # Plot 2
        self.frame.plot_file_name_entry.delete(0, tk.END)
        self.frame.plot_file_name_entry.insert(0, "test_file_2")
        self.frame.plot_type_entry.delete(0, tk.END)
        self.frame.plot_type_entry.insert(0, "type")
        self.frame.label_y_axes_entry.delete(0, tk.END)
        self.frame.label_y_axes_entry.insert(0, "Label")
        self.frame.add_plot_cb()
        mock_write_text.assert_not_called()
        mock_update_treeview.assert_has_calls([call(), call()])
        mock_insert_text.assert_called()
        self.assertFalse(self.frame.show_checkbutton_value.get())
        self.assertFalse(self.frame.save_checkbutton_value.get())
        self.assertEqual(0, self.frame.plot_type_entry.current())
        self.assertEqual(2, len(self.frame.plots))

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_open_line_min(self, mock_insert_text: MagicMock):
        """Test 'open_line' function with input in min_value_entry"""
        line = {"input": ["Line 1"], "min": 10}
        self.frame.open_line(line)
        calls = [
            call(self.frame.y_axis_entry, "Line 1"),
            call(self.frame.min_value_entry, 10),
            call(self.frame.max_value_entry, ""),
            call(self.frame.label_line_entry, ""),
        ]
        mock_insert_text.assert_has_calls(calls)

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_open_line_max(self, mock_insert_text: MagicMock):
        """Test 'open_line' function with input in max_value_entry"""
        line = {"input": ["Line 1"], "max": 10}
        self.frame.open_line(line)
        calls = [
            call(self.frame.y_axis_entry, "Line 1"),
            call(self.frame.min_value_entry, ""),
            call(self.frame.max_value_entry, 10),
            call(self.frame.label_line_entry, ""),
        ]
        mock_insert_text.assert_has_calls(calls)

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_open_line_labels(self, mock_insert_text: MagicMock):
        """Test 'open_line' function with input in label_line_entry"""
        line = {"input": ["Line 1"], "labels": ["Label 1"]}
        self.frame.open_line(line)
        calls = [
            call(self.frame.y_axis_entry, "Line 1"),
            call(self.frame.min_value_entry, ""),
            call(self.frame.max_value_entry, ""),
            call(self.frame.label_line_entry, "Label 1"),
        ]
        mock_insert_text.assert_has_calls(calls)

    def test_update_treeview_no_plots(self):
        """Test 'update_treeview' function when plots is empty"""
        self.frame.plot_treeview.insert("", tk.END, "Plot 1")
        self.frame.plot_treeview.insert("", tk.END, "Plot 2")
        self.assertEqual(len(self.frame.plot_treeview.get_children()), 2)
        self.frame.update_treeview()
        self.assertEqual(len(self.frame.plot_treeview.get_children()), 0)

    def test_update_treeview_plots_without_lines(self):
        """Test 'update_treeview' function when the plots don't have lines"""
        self.frame.plot_treeview.insert("", tk.END, "Plot 1")
        self.frame.plot_treeview.insert("", tk.END, "Plot 2")
        plot_1 = {"name": "Plot 1", "mapping": {}}
        plot_2 = {"name": "Plot 2", "mapping": {}}
        self.frame.plots.append(plot_1)
        self.frame.plots.append(plot_2)
        self.assertEqual(len(self.frame.plot_treeview.get_children()), 2)
        self.frame.update_treeview()
        children = self.frame.plot_treeview.get_children()
        self.assertEqual(len(children), 2)
        for child in children:
            self.assertEqual(len(self.frame.plot_treeview.item(child)["values"]), 0)

    def test_update_treeview_plots_with_lines(self):
        """Test 'update_treeview' function with plots and valid lines"""
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
        self.assertEqual(len(self.frame.plot_treeview.get_children()), 0)
        self.frame.update_treeview()
        children = self.frame.plot_treeview.get_children()
        self.assertEqual(len(children), 2)
        for child in children:
            lines = self.frame.plot_treeview.get_children(child)
            self.assertEqual(len(lines), 2)
            for line in lines:
                self.assertIn(
                    line, ("Line 1_y1", "Line 2_y2", "Line 3_y1", "Line 4_y2")
                )

    def test_update_treeview_plots_with_invalid_lines(self):
        """Test 'update_treeview' function with plots and invalid lines"""
        self.frame.plot_treeview.insert("", tk.END, "Plot 1")
        self.frame.plot_treeview.insert("", tk.END, "Plot 2")
        plot_1 = {"name": "Plot 1", "mapping": {"invalid_1": {}, "Invalid_2": {}}}
        plot_2 = {"name": "Plot 2", "mapping": {"not_valid_1": {}, "Not_Valid_2": {}}}
        self.frame.plots.append(plot_1)
        self.frame.plots.append(plot_2)
        self.assertEqual(len(self.frame.plot_treeview.get_children()), 2)
        self.frame.update_treeview()
        children = self.frame.plot_treeview.get_children()
        self.assertEqual(len(children), 2)
        for child in children:
            self.assertEqual(len(self.frame.plot_treeview.get_children(child)), 0)

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_open_selected_item_cb_error(self, mock_get_item: MagicMock):
        """Test 'open_selected_item_cb' function when get_selected_item raises an Error"""
        mock_get_item.side_effect = ValueError("Item could not be found\n")
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.open_selected_item_cb()
        mock_write_text.assert_called_once_with("Item could not be found\n")
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_open_selected_item_cb_empty(self, mock_get_item: MagicMock):
        """Test 'open_selected_item_cb' function when get_selected_item returns None"""
        mock_get_item.return_value = None
        self.frame.open_selected_item_cb()
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.open_plot")
    def test_open_selected_item_cb_plot(
        self, mock_open_plot: MagicMock, mock_get_item: MagicMock
    ):
        """Test 'open_selected_item_cb' function when the item is a plot"""
        mock_get_item.return_value = ("Plot", "Plot", 0)
        plot = {}
        self.frame.plots.append(plot)
        self.frame.open_selected_item_cb()
        mock_open_plot.assert_called_once_with(plot)
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.open_line")
    def test_open_selected_item_cb_line(
        self, mock_open_line: MagicMock, mock_get_item: MagicMock
    ):
        """Test 'open_selected_item_cb' function when the item is a line"""
        mock_get_item.return_value = ("Plot", "y1", 0)
        plot = {"mapping": {"y1": {}}}
        self.frame.plots.append(plot)
        self.frame.open_selected_item_cb()
        mock_open_line.assert_called_once_with(plot["mapping"]["y1"])
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_remove_selected_item_cb_error(self, mock_get_item: MagicMock):
        """Test 'remove_selected_item_cb' function when get_selected_item raises an Error"""
        mock_get_item.side_effect = ValueError("Item could not be found\n")
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.remove_selected_item_cb()
        mock_write_text.assert_called_once_with("Item could not be found\n")
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_remove_selected_item_cb_empty(self, mock_get_item: MagicMock):
        """Test 'remove_selected_item_cb' function when get_selected_item returns None"""
        mock_get_item.return_value = None
        self.frame.remove_selected_item_cb()
        mock_get_item.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    def test_remove_selected_item_cb_plot(
        self, mock_update_treeview: MagicMock, mock_get_item: MagicMock
    ):
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
        "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.remove_line_from_plot"
    )
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    def test_remove_selected_item_cb_line(
        self,
        mock_update_treeview: MagicMock,
        mock_remove_line: MagicMock,
        mock_get_item: MagicMock,
    ):
        """Test 'remove_selected_item_cb' function when the item is a line"""
        mock_get_item.return_value = ("Plot", "y1", 0)
        plot = {"mapping": {"y1": {}}}
        self.frame.plots.append(plot)
        self.frame.remove_selected_item_cb()
        mock_remove_line.assert_called_once_with(plot["mapping"], "y1")
        mock_update_treeview.assert_called_once()
        mock_get_item.assert_called_once()

    def test_get_selected_item_empty(self):
        """Test 'get_selected_item' function when no item is selected"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = ""
        mock_write_text = MagicMock()
        self.frame.plot_treeview.focus = mock_treeview_focus
        self.frame.root.write_text = mock_write_text
        result = self.frame.get_selected_item()
        mock_write_text.assert_called_once_with("Select an item from the table\n")
        mock_treeview_focus.assert_called_once()
        self.assertIsNone(result)

    def test_get_selected_item_plot(self):
        """Test 'get_selected_item' function when a plot is selected"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_1"
        mock_write_text = MagicMock()
        self.frame.plot_treeview.focus = mock_treeview_focus
        self.frame.root.write_text = mock_write_text
        self.frame.plots = [{"name": "plot_2"}, {"name": "plot_1"}]
        result = self.frame.get_selected_item()
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual(("plot_1", "plot_1", 1), result)

    def test_get_selected_item_line(self):
        """Test 'get_selected_item' function when a line is selected"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_y1"
        mock_treeview_parent = MagicMock()
        mock_treeview_parent.return_value = "plot_1"
        mock_write_text = MagicMock()
        self.frame.plot_treeview.focus = mock_treeview_focus
        self.frame.plot_treeview.parent = mock_treeview_parent
        self.frame.root.write_text = mock_write_text
        self.frame.plots = [{"name": "plot_2"}, {"name": "plot_3"}, {"name": "plot_1"}]
        result = self.frame.get_selected_item()
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        mock_treeview_parent.assert_called_once_with(mock_treeview_focus.return_value)
        self.assertEqual(("plot_1", "y1", 2), result)

    def test_get_selected_item_error(self):
        """Test 'get_selected_item' function when the selected item not in list"""
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_not_existing"
        mock_write_text = MagicMock()
        self.frame.plot_treeview.focus = mock_treeview_focus
        self.frame.root.write_text = mock_write_text
        self.frame.plots = [{"name": "plot_2"}, {"name": "plot_3"}, {"name": "plot_1"}]
        with self.assertRaises(ValueError) as e:
            self.frame.get_selected_item()
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual("Item couldn't be found\n", str(e.exception))

    def test_generate_plot_config_cb_no_plots(self):
        """test 'generate_plot_config_cb' function when there are no plots"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.assertEqual(0, len(self.frame.plots))
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with(
            "Add Plots to generate a Plot Configuration File\n"
        )

    def test_generate_plot_config_cb_no_lines(self):
        """test 'generate_plot_config_cb' function when there are no lines"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots.append({"mapping": {}})
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with(
            "Every Plot has to contain at least one line\n"
        )

    def test_generate_plot_config_cb_invalid_path(self):
        """test 'generate_plot_config_cb' function when the file path is not valid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.plots.append({"mapping": {"y1": {}}})
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with(
            "Path of the Plot Configuration File has to be given as a valid path\n"
        )

    def test_generate_plot_config_cb(self):
        """test 'generate_plot_config_cb' function"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_tab_plot = MagicMock()
        self.frame.root.tab_plot = mock_tab_plot
        self.frame.plots.append({"mapping": {"y1": {}}})
        plot_file_path = str(Path(__file__).parent / "test.yaml")
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.file_path_entry.insert(tk.END, plot_file_path)
        self.frame.generate_plot_config_cb()
        mock_write_text.assert_called_once_with(
            f"Plot Configuration File has been saved in {plot_file_path}\n"
        )
        mock_tab_plot.plot_config_entry.delete.assert_called_once_with(0, tk.END)
        mock_tab_plot.plot_config_entry.insert.assert_called_once_with(
            tk.END, plot_file_path
        )
        with open(plot_file_path, encoding="utf-8") as f:
            self.assertEqual(f.read(), '[{"mapping": {"y1": {}}}]')
        Path(plot_file_path).unlink()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_empty(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when get_selected_item returns None"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = None
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_not_called()

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_error(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when get_selected_item raises an Error"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.side_effect = ValueError("Item couldn't be found\n")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Item couldn't be found\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_line(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when a line is selected"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_y1", 0)
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("The selected item has to be a Plot\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_full_plot(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when selected plot has too many lines"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append(
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}, "y3": {}}}
        )
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("A Plot can only contain 3 lines\n")

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_no_column(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when no input column is specified"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append({"name": "plot_1", "mapping": {"y1": {}, "y2": {}}})
        self.frame.y_axis_entry.delete(0, tk.END)
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Input columns for y-axis has to be given\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_min_error(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when value for min is not a number"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append({"name": "plot_1", "mapping": {"y1": {}, "y2": {}}})
        self.frame.y_axis_entry.delete(0, tk.END)
        self.frame.y_axis_entry.insert(tk.END, "column")
        self.frame.min_value_entry.delete(0, tk.END)
        self.frame.min_value_entry.insert(tk.END, "not a number")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Minimum y-value has to be given as a number\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    def test_add_line_cb_max_error(self, mock_get_item: MagicMock):
        """test 'add_line_cb' function when value for max is not a number"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        self.frame.plots.append({"name": "plot_1", "mapping": {"y1": {}, "y2": {}}})
        self.frame.y_axis_entry.delete(0, tk.END)
        self.frame.y_axis_entry.insert(tk.END, "column")
        self.frame.max_value_entry.delete(0, tk.END)
        self.frame.max_value_entry.insert(tk.END, "not a number")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Maximum y-value has to be given as a number\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    def test_add_line_cb_update_treeview_error(
        self, mock_update_treeview: MagicMock, mock_get_item: MagicMock
    ):
        """test 'add_line_cb' function when the treeview can't be updated"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_update_treeview.side_effect = tk.TclError("Could not update treeview\n")
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        self.frame.plots.append(plot)
        self.frame.y_axis_entry.delete(0, tk.END)
        self.frame.y_axis_entry.insert(tk.END, "column")
        self.frame.add_line_cb()
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        mock_write_text.assert_called_once_with("Could not update treeview\n")
        self.assertEqual(self.frame.plots[0], plot)

    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.get_selected_item")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.update_treeview")
    @patch("cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text")
    def test_add_line_cb(
        self,
        mock_insert_text: MagicMock,
        mock_update_treeview: MagicMock,
        mock_get_item: MagicMock,
    ):
        """test 'add_line_cb' function"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        self.frame.plots.append(plot)
        self.frame.y_axis_entry.delete(0, tk.END)
        self.frame.y_axis_entry.insert(tk.END, "column")
        self.frame.max_value_entry.delete(0, tk.END)
        self.frame.max_value_entry.insert(tk.END, "0")
        self.frame.min_value_entry.delete(0, tk.END)
        self.frame.min_value_entry.insert(tk.END, "0")
        self.frame.label_line_entry.delete(0, tk.END)
        self.frame.label_line_entry.insert(tk.END, "Label")
        self.frame.add_line_cb()
        plot["mapping"]["y3"] = {
            "input": ["column"],
            "labels": ["Label"],
            "max": 0,
            "min": 0,
        }
        calls = [
            call(self.frame.y_axis_entry, ""),
            call(self.frame.label_line_entry, "optional"),
            call(self.frame.min_value_entry, "optional"),
            call(self.frame.max_value_entry, "optional"),
        ]
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        self.assertEqual(self.frame.plots[0], plot)
        mock_write_text.assert_not_called()
        mock_insert_text.assert_has_calls(calls)

    def test_change_font_cb(self) -> None:
        """Test 'change_font_cb' function"""
        mock_event = MagicMock()
        mock_event.widget = self.frame.label_y_axes_entry
        self.assertEqual(
            font.nametofont(str(mock_event.widget.cget("font"))).actual("slant"),
            "italic",
        )
        self.frame.change_font_cb(mock_event)
        self.assertEqual(
            font.nametofont(str(mock_event.widget.cget("font"))).actual("slant"),
            "roman",
        )


# pylint: disable=too-many-public-methods
class TestPlotConfigFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the PlotConfigFrame class"""

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_open_file_cb(self, mock_filename: MagicMock):
        """Test 'open_file_cb' function"""
        mock_plot_config_file = MagicMock()
        mock_filename.return_value = "Plot Configuration File"
        PlotConfigFrame.open_file_cb(mock_plot_config_file)
        mock_plot_config_file.file_path_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_config_file.file_path_entry.insert.assert_called_once_with(
            tk.END, "Plot Configuration File"
        )

    def test_add_plot_cb_name_invalid(self):
        """Test 'add_plot_cb' function if the file name is invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = ""
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "Name of the Plot-File has to be given as a valid path\n"
        )

    def test_add_plot_cb_no_x_input(self):
        """Test 'add_plot_cb' function if input for x-axis is invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = ""
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "Input column for x-axis has to be given\n"
        )

    def test_add_plot_cb_title_invalid(self):
        """Test 'add_plot_cb' function if title is invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = ""
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = "Label"
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with("Title of the plot has to be given\n")

    def test_add_plot_cb_label_x_invalid(self):
        """Test 'add_plot_cb' function if label for x-axis is invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = ""
        mock_plot_config_file.label_y_axes_entry.get.return_value = "Label"
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "Label for the x-axis has to be given\n"
        )

    def test_add_plot_cb_label_y_invalid(self):
        """Test 'add_plot_cb' function if labels for y-axes are invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = ""
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "Labels for the y-axes have to be given\n"
        )

    def test_add_plot_cb_type_invalid(self):
        """Test 'add_plot_cb' function if type is invalid"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = "Label"
        mock_plot_config_file.plot_type_entry.get.return_value = ""
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with("Plot Type has to be given\n")

    def test_add_plot_cb_lines(self):
        """Test 'add_plot_cb' function if there are more than 3 labels for the y-axes"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = (
            "Label 1, Label 2, Label 3, Label 4"
        )
        mock_plot_config_file.plot_type_entry.get.return_value = "type"
        PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "One Plot can not contain more than 3 lines\n"
        )

    def test_add_plot_cb(self):
        """Test 'add_plot_cb' function"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_insert_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = (
            "Label 1, Label 2, Label 3"
        )
        mock_plot_config_file.plot_type_entry.get.return_value = "type"
        mock_plot_config_file.show_checkbutton_value.get.return_value = False
        mock_plot_config_file.save_checkbutton_value.get.return_value = False
        mock_plot_config_file.plots = []
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        plot = {
            "name": "test_file",
            "type": "type",
            "mapping": {"x": "Column"},
            "description": {
                "title": "Title",
                "x_axis": "Label",
                "y_axes": ["Label 1", "Label 2", "Label 3"],
            },
            "graph": {"show": False, "save": False},
        }
        mock_write_text.assert_not_called()
        mock_plot_config_file.update_treeview.assert_called_once()
        mock_insert_text.assert_has_calls(
            [
                call(mock_plot_config_file.plot_file_name_entry, ""),
                call(mock_plot_config_file.plot_title_entry, ""),
                call(mock_plot_config_file.x_axis_entry, ""),
                call(mock_plot_config_file.label_x_axis_entry, ""),
                call(
                    mock_plot_config_file.label_y_axes_entry,
                    "separate labels with a comma",
                ),
            ]
        )
        mock_plot_config_file.show_checkbutton_value.set.assert_called_once_with(False)
        mock_plot_config_file.save_checkbutton_value.set.assert_called_once_with(False)
        mock_plot_config_file.plot_type_entry.current.assert_called_once_with(0)
        self.assertEqual(1, len(mock_plot_config_file.plots))
        self.assertEqual(plot, mock_plot_config_file.plots[0])

    def test_add_plot_cb_identical_names(self):
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_insert_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.return_value = "test_file"
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = (
            "Label 1, Label 2, Label 3"
        )
        mock_plot_config_file.plot_type_entry.get.return_value = "type"
        mock_plot_config_file.show_checkbutton_value.get.return_value = False
        mock_plot_config_file.save_checkbutton_value.get.return_value = False
        mock_plot_config_file.plots = []
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.add_plot_cb(mock_plot_config_file)
            PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_called_once_with(
            "Name of the Plot-File has to be unique for each Plot\n"
        )
        plot = {
            "name": "test_file",
            "type": "type",
            "mapping": {"x": "Column"},
            "description": {
                "title": "Title",
                "x_axis": "Label",
                "y_axes": ["Label 1", "Label 2", "Label 3"],
            },
            "graph": {"show": False, "save": False},
        }
        mock_plot_config_file.update_treeview.assert_called_once()
        mock_insert_text.assert_has_calls(
            [
                call(mock_plot_config_file.plot_file_name_entry, ""),
                call(mock_plot_config_file.plot_title_entry, ""),
                call(mock_plot_config_file.x_axis_entry, ""),
                call(mock_plot_config_file.label_x_axis_entry, ""),
                call(
                    mock_plot_config_file.label_y_axes_entry,
                    "separate labels with a comma",
                ),
            ]
        )
        mock_plot_config_file.show_checkbutton_value.set.assert_called_once_with(False)
        mock_plot_config_file.save_checkbutton_value.set.assert_called_once_with(False)
        mock_plot_config_file.plot_type_entry.current.assert_called_once_with(0)
        self.assertEqual(1, len(mock_plot_config_file.plots))
        self.assertEqual(plot, mock_plot_config_file.plots[0])

    def test_add_plot_cb_various_names(self):
        """Test 'add_plot_cb' function if several plots have the same name"""
        mock_plot_config_file = MagicMock()
        mock_write_text = MagicMock()
        mock_insert_text = MagicMock()
        mock_plot_config_file.root.write_text = mock_write_text
        mock_plot_config_file.plot_file_name_entry.get.side_effect = [
            "test_file",
            "test_file_2",
        ]
        mock_plot_config_file.x_axis_entry.get.return_value = "Column"
        mock_plot_config_file.plot_title_entry.get.return_value = "Title"
        mock_plot_config_file.label_x_axis_entry.get.return_value = "Label"
        mock_plot_config_file.label_y_axes_entry.get.return_value = (
            "Label 1, Label 2, Label 3"
        )
        mock_plot_config_file.plot_type_entry.get.return_value = "type"
        mock_plot_config_file.show_checkbutton_value.get.return_value = False
        mock_plot_config_file.save_checkbutton_value.get.return_value = False
        mock_plot_config_file.plots = []
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.add_plot_cb(mock_plot_config_file)
            PlotConfigFrame.add_plot_cb(mock_plot_config_file)
        mock_write_text.assert_not_called()
        mock_plot_config_file.update_treeview.assert_has_calls([call(), call()])
        mock_insert_text.assert_called()
        mock_plot_config_file.show_checkbutton_value.set.assert_has_calls(
            [call(False), call(False)]
        )
        mock_plot_config_file.save_checkbutton_value.set.assert_has_calls(
            [call(False), call(False)]
        )
        mock_plot_config_file.plot_type_entry.current.assert_has_calls(
            [call(0), call(0)]
        )
        self.assertEqual(2, len(mock_plot_config_file.plots))

    def test_open_line_min(self):
        """Test 'open_line' function with input in min_value_entry"""
        mock_plot_config_frame = MagicMock()
        mock_insert_text = MagicMock()
        line = {"input": ["Line 1"], "min": 10}
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.open_line(mock_plot_config_frame, line)
        calls = [
            call(mock_plot_config_frame.y_axis_entry, "Line 1"),
            call(mock_plot_config_frame.min_value_entry, 10),
            call(mock_plot_config_frame.max_value_entry, ""),
            call(mock_plot_config_frame.label_line_entry, ""),
        ]
        mock_insert_text.assert_has_calls(calls)

    def test_open_line_max(self):
        """Test 'open_line' function with input in max_value_entry"""
        mock_plot_config_frame = MagicMock()
        mock_insert_text = MagicMock()
        line = {"input": ["Line 1"], "max": 10}
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.open_line(mock_plot_config_frame, line)
        calls = [
            call(mock_plot_config_frame.y_axis_entry, "Line 1"),
            call(mock_plot_config_frame.min_value_entry, ""),
            call(mock_plot_config_frame.max_value_entry, 10),
            call(mock_plot_config_frame.label_line_entry, ""),
        ]
        mock_insert_text.assert_has_calls(calls)

    def test_open_line_labels(self):
        """Test 'open_line' function with input in label_line_entry"""
        mock_plot_config_frame = MagicMock()
        mock_insert_text = MagicMock()
        line = {"input": ["Line 1"], "labels": ["Label 1"]}
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.open_line(mock_plot_config_frame, line)
        calls = [
            call(mock_plot_config_frame.y_axis_entry, "Line 1"),
            call(mock_plot_config_frame.min_value_entry, ""),
            call(mock_plot_config_frame.max_value_entry, ""),
            call(mock_plot_config_frame.label_line_entry, "Label 1"),
        ]
        mock_insert_text.assert_has_calls(calls)

    def test_update_treeview_no_plots(self):
        """Test 'update_treeview' function when plots is empty"""
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plot_treeview = MagicMock()
        mock_plot_config_frame.plot_treeview.get_children.return_value = [
            "Child 1",
            "Child 2",
        ]
        mock_plot_config_frame.plots = []
        PlotConfigFrame.update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plot_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plot_treeview.delete.assert_has_calls(
            [call("Child 1"), call("Child 2")]
        )

    def test_update_treeview_plots_without_lines(self):
        """Test 'update_treeview' function when the plots don't have lines"""
        plot_1 = {"name": "Plot 1", "mapping": {}}
        plot_2 = {"name": "Plot 2", "mapping": {}}
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plot_treeview = MagicMock()
        mock_plot_config_frame.plot_treeview.get_children.return_value = []
        mock_plot_config_frame.plots = [plot_1, plot_2]
        PlotConfigFrame.update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plot_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plot_treeview.delete.assert_not_called()
        mock_plot_config_frame.plot_treeview.insert.assert_has_calls(
            [
                call("", tk.END, "Plot 1", text="Plot 1"),
                call("", tk.END, "Plot 2", text="Plot 2"),
            ]
        )

    def test_update_treeview_plots_with_lines(self):
        """Test 'update_treeview' function with plots and valid lines"""
        plot_1 = {
            "name": "Plot 1",
            "mapping": {"y1": {"input": ["Line 1"]}, "y2": {"input": ["Line 2"]}},
        }
        plot_2 = {
            "name": "Plot 2",
            "mapping": {"y1": {"input": ["Line 3"]}, "y2": {"input": ["Line 4"]}},
        }
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plot_treeview = MagicMock()
        mock_plot_config_frame.plot_treeview.get_children.return_value = []
        mock_plot_config_frame.plots = [plot_1, plot_2]
        PlotConfigFrame.update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plot_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plot_treeview.delete.assert_not_called()
        mock_plot_config_frame.plot_treeview.insert.assert_has_calls(
            [
                call("", tk.END, "Plot 1", text="Plot 1"),
                call("Plot 1", tk.END, "Line 1_y1", text="Line 1"),
                call("Plot 1", tk.END, "Line 2_y2", text="Line 2"),
                call("", tk.END, "Plot 2", text="Plot 2"),
                call("Plot 2", tk.END, "Line 3_y1", text="Line 3"),
                call("Plot 2", tk.END, "Line 4_y2", text="Line 4"),
            ]
        )

    def test_update_treeview_plots_with_invalid_lines(self):
        """Test 'update_treeview' function with plots and invalid lines"""
        plot_1 = {"name": "Plot 1", "mapping": {"invalid_1": {}, "Invalid_2": {}}}
        plot_2 = {"name": "Plot 2", "mapping": {"not_valid_1": {}, "Not_Valid_2": {}}}
        mock_plot_config_frame = MagicMock()
        mock_plot_config_frame.plot_treeview = MagicMock()
        mock_plot_config_frame.plot_treeview.get_children.return_value = []
        mock_plot_config_frame.plots = [plot_1, plot_2]
        PlotConfigFrame.update_treeview(mock_plot_config_frame)
        mock_plot_config_frame.plot_treeview.get_children.assert_called_once()
        mock_plot_config_frame.plot_treeview.delete.assert_not_called()
        mock_plot_config_frame.plot_treeview.insert.assert_has_calls(
            [
                call("", tk.END, "Plot 1", text="Plot 1"),
                call("", tk.END, "Plot 2", text="Plot 2"),
            ]
        )

    def test_open_selected_item_cb_error(self):
        """Test 'open_selected_item_cb' function when get_selected_item raises an Error"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.side_effect = ValueError("Item could not be found\n")
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.open_selected_item_cb(mock_plot_config_frame)
        mock_plot_config_frame.root.write_text.assert_called_once_with(
            "Item could not be found\n"
        )
        mock_get_item.assert_called_once()

    def test_open_selected_item_cb_empty(self):
        """Test 'open_selected_item_cb' function when get_selected_item returns None"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = None
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.open_selected_item_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()

    def test_open_selected_item_cb_plot(self):
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

    def test_open_selected_item_cb_line(self):
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

    def test_remove_selected_item_cb_error(self):
        """Test 'remove_selected_item_cb' function when get_selected_item raises an Error"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.side_effect = ValueError("Item could not be found\n")
        mock_write_text = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.root.write_text = mock_write_text
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with("Item could not be found\n")
        mock_get_item.assert_called_once()

    def test_remove_selected_item_cb_empty(self):
        """Test 'remove_selected_item_cb' function when get_selected_item returns None"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_get_item.return_value = None
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()

    def test_remove_selected_item_cb_plot(self):
        """Test 'remove_selected_item_cb' function when the item is a plot"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_update_treeview = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.update_treeview = mock_update_treeview
        mock_plot_config_frame.plots = [{}]
        mock_get_item.return_value = ("Plot", "Plot", 0)
        PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        self.assertEqual(0, len(mock_plot_config_frame.plots))
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()

    def test_remove_selected_item_cb_line(self):
        """Test 'remove_selected_item_cb' function when the item is a line"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_update_treeview = MagicMock()
        mock_remove_line = MagicMock()
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.update_treeview = mock_update_treeview
        plot = {"mapping": {"y1": {}}}
        mock_plot_config_frame.plots = [plot]
        mock_get_item.return_value = ("Plot", "y1", 0)
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.remove_line_from_plot",
            mock_remove_line,
        ):
            PlotConfigFrame.remove_selected_item_cb(mock_plot_config_frame)
        mock_remove_line.assert_called_once_with(plot["mapping"], "y1")
        mock_update_treeview.assert_called_once()
        mock_get_item.assert_called_once()

    def test_get_selected_item_empty(self):
        """Test 'get_selected_item' function when no item is selected"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = ""
        mock_write_text = MagicMock()
        mock_plot_config_frame.plot_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.root.write_text = mock_write_text
        result = PlotConfigFrame.get_selected_item(mock_plot_config_frame)
        mock_write_text.assert_called_once_with("Select an item from the table\n")
        mock_treeview_focus.assert_called_once()
        self.assertIsNone(result)

    def test_get_selected_item_plot(self):
        """Test 'get_selected_item' function when a plot is selected"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_1"
        mock_write_text = MagicMock()
        mock_plot_config_frame.plot_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [{"name": "plot_2"}, {"name": "plot_1"}]
        result = PlotConfigFrame.get_selected_item(mock_plot_config_frame)
        mock_write_text.assert_not_called()
        mock_treeview_focus.assert_called_once()
        self.assertEqual(("plot_1", "plot_1", 1), result)

    def test_get_selected_item_line(self):
        """Test 'get_selected_item' function when a line is selected"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_y1"
        mock_treeview_parent = MagicMock()
        mock_treeview_parent.return_value = "plot_1"
        mock_write_text = MagicMock()
        mock_plot_config_frame.plot_treeview.focus = mock_treeview_focus
        mock_plot_config_frame.plot_treeview.parent = mock_treeview_parent
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

    def test_get_selected_item_error(self):
        """Test 'get_selected_item' function when the selected item not in list"""
        mock_plot_config_frame = MagicMock()
        mock_treeview_focus = MagicMock()
        mock_treeview_focus.return_value = "plot_not_existing"
        mock_write_text = MagicMock()
        mock_plot_config_frame.plot_treeview.focus = mock_treeview_focus
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
        self.assertEqual("Item couldn't be found\n", str(e.exception))

    def test_generate_plot_config_cb_no_plots(self):
        """test 'generate_plot_config_cb' function when there are no plots"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = []
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            "Add Plots to generate a Plot Configuration File\n"
        )

    def test_generate_plot_config_cb_no_lines(self):
        """test 'generate_plot_config_cb' function when there are no lines"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [{"mapping": {}}]
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            "Every Plot has to contain at least one line\n"
        )

    def test_generate_plot_config_cb_invalid_path(self):
        """test 'generate_plot_config_cb' function when the file path is not valid"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_file_path_entry = MagicMock()
        mock_file_path_entry.get.return_value = ""
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.plots = [{"mapping": {"y1": {}}}]
        mock_plot_config_frame.file_path_entry = mock_file_path_entry
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            "Path of the Plot Configuration File has to be given as a valid path\n"
        )

    def test_generate_plot_config_cb(self):
        """test 'generate_plot_config_cb' function"""
        mock_plot_config_frame = MagicMock()
        mock_write_text = MagicMock()
        mock_file_path_entry = MagicMock()
        plot_file_path = str(Path(__file__).parent / "test.yaml")
        mock_file_path_entry.get.return_value = plot_file_path
        mock_tab_plot = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.file_path_entry = mock_file_path_entry
        mock_plot_config_frame.root.tab_plot = mock_tab_plot
        mock_plot_config_frame.plots = [{"mapping": {"y1": {}}}]
        PlotConfigFrame.generate_plot_config_cb(mock_plot_config_frame)
        mock_write_text.assert_called_once_with(
            f"Plot Configuration File has been saved in {plot_file_path}\n"
        )
        mock_tab_plot.plot_config_entry.delete.assert_called_once_with(0, tk.END)
        mock_tab_plot.plot_config_entry.insert.assert_called_once_with(
            tk.END, plot_file_path
        )
        with open(plot_file_path, encoding="utf-8") as f:
            self.assertEqual(f.read(), '[{"mapping": {"y1": {}}}]')
        Path(plot_file_path).unlink()

    def test_add_line_cb_empty(self):
        """test 'add_line_cb' function when get_selected_item returns None"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = None
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_not_called()

    def test_add_line_cb_error(self):
        """test 'add_line_cb' function when get_selected_item raises an Error"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.side_effect = ValueError("Item couldn't be found\n")
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("Item couldn't be found\n")

    def test_add_line_cb_line(self):
        """test 'add_line_cb' function when a line is selected"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_y1", 0)
        mock_write_text = MagicMock()
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with("The selected item has to be a Plot\n")

    def test_add_line_cb_full_plot(self):
        """test 'add_line_cb' function when selected plot has too many lines"""
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
        mock_write_text.assert_called_once_with("A Plot can only contain 3 lines\n")

    def test_add_line_cb_no_column(self):
        """test 'add_line_cb' function when no input column is specified"""
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
        mock_plot_config_frame.y_axis_entry = mock_y_axis_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Input columns for y-axis has to be given\n"
        )

    def test_add_line_cb_min_error(self):
        """test 'add_line_cb' function when value for min is not a number"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_min_value_entry = MagicMock()
        mock_min_value_entry.get.return_value = "not a number"
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.plots = [
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        ]
        mock_plot_config_frame.y_axis_entry = mock_y_axis_entry
        mock_plot_config_frame.min_value_entry = mock_min_value_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Minimum y-value has to be given as a number\n"
        )

    def test_add_line_cb_max_error(self):
        """test 'add_line_cb' function when value for max is not a number"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_max_value_entry = MagicMock()
        mock_max_value_entry.get.return_value = "not a number"
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.plots = [
            {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        ]
        mock_plot_config_frame.y_axis_entry = mock_y_axis_entry
        mock_plot_config_frame.max_value_entry = mock_max_value_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Maximum y-value has to be given as a number\n"
        )

    def test_add_line_cb_update_treeview_error(self):
        """test 'add_line_cb' function when the treeview can't be updated"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_update_treeview = MagicMock()
        mock_update_treeview.side_effect = tk.TclError("Could not update treeview\n")
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.update_treeview = mock_update_treeview
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        mock_plot_config_frame.plots = [plot]
        mock_plot_config_frame.y_axis_entry = mock_y_axis_entry
        PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        mock_write_text.assert_called_once_with("Could not update treeview\n")
        self.assertEqual(mock_plot_config_frame.plots[0], plot)

    def test_add_line_cb(self):
        """test 'add_line_cb' function"""
        mock_plot_config_frame = MagicMock()
        mock_get_item = MagicMock()
        mock_get_item.return_value = ("plot_1", "plot_1", 0)
        mock_write_text = MagicMock()
        mock_update_treeview = MagicMock()
        mock_insert_text = MagicMock()
        mock_y_axis_entry = MagicMock()
        mock_y_axis_entry.get.return_value = "column"
        mock_max_value_entry = MagicMock()
        mock_max_value_entry.get.return_value = "0"
        mock_min_value_entry = MagicMock()
        mock_min_value_entry.get.return_value = "0"
        mock_label_line_entry = MagicMock()
        mock_label_line_entry.get.return_value = "Label"
        mock_plot_config_frame.root.write_text = mock_write_text
        mock_plot_config_frame.get_selected_item = mock_get_item
        mock_plot_config_frame.update_treeview = mock_update_treeview
        plot = {"name": "plot_1", "mapping": {"y1": {}, "y2": {}}}
        mock_plot_config_frame.plots = [plot]
        mock_plot_config_frame.y_axis_entry = mock_y_axis_entry
        mock_plot_config_frame.max_value_entry = mock_max_value_entry
        mock_plot_config_frame.min_value_entry = mock_min_value_entry
        mock_plot_config_frame.label_line_entry = mock_label_line_entry
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.add_line_cb(mock_plot_config_frame)
        plot["mapping"]["y3"] = {
            "input": ["column"],
            "labels": ["Label"],
            "max": 0,
            "min": 0,
        }
        calls = [
            call(mock_plot_config_frame.y_axis_entry, ""),
            call(mock_plot_config_frame.label_line_entry, "optional"),
            call(mock_plot_config_frame.min_value_entry, "optional"),
            call(mock_plot_config_frame.max_value_entry, "optional"),
        ]
        mock_get_item.assert_called_once()
        mock_update_treeview.assert_called_once()
        self.assertEqual(mock_plot_config_frame.plots[0], plot)
        mock_write_text.assert_not_called()
        mock_insert_text.assert_has_calls(calls)

    def test_change_font_cb(self) -> None:
        """Test 'change_font_cb' function"""
        mock_plot_config_frame = MagicMock()
        mock_event = MagicMock()
        mock_event.widget = MagicMock()
        PlotConfigFrame.change_font_cb(mock_plot_config_frame, mock_event)
        mock_event.widget.configure.assert_called_once_with(
            font=mock_plot_config_frame.font_default
        )


class TestOpenPlot(unittest.TestCase):
    """Test of the 'open_plot' function of the PlotConfigFrame class"""

    def test_open_plot(self):
        """Test the function"""
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
        mock_insert_text = MagicMock()
        mock_plot_config_frame.show_checkbutton_value.set = MagicMock()
        mock_plot_config_frame.save_checkbutton_value.set = MagicMock()
        with patch(
            "cli.cmd_gui.frame_plot.frame_plot_config.PlotConfigFrame.insert_text",
            mock_insert_text,
        ):
            PlotConfigFrame.open_plot(mock_plot_config_frame, plot)
        mock_plot_config_frame.show_checkbutton_value.set.assert_called_once_with(
            plot["graph"]["show"]
        )
        mock_plot_config_frame.save_checkbutton_value.set.assert_called_once_with(
            plot["graph"]["save"]
        )
        calls = [
            call(mock_plot_config_frame.plot_file_name_entry, plot["name"]),
            call(mock_plot_config_frame.plot_type_entry, plot["type"]),
            call(mock_plot_config_frame.x_axis_entry, plot["mapping"]["x"]),
            call(mock_plot_config_frame.plot_title_entry, plot["description"]["title"]),
            call(
                mock_plot_config_frame.label_x_axis_entry, plot["description"]["x_axis"]
            ),
            call(
                mock_plot_config_frame.label_y_axes_entry,
                "Y-Axis 1, Y-Axis 2",
            ),
        ]
        mock_insert_text.assert_has_calls(calls)


class TestRemoveLine(unittest.TestCase):
    """Test of the 'remove_line_from_plot' function of the PlotConfigFrame class"""

    def test_y3(self):
        """Tests removing line y3"""
        mapping = {"y1": {}, "y2": {}, "y3": {}}
        PlotConfigFrame.remove_line_from_plot(mapping, "y3")
        self.assertNotIn("y3", mapping)

    def test_y2_with_y3(self):
        """Tests removing line y2 when line y3 exists"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
            "y3": {"former_key": "y3"},
        }
        PlotConfigFrame.remove_line_from_plot(mapping, "y2")
        self.assertNotIn("y3", mapping)
        self.assertIn("y2", mapping)
        self.assertEqual(mapping["y2"]["former_key"], "y3")

    def test_y2_without_y3(self):
        """Tests removing line y2 without line y3"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
        }
        PlotConfigFrame.remove_line_from_plot(mapping, "y2")
        self.assertNotIn("y3", mapping)
        self.assertNotIn("y2", mapping)

    def test_y1_with_y3(self):
        """Tests removing line y1 when line y3 exists"""
        mapping = {"y1": {"former_key": "y1"}, "y3": {"former_key": "y3"}}
        PlotConfigFrame.remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y3", mapping)
        self.assertIn("y1", mapping)
        self.assertEqual(mapping["y1"]["former_key"], "y3")

    def test_y1_with_y2(self):
        """Tests removing line y1 when line y2 exists"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
        }
        PlotConfigFrame.remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y2", mapping)
        self.assertIn("y1", mapping)
        self.assertEqual(mapping["y1"]["former_key"], "y2")

    def test_y1_with_y2_y3(self):
        """Tests removing line y1 when lines y2 and y3 exist"""
        mapping = {
            "y1": {"former_key": "y1"},
            "y2": {"former_key": "y2"},
            "y3": {"former_key": "y3"},
        }
        PlotConfigFrame.remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y3", mapping)
        self.assertIn("y2", mapping)
        self.assertIn("y1", mapping)
        self.assertEqual(mapping["y1"]["former_key"], "y3")

    def test_y1_without_y2_y3(self):
        """Tests removing line y1 without lines y2 and y3"""
        mapping = {
            "y1": {"former_key": "y1"},
        }
        PlotConfigFrame.remove_line_from_plot(mapping, "y1")
        self.assertNotIn("y3", mapping)
        self.assertNotIn("y2", mapping)
        self.assertNotIn("y1", mapping)


class TestInsertText(unittest.TestCase):
    """Test of the 'insert_text' function of the PlotConfigFrame class"""

    def test_insert_text(self):
        """Test 'insert_text' function"""
        entry_widget = MagicMock()
        PlotConfigFrame.insert_text(entry_widget, "New Entry")
        entry_widget.delete.assert_called_once_with(0, tk.END)
        entry_widget.insert(tk.END, "New Entry")


if __name__ == "__main__":
    unittest.main()
