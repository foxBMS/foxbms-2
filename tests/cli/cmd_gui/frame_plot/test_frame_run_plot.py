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

"""Testing file 'cli/cmd_gui/frame_plot/frame_run_plot.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_plot.frame_run_plot import RunPlotFrame
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot.frame_run_plot import RunPlotFrame
    from cli.helpers.project_context import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "gui"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestRunPlotFrame(unittest.TestCase):
    """Test of the RunPlotFrame class"""

    def setUp(self) -> None:  # noqa: D102
        parent = tk.Tk()
        parent.withdraw()
        self.frame = RunPlotFrame(parent, parent)

    def tearDown(self) -> None:  # noqa: D102
        self.frame.root.update()
        self.frame.root.destroy()

    @patch("tkinter.filedialog.askdirectory")
    def test_select_directory(self, mock_askdirectory: MagicMock) -> None:
        """Test 'select_directory_cb' function"""
        mock_askdirectory.return_value = "Directory"
        self.frame.output_directory_entry.delete(0, tk.END)
        self.frame.output_directory_entry.insert(tk.END, "dir")
        self.frame.select_directory_cb()
        self.assertIn("Directory", self.frame.output_directory_entry.get().strip())

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_data_source(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function for the data source"""
        mock_askopenfilename.return_value = "Data Source"
        self.frame.data_source_entry.delete(0, tk.END)
        self.frame.data_source_entry.insert(tk.END, "data_source.csv")
        self.frame.select_file_cb(self.frame.data_source_entry)
        self.assertEqual("Data Source", self.frame.data_source_entry.get().strip())

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_plot_config(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function for the plot configuration"""
        mock_askopenfilename.return_value = "Plot Configuration"
        self.frame.plot_config_entry.delete(0, tk.END)
        self.frame.plot_config_entry.insert(tk.END, "plot_config.yaml")
        self.frame.select_file_cb(self.frame.plot_config_entry)
        self.assertEqual(
            "Plot Configuration",
            self.frame.plot_config_entry.get().strip(),
        )

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_data_config(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function for the data configuration"""
        mock_askopenfilename.return_value = "Data Configuration"
        self.frame.data_config_entry.delete(0, tk.END)
        self.frame.data_config_entry.insert(tk.END, "data_config.yaml")
        self.frame.select_file_cb(self.frame.data_config_entry)
        self.assertEqual(
            "Data Configuration",
            self.frame.data_config_entry.get().strip(),
        )


class TestRunPlotFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the PlotFrame class"""

    @patch("tkinter.filedialog.askdirectory")
    def test_select_directory(self, mock_askdirectory: MagicMock) -> None:
        """Test 'select_directory_cb' function"""
        mock_askdirectory.return_value = "Directory"
        mock_plot_frame = MagicMock()
        RunPlotFrame.select_directory_cb(mock_plot_frame)
        mock_plot_frame.output_directory_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_frame.output_directory_entry.insert.assert_called_once_with(
            tk.END, "Directory"
        )

    @patch("tkinter.filedialog.askdirectory")
    def test_select_directory_no_selection(self, mock_askdirectory: MagicMock) -> None:
        """Test 'select_directory_cb' function when no directory was selected"""
        mock_askdirectory.return_value = ""
        mock_plot_frame = MagicMock()
        RunPlotFrame.select_directory_cb(mock_plot_frame)
        mock_plot_frame.output_directory_entry.delete.assert_not_called()
        mock_plot_frame.output_directory_entry.insert.assert_not_called()

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_data_source(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function for the data source"""
        mock_askopenfilename.return_value = "Data Source"
        mock_plot_frame = MagicMock()
        mock_entry = MagicMock()
        RunPlotFrame.select_file_cb(mock_plot_frame, mock_entry)
        mock_entry.delete.assert_called_once_with(0, tk.END)
        mock_entry.insert.assert_called_once_with(tk.END, "Data Source")

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_plot_config(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function for the plot configuration"""
        mock_askopenfilename.return_value = "Plot Configuration"
        mock_plot_frame = MagicMock()
        mock_entry = MagicMock()
        RunPlotFrame.select_file_cb(mock_plot_frame, mock_entry)
        mock_entry.delete.assert_called_once_with(0, tk.END)
        mock_entry.insert.assert_called_once_with(tk.END, "Plot Configuration")

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_data_config(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function for the data configuration"""
        mock_askopenfilename.return_value = "Data Configuration"
        mock_plot_frame = MagicMock()
        mock_entry = MagicMock()
        RunPlotFrame.select_file_cb(mock_plot_frame, mock_entry)
        mock_entry.delete.assert_called_once_with(0, tk.END)
        mock_entry.insert.assert_called_once_with(tk.END, "Data Configuration")

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_no_selection(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function when no file was selected"""
        mock_askopenfilename.return_value = ""
        mock_plot_frame = MagicMock()
        mock_entry = MagicMock()
        RunPlotFrame.select_file_cb(mock_plot_frame, mock_entry)
        mock_entry.delete.assert_not_called()
        mock_entry.insert.assert_not_called()


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestRunPlotFrameInit(unittest.TestCase):
    """Test initialization of the RunPlotFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.parent = tk.Tk()
        self.parent.withdraw()

    def tearDown(self) -> None:  # noqa: D102
        self.parent.update()
        self.parent.destroy()

    @patch("cli.cmd_gui.frame_plot.frame_run_plot.Path.is_file")
    def test_files(self, mock_is_file: MagicMock) -> None:
        """Test initialization when the files exist"""
        mock_is_file.return_value = True
        run_plot_frame = RunPlotFrame(self.parent, self.parent)
        self.assertNotEqual("", run_plot_frame.data_source_entry.get().strip())
        self.assertNotEqual("", run_plot_frame.data_config_entry.get().strip())
        self.assertNotEqual("", run_plot_frame.plot_config_entry.get().strip())

    @patch("cli.cmd_gui.frame_plot.frame_run_plot.Path.is_file")
    def test_no_files(self, mock_is_file: MagicMock) -> None:
        """Test initialization when the files do not exist"""
        mock_is_file.return_value = False
        run_plot_frame = RunPlotFrame(self.parent, self.parent)
        self.assertEqual("", run_plot_frame.data_source_entry.get().strip())
        self.assertEqual("", run_plot_frame.data_config_entry.get().strip())
        self.assertEqual("", run_plot_frame.plot_config_entry.get().strip())


if __name__ == "__main__":
    unittest.main()
