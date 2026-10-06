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

"""Testing file 'cli/cmd_gui/frame_build/build_gui.py'."""

import importlib
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
    from cli.cmd_gui.frame_build import build_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_build import build_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "build_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_build.build_gui.BaseFrame.write_text")
class TestCheckThread(unittest.TestCase):
    """Test of the 'check_thread' function of the BuildFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            self.frame = build_gui.BuildFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.after")
    def test_check_thread_alive(
        self, mock_after: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test 'check_thread' function when the Thread is alive"""
        self.frame.current_command = "command"
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = True
        self.frame.check_thread()

        mock_after.assert_called_once_with(50, self.frame.check_thread)
        self.frame.build_process.is_alive.assert_called_once()
        mock_write_text.assert_called_once()

    def test_check_thread_dead_empty(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and result_queue is empty
        """
        self.frame.current_command = "command"
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        self.frame.result_queue = MagicMock()
        self.frame.result_queue.empty.return_value = True
        self.frame.check_thread()

        mock_write_text.assert_called_once()
        self.frame.build_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.assertEqual(self.frame.command_list_button.state(), ())
        self.assertEqual(self.frame.run_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.result_queue.empty.assert_called_once()

    def test_check_thread_dead_success(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and the command was successful
        """
        self.frame.current_command = "command"
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        self.frame.result_queue = MagicMock()
        self.frame.result_queue.empty.return_value = False
        mock_queue_element = MagicMock()
        mock_queue_element.returncode = 0
        self.frame.result_queue.get.return_value = mock_queue_element
        self.frame.check_thread()

        mock_write_text.assert_called_once()
        self.frame.build_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.assertEqual(self.frame.command_list_button.state(), ())
        self.assertEqual(self.frame.run_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.result_queue.empty.assert_called_once()
        self.frame.result_queue.get.assert_called_once()
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"),
            "green",
        )
        self.assertEqual(
            self.frame.status_label.cget("text"), "Command 'command' was successful."
        )

    def test_check_thread_dead_failure(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and the command failed
        """
        self.frame.current_command = "command"
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        self.frame.result_queue = MagicMock()
        self.frame.result_queue.empty.return_value = False
        mock_queue_element = MagicMock()
        mock_queue_element.returncode = 1
        self.frame.result_queue.get.return_value = mock_queue_element
        self.frame.check_thread()

        mock_write_text.assert_called_once()
        self.frame.build_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.assertEqual(self.frame.command_list_button.state(), ())
        self.assertEqual(self.frame.run_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.result_queue.empty.assert_called_once()
        self.frame.result_queue.get.assert_called_once()
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"), "red"
        )
        self.assertEqual(
            self.frame.status_label.cget("text"), "Command 'command' failed."
        )

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.generate_command_list")
    def test_check_thread_command_list(
        self, mock_generate_list: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test 'check_thread' function when generating the command list"""
        self.frame.current_command = "Generate Command List"
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.file_stream = MagicMock()
        self.frame.result_queue = MagicMock()
        self.frame.result_queue.empty.return_value = False
        mock_queue_element = MagicMock()
        mock_queue_element.returncode = 0
        self.frame.result_queue.get.return_value = mock_queue_element
        self.frame.check_thread()

        mock_write_text.assert_not_called()
        self.frame.build_process.is_alive.assert_called_once()
        mock_generate_list.assert_called_once()
        self.assertEqual(self.frame.save_log_button.state(), (tk.DISABLED,))
        self.assertEqual(self.frame.command_list_button.state(), ())
        self.assertEqual(self.frame.run_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.result_queue.empty.assert_called_once()
        self.frame.result_queue.get.assert_called_once()
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"),
            "green",
        )
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Command 'Generate Command List' was successful.",
        )


class TestCheckThreadNoUiTestableMethods(unittest.TestCase):
    """Test of the 'check_thread' function of the BuildFrame class"""

    def test_check_thread_alive(self) -> None:
        """Test 'check_thread' function when the Thread is alive"""
        mock_build_frame = MagicMock()
        mock_build_frame.current_command = "command"
        mock_is_alive = MagicMock(return_value=True)
        mock_build_frame.build_process.is_alive = mock_is_alive
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.write_text.assert_called_once()
        mock_is_alive.assert_called_once()
        mock_build_frame.after.assert_called_once_with(
            50, mock_build_frame.check_thread
        )

    def test_check_thread_dead_empty(self) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and result_queue is empty
        """
        mock_build_frame = MagicMock()
        mock_build_frame.current_command = "command"
        mock_is_alive = MagicMock(return_value=False)
        mock_build_frame.build_process.is_alive = mock_is_alive
        mock_empty = MagicMock(return_value=True)
        mock_build_frame.result_queue.empty = mock_empty
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.write_text.assert_called_once()
        mock_is_alive.assert_called_once()
        mock_build_frame.save_log_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.command_list_button.state.assert_called_once_with(
            ["!disabled"]
        )
        mock_build_frame.run_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.file_stream.close.assert_called_once()
        mock_empty.assert_called_once()

    def test_check_thread_dead_success(self) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and the command was successful
        """
        mock_build_frame = MagicMock()
        mock_build_frame.current_command = "command"
        mock_is_alive = MagicMock(return_value=False)
        mock_build_frame.build_process.is_alive = mock_is_alive
        mock_empty = MagicMock(return_value=False)
        mock_build_frame.result_queue.empty = mock_empty
        mock_get = MagicMock(return_value=MagicMock(returncode=0))
        mock_build_frame.result_queue.get = mock_get
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.write_text.assert_called_once()
        mock_is_alive.assert_called_once()
        mock_build_frame.save_log_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.command_list_button.state.assert_called_once_with(
            ["!disabled"]
        )
        mock_build_frame.run_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.file_stream.close.assert_called_once()
        mock_empty.assert_called_once()
        mock_get.assert_called_once()
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="green"
        )
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Command 'command' was successful."
        )

    def test_check_thread_dead_failure(self) -> None:
        """Test 'check_thread' function
        when the Thread is not alive and the command failed
        """
        mock_build_frame = MagicMock()
        mock_build_frame.current_command = "command"
        mock_is_alive = MagicMock(return_value=False)
        mock_build_frame.build_process.is_alive = mock_is_alive
        mock_empty = MagicMock(return_value=False)
        mock_build_frame.result_queue.empty = mock_empty
        mock_get = MagicMock(return_value=MagicMock(returncode=1))
        mock_build_frame.result_queue.get = mock_get
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.write_text.assert_called_once()
        mock_is_alive.assert_called_once()
        mock_build_frame.save_log_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.command_list_button.state.assert_called_once_with(
            ["!disabled"]
        )
        mock_build_frame.run_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.file_stream.close.assert_called_once()
        mock_empty.assert_called_once()
        mock_get.assert_called_once()
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="red"
        )
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Command 'command' failed."
        )

    def test_check_thread_command_list(self) -> None:
        """Test 'check_thread' function when generating the command list"""
        mock_build_frame = MagicMock()
        mock_build_frame.current_command = "Generate Command List"
        mock_is_alive = MagicMock(return_value=False)
        mock_build_frame.build_process.is_alive = mock_is_alive
        mock_empty = MagicMock(return_value=False)
        mock_build_frame.result_queue.empty = mock_empty
        mock_get = MagicMock(return_value=MagicMock(returncode=0))
        mock_build_frame.result_queue.get = mock_get
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.write_text.assert_not_called()
        mock_is_alive.assert_called_once()
        mock_build_frame.generate_command_list.assert_called_once()
        mock_build_frame.save_log_button.state.assert_not_called()
        mock_build_frame.command_list_button.state.assert_called_once_with(
            ["!disabled"]
        )
        mock_build_frame.run_button.state.assert_called_once_with(["!disabled"])
        mock_build_frame.file_stream.close.assert_called_once()
        mock_empty.assert_called_once()
        mock_get.assert_called_once()
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="green"
        )
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Command 'Generate Command List' was successful."
        )


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestCallback(unittest.TestCase):
    """Test of all callback functions of the BuildFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            self.frame = build_gui.BuildFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.reset_text")
    def test_run_command_no_selection(self, mock_reset: MagicMock) -> None:
        """Test 'run_command_cb' function when no command is selected"""
        self.frame.commands_listbox.curselection = MagicMock(return_value=None)
        self.frame.run_command_cb()

        mock_reset.assert_not_called()
        self.assertEqual(
            self.frame.status_label.cget("text"), "Please select a command."
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"),
            "lightgrey",
        )

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.check_thread")
    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.reset_text")
    def test_run_command(
        self,
        mock_reset: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
    ) -> None:
        """Test 'run_command_cb' function"""
        self.frame.commands_listbox.curselection = MagicMock(return_value=(0,))
        self.frame.reduced_commands = [build_gui.Command("command", "help")]
        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            self.frame.run_command_cb()

        mock_reset.assert_called_once()
        self.assertEqual(self.frame.run_button.state(), (tk.DISABLED,))
        self.assertEqual(self.frame.command_list_button.state(), (tk.DISABLED,))
        self.assertEqual(
            self.frame.status_label.cget("text"), "Running command 'command'."
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"),
            "darkgrey",
        )
        mock_open_file.assert_called_once_with(
            self.frame.file_path, mode="w", encoding="utf-8"
        )
        self.assertEqual("command", self.frame.current_command)
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()

    def test_generate_command_list(self) -> None:
        """Test 'generate_command_list' function"""
        mock_open_file = mock_open(
            read_data="Main commands \ncommand :\ncommand : 1\n : command 2\ncommand 3\noptions:"
        )
        self.frame.commands_listbox.insert(tk.END, "Old Command", "Another old Command")
        self.frame.commands = ["First Command", "Second Command"]
        self.frame.reduced_commands = ["first command"]
        new_commands = [
            build_gui.Command(name="command", help=""),
            build_gui.Command(name="command", help="1"),
        ]
        with patch("builtins.open", mock_open_file):
            self.frame.generate_command_list()
        self.assertListEqual(self.frame.commands, new_commands)
        self.assertListEqual(self.frame.reduced_commands, new_commands)
        self.assertEqual(
            self.frame.commands_listbox.get(0, tk.END), ("command  ()", "command  (1)")
        )
        mock_open_file.assert_any_call(self.frame.file_path, encoding="utf-8")
        mock_open_file.assert_has_calls(
            [call(self.frame.file_path, mode="w", encoding="utf-8"), call().close()]
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"),
            "lightgrey",
        )
        self.assertEqual(
            self.frame.status_label.cget("text"), "Select a command to run."
        )

    def test_generate_command_list_invalid_file(self) -> None:
        """Test 'generate_command_list' function when file contains no commands"""
        mock_open_file = mock_open(
            read_data="command :\ncommand : 1\n : command 2\ncommand 3\n"
        )
        self.frame.commands_listbox.insert(tk.END, "Old Command", "Another old Command")
        self.frame.commands = ["First Command", "Second Command"]
        self.frame.reduced_commands = ["first command"]
        new_commands = []
        with patch("builtins.open", mock_open_file):
            self.frame.generate_command_list()
        self.assertListEqual(self.frame.commands, new_commands)
        self.assertListEqual(self.frame.reduced_commands, new_commands)
        self.assertEqual(self.frame.commands_listbox.get(0, tk.END), ())
        mock_open_file.assert_any_call(self.frame.file_path, encoding="utf-8")
        mock_open_file.assert_has_calls(
            [call(self.frame.file_path, mode="w", encoding="utf-8"), call().close()]
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"), "red"
        )
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Could not parse command list from 'waf --help' output.",
        )

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.check_thread")
    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.reset_text")
    def test_generate_command_list_cb(
        self,
        mock_reset: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
    ) -> None:
        """Test 'generate_command_list_command_cb' function"""
        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            self.frame.generate_command_list_cb()

        self.assertEqual(self.frame.current_command, "Generate Command List")
        mock_reset.assert_called_once()
        self.assertEqual(self.frame.run_button.state(), (tk.DISABLED,))
        self.assertEqual(self.frame.command_list_button.state(), (tk.DISABLED,))
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Running command 'Generate Command List'.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, option="fill"),
            "darkgrey",
        )
        mock_open_file.assert_called_once_with(
            self.frame.file_path, mode="w", encoding="utf-8"
        )
        mock_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()

    def test_select_command(self) -> None:
        """Test 'select_command_cb' function"""
        self.frame.search_command_entry.delete(0, tk.END)
        self.frame.search_command_entry.insert(tk.END, "command")
        command_1 = build_gui.Command("command_1", "")
        command_2 = build_gui.Command("command_2", "")
        command_3 = build_gui.Command("Command_3", "")
        command_4 = build_gui.Command(name="not_relevant", help="")
        self.frame.commands = [command_1, command_2, command_3, command_4]
        self.frame.reduced_commands = [command_4]
        self.frame.commands_listbox.insert(tk.END, "old command  (help text)")
        self.frame.select_command_cb("<KeyRelease>")
        self.assertListEqual(
            self.frame.reduced_commands, [command_1, command_2, command_3]
        )
        self.assertEqual(
            self.frame.commands_listbox.get(0, tk.END),
            ("command_1  ()", "command_2  ()", "Command_3  ()"),
        )


class TestCallbackNoUiTestableMethods(unittest.TestCase):
    """Test of all callback functions of the BuildFrame class"""

    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    def test_run_command_no_selection(self, mock_thread: MagicMock) -> None:
        """Test 'run_command_cb' function when no command is selected"""
        mock_build_frame = MagicMock()
        mock_build_frame.commands_listbox.curselection = MagicMock(return_value=None)
        build_gui.BuildFrame.run_command_cb(mock_build_frame)

        mock_build_frame.commands_listbox.curselection.assert_called_once()
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Please select a command."
        )
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="lightgrey"
        )
        mock_thread.assert_not_called()

    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    def test_run_command(self, mock_thread: MagicMock) -> None:
        """Test 'run_command_cb' function"""
        mock_build_frame = MagicMock()
        mock_build_frame.commands_listbox.curselection = MagicMock(return_value=(0,))
        mock_build_frame.reduced_commands = [build_gui.Command("command", "help")]
        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            build_gui.BuildFrame.run_command_cb(mock_build_frame)

        mock_build_frame.reset_text.assert_called_once()
        mock_build_frame.run_button.state.assert_called_once_with([tk.DISABLED])
        mock_build_frame.command_list_button.state.assert_called_once_with(
            [tk.DISABLED]
        )
        self.assertEqual(mock_build_frame.current_command, "command")
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Running command 'command'."
        )
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="darkgrey"
        )
        mock_open_file.assert_called_once_with(
            mock_build_frame.file_path, mode="w", encoding="utf-8"
        )
        mock_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_build_frame.check_thread.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.file_name_from_current_time")
    def test_save_log(self, mock_time: MagicMock) -> None:
        """Test 'save_log_cb' function"""
        mock_build_frame = MagicMock()
        mock_build_frame.current_command = "command"
        mock_time.return_value = "new"
        file_name = PROJECT_BUILD_ROOT / "gui" / "command_new.txt"
        mock_open_file = mock_open(read_data="log")
        with patch("builtins.open", mock_open_file):
            build_gui.BuildFrame.save_log_cb(mock_build_frame)
        mock_time.assert_called_once()
        mock_open_file.assert_any_call(
            mock_build_frame.file_path, encoding="utf-8", errors="ignore"
        )
        mock_open_file.assert_any_call(
            file_name, mode="w", encoding="utf-8", errors="ignore"
        )
        mock_open_stream = mock_open_file()
        mock_open_stream.write.assert_called_once_with("log")

    def test_generate_command_list(self) -> None:
        """Test 'generate_command_list' function"""
        mock_build_frame = MagicMock()
        mock_build_frame.commands = []
        mock_open_file = mock_open(
            read_data="Main commands \ncommand :\ncommand : 1\n : command 2\ncommand 3\noptions:"
        )
        with patch("builtins.open", mock_open_file):
            build_gui.BuildFrame.generate_command_list(mock_build_frame)
        mock_open_file.assert_any_call(mock_build_frame.file_path, encoding="utf-8")
        mock_open_file.assert_has_calls(
            [
                call(mock_build_frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
            ]
        )
        mock_build_frame.commands_listbox.insert.assert_has_calls(
            [call(tk.END, "command  ()"), call(tk.END, "command  (1)")]
        )
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Select a command to run."
        )
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="lightgrey"
        )

    def test_generate_command_list_invalid_file(self) -> None:
        """Test 'generate_command_list' function when file contains no commands"""
        mock_build_frame = MagicMock()
        mock_build_frame.commands = []
        mock_open_file = mock_open(
            read_data="command :\ncommand : 1\n : command 2\ncommand 3\noptions:"
        )
        with patch("builtins.open", mock_open_file):
            build_gui.BuildFrame.generate_command_list(mock_build_frame)
        mock_open_file.assert_any_call(mock_build_frame.file_path, encoding="utf-8")
        mock_open_file.assert_has_calls(
            [
                call(mock_build_frame.file_path, mode="w", encoding="utf-8"),
                call().close(),
            ]
        )
        mock_build_frame.commands_listbox.insert.assert_not_called()
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Could not parse command list from 'waf --help' output."
        )
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="red"
        )

    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    def test_generate_command_list_cb(self, mock_thread: MagicMock) -> None:
        """Test 'generate_command_list_command_cb' function"""
        mock_build_frame = MagicMock()
        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            build_gui.BuildFrame.generate_command_list_cb(mock_build_frame)
        self.assertEqual(mock_build_frame.current_command, "Generate Command List")
        mock_build_frame.reset_text.assert_called_once()
        mock_build_frame.run_button.state.assert_called_once_with([tk.DISABLED])
        mock_build_frame.command_list_button.state.assert_called_once_with(
            [tk.DISABLED]
        )
        mock_build_frame.status_label.config.assert_called_once_with(
            text="Running command 'Generate Command List'."
        )
        mock_build_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_build_frame.oval, fill="darkgrey"
        )
        mock_open_file.assert_called_once_with(
            mock_build_frame.file_path, mode="w", encoding="utf-8"
        )
        mock_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        mock_build_frame.check_thread.assert_called_once()

    def test_select_command(self) -> None:
        """Test 'select_command_cb' function"""
        mock_build_frame = MagicMock()
        mock_build_frame.search_command_entry.get.return_value = "command"
        command_1 = build_gui.Command("command_1", "")
        command_2 = build_gui.Command("command_2", "")
        command_3 = build_gui.Command("Command_3", "")
        command_4 = build_gui.Command(name="not_relevant", help="")
        mock_build_frame.commands = [command_1, command_2, command_3, command_4]
        mock_build_frame.reduced_commands = [command_4]
        build_gui.BuildFrame.select_command_cb(mock_build_frame, "<KeyRelease>")
        mock_build_frame.search_command_entry.get.assert_called_once()
        mock_build_frame.commands_listbox.delete.assert_called_once_with(0, tk.END)
        self.assertListEqual(
            mock_build_frame.reduced_commands, [command_1, command_2, command_3]
        )
        mock_build_frame.commands_listbox.insert.assert_has_calls(
            [
                call(tk.END, "command_1  ()"),
                call(tk.END, "command_2  ()"),
                call(tk.END, "Command_3  ()"),
            ]
        )


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestBuildImport(unittest.TestCase):
    """Test import of build_gui"""

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)
        importlib.reload(build_gui)

    @patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=False)
    def test_no_project(self) -> None:
        """Test import when ROOT_IS_PROJECT is False"""
        importlib.reload(frame_base)
        importlib.reload(build_gui)
        self.assertEqual(build_gui.dummy(None).returncode, 0)

    @patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=True)
    def test_project(self) -> None:
        """Test import when ROOT_IS_PROJECT is True"""
        importlib.reload(frame_base)
        importlib.reload(build_gui)


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
