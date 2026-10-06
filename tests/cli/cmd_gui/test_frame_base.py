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

"""Testing file 'cli/cmd_gui/frame_base.py'."""

import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, call, mock_open, patch

try:
    from cli.cmd_gui import frame_base
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_gui import frame_base
    from cli.helpers.project_context import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "base_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestBaseFrame(unittest.TestCase):
    """Test of the BaseFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        frame_base.PROJECT_BUILD_ROOT = PATH_GUI
        self.frame = frame_base.BaseFrame(self.root, text, "output_gui_base.txt")

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)
        frame_base.PROJECT_BUILD_ROOT = PROJECT_BUILD_ROOT

    def test_write_text_empty_file(self) -> None:
        """Test 'write_text' function when the file has no content"""
        mock_open_file = mock_open(read_data="")
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        with patch("builtins.open", mock_open_file):
            self.frame.write_text()
        mock_select.assert_called_once()
        mock_open_file.assert_called_once_with(
            self.frame.file_path, encoding="utf-8", errors="ignore"
        )
        mock_open_handle = mock_open_file()
        mock_open_handle.read.assert_called_once()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)
        self.assertEqual(tk.DISABLED, self.frame.text["state"])

    def test_write_text_file_content(self) -> None:
        """Test 'write_text' function when the file has content"""
        mock_open_file = mock_open(read_data="One content.")
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        with patch("builtins.open", mock_open_file):
            self.frame.write_text()
        mock_select.assert_called_once()
        mock_open_file.assert_called_once_with(
            self.frame.file_path, encoding="utf-8", errors="ignore"
        )
        mock_open_handle = mock_open_file()
        mock_open_handle.read.assert_called_once()
        self.assertEqual("One content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)
        self.assertEqual(tk.DISABLED, self.frame.text["state"])

    def test_write_text_widget_content(self) -> None:
        """Test 'write_text' function when the text widget has content"""
        mock_open_file = mock_open(read_data="Some content.\nMore content.")
        self.frame.text.config(state=tk.NORMAL)
        self.frame.text.insert(tk.END, "Some content.\n")
        self.frame.text_index = 14
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        with patch("builtins.open", mock_open_file):
            self.frame.write_text()
        mock_select.assert_called_once()
        mock_open_file.assert_called_once_with(
            self.frame.file_path, encoding="utf-8", errors="ignore"
        )
        mock_open_handle = mock_open_file()
        mock_open_handle.read.assert_called_once()
        self.assertEqual(
            "Some content.\nMore content.\n", self.frame.text.get("1.0", tk.END)
        )
        self.assertEqual(27, self.frame.text_index)
        self.assertEqual(tk.DISABLED, self.frame.text["state"])

    def test_write_text_wrong_widget(self) -> None:
        """Test 'write_text' function when the frame is not selected"""
        mock_open_file = mock_open(read_data="New content.\n")
        mock_select = MagicMock()
        mock_select.return_value = ""
        self.frame.parent.select = mock_select
        with patch("builtins.open", mock_open_file):
            self.frame.write_text()
        mock_select.assert_called_once()
        mock_open_file.assert_not_called()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text_file_input(self) -> None:
        """Test 'write_text' function when a string is passed to function and frame is selected"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.frame.write_text("Two content.")
        mock_select.assert_called_once()
        self.assertEqual("Two content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)
        self.assertEqual(tk.DISABLED, self.frame.text["state"])

    def test_reset_text(self) -> None:
        """Test 'reset_text' function"""
        self.frame.text_index = 10
        self.frame.text.config(state=tk.NORMAL)
        self.frame.text.insert(tk.END, "Some text.")
        self.frame.text.config(state=tk.DISABLED)
        self.frame.reset_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)


