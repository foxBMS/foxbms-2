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

"""Testing file 'cli/cmd_gui/frame_plot/frame_data_config.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_plot.frame_data_config import Column, DataConfigFrame
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot.frame_data_config import Column, DataConfigFrame
    from cli.helpers.misc import PROJECT_BUILD_ROOT
RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestDataConfigFrame(unittest.TestCase):
    """Test of the DataConfigFrame class"""

    def setUp(self):
        parent = tk.Tk()
        parent.withdraw()
        self.frame = DataConfigFrame(parent, parent)

    def tearDown(self):
        self.frame.root.update()
        self.frame.root.destroy()
        if hasattr(self.frame, "file_stream"):
            self.frame.file_stream.close()

    def test_add_column_cb_name_invalid(self):
        """Test 'add_column_cb' function if column_name is not valid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.add_column_cb()
        mock_write_text.assert_called_once_with("Column header has to be given\n")
        self.assertEqual(0, len(self.frame.columns_treeview.get_children()))

    def test_add_column_cb_type_invalid(self):
        """Test 'add_column_cb' function if column_type is not valid"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.columns_header_entry.delete(0, tk.END)
        self.frame.columns_header_entry.insert(0, "Header")
        self.frame.columns_type_entry.delete(0, tk.END)
        self.frame.add_column_cb()
        mock_write_text.assert_called_once_with("Column type has to be given\n")
        self.assertEqual(0, len(self.frame.columns_treeview.get_children()))

    def test_add_column_cb(self):
        """Test 'add_column_cb' function with valid input"""
        mock_write_text = MagicMock()
        self.frame.root.write_text = mock_write_text
        self.frame.columns_header_entry.delete(0, tk.END)
        self.frame.columns_header_entry.insert(0, "Header")
        self.frame.add_column_cb()
        mock_write_text.assert_not_called()
        self.assertEqual(1, len(self.frame.columns_treeview.get_children()))
        column_list = self.frame.columns
        self.assertEqual(1, len(column_list))
        self.assertEqual("Header", column_list[0].column_name)
        self.assertEqual("string", column_list[0].column_type)

    def test_remove_column_cb_no_selection(self):
        """Test 'remove_column_cb' function with no column selected"""
        self.frame.root.write_text = MagicMock()
        self.frame.remove_column_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Column has to be selected\n"
        )

    def test_remove_column_cb(self):
        """Test 'remove_column_cb' function"""
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
    def test_open_file_cb(self, mock_filename: MagicMock):
        """Test 'open_file_cb' function"""
        mock_filename.return_value = "Data Configuration File"
        self.frame.open_file_cb()
        self.assertEqual(self.frame.file_path_entry.get(), "Data Configuration File")

    def test_generate_data_config_cb_path_invalid(self):
        """Test 'generate_data_config_cb' function with invalid file path"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Path of the Data-File has to be given as a valid path\n"
        )

    def test_generate_data_config_cb_no_columns(self):
        """Test 'generate_data_config_cb' function when no columns have been given"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.insert(0, "FilePath")
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with("Columns have to be given\n")

    def test_generate_data_config_cb_not_integer(self):
        """Test 'generate_data_config_cb' function when input for
        'precision' or 'skip' are invalid"""
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.insert(0, "FilePath")
        self.frame.skip_entry.insert(0, "not_an_integer")
        self.frame.columns.append(Column("Header", "string"))
        self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with(
            "Number of Lines to skip and Precision of Data have to be given as integers\n"
        )

    def test_generate_data_config_cb(self):
        """Test 'generate_data_config_cb' function with valid input"""
        self.frame.root.tab_plot = MagicMock()
        self.frame.root.write_text = MagicMock()
        self.frame.file_path_entry.delete(0, tk.END)
        self.frame.file_path_entry.insert(0, __file__)
        self.frame.columns.append(Column("Header", "string"))
        self.frame.skip_entry.delete(0, tk.END)
        self.frame.precision_entry.delete(0, tk.END)
        self.frame.skip_entry.insert(0, "0")
        self.frame.precision_entry.insert(0, "0")
        with patch("builtins.open"):
            self.frame.generate_data_config_cb()
        self.frame.root.write_text.assert_called_once_with(
            f"Data Configuration File has been saved in {__file__}\n"
        )
        self.frame.root.tab_plot.data_config_entry.delete.assert_called_once_with(
            0, tk.END
        )
        self.frame.root.tab_plot.data_config_entry.insert.assert_called_once_with(
            tk.END, str(__file__)
        )


class TestDataConfigFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the DataConfigFrame class"""

    def test_add_column_cb_name_invalid(self):
        """Test 'add_column_cb' function if column_name is not valid"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.columns_header_entry.get.return_value = "name"
        mock_data_config_frame.columns_type_entry.get.return_value = ""
        mock_write_text = MagicMock()
        mock_data_config_frame.root.write_text = mock_write_text
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_write_text.assert_called_once_with("Column header has to be given\n")

    def test_add_column_cb_type_invalid(self):
        """Test 'add_column_cb' function if column_type is not valid"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.columns_header_entry.get.return_value = "Header"
        mock_data_config_frame.columns_type_entry.get.return_value = ""
        mock_write_text = MagicMock()
        mock_data_config_frame.root.write_text = mock_write_text
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_write_text.assert_called_once_with("Column type has to be given\n")

    def test_add_column_cb(self):
        """Test 'add_column_cb' function with valid input"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.columns_header_entry.get.return_value = "Header"
        mock_data_config_frame.columns_type_entry.get.return_value = "string"
        mock_write_text = MagicMock()
        mock_data_config_frame.root.write_text = mock_write_text
        DataConfigFrame.add_column_cb(mock_data_config_frame)
        mock_write_text.assert_not_called()
        mock_data_config_frame.columns.append.assert_called_once_with(  # pylint: disable=no-member
            Column("Header", "string")
        )
        mock_data_config_frame.columns_treeview.insert.assert_called_once_with(
            "", tk.END, values=("Header", "string")
        )
        mock_data_config_frame.columns_header_entry.delete.assert_called_once_with(
            0, tk.END
        )
        mock_data_config_frame.columns_type_entry.current.assert_called_once_with(0)

    def test_remove_column_cb_no_selection(self):
        """Test 'remove_column_cb' function with no column selected"""
        mock_data_config_frame = MagicMock()
        mock_data_config_frame.root.write_text = MagicMock()
        mock_data_config_frame.columns_treeview.focus.return_value = ""
        DataConfigFrame.remove_column_cb(mock_data_config_frame)
        mock_data_config_frame.root.write_text.assert_called_once_with(
            "Column has to be selected\n"
        )

    def test_remove_column_cb(self):
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
    def test_open_file_cb(self, mock_filename: MagicMock):
        """Test 'open_file_cb' function"""
        mock_data_config_file = MagicMock()
        mock_filename.return_value = "Data Configuration File"
        DataConfigFrame.open_file_cb(mock_data_config_file)
        mock_data_config_file.file_path_entry.delete.assert_called_once_with(0, tk.END)
        mock_data_config_file.file_path_entry.insert.assert_called_once_with(
            tk.END, "Data Configuration File"
        )

    def test_generate_data_config_cb_path_invalid(self):
        """Test 'generate_data_config_cb' function with invalid file path"""
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        mock_data_config_file.file_path_entry.get.return_value = ""
        DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            "Path of the Data-File has to be given as a valid path\n"
        )

    def test_generate_data_config_cb_no_columns(self):
        """Test 'generate_data_config_cb' function when no columns have been given"""
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        mock_data_config_file.file_path_entry.get.return_value = "file/path"
        mock_data_config_file.columns = []
        DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            "Columns have to be given\n"
        )

    def test_generate_data_config_cb_not_integer(self):
        """Test 'generate_data_config_cb' function when input for
        'precision' or 'skip' are invalid"""
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        mock_data_config_file.file_path_entry.get.return_value = "file/path"
        mock_data_config_file.skip_entry.get.return_value = "not_an_integer"
        mock_data_config_file.precision_entry.get.return_value = "not_an_integer"
        mock_data_config_file.columns = [Column("Header", "string")]
        DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            "Number of Lines to skip and Precision of Data have to be given as integers\n"
        )

    def test_generate_data_config_cb(self):
        """Test 'generate_data_config_cb' function with valid input"""
        PROJECT_BUILD_ROOT.mkdir(exist_ok=True, parents=True)
        mock_data_config_file = MagicMock()
        mock_data_config_file.root.write_text = MagicMock()
        out = PROJECT_BUILD_ROOT / "file/path"
        mock_data_config_file.file_path_entry.get.return_value = str(out)
        mock_data_config_file.skip_entry.get.return_value = "0"
        mock_data_config_file.precision_entry.get.return_value = "0"
        mock_data_config_file.columns = [Column("Header", "string")]
        with patch("builtins.open"):
            DataConfigFrame.generate_data_config_cb(mock_data_config_file)
        mock_data_config_file.root.write_text.assert_called_once_with(
            f"Data Configuration File has been saved in {out}\n"
        )
        mock_data_config_file.root.tab_plot.data_config_entry.delete.assert_called_once_with(
            0, tk.END
        )
        mock_data_config_file.root.tab_plot.data_config_entry.insert.assert_called_once_with(
            tk.END, str(out)
        )


if __name__ == "__main__":
    unittest.main()
