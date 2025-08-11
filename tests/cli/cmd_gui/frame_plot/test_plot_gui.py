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

"""Testing file 'cli/cmd_gui/frame_plot/plot_gui.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from queue import Queue
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_plot import plot_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_plot import plot_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT
    from cli.helpers.spr import SubprocessResult

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestPlotFrame(unittest.TestCase):
    """Test of the PlotFrame class"""

    def setUp(self):
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = plot_gui.PlotFrame(self.root, text)

    def tearDown(self):
        self.root.update()
        self.root.destroy()
        if hasattr(self.frame, "file_stream"):
            self.frame.file_stream.close()
        file_output = Path(PROJECT_BUILD_ROOT / "output_gui_plot.txt")
        if file_output.exists():
            file_output.unlink()

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.after")
    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    def test_check_thread_alive(
        self, mock_write_text: MagicMock, mock_after: MagicMock
    ):
        """Test 'check_thread' function when the Thread is still alive"""
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = True
        self.frame.check_thread()
        mock_after.assert_called_once_with(50, self.frame.check_thread)
        self.frame.plot_process.is_alive.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    def test_check_thread_success(self, mock_write_text: MagicMock):
        """Test 'check_thread' function
        when the Thread is not alive and plotting was successful"""
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = False
        self.frame.queue.put(SubprocessResult(returncode=0))
        self.frame.file_stream = open(  # pylint: disable=consider-using-with
            self.frame.file_path, encoding="utf-8", mode="a", errors="ignore"
        )
        self.frame.check_thread()
        self.assertTrue(self.frame.file_stream.closed)
        self.frame.plot_process.is_alive.assert_called_once()
        self.assertEqual("normal", str(self.frame.plot_button["state"]))
        with open(self.frame.file_path, encoding="utf-8") as f:
            self.assertEqual(f.read(), "Finished Plotting\n")
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    def test_check_thread_failure(self, mock_write_text: MagicMock):
        """Test 'check_thread' function
        when the Thread is not alive and plotting not successful"""
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = False
        self.frame.queue.put(SubprocessResult(returncode=1))
        self.frame.file_stream = open(  # pylint: disable=consider-using-with
            self.frame.file_path, encoding="utf-8", mode="a", errors="ignore"
        )
        self.frame.check_thread()
        self.assertTrue(self.frame.file_stream.closed)
        self.frame.plot_process.is_alive.assert_called_once()
        self.assertEqual("normal", str(self.frame.plot_button["state"]))
        with open(self.frame.file_path, encoding="utf-8") as f:
            self.assertEqual(f.read(), "Plotting was not successful\n")
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    def test_check_thread_empty(self, mock_write_text: MagicMock):
        """Test 'check_thread' function
        when the Thread is not alive but the queue is empty"""
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = False
        self.frame.file_stream = open(  # pylint: disable=consider-using-with
            self.frame.file_path, encoding="utf-8", mode="a", errors="ignore"
        )
        self.frame.check_thread()
        self.assertTrue(self.frame.file_stream.closed)
        self.frame.plot_process.is_alive.assert_called_once()
        self.assertEqual("normal", str(self.frame.plot_button["state"]))
        with open(self.frame.file_path, encoding="utf-8") as f:
            self.assertEqual(f.read(), "")
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.check_thread")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    def test_plot_command_cb(  # pylint: disable=too-many-arguments, too-many-positional-arguments
        self,
        mock_is_dir: MagicMock,
        mock_is_file: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
        mock_write_text: MagicMock,
    ):
        """Test 'run_command_cb' function
        when all input is correct"""
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        self.frame.plot_command_cb()
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()
        mock_write_text.assert_called_once_with("Plotting the given graphs\n")

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.check_thread")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    def test_plot_command_cb_no_file(  # pylint: disable=too-many-arguments, too-many-positional-arguments
        self,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
        mock_write_text: MagicMock,
    ):
        """Test 'run_command_cb' function
        when an input is not a file"""
        self.frame.tab_plot.data_config_entry.delete(0, tk.END)
        self.frame.tab_plot.plot_config_entry.delete(0, tk.END)
        self.frame.tab_plot.data_source_entry.delete(0, tk.END)
        self.frame.tab_plot.data_config_entry.insert(tk.END, "not-a-valid-file")
        self.frame.tab_plot.plot_config_entry.insert(tk.END, "not-a-valid-file")
        self.frame.tab_plot.data_source_entry.insert(tk.END, "not-a-valid-file")
        self.frame.plot_command_cb()
        mock_thread.assert_not_called()
        mock_check_thread.assert_not_called()
        mock_write_text.assert_called_once_with(
            "Configuration files and Data Source have to be given as valid file-paths\n"
        )

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Path.mkdir")
    def test_plot_command_cb_no_dir(  # pylint: disable=too-many-arguments, too-many-positional-arguments
        self,
        mock_mkdir: MagicMock,
        mock_write_text: MagicMock,
    ):
        """Test 'run_command_cb' function
        when an input is not a file"""
        self.frame.tab_plot.data_config_entry.delete(0, tk.END)
        self.frame.tab_plot.plot_config_entry.delete(0, tk.END)
        self.frame.tab_plot.data_source_entry.delete(0, tk.END)
        self.frame.tab_plot.output_entry.delete(0, tk.END)
        self.frame.tab_plot.data_config_entry.insert(tk.END, str(__file__))
        self.frame.tab_plot.plot_config_entry.insert(tk.END, str(__file__))
        self.frame.tab_plot.data_source_entry.insert(tk.END, str(__file__))
        self.frame.tab_plot.output_entry.insert(tk.END, ".")
        self.frame.plot_command_cb()
        mock_mkdir.assert_called_once()
        self.assertEqual(str(self.frame.plot_button["state"]), "normal")
        self.assertEqual(
            f"{PROJECT_BUILD_ROOT / 'plot'}",
            self.frame.tab_plot.output_entry.get().strip(),
        )
        mock_write_text.assert_called_once_with(
            f"Output directory has been set to '{PROJECT_BUILD_ROOT / 'plot'}'\n"
        )

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        self.frame.parent = MagicMock()
        self.frame.parent.nametowidget.return_value = self.frame
        self.frame.file_path.touch()
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        self.frame.parent = MagicMock()
        self.frame.parent.nametowidget.return_value = self.frame
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("New content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)

    def test_write_text_not_selected(self):
        """Test 'write_text' function when PlotFrame is not selected"""
        self.frame.parent = MagicMock()
        self.frame.parent.nametowidget.return_value = None
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text_input(self):
        """Test 'write_text' function when a string is passed"""
        self.frame.parent = MagicMock()
        self.frame.parent.nametowidget.return_value = self.frame
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.frame.write_text(file_input="New content.")
        self.assertEqual("New content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)


class TestPlotFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the PlotFrame class"""

    def setUp(self):
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        for file_path in PROJECT_BUILD_ROOT.glob("output_plot_*"):
            if file_path.suffix == ".txt" and file_path.exists():
                file_path.unlink()

    def test_check_thread_alive(self):
        """Test 'check_thread' function when the Thread is still alive"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = True
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.after.assert_called_once_with(50, mock_plot_frame.check_thread)
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.write_text.assert_called_once()

    def test_check_thread_success(self):
        """Test 'check_thread' function
        when the Thread is not alive and plotting was successful"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = False
        mock_plot_frame.file_stream.write = MagicMock()
        mock_plot_frame.queue = Queue()
        mock_plot_frame.queue.put(SubprocessResult(returncode=0))
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with(["!disabled"])
        mock_plot_frame.file_stream.write.assert_called_once_with("Finished Plotting\n")
        mock_plot_frame.write_text.assert_called_once()

    def test_check_thread_failure(self):
        """Test 'check_thread' function
        when the Thread is not alive and plotting not successful"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = False
        mock_plot_frame.file_stream.write = MagicMock()
        mock_plot_frame.queue = Queue()
        mock_plot_frame.queue.put(SubprocessResult(returncode=1))
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with(["!disabled"])
        mock_plot_frame.file_stream.write.assert_called_once_with(
            "Plotting was not successful\n"
        )
        mock_plot_frame.write_text.assert_called_once()

    def test_check_thread_empty(self):
        """Test 'check_thread' function
        when the Thread is not alive but the queue is empty"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = False
        mock_plot_frame.file_stream.write = MagicMock()
        mock_plot_frame.queue = Queue()
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with(["!disabled"])
        mock_plot_frame.file_stream.write.assert_not_called()
        mock_plot_frame.write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    def test_plot_command_cb(
        self, mock_is_dir: MagicMock, mock_is_file: MagicMock, mock_thread: MagicMock
    ):
        """Test 'run_command_cb' function"""
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        mock_plot_frame = MagicMock()
        mock_plot_frame.file_path = Path(PROJECT_BUILD_ROOT / "output_plot_command.txt")
        mock_plot_frame.file_path.touch()
        plot_gui.PlotFrame.plot_command_cb(mock_plot_frame)
        mock_plot_frame.file_stream.close()
        mock_plot_frame.plot_button.state.assert_called_once_with(["disabled"])
        mock_plot_frame.check_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_plot_frame.check_thread.assert_called_once()

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_plot_write_text_empty.txt"
        )
        mock_plot_frame.file_path.touch()
        mock_plot_frame.text = MagicMock()
        mock_plot_frame.text_index = 0
        mock_plot_frame.parent.nametowidget.return_value = mock_plot_frame
        plot_gui.PlotFrame.write_text(mock_plot_frame)
        self.assertEqual(mock_plot_frame.text_index, 0)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_plot_write_text.txt"
        )
        mock_plot_frame.file_path.write_text("New content.", encoding="utf-8")
        mock_plot_frame.text = MagicMock()
        mock_plot_frame.text_index = 0
        mock_plot_frame.parent.nametowidget.return_value = mock_plot_frame
        plot_gui.PlotFrame.write_text(mock_plot_frame)
        self.assertEqual(mock_plot_frame.text_index, 12)

    def test_write_text_string(self):
        """Test 'write_text' function when a string is passed"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_plot_write_text_string.txt"
        )
        mock_plot_frame.file_path.touch()
        mock_plot_frame.text = MagicMock()
        mock_plot_frame.text_index = 0
        mock_plot_frame.parent.nametowidget.return_value = mock_plot_frame
        plot_gui.PlotFrame.write_text(mock_plot_frame, "New content.")
        self.assertEqual(mock_plot_frame.text_index, 12)


if __name__ == "__main__":
    unittest.main()