class TestFrameBaseNoUiTestableMethods(unittest.TestCase):
    """Test of BaseFrame class"""

    def test_write_text_file_stream(self) -> None:
        """Test 'write_text' function when file_stream exists, is not closed,
        and wrong frame is selected
        """
        mock_base_frame = MagicMock()
        mock_file_stream = MagicMock()
        mock_file_stream.closed = False
        mock_base_frame.file_stream = mock_file_stream
        mock_parent = MagicMock()
        mock_parent.nametowidget.return_value = "random_frame"
        mock_base_frame.parent = mock_parent
        frame_base.BaseFrame.write_text(mock_base_frame, "New content.")
        mock_file_stream.write.assert_called_once_with("New content.")
        mock_base_frame.parent.nametowidget.assert_called_once()

    def test_write_text_no_file_stream(self) -> None:
        """Test 'write_text' function when file_stream does not exist and wrong frame is selected"""
        mock_open_file = mock_open()
        mock_base_frame = MagicMock()
        mock_parent = MagicMock()
        mock_parent.nametowidget.return_value = "random_frame"
        mock_base_frame.parent = mock_parent
        with patch("builtins.open", mock_open_file):
            frame_base.BaseFrame.write_text(mock_base_frame, "New content.")
        mock_open_file.assert_called_once_with(
            mock_base_frame.file_path, mode="a", encoding="utf-8", errors="ignore"
        )
        mock_open_handle = mock_open_file()
        mock_open_handle.write.assert_called_once_with("New content.")
        mock_base_frame.parent.nametowidget.assert_called_once()

    def test_write_text_closed_file_stream(self) -> None:
        """Test 'write_text' function when file_stream exists but is closed,
        and wrong frame is selected
        """
        mock_open_file = mock_open()
        mock_base_frame = MagicMock()
        mock_file_stream = MagicMock()
        mock_file_stream.closed = True
        mock_base_frame.file_stream = mock_file_stream
        mock_parent = MagicMock()
        mock_parent.nametowidget.return_value = "random_frame"
        mock_base_frame.parent = mock_parent
        with patch("builtins.open", mock_open_file):
            frame_base.BaseFrame.write_text(mock_base_frame, "New content.")
        mock_open_file.assert_called_once_with(
            mock_base_frame.file_path, mode="a", encoding="utf-8", errors="ignore"
        )
        mock_open_handle = mock_open_file()
        mock_open_handle.write.assert_called_once_with("New content.")
        mock_base_frame.parent.nametowidget.assert_called_once()

    def test_write_text_no_input(self) -> None:
        """Test 'write_text' function when no input given and wrong frame is selected"""
        mock_base_frame = MagicMock()
        mock_parent = MagicMock()
        mock_parent.nametowidget.return_value = "random_frame"
        mock_base_frame.parent = mock_parent
        frame_base.BaseFrame.write_text(mock_base_frame)
        mock_base_frame.parent.nametowidget.assert_called_once()

    def test_on_close_no_file_stream_path(self) -> None:
        """Test 'on_close' function when file_stream and file_path do not exist"""
        mock_base_frame = MagicMock()
        mock_base_frame.file_path.exists.return_value = False
        frame_base.BaseFrame.on_close(mock_base_frame)
        mock_base_frame.file_stream.close.assert_not_called()
        mock_base_frame.file_path.exists.assert_called_once()
        mock_base_frame.file_path.unlink.assert_not_called()

    def test_on_close_file_path(self) -> None:
        """Test 'on_close' function when file_path exists"""
        mock_base_frame = MagicMock()
        mock_base_frame.file_path.exists.return_value = True
        frame_base.BaseFrame.on_close(mock_base_frame)
        mock_base_frame.file_stream.close.assert_not_called()
        mock_base_frame.file_path.exists.assert_called_once()
        mock_base_frame.file_path.unlink.assert_called_once()

    def test_on_close_both_stream_path(self) -> None:
        """Test 'on_close' function when file_path and file_stream exist"""
        mock_base_frame = MagicMock()
        mock_base_frame.file_path.exists.return_value = True
        mock_file_stream = MagicMock()
        mock_file_stream.closed = False
        mock_base_frame.file_stream = mock_file_stream
        frame_base.BaseFrame.on_close(mock_base_frame)
        mock_file_stream.close.assert_called_once()
        mock_base_frame.file_path.exists.assert_called_once()
        mock_base_frame.file_path.unlink.assert_called_once()

    def test_reset_text(self) -> None:
        """Test 'reset_text' function"""
        mock_base_frame = MagicMock()
        frame_base.BaseFrame.reset_text(mock_base_frame)
        self.assertEqual(mock_base_frame.text_index, 0)
        mock_base_frame.text.config.assert_has_calls(
            [call(state=tk.NORMAL), call(state=tk.DISABLED)]
        )
        mock_base_frame.text.delete.assert_called_once_with("1.0", tk.END)


class TestGetLogTail(unittest.TestCase):
    """Test of the 'get_log_tail' function of the BaseFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)

    def test_no_lines(self) -> None:
        """Number of lines is invalid"""
        mock_base_frame = MagicMock()
        result = frame_base.BaseFrame.get_log_tail(mock_base_frame, 0)
        self.assertEqual("", result)

    def test_empty_file(self) -> None:
        """File is empty"""
        mock_base_frame = MagicMock()
        mock_base_frame.file_path = PATH_GUI / "content.txt"
        mock_base_frame.file_path.write_text("", encoding="utf-8")
        result = frame_base.BaseFrame.get_log_tail(mock_base_frame)
        self.assertEqual("", result)

    def test_content(self) -> None:
        """File has content"""
        mock_base_frame = MagicMock()
        mock_base_frame.file_path = PATH_GUI / "content.txt"
        mock_base_frame.file_path.write_text("Content.\n", encoding="utf-8")
        result = frame_base.BaseFrame.get_log_tail(mock_base_frame)
        self.assertEqual("Content.", result)

    def test_long_file(self) -> None:
        """File has more lines than have to be returned"""
        mock_base_frame = MagicMock()
        mock_base_frame.file_path = PATH_GUI / "content.txt"
        mock_base_frame.file_path.write_text(
            "Line 1.\nLine 2.\nLine 3.\nLine 4.\n", encoding="utf-8"
        )
        result = frame_base.BaseFrame.get_log_tail(mock_base_frame, 2)
        self.assertEqual("Line 3.\nLine 4.", result)


def remove_data(start_time: datetime) -> None:
    """Remove all data from the gui directory if it as been created after start_time"""
    if PATH_GUI.is_dir():
        if get_birthtime(PATH_GUI) >= start_time:
            shutil.rmtree(PATH_GUI)
        else:
            children = PATH_GUI.iterdir()
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
