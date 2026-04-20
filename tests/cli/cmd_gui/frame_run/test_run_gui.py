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

"""Testing file 'cli/cmd_gui/frame_run/run_gui.py'."""

import io
import json
import os
import shutil
import subprocess
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_run import run_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_run import run_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "run_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestRunFrame(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Test of the RunFrame class"""

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        self.cache_dir_bak = run_gui.CACHE_DIR
        run_gui.PROJECT_BUILD_ROOT = PATH_GUI
        run_gui.CACHE_DIR = PATH_GUI
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = run_gui.RunFrame(self.root, text)
        self.frame.file_path.write_text("", encoding="utf-8")

    def tearDown(self):
        if hasattr(self.frame, "file_stream"):
            self.frame.file_stream.close()
        self.root.update()
        self.root.destroy()
        run_gui.PROJECT_BUILD_ROOT = PROJECT_BUILD_ROOT
        run_gui.CACHE_DIR = self.cache_dir_bak
        remove_data(self.start_time)

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
        """Test 'write_text' function when RunFrame is not selected"""
        mock_select = MagicMock()
        mock_select.return_value = ""
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_target_cb_script(self, mock_askopenfilename: MagicMock):
        """Test selecting a script file target."""
        mock_askopenfilename.return_value = "script.py"
        self.frame.mode_combobox.set("Script")
        self.frame.open_target_cb()
        self.assertEqual(self.frame.target_entry.get(), "script.py")

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_target_cb_program(self, mock_askopenfilename: MagicMock):
        """Test selecting a program target in program mode."""
        mock_askopenfilename.return_value = "tool.exe"
        self.frame.mode_combobox.set("Program")
        self.frame.open_target_cb()
        self.assertEqual(self.frame.target_entry.get(), "tool.exe")

    @patch("tkinter.filedialog.askopenfilename")
    def test_open_target_cb_cancel(self, mock_askopenfilename: MagicMock):
        """Test target chooser cancel keeps the previous value."""
        self.frame.target_entry.delete(0, tk.END)
        self.frame.target_entry.insert(tk.END, "keep.me")
        mock_askopenfilename.return_value = ""
        self.frame.open_target_cb()
        self.assertEqual(self.frame.target_entry.get(), "keep.me")

    @patch("tkinter.filedialog.askdirectory")
    def test_open_cwd_cb(self, mock_askdirectory: MagicMock):
        """Test selecting a cwd directory."""
        mock_askdirectory.return_value = str(PATH_GUI)
        self.frame.open_cwd_cb()
        self.assertEqual(self.frame.cwd_entry.get(), str(PATH_GUI))

    @patch("tkinter.filedialog.askdirectory")
    def test_open_cwd_cb_cancel(self, mock_askdirectory: MagicMock):
        """Test cwd chooser cancel keeps the previous value."""
        initial = self.frame.cwd_entry.get()
        mock_askdirectory.return_value = ""
        self.frame.open_cwd_cb()
        self.assertEqual(self.frame.cwd_entry.get(), initial)

    def test_run_command_cb_missing_target(self):
        """Test run callback without target input."""
        self.frame.target_entry.delete(0, tk.END)
        self.frame.run_command_cb()
        self.assertEqual(
            self.frame.status_text.cget("text"), "Please provide a target file/program."
        )
        self.assertEqual(self.frame.canvas.itemcget(self.frame.oval, "fill"), "red")

    def test_run_command_cb_invalid_cwd(self):
        """Test run callback with invalid cwd input."""
        self.frame.target_entry.delete(0, tk.END)
        self.frame.target_entry.insert(tk.END, "target.py")
        self.frame.cwd_entry.delete(0, tk.END)
        self.frame.cwd_entry.insert(tk.END, "not-a-directory")
        self.frame.run_command_cb()
        self.assertEqual(
            self.frame.status_text.cget("text"),
            "Please provide a valid working directory.",
        )

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.check_thread")
    @patch("cli.cmd_gui.frame_run.run_gui.Thread")
    def test_run_command_cb_script(self, mock_thread: MagicMock, mock_check: MagicMock):
        """Test run callback in script mode."""
        self.frame.mode_combobox.set("Script")
        self.frame.target_entry.delete(0, tk.END)
        self.frame.target_entry.insert(tk.END, "script.py")
        self.frame.args_entry.delete(0, tk.END)
        self.frame.args_entry.insert(tk.END, "--help")
        self.frame.cwd_entry.delete(0, tk.END)
        self.frame.cwd_entry.insert(tk.END, str(PATH_GUI))
        self.frame.run_command_cb()
        mock_thread.return_value.start.assert_called_once()
        mock_check.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.check_thread")
    @patch("cli.cmd_gui.frame_run.run_gui.Thread")
    def test_run_command_cb_program(
        self, mock_thread: MagicMock, mock_check: MagicMock
    ):
        """Test run callback in program mode."""
        self.frame.mode_combobox.set("Program")
        self.frame.target_entry.delete(0, tk.END)
        self.frame.target_entry.insert(tk.END, "tool.exe")
        self.frame.args_entry.delete(0, tk.END)
        self.frame.args_entry.insert(tk.END, "--version")
        self.frame.cwd_entry.delete(0, tk.END)
        self.frame.cwd_entry.insert(tk.END, str(PATH_GUI))
        self.frame.run_command_cb()
        mock_thread.return_value.start.assert_called_once()
        mock_check.assert_called_once()
        self.assertEqual(self.frame.current_preset["mode"], "Program")
        self.assertEqual(self.frame.current_preset["target"], "tool.exe")

    def test_parse_args_empty_returns_empty_list(self):
        """Test parser returns an empty list for blank input."""
        # pylint: disable-next=protected-access
        parse_args = self.frame._parse_args
        self.assertEqual([], parse_args("   "))

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_check_thread_empty_queue(self, mock_write_text: MagicMock):
        """Test check_thread behavior when process ended but queue is empty."""
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = True
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        self.frame.file_stream.close.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.after")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_check_thread_alive(
        self, mock_write_text: MagicMock, mock_after: MagicMock
    ):
        """Test check_thread when thread is still alive."""
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = True
        self.frame.check_thread()
        self.frame.run_process.is_alive.assert_called_once()
        mock_after.assert_called_once_with(50, self.frame.check_thread)
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_check_thread_success(self, mock_write_text: MagicMock):
        """Test check_thread with successful command result."""
        self.frame.current_command = "script: script.py"
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 0
        self.frame.queue.get.return_value = queue_return_value
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        self.assertEqual(self.frame.canvas.itemcget(self.frame.oval, "fill"), "green")
        self.frame.file_stream.close.assert_called_once()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_check_thread_failure(self, mock_write_text: MagicMock):
        """Test check_thread with failed command result."""
        self.frame.current_command = "program: app.exe"
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 1
        self.frame.queue.get.return_value = queue_return_value
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        self.assertEqual(self.frame.canvas.itemcget(self.frame.oval, "fill"), "red")
        self.frame.file_stream.close.assert_called_once()
        mock_write_text.assert_called_once()

    def test_save_command_cb(self):
        """Test saving run log to timestamped file."""
        self.frame.file_path.write_text("log content", encoding="utf-8")
        self.frame.save_command_cb()
        self.assertIn("Log saved in file", self.frame.status_text.cget("text"))

    def test_remember_preset(self):
        """Test storing a run preset and exposing it in combobox values."""
        full_target = str(PATH_GUI / "x" / "script.py")
        preset = {
            "mode": "Script",
            "target": full_target,
            "args": "--help",
            "cwd": str(PATH_GUI),
        }
        self.frame.remember_preset(preset)
        self.assertEqual(1, len(self.frame.presets))
        self.assertEqual(
            self.frame.preset_combobox.get(),
            f"Script: {full_target} --help",
        )
        self.assertTrue((PATH_GUI / "run_frame_presets.json").is_file())

    def test_remember_preset_without_args_label(self):
        """Test preset labels omit args when args are empty."""
        preset = {
            "mode": "Program",
            "target": "app.exe",
            "args": "",
            "cwd": str(PATH_GUI),
        }
        self.frame.remember_preset(preset)
        values = self.frame.preset_combobox["values"]
        self.assertIn("Program: app.exe", values)

    def test_remember_preset_keeps_last_10(self):
        """Test that only the latest 10 presets are retained."""
        for i in range(12):
            self.frame.remember_preset(
                {
                    "mode": "Script",
                    "target": f"script_{i}.py",
                    "args": f"--opt {i}",
                    "cwd": str(PATH_GUI),
                }
            )
        self.assertEqual(10, len(self.frame.presets))
        self.assertEqual("script_11.py", self.frame.presets[0]["target"])
        self.assertEqual("script_2.py", self.frame.presets[-1]["target"])

    def test_remember_preset_persists_data(self):
        """Test presets are persisted into cache file."""
        preset = {
            "mode": "Script",
            "target": "script.py",
            "args": "--help",
            "cwd": str(PATH_GUI),
        }
        self.frame.remember_preset(preset)
        with open(PATH_GUI / "run_frame_presets.json", encoding="utf-8") as f:
            data = json.load(f)
        self.assertEqual(1, len(data))
        self.assertEqual(data[0]["target"], "script.py")

    def test_load_presets_from_cache(self):
        """Test presets are loaded from cache file on frame initialization."""
        cached = [
            {
                "mode": "Program",
                "target": "app.exe",
                "args": "--version",
                "cwd": str(PATH_GUI),
            }
        ]
        with open(PATH_GUI / "run_frame_presets.json", mode="w", encoding="utf-8") as f:
            json.dump(cached, f, ensure_ascii=True)
        frame = run_gui.RunFrame(self.root, tk.Text())
        self.assertEqual(1, len(frame.presets))
        self.assertEqual("app.exe", frame.presets[0]["target"])

    def test_load_presets_invalid_json(self):
        """Test invalid JSON in cache file does not crash and loads no presets."""
        with open(PATH_GUI / "run_frame_presets.json", mode="w", encoding="utf-8") as f:
            f.write("{invalid-json")
        frame = run_gui.RunFrame(self.root, tk.Text())
        self.assertEqual([], frame.presets)

    def test_load_presets_filters_invalid_entries(self):
        """Test loading presets ignores invalid entries and keeps valid ones."""
        cached = [
            "invalid",
            {"mode": "Script"},
            {
                "mode": "Program",
                "target": "app.exe",
                "args": "--version",
                "cwd": str(PATH_GUI),
            },
        ]
        with open(PATH_GUI / "run_frame_presets.json", mode="w", encoding="utf-8") as f:
            json.dump(cached, f, ensure_ascii=True)
        frame = run_gui.RunFrame(self.root, tk.Text())
        self.assertEqual(1, len(frame.presets))
        self.assertEqual("Program", frame.presets[0]["mode"])

    def test_load_presets_all_invalid_entries(self):
        """Test loading all-invalid preset entries keeps preset list empty."""
        cached = ["invalid", {"mode": "Script"}]
        with open(PATH_GUI / "run_frame_presets.json", mode="w", encoding="utf-8") as f:
            json.dump(cached, f, ensure_ascii=True)
        frame = run_gui.RunFrame(self.root, tk.Text())
        self.assertEqual([], frame.presets)

    def test_apply_preset_cb(self):
        """Test applying selected preset values to input fields."""
        preset = {
            "mode": "Program",
            "target": "app.exe",
            "args": "--version",
            "cwd": str(PATH_GUI),
        }
        self.frame.remember_preset(preset)
        self.frame.apply_preset_cb()
        self.assertEqual("Program", self.frame.mode_combobox.get())
        self.assertEqual("app.exe", self.frame.target_entry.get())
        self.assertEqual("--version", self.frame.args_entry.get())
        self.assertEqual(str(PATH_GUI), self.frame.cwd_entry.get())

    def test_apply_preset_cb_no_selection(self):
        """Test applying presets with empty selection keeps current inputs."""
        self.frame.target_entry.delete(0, tk.END)
        self.frame.target_entry.insert(tk.END, "unchanged")
        self.frame.preset_combobox.set("")
        self.frame.apply_preset_cb()
        self.assertEqual("unchanged", self.frame.target_entry.get())

    def test_apply_preset_cb_no_matching_label(self):
        """Test applying unknown preset label does not modify existing values."""
        self.frame.presets = [
            {
                "mode": "Script",
                "target": "script.py",
                "args": "--arg",
                "cwd": str(PATH_GUI),
            }
        ]
        self.frame.target_entry.delete(0, tk.END)
        self.frame.target_entry.insert(tk.END, "keep-target")
        self.frame.preset_combobox.set("Program: unknown.exe")
        self.frame.apply_preset_cb()
        self.assertEqual("keep-target", self.frame.target_entry.get())

    @patch("cli.cmd_gui.frame_run.run_gui.run_script_impl.run_python_script")
    def test_run_selected_command_script_mode(self, mock_run_script: MagicMock):
        """Test script mode runner dispatch enqueues returned process result."""
        result = subprocess.CompletedProcess(args=["a"], returncode=0)
        mock_run_script.return_value = result
        # pylint: disable-next=protected-access
        run_selected = self.frame._run_selected_command
        run_selected(
            mode="Script",
            command_args=["script.py", "--x"],
            cwd=str(PATH_GUI),
            stdout=io.StringIO(),
            stderr=io.StringIO(),
        )
        queued = self.frame.queue.get_nowait()
        self.assertIs(queued, result)

    @patch("cli.cmd_gui.frame_run.run_gui.run_program_impl.run_program")
    def test_run_selected_command_program_mode(self, mock_run_program: MagicMock):
        """Test program mode runner dispatch enqueues returned process result."""
        result = subprocess.CompletedProcess(args=["a"], returncode=1)
        mock_run_program.return_value = result
        # pylint: disable-next=protected-access
        run_selected = self.frame._run_selected_command
        run_selected(
            mode="Program",
            command_args=["app.exe", "--x"],
            cwd=str(PATH_GUI),
            stdout=io.StringIO(),
            stderr=io.StringIO(),
        )
        queued = self.frame.queue.get_nowait()
        self.assertIs(queued, result)

    def test_write_text_with_input_appends_to_log_file(self):
        """Test write_text(file_input=...) appends raw input into log file."""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.frame.write_text("abc")
        with open(self.frame.file_path, encoding="utf-8") as f:
            self.assertIn("abc", f.read())

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_write_debug_enabled(self, mock_write_text: MagicMock):
        """Test write_debug writes output when debug mode is active."""
        with patch.object(self.frame, "is_debug_enabled", return_value=True):
            self.frame.write_debug("hello")
        mock_write_text.assert_called_once_with("[debug] hello\n")

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_write_debug_disabled(self, mock_write_text: MagicMock):
        """Test write_debug does nothing when debug mode is inactive."""
        with patch.object(self.frame, "is_debug_enabled", return_value=False):
            self.frame.write_debug("hello")
        mock_write_text.assert_not_called()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.write_text")
    def test_check_thread_success_stores_preset(self, mock_write_text: MagicMock):
        """Test successful run stores preset for later reuse."""
        self.frame.current_command = "script: script.py"
        self.frame.current_preset = {
            "mode": "Script",
            "target": "script.py",
            "args": "--help",
            "cwd": str(PATH_GUI),
        }
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = False
        queue_return_value = MagicMock()
        queue_return_value.returncode = 0
        self.frame.queue.get.return_value = queue_return_value
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        self.assertEqual(1, len(self.frame.presets))
        mock_write_text.assert_called_once()


class TestRunFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the RunFrame class"""

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        remove_data(self.start_time)

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_run_frame = MagicMock()
        mock_run_frame.parent.nametowidget.return_value = mock_run_frame  # pylint: disable=no-member,useless-suppression
        mock_run_frame.file_path = Path(PATH_GUI / "output_run_write_text_empty.txt")
        mock_run_frame.file_path.touch()
        mock_run_frame.text = MagicMock()
        mock_run_frame.text_index = 0
        run_gui.RunFrame.write_text(mock_run_frame)
        mock_run_frame.text.insert.assert_called_once_with(tk.END, "")

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_run_frame = MagicMock()
        mock_run_frame.parent.nametowidget.return_value = mock_run_frame  # pylint: disable=no-member,useless-suppression
        mock_run_frame.file_path = Path(PATH_GUI / "output_run_write_text.txt")
        mock_run_frame.file_path.write_text("New content.", encoding="utf-8")
        mock_run_frame.text = MagicMock()
        mock_run_frame.text_index = 0
        run_gui.RunFrame.write_text(mock_run_frame)
        mock_run_frame.text.insert.assert_called_once_with(tk.END, "New content.")


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
