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

"""Testing file 'cli/cmd_gui/frame_plot/plot_gui.py'."""

import importlib
import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from queue import Queue
from unittest.mock import MagicMock, call, mock_open, patch

try:
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_plot import plot_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_plot import plot_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
    from cli.helpers.spr import SubprocessResult

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "plot_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
class TestCheckThread(unittest.TestCase):
    """Test of the 'check_thread' function of the PlotFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = plot_gui.PlotFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.after")
    def test_check_thread_alive(
        self, mock_after: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test 'check_thread' function when the Thread is alive"""
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = True
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.frame.plot_process.is_alive.assert_called_once()
        mock_after.assert_called_once_with(50, self.frame.check_thread)

    def test_check_thread_success(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and plotting was successful
        """
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = False
        mock_file_stream = MagicMock()
        self.frame.file_stream = mock_file_stream
        self.frame.queue.put(SubprocessResult(returncode=0))
        self.frame.check_thread()
        self.frame.plot_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.plot_button.state(), ())
        mock_file_stream.close.assert_called_once()
        mock_write_text.assert_has_calls([call(), call("Plotting was successful.\n")])

    def test_check_thread_failure(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and plotting failed
        """
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = False
        self.frame.queue.put(SubprocessResult(returncode=1))
        mock_file_stream = MagicMock()
        self.frame.file_stream = mock_file_stream
        self.frame.check_thread()
        self.frame.plot_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.plot_button.state(), ())
        mock_file_stream.close.assert_called_once()
        mock_write_text.assert_has_calls(
            [call(), call("Plotting failed with exit code 1.\n")]
        )

    def test_check_thread_empty(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function
        when the Thread is not alive but the queue is empty
        """
        self.frame.plot_process = MagicMock()
        self.frame.plot_process.is_alive.return_value = False
        mock_file_stream = MagicMock()
        self.frame.file_stream = mock_file_stream
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.frame.plot_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.plot_button.state(), ())
        mock_file_stream.close.assert_called_once()


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.reset_text")
@patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.write_text")
class TestPlotCommand(unittest.TestCase):
    """Test of the 'plot_command_cb' function of the PlotFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = plot_gui.PlotFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.check_thread")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    # pylint: disable-next=too-many-arguments, too-many-positional-arguments
    def test_plot_command_correct(  # noqa: PLR0913
        self,
        mock_is_dir: MagicMock,
        mock_is_file: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
        mock_write_text: MagicMock,
        mock_reset_text: MagicMock,
    ) -> None:
        """Test 'run_command_cb' function with correct input values"""
        mock_open_file = mock_open()
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        with patch("builtins.open", mock_open_file):
            self.frame.plot_command_cb()
        mock_reset_text.assert_called_once()
        self.assertEqual(self.frame.plot_button.state(), ("disabled",))
        mock_write_text.assert_called_once_with("Running plot command.\n")
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()
        mock_open_file.assert_has_calls(
            [
                call(self.frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
                call(self.frame.file_path, mode="a", encoding="utf-8"),
            ]
        )

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.check_thread")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    # pylint: disable-next=too-many-arguments, too-many-positional-arguments
    def test_plot_command_no_project(  # noqa: PLR0913
        self,
        mock_is_dir: MagicMock,
        mock_is_file: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
        mock_write_text: MagicMock,
        mock_reset_text: MagicMock,
    ) -> None:
        """Test 'run_command_cb' function when ROOT_IS_PROJECT is False and
        all input is correct
        """
        mock_open_file = mock_open()
        plot_gui.ROOT_IS_PROJECT = False
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        with patch("builtins.open", mock_open_file):
            self.frame.plot_command_cb()
        mock_reset_text.assert_called_once()
        self.assertEqual(self.frame.plot_button.state(), ("disabled",))
        mock_write_text.assert_called_once_with("Running plot command.\n")
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()
        mock_open_file.assert_has_calls(
            [
                call(self.frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
                call(self.frame.file_path, mode="a", encoding="utf-8"),
            ]
        )

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.check_thread")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    def test_plot_command_no_file(
        self,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
        mock_write_text: MagicMock,
        mock_reset_text: MagicMock,
    ) -> None:
        """Test 'run_command_cb' function when input is not a file"""
        mock_open_file = mock_open()
        self.frame.run_plot_tab.data_config_entry.delete(0, tk.END)
        self.frame.run_plot_tab.plot_config_entry.delete(0, tk.END)
        self.frame.run_plot_tab.data_source_entry.delete(0, tk.END)
        self.frame.run_plot_tab.data_config_entry.insert(tk.END, "invalid-file")
        self.frame.run_plot_tab.plot_config_entry.insert(tk.END, "invalid-file")
        self.frame.run_plot_tab.data_source_entry.insert(tk.END, "invalid-file")
        with patch("builtins.open", mock_open_file):
            self.frame.plot_command_cb()
        mock_reset_text.assert_called_once()
        mock_write_text.assert_called_once_with(
            "Configuration files and Data Source have to be given as valid file-paths.\n"
        )
        self.assertEqual(self.frame.plot_button.state(), ())
        mock_open_file.assert_not_called()
        mock_thread.assert_not_called()
        mock_check_thread.assert_not_called()

    @patch("cli.cmd_gui.frame_plot.plot_gui.Path.mkdir")
    def test_plot_command_no_dir(
        self,
        mock_mkdir: MagicMock,
        mock_write_text: MagicMock,
        mock_reset_text: MagicMock,
    ) -> None:
        """Test 'run_command_cb' function when an input is not a file"""
        mock_open_file = mock_open()
        self.frame.run_plot_tab.data_config_entry.delete(0, tk.END)
        self.frame.run_plot_tab.plot_config_entry.delete(0, tk.END)
        self.frame.run_plot_tab.data_source_entry.delete(0, tk.END)
        self.frame.run_plot_tab.output_directory_entry.delete(0, tk.END)
        self.frame.run_plot_tab.data_config_entry.insert(tk.END, str(__file__))
        self.frame.run_plot_tab.plot_config_entry.insert(tk.END, str(__file__))
        self.frame.run_plot_tab.data_source_entry.insert(tk.END, str(__file__))
        self.frame.run_plot_tab.output_directory_entry.insert(tk.END, ".")
        with patch("builtins.open", mock_open_file):
            self.frame.plot_command_cb()
        mock_reset_text.assert_called_once()
        mock_mkdir.assert_called_once()
        mock_write_text.assert_called_once_with(
            f"Output directory has been set to '{PROJECT_BUILD_ROOT / 'plot'}'.\n"
        )
        self.assertEqual(self.frame.plot_button.state(), ())
        mock_open_file.assert_not_called()
        self.assertEqual(
            f"{PROJECT_BUILD_ROOT / 'plot'}",
            self.frame.run_plot_tab.output_directory_entry.get().strip(),
        )

    @patch("cli.cmd_gui.frame_plot.plot_gui.PlotFrame.check_thread")
    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    # pylint: disable-next=too-many-arguments, too-many-positional-arguments
    def test_plot_command_parquet(  # noqa: PLR0913
        self,
        mock_is_dir: MagicMock,
        mock_is_file: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
        mock_write_text: MagicMock,
        mock_reset_text: MagicMock,
    ) -> None:
        """Test 'run_command_cb' function
        when data type is set to PARQUET
        """
        mock_open_file = mock_open()
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        self.frame.run_plot_tab.data_type_entry.set("PARQUET")
        with patch("builtins.open", mock_open_file):
            self.frame.plot_command_cb()
        mock_reset_text.assert_called_once()
        self.assertEqual(self.frame.plot_button.state(), ("disabled",))
        mock_write_text.assert_called_once_with("Running plot command.\n")
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()
        mock_open_file.assert_has_calls(
            [
                call(self.frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
                call(self.frame.file_path, mode="a", encoding="utf-8"),
            ]
        )


class TestPlotFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the PlotFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)

    def test_check_thread_alive(self) -> None:
        """Test 'check_thread' function when the Thread is alive"""
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = True
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.write_text.assert_called_once()
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.after.assert_called_once_with(50, mock_plot_frame.check_thread)

    def test_check_thread_success(self) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and plotting was successful
        """
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = False
        mock_plot_frame.queue = Queue()
        mock_plot_frame.queue.put(SubprocessResult(returncode=0))
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with(["!disabled"])
        mock_plot_frame.file_stream.close.assert_called_once()
        mock_plot_frame.write_text.assert_has_calls(
            [call(), call("Plotting was successful.\n")]
        )

    def test_check_thread_failure(self) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and plotting failed
        """
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = False
        mock_plot_frame.queue = Queue()
        mock_plot_frame.queue.put(SubprocessResult(returncode=1))
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with(["!disabled"])
        mock_plot_frame.file_stream.close.assert_called_once()
        mock_plot_frame.write_text.assert_has_calls(
            [call(), call("Plotting failed with exit code 1.\n")]
        )

    def test_check_thread_empty(self) -> None:
        """Test 'check_thread' function
        when the Thread is not alive but the queue is empty
        """
        mock_plot_frame = MagicMock()
        mock_plot_frame.plot_process.is_alive.return_value = False
        mock_plot_frame.queue = Queue()
        plot_gui.PlotFrame.check_thread(mock_plot_frame)
        mock_plot_frame.write_text.assert_called_once()
        mock_plot_frame.plot_process.is_alive.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with(["!disabled"])
        mock_plot_frame.file_stream.close.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    def test_plot_command_correct(
        self, mock_is_dir: MagicMock, mock_is_file: MagicMock, mock_thread: MagicMock
    ) -> None:
        """Test 'run_command_cb' function with correct input"""
        mock_open_file = mock_open()
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        mock_plot_frame = MagicMock()
        with patch("builtins.open", mock_open_file):
            plot_gui.PlotFrame.plot_command_cb(mock_plot_frame)
        mock_plot_frame.reset_text.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with([tk.DISABLED])
        mock_plot_frame.check_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_plot_frame.check_thread.assert_called_once()

    @patch("cli.cmd_gui.frame_plot.plot_gui.Thread")
    @patch("pathlib.Path.is_file")
    @patch("pathlib.Path.is_dir")
    def test_plot_command_no_project(
        self, mock_is_dir: MagicMock, mock_is_file: MagicMock, mock_thread: MagicMock
    ) -> None:
        """Test 'run_command_cb' function when not in the foxBMS repository"""
        mock_open_file = mock_open()
        plot_gui.ROOT_IS_PROJECT = False
        mock_is_file.return_value = True
        mock_is_dir.return_value = True
        mock_plot_frame = MagicMock()
        with patch("builtins.open", mock_open_file):
            plot_gui.PlotFrame.plot_command_cb(mock_plot_frame)
        mock_plot_frame.reset_text.assert_called_once()
        mock_plot_frame.plot_button.state.assert_called_once_with([tk.DISABLED])
        mock_plot_frame.check_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_plot_frame.check_thread.assert_called_once()


def remove_data(start_time: datetime) -> None:
    """Remove all data from the gui directory if it as been created after start_time"""
    if PATH_GUI.is_dir():
        if get_birthtime(PATH_GUI) >= start_time:
            shutil.rmtree(PATH_GUI)
        else:
            children = list(PATH_GUI.iterdir())
            for child in children:
                if get_birthtime(child) >= start_time:
                    if child.is_dir():
                        shutil.rmtree(child)
                    else:
                        child.unlink()


def get_birthtime(object_name: Path) -> datetime:
    """Return the birthtime of the given object"""
    try:
        birthtime = datetime.fromtimestamp(object_name.stat().st_birthtime, tz=UTC)
    except AttributeError:
        birthtime = datetime.fromtimestamp(object_name.stat().st_atime, tz=UTC)
    return birthtime


if __name__ == "__main__":
    unittest.main()
