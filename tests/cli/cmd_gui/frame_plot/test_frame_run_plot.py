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

"""Testing file 'cli/cmd_gui/frame_plot/frame_run_plot.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_plot.frame_run_plot import RunPlotFrame
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot.frame_run_plot import RunPlotFrame
    from cli.helpers.misc import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestRunPlotFrame(unittest.TestCase):
    """Test of the RunPlotFrame class"""

    def setUp(self):
        parent = tk.Tk()
        parent.withdraw()
        self.frame = RunPlotFrame(parent, parent)

    def tearDown(self):
        self.frame.root.update()
        self.frame.root.destroy()
        if hasattr(self.frame, "file_stream"):
            self.frame.file_stream.close()

    @patch("tkinter.filedialog.askdirectory")
    def test_open_output_directory_cb(self, mock_askdirectory: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askdirectory.return_value = "Directory"
        self.assertEqual(
            f"{PROJECT_BUILD_ROOT}",
            self.frame.output_entry.get().strip(),
        )
        self.frame.open_output_directory_cb()
        self.assertIn("Directory", self.frame.output_entry.get().strip())

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_data_source_cb(self, mock_askopenfilename: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askopenfilename.return_value = "Data Source"
        self.assertIn("example_data.csv", self.frame.data_source_entry.get().strip())
        self.frame.open_data_source_cb()
        self.assertEqual("Data Source", self.frame.data_source_entry.get().strip())

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_plot_config_cb(self, mock_askopenfilename: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askopenfilename.return_value = "Plot Configuration"
        self.assertIn("plot_config.yaml", self.frame.plot_config_entry.get().strip())
        self.frame.open_plot_config_cb()
        self.assertEqual(
            "Plot Configuration",
            self.frame.plot_config_entry.get().strip(),
        )

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_data_config_cb(self, mock_askopenfilename: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askopenfilename.return_value = "Data Configuration"
        self.assertIn("csv_config.yaml", self.frame.data_config_entry.get().strip())
        self.frame.open_data_config_cb()
        self.assertEqual(
            "Data Configuration",
            self.frame.data_config_entry.get().strip(),
        )


class TestRunPlotFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the PlotFrame class"""

    @patch("tkinter.filedialog.askdirectory")
    def test_open_output_directory_cb(self, mock_askdirectory: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askdirectory.return_value = "Directory"
        mock_plot_frame = MagicMock()
        RunPlotFrame.open_output_directory_cb(mock_plot_frame)
        mock_plot_frame.output_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_frame.output_entry.insert.assert_called_once_with(tk.END, "Directory")

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_data_source_cb(self, mock_askopenfilename: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askopenfilename.return_value = "Data Source"
        mock_plot_frame = MagicMock()
        RunPlotFrame.open_data_source_cb(mock_plot_frame)
        mock_plot_frame.data_source_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_frame.data_source_entry.insert.assert_called_once_with(
            tk.END, "Data Source"
        )

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_plot_config_cb(self, mock_askopenfilename: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askopenfilename.return_value = "Plot Configuration"
        mock_plot_frame = MagicMock()
        RunPlotFrame.open_plot_config_cb(mock_plot_frame)
        mock_plot_frame.plot_config_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_frame.plot_config_entry.insert.assert_called_once_with(
            tk.END, "Plot Configuration"
        )

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_data_config_cb(self, mock_askopenfilename: MagicMock):
        """Test 'open_output_directory_cb' function"""
        mock_askopenfilename.return_value = "Data Configuration"
        mock_plot_frame = MagicMock()
        RunPlotFrame.open_data_config_cb(mock_plot_frame)
        mock_plot_frame.data_config_entry.delete.assert_called_once_with(0, tk.END)
        mock_plot_frame.data_config_entry.insert.assert_called_once_with(
            tk.END, "Data Configuration"
        )


if __name__ == "__main__":
    unittest.main()
