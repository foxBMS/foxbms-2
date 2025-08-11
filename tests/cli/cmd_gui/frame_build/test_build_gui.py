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

"""Testing file 'cli/cmd_gui/frame_build/build_gui.py'."""

import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import datetime
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.cmd_gui.frame_build import build_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_build import build_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestBuildFrame(unittest.TestCase):
    """Test of the BuildFrame class"""

    def setUp(self):
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = build_gui.BuildFrame(self.root, text)
        self.start_time = datetime.now()

    def tearDown(self):
        self.root.update()
        self.root.destroy()
        file_output = Path(PROJECT_BUILD_ROOT / "output_gui_build.txt")
        if file_output.exists():
            file_output.unlink()
        gui_directory = Path(PROJECT_BUILD_ROOT / "gui")
        if gui_directory.is_dir():
            if (
                datetime.fromtimestamp(gui_directory.stat().st_birthtime)
                >= self.start_time
            ):
                shutil.rmtree(gui_directory)

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
        """Test 'write_text' function when BuildFrame is not selected"""
        mock_select = MagicMock()
        mock_select.return_value = ""
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.after")
    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.write_text")
    def test_check_thread_alive(
        self, mock_write_text: MagicMock, mock_after: MagicMock
    ):
        """Test 'check_thread' function when the Thread is still alive"""
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = True
        self.frame.current_command = "command"
        self.frame.check_thread()

        mock_after.assert_called_once_with(50, self.frame.check_thread)
        self.frame.build_process.is_alive.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.write_text")
    def test_check_thread_dead_empty(self, mock_write_text: MagicMock):
        """Test 'check_thread' function
        when the Thread is not alive and there is no object in the queue"""
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        mock_empty = MagicMock()
        self.frame.queue.empty = mock_empty
        mock_empty.return_value = True
        self.frame.file_stream = MagicMock()
        self.frame.current_command = "command"
        self.frame.check_thread()

        self.frame.build_process.is_alive.assert_called_once()
        mock_empty.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.write_text")
    def test_check_thread_dead_success(self, mock_write_text: MagicMock):
        """Test 'check_thread' function
        when the Thread is not alive and the build was successful"""
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        mock_empty = MagicMock()
        self.frame.queue.empty = mock_empty
        mock_empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 0
        self.frame.queue.get.return_value = queue_return_value
        self.frame.file_stream = MagicMock()
        self.frame.current_command = "command"
        self.frame.check_thread()

        self.frame.build_process.is_alive.assert_called_once()
        mock_empty.assert_called_once()
        self.assertEqual(
            self.frame.canvas.itemcget(self.frame.oval, option="fill"), "green"
        )
        self.assertEqual(
            self.frame.status_text.cget("text"), "Command 'command' was successful."
        )
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.write_text")
    def test_check_thread_dead_failure(self, mock_write_text: MagicMock):
        """Test 'check_thread' function
        when the Thread is not alive and the build was not successful"""
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        mock_empty = MagicMock()
        self.frame.queue.empty = mock_empty
        mock_empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 1
        self.frame.queue.get.return_value = queue_return_value
        self.frame.file_stream = MagicMock()
        self.frame.current_command = "command"
        self.frame.check_thread()

        self.frame.build_process.is_alive.assert_called_once()
        mock_empty.assert_called_once()
        self.assertEqual(
            self.frame.canvas.itemcget(self.frame.oval, option="fill"), "red"
        )
        self.assertEqual(
            self.frame.status_text.cget("text"), "Command 'command' was not successful."
        )
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.write_text")
    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.generate_command_list")
    def test_check_thread_command_list(
        self, mock_generate_list: MagicMock, mock_write_text: MagicMock
    ):
        """Test 'check_thread' function
        when generating the command list"""
        self.frame.current_command = "Generate Command List"
        self.frame.build_process = MagicMock()
        self.frame.build_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        mock_empty = MagicMock()
        self.frame.queue.empty = mock_empty
        mock_empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 0
        self.frame.queue.get.return_value = queue_return_value
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        self.assertEqual(
            self.frame.canvas.itemcget(self.frame.oval, option="fill"), "green"
        )
        self.assertEqual(
            self.frame.status_text.cget("text"),
            "Command 'Generate Command List' was successful.",
        )
        mock_write_text.assert_not_called()
        mock_generate_list.assert_called_once()
        mock_empty.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.check_thread")
    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    @patch("cli.cmd_build.build_impl.run_top_level_waf")
    def test_run_command_cb(
        self, mock_waf: MagicMock, mock_thread: MagicMock, mock_check_thread: MagicMock
    ):
        """Test 'run_command_cb' function"""
        mock_thread.return_value = MagicMock()
        self.frame.listbox.curselection = MagicMock()
        self.frame.listbox.curselection.return_value = (0,)
        self.frame.reduced_commands = [build_gui.Command("command", "help")]
        self.frame.run_command_cb()
        self.frame.file_stream.close()

        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()
        self.assertEqual(
            self.frame.status_text.cget("text"), "Running command: command"
        )
        self.assertEqual(
            self.frame.canvas.itemcget(self.frame.oval, option="fill"), "darkgrey"
        )
        self.assertEqual("command", self.frame.current_command)

    @patch("cli.cmd_gui.frame_build.build_gui.file_name_from_current_time")
    def test_save_command(self, mock_time: MagicMock):
        """Test 'save_command' function"""
        mock_time.return_value = "new"
        self.frame.current_command = "command"
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.save_command_cb()
        with open(
            Path(PROJECT_BUILD_ROOT / "gui" / "command_new.txt"),
            encoding="utf-8",
        ) as f:
            self.assertEqual("New content.", f.read())
        with open(self.frame.file_path, encoding="utf-8") as f:
            self.assertEqual("New content.", f.read())
        gui_directory = Path(PROJECT_BUILD_ROOT / "gui")
        if gui_directory.is_dir():
            test_file = Path(gui_directory / "command_new.txt")
            if test_file.exists():
                test_file.unlink()

    def test_generate_command_list(self):
        """Test 'generate_command_list' function"""
        with open(self.frame.file_path, mode="w", encoding="utf-8") as f:
            f.write("Main commands \ncommand :\ncommand : 1\ncommand 2\noptions:")
        self.frame.listbox = MagicMock()
        mock_insert = MagicMock()
        self.frame.listbox.insert = mock_insert
        self.frame.generate_command_list()
        calls = [
            call(tk.END, "command  ()"),
            call(tk.END, "command  (1)"),
        ]
        mock_insert.assert_has_calls(calls)
        self.assertEqual(2, len(self.frame.commands))

    @patch("cli.cmd_gui.frame_build.build_gui.BuildFrame.check_thread")
    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    @patch("cli.cmd_build.build_impl.run_top_level_waf")
    def test_generate_command_list_command_cb(
        self,
        mock_run_waf: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
    ):
        """Test 'generate_command_list_command_cb' function"""
        mock_thread.return_value = MagicMock()
        self.frame.generate_command_list_command_cb()
        self.frame.file_stream.close()
        mock_check_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()
        self.assertEqual(
            self.frame.status_text.cget("text"),
            "Running command: Generate Command List",
        )
        self.assertEqual(
            self.frame.canvas.itemcget(self.frame.oval, option="fill"), "darkgrey"
        )

    def test_select_command_cb(self):
        """Test 'select_command_cb' function"""
        mock_entry_get = MagicMock()
        mock_entry_get.return_value = "command"
        command_1 = build_gui.Command("command_1", "")
        command_2 = build_gui.Command("command_2", "")
        command_3 = build_gui.Command("Command_3", "")
        command_4 = build_gui.Command(name="not_relevant", help="")
        self.frame.commands = [command_1, command_2, command_3, command_4]
        self.frame.reduced_commands = [command_4]
        self.frame.search_command_entry.get = mock_entry_get
        self.frame.select_command_cb("<KeyRelease>")
        self.assertEqual(3, len(self.frame.reduced_commands))
        self.assertEqual(3, len(self.frame.listbox.get(0, tk.END)))


class TestBuildFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the BuildFrame class"""

    def setUp(self):
        self.start_time = datetime.now()
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        for file_path in PROJECT_BUILD_ROOT.glob("output_build_*"):
            if file_path.suffix == ".txt" and file_path.exists():
                file_path.unlink()
        gui_directory = Path(PROJECT_BUILD_ROOT / "gui")
        if gui_directory.is_dir():
            if (
                datetime.fromtimestamp(os.path.getctime(gui_directory))
                >= self.start_time
            ):
                shutil.rmtree(gui_directory)

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_build_frame = MagicMock()
        mock_build_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_build_write_text_empty.txt"
        )
        mock_build_frame.text = MagicMock()
        mock_build_frame.text_index = MagicMock()
        mock_build_frame.current_command = "command"
        mock_build_frame.file_path.touch()
        build_gui.BuildFrame.write_text(mock_build_frame)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_build_frame = MagicMock()
        mock_build_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_build_write_text.txt"
        )
        mock_build_frame.text = MagicMock()
        mock_build_frame.text_index = MagicMock()
        mock_build_frame.current_command = "command"
        mock_build_frame.file_path.write_text("New content.", encoding="utf-8")
        build_gui.BuildFrame.write_text(mock_build_frame)

    def test_check_thread_alive(self):
        """Test 'check_thread' function when the Thread is still alive"""
        mock_build_frame = MagicMock()
        mock_build_frame.build_process = MagicMock()
        mock_build_frame.build_process.is_alive.return_value = True
        mock_build_frame.current_command = "command"
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.after.assert_called_once_with(
            50, mock_build_frame.check_thread
        )
        mock_build_frame.build_process.is_alive.assert_called_once()
        mock_build_frame.write_text.assert_called_once()

    def test_check_thread_dead_empty(self):
        """Test 'check_thread' function
        when the Thread is not alive and there is no object in the queue"""
        mock_build_frame = MagicMock()
        mock_build_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_build_check_thread_empty.txt"
        )
        mock_build_frame.file_path.write_text("New content.", encoding="utf-8")
        mock_build_frame.build_process = MagicMock()
        mock_build_frame.build_process.is_alive.return_value = False
        mock_build_frame.queue = MagicMock()
        mock_build_frame.queue.empty.return_value = True
        mock_build_frame.current_command = "command"
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.build_process.is_alive.assert_called_once()
        mock_build_frame.queue.empty.assert_called_once()
        mock_build_frame.write_text.assert_called_once()

    def test_check_thread_dead_success(self):
        """Test 'check_thread' function
        when the Thread is not alive and the build was successful"""
        mock_build_frame = MagicMock()
        mock_build_frame.build_process = MagicMock()
        mock_build_frame.build_process.is_alive.return_value = False
        mock_build_frame.queue = MagicMock()
        mock_build_frame.queue.empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 0
        mock_build_frame.queue.get.return_value = queue_return_value
        mock_build_frame.current_command = "command"
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.build_process.is_alive.assert_called_once()
        mock_build_frame.queue.empty.assert_called_once()
        mock_build_frame.write_text.assert_called_once()

    def test_check_thread_dead_failure(self):
        """Test 'check_thread' function
        when the Thread is not alive and the build was not successful"""
        mock_build_frame = MagicMock()
        mock_build_frame.build_process = MagicMock()
        mock_build_frame.build_process.is_alive.return_value = False
        mock_build_frame.queue = MagicMock()
        mock_build_frame.queue.empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 1
        mock_build_frame.queue.get.return_value = queue_return_value
        mock_build_frame.current_command = "command"
        build_gui.BuildFrame.check_thread(mock_build_frame)

        mock_build_frame.build_process.is_alive.assert_called_once()
        mock_build_frame.queue.empty.assert_called_once()
        mock_build_frame.write_text.assert_called_once()

    def test_check_thread_command_list(self):
        """Test 'check_thread' function
        when generating the command list"""
        mock_build_frame = MagicMock()
        mock_build_frame.build_process = MagicMock()
        mock_build_frame.build_process.is_alive.return_value = False
        mock_build_frame.current_command = "Generate Command List"
        mock_build_frame.build_process = MagicMock()
        mock_build_frame.build_process.is_alive.return_value = False
        mock_build_frame.queue = MagicMock()
        mock_build_frame.queue.empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 0
        mock_build_frame.queue.get.return_value = queue_return_value
        mock_build_frame.file_stream = MagicMock()
        build_gui.BuildFrame.check_thread(mock_build_frame)
        mock_build_frame.write_text.assert_not_called()
        mock_build_frame.generate_command_list.assert_called_once()

    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    @patch("cli.cmd_build.build_impl.run_top_level_waf")
    def test_run_command_cb(self, mock_waf: MagicMock, mock_thread: MagicMock):
        """Test 'run_command_cb' function"""
        mock_build_frame = MagicMock()
        mock_thread.return_value = MagicMock()
        mock_build_frame.listbox.curselection = MagicMock()
        mock_build_frame.listbox.curselection.return_value = (0,)
        mock_build_frame.reduced_commands = [build_gui.Command("command", "help")]
        build_gui.BuildFrame.run_command_cb(mock_build_frame)
        mock_build_frame.file_stream.close()

        mock_thread.return_value.start.assert_called_once()
        mock_build_frame.check_thread.assert_called_once()
        self.assertEqual("command", mock_build_frame.current_command)

    @patch("cli.cmd_gui.frame_build.build_gui.file_name_from_current_time")
    def test_save_command(self, mock_time: MagicMock):
        """Test 'save_command' function"""
        mock_build_frame = MagicMock()
        mock_build_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_build_save_command.txt"
        )
        mock_time.return_value = "new"
        mock_build_frame.current_command = "command"
        mock_build_frame.file_path.write_text("New content.", encoding="utf-8")
        build_gui.BuildFrame.save_command_cb(mock_build_frame)
        with open(
            Path(PROJECT_BUILD_ROOT / "gui" / "command_new.txt"),
            encoding="utf-8",
        ) as f:
            self.assertEqual("New content.", f.read())
        with open(mock_build_frame.file_path, encoding="utf-8") as f:
            self.assertEqual("New content.", f.read())
        gui_directory = Path(PROJECT_BUILD_ROOT / "gui")
        if gui_directory.is_dir():
            test_file = Path(gui_directory / "command_new.txt")
            if test_file.exists():
                test_file.unlink()

    def test_generate_command_list(self):
        """Test 'generate_command_list' function"""
        mock_build_frame = MagicMock()
        mock_build_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_build_command_list.txt"
        )
        mock_build_frame.commands = []
        with open(mock_build_frame.file_path, mode="w", encoding="utf-8") as f:
            f.write("Main commands \ncommand :\ncommand : 1:\ncommand 2\noptions:")
        build_gui.BuildFrame.generate_command_list(mock_build_frame)
        calls = [
            call(tk.END, "command  ()"),
            call(tk.END, "command  (1)"),
        ]
        mock_build_frame.listbox.insert.assert_has_calls(calls)
        self.assertEqual(2, len(mock_build_frame.commands))

    @patch("cli.cmd_gui.frame_build.build_gui.Thread")
    @patch("cli.cmd_build.build_impl.run_top_level_waf")
    def test_generate_command_list_command_cb(
        self, mock_run_waf: MagicMock, mock_thread: MagicMock
    ):
        """Test 'generate_command_list_command_cb' function"""
        mock_thread.return_value = MagicMock()
        mock_build_frame = MagicMock()
        mock_build_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_build_list_cb.txt"
        )
        build_gui.BuildFrame.generate_command_list_command_cb(mock_build_frame)
        mock_build_frame.file_stream.close()
        mock_build_frame.check_thread.assert_called_once()
        mock_thread.return_value.start.assert_called_once()

    def test_select_command_cb(self):
        """Test 'select_command_cb' function"""
        mock_build_frame = MagicMock()
        mock_entry = MagicMock()
        mock_entry.get.return_value = "command"
        mock_build_frame.search_command_entry = mock_entry
        command_1 = build_gui.Command("command_1", "")
        command_2 = build_gui.Command("command_2", "")
        command_3 = build_gui.Command("Command_3", "")
        command_4 = build_gui.Command(name="not_relevant", help="")
        mock_build_frame.commands = [command_1, command_2, command_3, command_4]
        mock_build_frame.reduced_commands = [command_4]
        build_gui.BuildFrame.select_command_cb(mock_build_frame, "<KeyRelease>")
        self.assertEqual(3, len(mock_build_frame.reduced_commands))
        mock_build_frame.listbox.insert.assert_has_calls(
            [
                call(tk.END, "command_1  ()"),
                call(tk.END, "command_2  ()"),
                call(tk.END, "Command_3  ()"),
            ]
        )


if __name__ == "__main__":
    unittest.main()
