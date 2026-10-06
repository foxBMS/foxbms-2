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

"""Testing file 'cli/cmd_gui/frame_bootloader/bootloader_gui.py'."""

import importlib
import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, call, mock_open, patch

from click import exceptions

try:
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_bootloader import bootloader_gui
    from cli.helpers import io
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_bootloader import bootloader_gui
    from cli.helpers import io
    from cli.helpers.project_context import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "bootloader_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BaseFrame.write_text")
class TestCheckThread(unittest.TestCase):
    """Test of the 'check_thread' function of the BootloaderFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            self.frame = bootloader_gui.BootloaderFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.after")
    def test_check_thread_alive(
        self, mock_after: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test 'check_thread' function when the Thread is still alive"""
        self.frame.bootloader_process = MagicMock()
        self.frame.bootloader_process.is_alive.return_value = True
        self.frame.check_thread()

        mock_after.assert_called_once_with(50, self.frame.check_thread)
        self.frame.bootloader_process.is_alive.assert_called_once()
        mock_write_text.assert_called_once()

    def test_check_thread_dead(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function when the Thread is not alive"""
        self.frame.bootloader_process = MagicMock()
        self.frame.bootloader_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()

        self.frame.bootloader_process.is_alive.assert_called_once()
        self.frame.file_stream.close.assert_called_once()
        mock_write_text.assert_called_once()

    def test_check_thread_dead_file_stream(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function when the Thread is not alive
        and stdout and stderr have to be reset
        """
        self.frame.bootloader_process = MagicMock()
        self.frame.bootloader_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        io.STDOUT = self.frame.file_stream
        io.STDERR = self.frame.file_stream
        self.frame.check_thread()
        self.frame.bootloader_process.is_alive.assert_called_once()
        self.frame.file_stream.close.assert_called_once()
        self.assertIsNone(io.STDERR)
        self.assertIsNone(io.STDOUT)
        mock_write_text.assert_called_once()


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestCallback(unittest.TestCase):
    """Test of all callback functions of the BootloaderFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            self.frame = bootloader_gui.BootloaderFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Path.is_file")
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Thread")
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BootloaderFrame.check_thread")
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BaseFrame.write_text")
    def test_load_app_command_valid_files(
        self,
        mock_write_text: MagicMock,
        mock_check_thread: MagicMock,
        mock_thread: MagicMock,
        mock_is_file: MagicMock,
    ) -> None:
        """Test 'load_app_command_cb' function with only valid files"""
        mock_is_file.return_value = True
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.bus_channel_combobox.insert(tk.END, "channel")
        self.frame.bus_bitrate_combobox.delete(0, tk.END)
        self.frame.bus_bitrate_combobox.insert(tk.END, "500000")
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "interface")
        self.frame.bootloader_dbc_entry.delete(0, tk.END)
        self.frame.bootloader_dbc_entry.insert(tk.END, "bootloader/dbc/file")
        self.frame.app_dbc_entry.delete(0, tk.END)
        self.frame.app_dbc_entry.insert(tk.END, "app/dbc/file")
        self.frame.foxbms_bin_entry.delete(0, tk.END)
        self.frame.foxbms_bin_entry.insert(tk.END, "foxbms/bin/file")
        self.frame.foxbms_crc_csv_entry.delete(0, tk.END)
        self.frame.foxbms_crc_csv_entry.insert(tk.END, "foxbms/crc/csv/file")
        self.frame.foxbms_crc_json_entry.delete(0, tk.END)
        self.frame.foxbms_crc_json_entry.insert(tk.END, "foxbms/crc/json/file")

        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            self.frame.load_app_command_cb()
        mock_open_file.assert_has_calls(
            [
                call(self.frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
                call(self.frame.file_path, mode="a", encoding="utf-8"),
            ]
        )
        self.assertEqual(self.frame.load_app_button.state(), (tk.DISABLED,))
        mock_write_text.assert_called_once_with(
            "Running load-app with interface=interface, "
            "channel=channel, bitrate=500000.\n"
        )
        mock_thread.assert_called_once_with(
            target=self.frame.run_load_app,
            kwargs={
                "kwargs": {
                    "interface": "interface",
                    "channel": "channel",
                    "bitrate": "500000",
                    "bootloader_dbc": Path("bootloader/dbc/file"),
                    "app_dbc": Path("app/dbc/file"),
                    "foxbms_bin": Path("foxbms/bin/file"),
                    "foxbms_app_crc": Path("foxbms/crc/csv/file"),
                    "foxbms_app_info": Path("foxbms/crc/json/file"),
                },
                "timeout": None,
            },
            daemon=True,
        )
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Path.is_file")
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.BaseFrame.write_text")
    def test_load_app_command_invalid_files(
        self, mock_write_text: MagicMock, mock_is_file: MagicMock
    ) -> None:
        """Test 'load_app_command_cb' function with invalid files"""
        mock_is_file.side_effect = [False, True, True, True, True]
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.bus_channel_combobox.insert(tk.END, "channel")
        self.frame.bus_bitrate_combobox.delete(0, tk.END)
        self.frame.bus_bitrate_combobox.insert(tk.END, "500000")
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "interface")
        self.frame.bootloader_dbc_entry.delete(0, tk.END)
        self.frame.bootloader_dbc_entry.insert(tk.END, "bootloader/dbc/file")
        self.frame.app_dbc_entry.delete(0, tk.END)
        self.frame.app_dbc_entry.insert(tk.END, "app/dbc/file")
        self.frame.foxbms_bin_entry.delete(0, tk.END)
        self.frame.foxbms_bin_entry.insert(tk.END, "foxbms/bin/file")
        self.frame.foxbms_crc_csv_entry.delete(0, tk.END)
        self.frame.foxbms_crc_csv_entry.insert(tk.END, "foxbms/crc/csv/file")
        self.frame.foxbms_crc_json_entry.delete(0, tk.END)
        self.frame.foxbms_crc_json_entry.insert(tk.END, "foxbms/crc/json/file")

        self.frame.load_app_command_cb()
        mock_write_text.assert_called_once_with(
            "Invalid input files: Bootloader DBC. \nPlease select existing files.\n"
        )

    def test_change_interface(self) -> None:
        """Test 'change_interface_cb' function"""
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "pcan")
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.change_interface_cb(None)
        self.assertEqual(self.frame.bus_channel_combobox.get(), "PCAN_USBBUS1")

    def test_change_interface_invalid(self) -> None:
        """Test 'change_interface_cb' function for invalid interface"""
        self.frame.bus_interface_combobox.delete(0, tk.END)
        self.frame.bus_interface_combobox.insert(tk.END, "interface")
        self.frame.bus_channel_combobox.delete(0, tk.END)
        self.frame.bus_channel_combobox.insert(tk.END, "channel")
        self.frame.change_interface_cb(None)
        self.assertEqual(self.frame.bus_channel_combobox.get(), "channel")

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function"""
        mock_askopenfilename.return_value = "File Path"
        content = self.frame.bootloader_dbc_entry.get().strip()
        self.frame.select_file_cb("type", self.frame.bootloader_dbc_entry)
        new_content = self.frame.bootloader_dbc_entry.get().strip()
        self.assertEqual("File Path", new_content)
        self.assertNotEqual(content, new_content)
        mock_askopenfilename.assert_called_once_with(
            filetypes=[("TYPE Files", "*.type")]
        )

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_file_empty(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_file_cb' function when askopenfilename returns empty string"""
        mock_askopenfilename.return_value = ""
        content = self.frame.bootloader_dbc_entry.get().strip()
        self.frame.select_file_cb("type", self.frame.bootloader_dbc_entry)
        new_content = self.frame.bootloader_dbc_entry.get().strip()
        self.assertEqual(content, new_content)
        mock_askopenfilename.assert_called_once_with(
            filetypes=[("TYPE Files", "*.type")]
        )


class TestBootloaderFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the BootloaderFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)

    def test_check_thread_alive(self) -> None:
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

    def test_check_thread_dead(self) -> None:
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
    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Path.is_file")
    def test_load_app_command_valid_files(
        self, mock_is_file: MagicMock, mock_thread: MagicMock
    ) -> None:
        """Test 'load_app_command_cb' function with only valid files"""
        mock_is_file.return_value = True
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.file_path = Path(
            PATH_GUI / "output_bootloader_load_app.txt"
        )
        mock_bootloader_frame.bus_channel_combobox.get.return_value = "channel"
        mock_bootloader_frame.bus_bitrate_combobox.get.return_value = "bitrate"
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "interface"
        mock_bootloader_frame.bootloader_dbc_file.get.return_value = (
            "bootloader/dbc/file"
        )
        mock_bootloader_frame.app_dbc_file.get.return_value = "app/dbc/file"
        mock_bootloader_frame.foxbms_bin_file.get.return_value = "foxbms/bin/file"
        mock_bootloader_frame.foxbms_crc_csv.get.return_value = "foxbms/crc/csv/file"
        mock_bootloader_frame.foxbms_crc_json.get.return_value = "foxbms/crc/json/file"

        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            bootloader_gui.BootloaderFrame.load_app_command_cb(mock_bootloader_frame)
        mock_open_file.assert_has_calls(
            [
                call(mock_bootloader_frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
                call(mock_bootloader_frame.file_path, mode="a", encoding="utf-8"),
            ]
        )
        mock_bootloader_frame.load_app_button.state.assert_called_once_with(
            [tk.DISABLED]
        )
        mock_bootloader_frame.write_text.assert_called_once_with(
            "Running load-app with interface=interface, channel=channel, bitrate=bitrate.\n"
        )
        mock_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_bootloader_frame.check_thread.assert_called_once()

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Path.is_file")
    def test_load_app_command_invalid_files(self, mock_is_file: MagicMock) -> None:
        """Test 'load_app_command_cb' function with only valid files"""
        mock_is_file.side_effect = [False, True, True, True, True]
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.file_path = Path(
            PATH_GUI / "output_bootloader_load_app.txt"
        )
        mock_bootloader_frame.bus_channel_combobox.get.return_value = "channel"
        mock_bootloader_frame.bus_bitrate_combobox.get.return_value = "bitrate"
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "interface"
        mock_bootloader_frame.bootloader_dbc_file.get.return_value = (
            "bootloader/dbc/file"
        )
        mock_bootloader_frame.app_dbc_file.get.return_value = "app/dbc/file"
        mock_bootloader_frame.foxbms_bin_file.get.return_value = "foxbms/bin/file"
        mock_bootloader_frame.foxbms_crc_csv.get.return_value = "foxbms/crc/csv/file"
        mock_bootloader_frame.foxbms_crc_json.get.return_value = "foxbms/crc/json/file"

        bootloader_gui.BootloaderFrame.load_app_command_cb(mock_bootloader_frame)
        mock_bootloader_frame.write_text.assert_called_once_with(
            "Invalid input files: Bootloader DBC. \nPlease select existing files.\n"
        )

    def test_change_interface(self) -> None:
        """Test 'change_interface_cb' function"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "pcan"
        mock_bootloader_frame.bus_channel_combobox = MagicMock()
        bootloader_gui.BootloaderFrame.change_interface_cb(mock_bootloader_frame, None)
        mock_bootloader_frame.bus_channel_combobox.set.assert_called_once_with(
            "PCAN_USBBUS1"
        )

    def test_change_interface_invalid(self) -> None:
        """Test 'change_interface_cb' function for invalid interface"""
        mock_bootloader_frame = MagicMock()
        mock_bootloader_frame.bus_interface_combobox.get.return_value = "interface"
        mock_bootloader_frame.bus_channel_combobox = MagicMock()
        bootloader_gui.BootloaderFrame.change_interface_cb(mock_bootloader_frame, None)
        mock_bootloader_frame.bus_channel_combobox.set.assert_not_called()


@patch("cli.cmd_gui.frame_bootloader.bootloader_gui.cmd_load_app")
@patch("cli.cmd_gui.frame_bootloader.bootloader_gui.click")
class TestRunLoadApp(unittest.TestCase):
    """Test of the 'run_load_app' function"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        cls.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        remove_data(cls.start_time)

    def test_run_load_app(
        self, mock_click: MagicMock, mock_load_app: MagicMock
    ) -> None:
        """Test 'run_load_app' function without Exception"""
        mock_bootloader_frame = MagicMock()
        mock_redirect_io = MagicMock()
        # pylint: disable-next=protected-access
        mock_bootloader_frame._redirect_io = mock_redirect_io
        mock_context = MagicMock()
        mock_click.Context.return_value = mock_context
        kwargs = {
            "interface": "virtual",
            "channel": "channel",
            "bitrate": "500000",
            "bootloader_dbc": "bootloader/dbc/file",
            "app_dbc": "app/dbc/file",
            "foxbms_bin": "foxbms/bin/file",
            "foxbms_app_crc": "foxbms/crc/csv/file",
            "foxbms_app_info": "foxbms/crc/json/file",
        }
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame,
            kwargs,
            None,
        )
        mock_context.invoke.assert_called_once_with(mock_load_app, **kwargs)
        mock_redirect_io.return_value.__enter__.assert_called_once_with()
        mock_redirect_io.return_value.__exit__.assert_called_once_with(None, None, None)
        mock_bootloader_frame.write_text.assert_not_called()

    def test_run_load_app_exit(
        self, mock_click: MagicMock, mock_load_app: MagicMock
    ) -> None:
        """Test 'run_load_app' function when 'cmd_load_app' throws exceptions.Exit"""
        mock_bootloader_frame = MagicMock()
        mock_redirect_io = MagicMock()
        # pylint: disable-next=protected-access
        mock_bootloader_frame._redirect_io = mock_redirect_io
        mock_context = MagicMock()
        mock_invoke = MagicMock(side_effect=exceptions.Exit(0))
        mock_context.invoke = mock_invoke
        mock_click.Context.return_value = mock_context
        kwargs = {
            "interface": "virtual",
            "channel": "channel",
            "bitrate": "500000",
            "bootloader_dbc": "bootloader/dbc/file",
            "app_dbc": "app/dbc/file",
            "foxbms_bin": "foxbms/bin/file",
            "foxbms_app_crc": "foxbms/crc/csv/file",
            "foxbms_app_info": "foxbms/crc/json/file",
        }
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame,
            kwargs,
            None,
        )
        mock_invoke.assert_called_once_with(mock_load_app, **kwargs)
        mock_redirect_io.return_value.__enter__.assert_called_once_with()
        mock_redirect_io.return_value.__exit__.assert_called_once_with(None, None, None)
        mock_bootloader_frame.write_text.assert_called_once()

    def test_run_load_app_timeout(
        self, mock_click: MagicMock, mock_load_app: MagicMock
    ) -> None:
        """Test 'run_load_app' function with timeout given"""
        mock_bootloader_frame = MagicMock()
        mock_redirect_io = MagicMock()
        # pylint: disable-next=protected-access
        mock_bootloader_frame._redirect_io = mock_redirect_io
        mock_context = MagicMock()
        mock_click.Context.return_value = mock_context
        kwargs = {
            "interface": "virtual",
            "channel": "channel",
            "bitrate": "500000",
            "bootloader_dbc": "bootloader/dbc/file",
            "app_dbc": "app/dbc/file",
            "foxbms_bin": "foxbms/bin/file",
            "foxbms_app_crc": "foxbms/crc/csv/file",
            "foxbms_app_info": "foxbms/crc/json/file",
        }
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame,
            kwargs,
            "timeout",
        )
        mock_context.invoke.assert_called_once_with(mock_load_app, **kwargs)
        mock_redirect_io.return_value.__enter__.assert_called_once_with()
        mock_redirect_io.return_value.__exit__.assert_called_once_with(None, None, None)
        mock_bootloader_frame.write_text.assert_not_called()

    def test_run_load_app_redirect_io(
        self, mock_click: MagicMock, mock_load_app: MagicMock
    ) -> None:
        """Test 'run_load_app' function without Exception"""
        mock_bootloader_frame = MagicMock()
        file_stream = MagicMock()
        mock_bootloader_frame.file_stream = file_stream
        # pylint: disable-next=protected-access
        mock_bootloader_frame._redirect_io = bootloader_gui.BootloaderFrame._redirect_io
        mock_context = MagicMock()
        mock_click.Context.return_value = mock_context
        kwargs = {
            "interface": "virtual",
            "channel": "channel",
            "bitrate": "500000",
            "bootloader_dbc": "bootloader/dbc/file",
            "app_dbc": "app/dbc/file",
            "foxbms_bin": "foxbms/bin/file",
            "foxbms_app_crc": "foxbms/crc/csv/file",
            "foxbms_app_info": "foxbms/crc/json/file",
        }
        bootloader_gui.BootloaderFrame.run_load_app(
            mock_bootloader_frame,
            kwargs,
            None,
        )
        mock_context.invoke.assert_called_once_with(mock_load_app, **kwargs)
        mock_bootloader_frame.write_text.assert_not_called()


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestBootloaderFrameInit(unittest.TestCase):
    """Test initialization of the BootloaderFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        cls.start_time = datetime.now(tz=UTC)
        cls.root = tk.Tk()
        cls.root.withdraw()
        cls.text = tk.Text()

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        cls.root.update()
        cls.root.destroy()
        remove_data(cls.start_time)
        importlib.reload(frame_base)

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Path.is_file")
    def test_files(self, mock_is_file: MagicMock) -> None:
        """Test initialization when the files exist"""
        mock_is_file.return_value = True
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            bootloader_frame = bootloader_gui.BootloaderFrame(self.root, self.text)
        self.assertNotEqual("", bootloader_frame.bootloader_dbc_entry.get().strip())
        self.assertNotEqual("", bootloader_frame.app_dbc_entry.get().strip())
        self.assertNotEqual("", bootloader_frame.foxbms_bin_entry.get().strip())
        self.assertNotEqual("", bootloader_frame.foxbms_crc_csv_entry.get().strip())
        self.assertNotEqual("", bootloader_frame.foxbms_crc_json_entry.get().strip())

    @patch("cli.cmd_gui.frame_bootloader.bootloader_gui.Path.is_file")
    def test_no_files(self, mock_is_file: MagicMock) -> None:
        """Test initialization when the files do not exist"""
        mock_is_file.return_value = False
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            bootloader_frame = bootloader_gui.BootloaderFrame(self.root, self.text)
        self.assertEqual("", bootloader_frame.bootloader_dbc_entry.get().strip())
        self.assertEqual("", bootloader_frame.app_dbc_entry.get().strip())
        self.assertEqual("", bootloader_frame.foxbms_bin_entry.get().strip())
        self.assertEqual("", bootloader_frame.foxbms_crc_csv_entry.get().strip())
        self.assertEqual("", bootloader_frame.foxbms_crc_json_entry.get().strip())


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
