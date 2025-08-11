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

"""Testing file 'cli/cmd_gui/frame_bootloader/bootloader_gui.py'."""

import os
import sys
import tkinter as tk
import unittest
from datetime import datetime
from pathlib import Path
from unittest.mock import MagicMock, patch

from click import exceptions

try:
    from cli.cmd_gui.frame_bootloader import bootloader_gui
    from cli.helpers import io
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_bootloader import bootloader_gui
    from cli.helpers import io
    from cli.helpers.misc import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestBootloaderFrame(unittest.TestCase):
    """Test of the BootloaderFrame class"""

    def setUp(self):
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = bootloader_gui.BootloaderFrame(self.root, text)
        self.start_time = datetime.now()

    def tearDown(self):
        self.root.update()
        self.root.destroy()
        io.STDERR = None
        io.STDOUT = None
        file_output = Path(PROJECT_BUILD_ROOT / "output_gui_bootloader.txt")
        if file_output.exists():
            file_output.unlink()

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.frame.file_path.touch()
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("New content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)

    def test_write_text_not_selected(self):
        """Test 'write_text' function when BootloaderFrame is not selected"""
        mock_select = MagicMock()
        mock_select.return_value = ""
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text_input(self):
        """Test 'write_text' function when a string is passed"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.frame.write_text(file_input="New content.")
        self.assertEqual("New content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.after")
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.write_text")
    def test_check_thread_alive(
        self, mock_write_text: MagicMock, mock_after: MagicMock
    ):
        """Test 'check_thread' function when the Thread is still alive"""
        self.frame.bootloader_process = MagicMock()
        self.frame.bootloader_process.is_alive.return_value = True
        self.frame.check_thread()

        mock_after.assert_called_once_with(50, self.frame.check_thread)
        self.frame.bootloader_process.is_alive.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.write_text")
    def test_check_thread_dead(self, mock_write_text: MagicMock):
        """Test 'check_thread' function when the Thread is not alive"""
        self.frame.bootloader_process = MagicMock()
        self.frame.bootloader_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()

        self.frame.bootloader_process.is_alive.assert_called_once()
        self.frame.file_stream.close.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.write_text")
    def test_check_thread_dead_file_stream(self, mock_write_text: MagicMock):
        """Test 'check_thread' function when the Thread is not alive
        and stdout and stderr have to be reset"""
        self.frame.bootloader_process = MagicMock()
        self.frame.bootloader_process.is_alive.return_value = False
        # pylint: disable-next=consider-using-with
        self.frame.file_stream = open(self.frame.file_path, mode="w", encoding="utf-8")
        io.STDOUT = self.frame.file_stream
        io.STDERR = self.frame.file_stream
        self.frame.check_thread()
        self.frame.bootloader_process.is_alive.assert_called_once()
        self.assertTrue(self.frame.file_stream.closed)
        self.assertIsNone(io.STDERR)
        self.assertIsNone(io.STDOUT)
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Thread")
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.check_thread")
    def test_load_app_command_cb(
        self, mock_check_thread: MagicMock, mock_thread: MagicMock
    ):
        """Test 'load_app_command_cb' function"""
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.bus_channel_combobox.insert(tk.END, "channel")
        self.frame.bus_bitrate_combobox.delete(0, tk.END)
        self.frame.bus_bitrate_combobox.insert(tk.END, "500000")
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "interface")

        self.frame.load_app_command_cb()
        self.frame.file_stream.close()
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()
        self.assertEqual("Started the load process\n", self.frame.file_path.read_text())
        mock_thread.assert_called_once_with(
            target=self.frame.run_load_app,
            kwargs={
                "interface": "interface",
                "channel": "channel",
                "timeout": None,
                "bitrate": "500000",
            },
            daemon=True,
        )

    def test_change_interface_cb(self):
        """Test 'change_interface_cb' function"""
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "pcan")
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.change_interface_cb(None)
        self.assertEqual(self.frame.bus_channel_combobox.get(), "PCAN_USBBUS1")

    def test_change_interface_cb_invalid(self):
        """Test 'change_interface_cb' function for invalid interface"""
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "interface")
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.bus_channel_combobox.insert(tk.END, "channel")
        self.frame.change_interface_cb(None)
        self.assertEqual(self.frame.bus_channel_combobox.get(), "channel")


class TestBootloaderFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the BootloaderFrame class"""

    def setUp(self):
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        io.STDERR = None
        io.STDOUT = None
        for file_path in PROJECT_BUILD_ROOT.glob("output_bootloader_*"):
            if file_path.suffix == ".txt" and file_path.exists():
                file_path.unlink()

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.parent.nametowidget.return_value = mock_bootloader_frame
        mock_bootloader_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_bootloader_write_text_empty.txt"
        )
        mock_bootloader_frame.text = MagicMock()
        mock_bootloader_frame.text_index = 0
        mock_bootloader_frame.file_path.touch()
        bootloader_gui.BootloaderFrame.write_text(mock_bootloader_frame)
        mock_bootloader_frame.text.insert.assert_called_once_with(tk.END, "")
        self.assertEqual(mock_bootloader_frame.text_index, 0)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.parent.nametowidget.return_value = mock_bootloader_frame
        mock_bootloader_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_bootloader_write_text.txt"
        )
        mock_bootloader_frame.text = MagicMock()
        mock_bootloader_frame.text_index = 0
        mock_bootloader_frame.file_path.write_text("New content.", encoding="utf-8")
        bootloader_gui.BootloaderFrame.write_text(mock_bootloader_frame)
        mock_bootloader_frame.text.insert.assert_called_once_with(
            tk.END, "New content."
        )
        self.assertEqual(mock_bootloader_frame.text_index, 12)

    def test_write_text_not_selected(self):
        """Test 'write_text' function when BootloaderFrame is not selected"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.parent.select.return_value = ""
        mock_bootloader_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_bootloader_write_text.txt"
        )
        mock_bootloader_frame.text = MagicMock()
        mock_bootloader_frame.text_index = 0
        mock_bootloader_frame.file_path.write_text("New content.", encoding="utf-8")
        bootloader_gui.BootloaderFrame.write_text(mock_bootloader_frame)
        mock_bootloader_frame.text.insert.assert_not_called()
        self.assertEqual(mock_bootloader_frame.text_index, 0)

    def test_write_text_input(self):
        """Test 'write_text' function when a string is passed"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.parent.nametowidget.return_value = mock_bootloader_frame
        mock_bootloader_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_bootloader_write_text.txt"
        )
        mock_bootloader_frame.text = MagicMock()
        mock_bootloader_frame.text_index = 0
        mock_bootloader_frame.file_path.touch()
        bootloader_gui.BootloaderFrame.write_text(mock_bootloader_frame, "New content.")
        mock_bootloader_frame.text.insert.assert_called_once_with(
            tk.END, "New content."
        )
        self.assertEqual(mock_bootloader_frame.text_index, 12)

    def test_check_thread_alive(self):
        """Test 'check_thread' function when the Thread is still alive"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.bootloader_process = MagicMock()
        mock_bootloader_frame.bootloader_process.is_alive.return_value = True
        bootloader_gui.BootloaderFrame.check_thread(mock_bootloader_frame)
        mock_bootloader_frame.after.assert_called_once_with(
            50, mock_bootloader_frame.check_thread
        )
        mock_bootloader_frame.bootloader_process.is_alive.assert_called_once()
        mock_bootloader_frame.write_text.assert_called_once()

    def test_check_thread_dead(self):
        """Test 'check_thread' function when the Thread is not alive"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.bootloader_process = MagicMock()
        mock_bootloader_frame.bootloader_process.is_alive.return_value = False
        bootloader_gui.BootloaderFrame.check_thread(mock_bootloader_frame)
        mock_bootloader_frame.after.assert_not_called()
        mock_bootloader_frame.bootloader_process.is_alive.assert_called_once()
        mock_bootloader_frame.write_text.assert_called_once()
        mock_bootloader_frame.file_stream.close.assert_called_once()
        self.assertIsNone(io.STDERR)
        self.assertIsNone(io.STDOUT)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Thread")
    def test_load_app_command_cb(self, mock_thread: MagicMock):
        """Test 'load_app_command_cb' function"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_bootloader_load_app.txt"
        )
        mock_bootloader_frame.bus_channel_combobox.get.return_value = "channel"
        mock_bootloader_frame.bus_bitrate_combobox.get.return_value = "bitrate"
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "interface"

        bootloader_gui.BootloaderFrame.load_app_command_cb(mock_bootloader_frame)
        mock_bootloader_frame.file_stream.close()
        mock_thread.return_value.start.assert_called_once()
        mock_bootloader_frame.check_thread.assert_called_once()
        self.assertEqual(
            "Started the load process\n",
            mock_bootloader_frame.file_path.read_text(encoding="utf-8"),
        )

    def test_change_interface_cb(self):
        """Test 'change_interface_cb' function"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "pcan"
        mock_bootloader_frame.bus_channel_combobox = MagicMock()
        bootloader_gui.BootloaderFrame.change_interface_cb(mock_bootloader_frame, None)
        mock_bootloader_frame.bus_channel_combobox.set.assert_called_once_with(
            "PCAN_USBBUS1"
        )

    def test_change_interface_cb_invalid(self):
        """Test 'change_interface_cb' function for invalid interface"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "interface"
        mock_bootloader_frame.bus_channel_combobox = MagicMock()
        bootloader_gui.BootloaderFrame.change_interface_cb(mock_bootloader_frame, None)
        mock_bootloader_frame.bus_channel_combobox.set.assert_not_called()


class TestRunLoadApp(unittest.TestCase):
    """Test of the 'run_load_app' function"""

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.cmd_load_app")
    def test_run_load_app(self, mock_load_app: MagicMock):
        """Test 'run_load_app' function"""
        mock_bootloader_frame = MagicMock()
        kwargs = {"interface": "virtual", "channel": "channel", "bitrate": "500000"}
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame, None, "virtual", "channel", "500000"
        )
        mock_load_app.assert_called_once_with(**kwargs)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.cmd_load_app")
    def test_run_load_app_exit_0(self, mock_load_app: MagicMock):
        """'run_load_app' function throws an Exit exception with
        return_code 0"""
        mock_bootloader_frame = MagicMock()
        kwargs = {"interface": "virtual", "channel": "channel", "bitrate": "500000"}
        mock_load_app.side_effect = exceptions.Exit(0)
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame, None, "virtual", "channel", "500000"
        )
        mock_load_app.assert_called_once_with(**kwargs)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.cmd_load_app")
    def test_run_load_app_exit_1(self, mock_load_app: MagicMock):
        """'run_load_app' function throws an Exit exception with
        return_code 1"""
        mock_bootloader_frame = MagicMock()
        kwargs = {"interface": "virtual", "channel": "channel", "bitrate": "500000"}
        mock_load_app.side_effect = exceptions.Exit(1)
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame, None, "virtual", "channel", "500000"
        )
        mock_load_app.assert_called_once_with(**kwargs)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.cmd_load_app")
    def test_run_load_app_timeout(self, mock_load_app: MagicMock):
        """Timeout is given"""
        mock_bootloader_frame = MagicMock()
        kwargs = {
            "timeout": "timeout",
            "interface": "virtual",
            "channel": "channel",
            "bitrate": "500000",
        }
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame, "timeout", "virtual", "channel", "500000"
        )
        mock_load_app.assert_called_once_with(**kwargs)


if __name__ == "__main__":
    unittest.main()
