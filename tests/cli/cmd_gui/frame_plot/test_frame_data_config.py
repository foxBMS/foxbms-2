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

"""Testing file 'cli/cmd_gui/frame_plot/frame_data_config.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_plot.frame_data_config import Column, DataConfigFrame
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot.frame_data_config import Column, DataConfigFrame

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestDataConfigFrame(unittest.TestCase):
    """Test of the DataConfigFrame class"""

    def setUp(self) -> None:  # noqa: D102
        parent = tk.Tk()
        parent.withdraw()
        self.frame = DataConfigFrame(parent, parent)

    def tearDown(self) -> None:  # noqa: D102
        self.frame.root.update()
        self.frame.root.destroy()

    def test_add_column_invalid_name(self) -> None:
        """Test 'add_column_cb' function if column_name is not valid"""
        self.frame.root.write_text = MagicMock()
        self.frame.columns_header_entry.delete(0, tk.END)
        self.frame.add_column_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Column header is missing.\n"
        )

    def test_add_column_missing_type(self) -> None:
        """Test 'add_column_cb' function if column_type is not given"""
        self.frame.root.write_text = MagicMock()
        self.frame.columns_header_entry.delete(0, tk.END)
        self.frame.columns_header_entry.insert(0, "Header")
        self.frame.columns_type_entry.delete(0, tk.END)
        self.frame.add_column_cb()
        self.frame.root.write_text.assert_called_once_with("Column type is missing.\n")

    def test_add_column_invalid_type(self) -> None:
        """Test 'add_column_cb' function if column_type is not given"""
        self.frame.root.write_text = MagicMock()
        self.frame.columns_header_entry.delete(0, tk.END)
        self.frame.columns_header_entry.insert(0, "Header")
        self.frame.columns_type_entry.delete(0, tk.END)
        self.frame.columns_type_entry.insert(tk.END, "Type")
        self.frame.add_column_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Column type is not valid.\n"
        )

    def test_add_column(self) -> None:
        """Test 'add_column_cb' function with valid input"""
        self.frame.root.write_text = MagicMock()
        self.frame.columns_header_entry.delete(0, tk.END)
        self.frame.columns_header_entry.insert(0, "Header")
        self.frame.add_column_cb()
        self.frame.root.write_text.assert_not_called()
        self.assertEqual(1, len(self.frame.columns_treeview.get_children()))
        self.assertEqual(1, len(self.frame.columns))
        self.assertEqual("Header", self.frame.columns[0].column_name)
        self.assertEqual("string", self.frame.columns[0].column_type)

    def test_remove_column_no_selection(self) -> None:
        """Test 'remove_column_cb' function with no column selected"""
        self.frame.root.write_text = MagicMock()
        self.frame.remove_column_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Please select a Column from the list.\n"
        )

    def test_remove_column(self) -> None:
        """Test 'remove_column_cb' function with an item selected"""
        self.frame.root.write_text = MagicMock()
        item = self.frame.columns_treeview.insert(
            "", tk.END, values=("Header", "string")
        )
        self.frame.columns_treeview.focus(item)
        self.frame.columns.append(Column("Header", "string"))
        self.frame.remove_column_cb()
        self.assertEqual(0, len(self.frame.columns))
        self.assertEqual(0, len(self.frame.columns_treeview.get_children()))
        self.frame.root.write_text.assert_not_called()

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function"""
        mock_filename.return_value = "Data Configuration File"
        self.frame.select_file_cb()
        self.assertEqual(self.frame.file_path_entry.get(), "Data Configuration File")

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file_no_selection(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function when no file is selected"""
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.file_path_entry.insert(tk.END, "File Path")
        mock_filename.return_value = ""
        self.frame.select_file_cb()
        self.assertEqual(self.frame.file_path_entry.get(), "File Path")

    def test_generate_data_config_invalid_path(self) -> None:
        """Test 'generate_data_config_cb' function with invalid file path"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Path of the Data Configuration File has to be given as a valid path.\n"
        )

    def test_generate_data_config_no_columns(self) -> None:
        """Test 'generate_data_config_cb' function when no columns are given"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.insert(0, "FilePath")
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with("Please add Columns.\n")

    def test_generate_data_config_invalid_skip(self) -> None:
        """Test 'generate_data_config_cb' function when input for 'skip' is invalid"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.insert(0, "FilePath")
        self.frame.skip_entry.insert(0, "number")
        self.frame.columns.append(Column("Header", "string"))
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Number of Lines to skip and Precision of Data have to be given as integers.\n"
        )

    def test_generate_data_config_invalid_precision(self) -> None:
        """Test 'generate_data_config_cb' function when input for 'precision' is invalid"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.insert(0, "FilePath")
        self.frame.precision_entry.insert(0, "number")
        self.frame.columns.append(Column("Header", "string"))
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Number of Lines to skip and Precision of Data have to be given as integers.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_data_config.Path.mkdir")
    def test_generate_data_config(self, mock_dir: MagicMock) -> None:
        """Test 'generate_data_config_cb' function with valid input"""
        self.frame.root.write_text = MagicMock()
        self.frame.root.run_plot_tab = MagicMock()
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.file_path_entry.insert(0, "data_config.txt")
        self.frame.columns.append(Column("Header", "string"))
        self.frame.skip_entry.delete(0, tk.END)
        self.frame.precision_entry.delete(0, tk.END)
        self.frame.skip_entry.insert(0, "0")
        self.frame.precision_entry.insert(0, "0")
        with patch("builtins.open"):
            self.frame.generate_data_config_cb()
        mock_dir.assert_called_once_with(parents=True, exist_ok=True)
        self.frame.root.write_text.assert_called_once_with(
            f"Data Configuration File has been saved in '{'data_config.txt'}'.\n"
        )
        self.frame.root.run_plot_tab.data_config_entry.delete.assert_called_once_with(
            0, tk.END
        )
        self.frame.root.run_plot_tab.data_config_entry.insert.assert_called_once_with(
            tk.END, "data_config.txt"
        )


class TestDataConfigFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the DataConfigFrame class"""

    def test_add_column_invalid_name(self) -> None:
        """Test 'add_column_cb' function if column_name is invalid"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.columns_header_entry.get.return_value = "name"
        mock_data_config_frame.root.write_text = MagicMock()
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_data_config_frame.root.write_text.assert_called_once_with(
            "Column header is missing.\n"
        )

    def test_add_column_missing_type(self) -> None:
        """Test 'add_column_cb' function if column_type is missing"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.columns_header_entry.get.return_value = "Header"
        mock_data_config_frame.columns_type_entry.get.return_value = ""
        mock_data_config_frame.root.write_text = MagicMock()
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_data_config_frame.root.write_text.assert_called_once_with(
            "Column type is missing.\n"
        )

    def test_add_column_invalid_type(self) -> None:
        """Test 'add_column_cb' function if column_type is invalid"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.valid_column_types = ["valid", "correct"]
        mock_data_config_frame.columns_header_entry.get.return_value = "Header"
        mock_data_config_frame.columns_type_entry.get.return_value = "type"
        mock_data_config_frame.root.write_text = MagicMock()
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_data_config_frame.root.write_text.assert_called_once_with(
            "Column type is not valid.\n"
        )

    def test_add_column(self) -> None:
        """Test 'add_column_cb' function with valid input"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.valid_column_types = ["valid", "correct"]
        mock_data_config_frame.columns_header_entry.get.return_value = "Header"
        mock_data_config_frame.columns_type_entry.get.return_value = "valid"
        mock_data_config_frame.root.write_text = MagicMock()
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_data_config_frame.root.write_text.assert_not_called()
        mock_data_config_frame.columns.append.assert_called_once_with(  # pylint: disable=no-member
            Column("Header", "valid")
        )
        mock_data_config_frame.columns_treeview.insert.assert_called_once_with(
            "", tk.END, values=("Header", "valid")
        )
        mock_data_config_frame.columns_header_entry.delete.assert_called_once_with(
            0, tk.END
        )
        mock_data_config_frame.columns_type_entry.current.assert_called_once_with(0)

    def test_remove_column_no_selection(self) -> None:
        """Test 'remove_column_cb' function with no column selected"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.root.write_text = MagicMock()
        mock_data_config_frame.columns_treeview.focus.return_value = ""
        DataConfigFrame.remove_column_cb(mock_data_config_frame)
        mock_data_config_frame.root.write_text.assert_called_once_with(
            "Please select a Column from the list.\n"
        )

    def test_remove_column(self) -> None:
        """Test 'remove_column_cb' function"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.root.write_text = MagicMock()
        mock_treeview = MagicMock()
        mock_treeview.focus.return_value = "item"
        mock_treeview.index.return_value = 0
        mock_data_config_frame.columns_treeview = mock_treeview
        mock_data_config_frame.root.write_text = MagicMock()
        mock_data_config_frame.columns = []
        mock_data_config_frame.columns.append(Column("", ""))
        DataConfigFrame.remove_column_cb(mock_data_config_frame)
        self.assertEqual(0, len(mock_data_config_frame.columns))
        mock_treeview.delete.assert_called_once_with("item")
        mock_data_config_frame.root.write_text.assert_not_called()

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function"""
        mock_data_config_file = MagicMock()
        mock_filename.return_value = "Data Configuration File"
        DataConfigFrame.select_file_cb(mock_data_config_file)
        mock_data_config_file.file_path_entry.delete.assert_called_once_with(0, tk.END)
        mock_data_config_file.file_path_entry.insert.assert_called_once_with(
            tk.END, "Data Configuration File"
        )

    @patch("tkinter.filedialog.asksaveasfilename")
    def test_select_file_no_selection(self, mock_filename: MagicMock) -> None:
        """Test 'select_file_cb' function when no file is selected"""
        mock_data_config_file = MagicMock()
        mock_filename.return_value = ""
        DataConfigFrame.select_file_cb(mock_data_config_file)
        mock_data_config_file.file_path_entry.delete.assert_not_called()
        mock_data_config_file.file_path_entry.insert.assert_not_called()

    def test_generate_data_config_invalid_path(self) -> None:
        """Test 'generate_data_config_cb' function with invalid file path"""
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        mock_data_config_file.file_path_entry.get.return_value = ""
        DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            "Path of the Data Configuration File has to be given as a valid path.\n"
        )

    def test_generate_data_config_no_columns(self) -> None:
        """Test 'generate_data_config_cb' function when no columns have been given"""
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        mock_data_config_file.file_path_entry.get.return_value = "file_path.txt"
        mock_data_config_file.columns = []
        DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            "Please add Columns.\n"
        )

    def test_generate_data_config_not_integer(self) -> None:
        """Test 'generate_data_config_cb' function when input for
        'precision' or 'skip' are invalid
        """
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        mock_data_config_file.file_path_entry.get.return_value = "file_path.txt"
        mock_data_config_file.skip_entry.get.return_value = "number"
        mock_data_config_file.precision_entry.get.return_value = "number"
        mock_data_config_file.columns = [Column("Header", "string")]
        DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            "Number of Lines to skip and Precision of Data have to be given as integers.\n"
        )

    @patch("cli.cmd_gui.frame_plot.frame_data_config.Path.mkdir")
    def test_generate_data_config(self, mock_dir: MagicMock) -> None:
        """Test 'generate_data_config_cb' function with valid input"""
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        file_path = "file_path.txt"
        mock_data_config_file.file_path_entry.get.return_value = str(file_path)
        mock_data_config_file.skip_entry.get.return_value = "0"
        mock_data_config_file.precision_entry.get.return_value = "0"
        mock_data_config_file.columns = [Column("Header", "string")]
        with patch("builtins.open"):
            DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_dir.assert_called_once_with(parents=True, exist_ok=True)
        mock_data_config_file.root.write_text.assert_called_once_with(
            f"Data Configuration File has been saved in '{file_path}'.\n"
        )
        mock_data_config_file.root.run_plot_tab.data_config_entry.delete.assert_called_once_with(
            0, tk.END
        )
        mock_data_config_file.root.run_plot_tab.data_config_entry.insert.assert_called_once_with(
            tk.END, str(file_path)
        )


if __name__ == "__main__":
    unittest.main()
