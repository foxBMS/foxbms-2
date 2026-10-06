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

# spell:ignore initialdir

import importlib
import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from tkinter import ttk
from unittest.mock import MagicMock, call, mock_open, patch

try:
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_run import run_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_run import run_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
    from cli.helpers.spr import SubprocessResult

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "run_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestSelectFileDirectory(unittest.TestCase):
    """Test of the 'select_target_file_cb' and 'select_directory_cb' functions"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
        cls.cache_dir = run_gui.CACHE_DIR
        run_gui.CACHE_DIR = PATH_GUI

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)
        run_gui.CACHE_DIR = cls.cache_dir

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        with patch(
            "cli.cmd_gui.frame_run.run_gui.RunFrame.load_presets", return_value=[]
        ):
            self.frame = run_gui.RunFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_target_file_script(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_target_file_cb' function for a script."""
        mock_askopenfilename.return_value = "script.py"
        self.frame.mode_combobox.set("Script")
        self.frame.select_target_file_cb()
        mock_askopenfilename.assert_called_once_with(
            filetypes=[("Python Files", "*.py"), ("All Files", "*.*")]
        )
        self.assertEqual(self.frame.target_script_entry.get(), "script.py")

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_target_file_program(self, mock_askopenfilename: MagicMock) -> None:
        """Test 'select_target_file_cb' function in program mode."""
        mock_askopenfilename.return_value = "tool.exe"
        self.frame.mode_combobox.set("Program")
        self.frame.select_target_file_cb()
        mock_askopenfilename.assert_called_once_with(filetypes=[("All Files", "*.*")])
        self.assertEqual(self.frame.target_script_entry.get(), "tool.exe")

    @patch("tkinter.filedialog.askopenfilename")
    def test_select_target_file_no_selection(
        self, mock_askopenfilename: MagicMock
    ) -> None:
        """Test 'select_target_file_cb' function when no file is selected."""
        self.frame.target_script_entry.delete(0, tk.END)
        self.frame.target_script_entry.insert(tk.END, "file_path")
        mock_askopenfilename.return_value = ""
        self.frame.select_target_file_cb()
        self.assertEqual(self.frame.target_script_entry.get(), "file_path")

    @patch("tkinter.filedialog.askdirectory")
    def test_select_directory(self, mock_askdirectory: MagicMock) -> None:
        """Test 'select_directory_cb' function."""
        mock_askdirectory.return_value = str(PATH_GUI)
        self.frame.select_directory_cb()
        mock_askdirectory.assert_called_once_with(initialdir=str(run_gui.PROJECT_ROOT))
        self.assertEqual(self.frame.working_directory_entry.get(), str(PATH_GUI))

    @patch("tkinter.filedialog.askdirectory")
    def test_select_directory_cancel(self, mock_askdirectory: MagicMock) -> None:
        """Test 'select_directory_cb' function when no directory is selected."""
        self.frame.working_directory_entry.delete(0, tk.END)
        self.frame.working_directory_entry.insert(tk.END, "directory_path")
        mock_askdirectory.return_value = ""
        self.frame.select_directory_cb()
        mock_askdirectory.assert_called_once_with(initialdir=str(run_gui.PROJECT_ROOT))
        self.assertEqual(self.frame.working_directory_entry.get(), "directory_path")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestCheckThread(unittest.TestCase):
    """Test of the 'check_thread' function of the RunFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
        cls.cache_dir = run_gui.CACHE_DIR
        run_gui.CACHE_DIR = PATH_GUI

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)
        run_gui.CACHE_DIR = cls.cache_dir

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        with patch(
            "cli.cmd_gui.frame_run.run_gui.RunFrame.load_presets", return_value=[]
        ):
            self.frame = run_gui.RunFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.after")
    @patch("cli.cmd_gui.frame_run.run_gui.BaseFrame.write_text")
    def test_check_thread_alive(
        self, mock_write_text: MagicMock, mock_after: MagicMock
    ) -> None:
        """Test 'check_thread' function when Thread is alive."""
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = True
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.frame.run_process.is_alive.assert_called_once()
        mock_after.assert_called_once_with(50, self.frame.check_thread)

    @patch("cli.cmd_gui.frame_run.run_gui.BaseFrame.write_text")
    def test_check_thread_empty(self, mock_write_text: MagicMock) -> None:
        """Test 'check_thread' function when Thread is not alive but queue is empty."""
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = True
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.assertEqual(self.frame.run_button.state(), ())
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.queue.empty.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.BaseFrame.write_text")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.save_preset")
    def test_check_thread_success(
        self,
        mock_save_preset: MagicMock,
        mock_generate_label: MagicMock,
        mock_write_text: MagicMock,
    ) -> None:
        """Test 'check_thread' function when command was successful."""
        mock_generate_label.return_value = "Script 'script.py'"
        self.frame.current_command = {
            "mode": "Script",
            "target": "script.py",
            "args": "",
            "cwd": "",
        }
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = False
        mock_queue_element = MagicMock()
        mock_queue_element.returncode = 0
        self.frame.queue.get.return_value = mock_queue_element
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.frame.run_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.run_button.state(), ())
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.queue.empty.assert_called_once()
        self.frame.queue.get.assert_called_once()
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "green"
        )
        self.assertEqual(
            self.frame.status_label.cget("text"), "Script 'script.py' was successful."
        )
        mock_generate_label.assert_called_once_with(self.frame.current_command)
        mock_save_preset.assert_called_once_with(self.frame.current_command)

    @patch("cli.cmd_gui.frame_run.run_gui.BaseFrame.write_text")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.save_preset")
    def test_check_thread_success_empty_command(
        self,
        mock_save_preset: MagicMock,
        mock_generate_label: MagicMock,
        mock_write_text: MagicMock,
    ) -> None:
        """Test 'check_thread' function when command was successful and current command is None."""
        mock_generate_label.return_value = "Command"
        self.frame.current_command = None
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = False
        mock_queue_element = MagicMock()
        mock_queue_element.returncode = 0
        self.frame.queue.get.return_value = mock_queue_element
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.frame.run_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.run_button.state(), ())
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.queue.empty.assert_called_once()
        self.frame.queue.get.assert_called_once()
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "green"
        )
        self.assertEqual(
            self.frame.status_label.cget("text"), "Command was successful."
        )
        mock_generate_label.assert_called_once_with(self.frame.current_command)
        mock_save_preset.assert_not_called()

    @patch("cli.cmd_gui.frame_run.run_gui.BaseFrame.write_text")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    def test_check_thread_failure(
        self, mock_generate_label: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test 'check_thread' function when command failed."""
        mock_generate_label.return_value = "Script 'script.py'"
        self.frame.current_command = {
            "mode": "Script",
            "target": "script.py",
            "args": "",
            "cwd": "",
        }
        self.frame.run_process = MagicMock()
        self.frame.run_process.is_alive.return_value = False
        self.frame.queue = MagicMock()
        self.frame.queue.empty.return_value = False
        mock_queue_element = MagicMock()
        mock_queue_element.returncode = 1
        self.frame.queue.get.return_value = mock_queue_element
        self.frame.file_stream = MagicMock()
        self.frame.check_thread()
        mock_write_text.assert_called_once()
        self.frame.run_process.is_alive.assert_called_once()
        self.assertEqual(self.frame.run_button.state(), ())
        self.assertEqual(self.frame.save_log_button.state(), ())
        self.frame.file_stream.close.assert_called_once()
        self.frame.queue.empty.assert_called_once()
        self.frame.queue.get.assert_called_once()
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "red"
        )
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Script 'script.py' failed with exit code 1.",
        )
        mock_generate_label.assert_called_once_with(self.frame.current_command)


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestRunFrame(unittest.TestCase):
    """Test of the RunFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
        cls.cache_dir = run_gui.CACHE_DIR
        run_gui.CACHE_DIR = PATH_GUI

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)
        run_gui.CACHE_DIR = cls.cache_dir

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = run_gui.RunFrame(self.root, text)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.reset_text")
    def test_run_no_target(self, mock_reset_text: MagicMock) -> None:
        """Test of the 'run_cb' function without target input."""
        self.frame.target_script_entry.delete(0, tk.END)
        self.frame.run_cb()
        mock_reset_text.assert_called_once()
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Please provide a target file/program.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "lightgrey"
        )

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.reset_text")
    def test_run_no_file(self, mock_reset_text: MagicMock) -> None:
        """Test of the 'run_cb' function in Script mode when target is no file."""
        self.frame.mode_combobox.set("Script")
        self.frame.target_script_entry.delete(0, tk.END)
        self.frame.target_script_entry.insert(tk.END, "script")
        self.frame.run_cb()
        mock_reset_text.assert_called_once()
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Please provide a target file/program.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "lightgrey"
        )

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.reset_text")
    def test_run_invalid_dir(self, mock_reset_text: MagicMock) -> None:
        """Test of the 'run_cb' function with invalid directory."""
        self.frame.mode_combobox.set("Script")
        self.frame.target_script_entry.delete(0, tk.END)
        self.frame.target_script_entry.insert(tk.END, __file__)
        self.frame.working_directory_entry.delete(0, tk.END)
        self.frame.working_directory_entry.insert(tk.END, "not-a-directory")
        self.frame.run_cb()
        mock_reset_text.assert_called_once()
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Please provide a valid working directory.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "lightgrey"
        )

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.check_thread")
    @patch("cli.cmd_gui.frame_run.run_gui.Thread")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.reset_text")
    def test_run_script(
        self,
        mock_reset_text: MagicMock,
        mock_generate_label: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
    ) -> None:
        """Test of the 'run_cb' function in Script mode."""
        mock_open_file = mock_open()
        mock_generate_label.return_value = "Label"
        self.frame.mode_combobox.set("Script")
        self.frame.target_script_entry.delete(0, tk.END)
        self.frame.target_script_entry.insert(tk.END, __file__)
        self.frame.arguments_entry.delete(0, tk.END)
        self.frame.arguments_entry.insert(tk.END, "--help")
        self.frame.working_directory_entry.delete(0, tk.END)
        self.frame.working_directory_entry.insert(tk.END, str(PATH_GUI))
        with patch("builtins.open", mock_open_file):
            self.frame.run_cb()
        mock_reset_text.assert_called_once()
        self.assertDictEqual(
            {
                "mode": "Script",
                "target": __file__,
                "args": "--help",
                "cwd": str(PATH_GUI),
            },
            self.frame.current_command,
        )
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Running 'Label'.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "darkgrey"
        )
        self.assertEqual(self.frame.run_button.state(), (tk.DISABLED,))
        self.assertEqual(self.frame.save_log_button.state(), (tk.DISABLED,))
        mock_open_file.assert_called_once_with(
            self.frame.file_path, mode="w", encoding="utf-8"
        )
        mock_thread.assert_called()
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.check_thread")
    @patch("cli.cmd_gui.frame_run.run_gui.Thread")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.reset_text")
    def test_run_program(
        self,
        mock_reset_text: MagicMock,
        mock_generate_label: MagicMock,
        mock_thread: MagicMock,
        mock_check_thread: MagicMock,
    ) -> None:
        """Test of the 'run_cb' function in Program mode."""
        mock_open_file = mock_open()
        mock_generate_label.return_value = "Label"
        self.frame.mode_combobox.set("Program")
        self.frame.target_script_entry.delete(0, tk.END)
        self.frame.target_script_entry.insert(tk.END, "program.exe")
        self.frame.arguments_entry.delete(0, tk.END)
        self.frame.arguments_entry.insert(tk.END, "--version")
        self.frame.working_directory_entry.delete(0, tk.END)
        self.frame.working_directory_entry.insert(tk.END, str(PATH_GUI))
        with patch("builtins.open", mock_open_file):
            self.frame.run_cb()
        mock_reset_text.assert_called_once()
        self.assertDictEqual(
            {
                "mode": "Program",
                "target": "program.exe",
                "args": "--version",
                "cwd": str(PATH_GUI),
            },
            self.frame.current_command,
        )
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Running 'Label'.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "darkgrey"
        )
        self.assertEqual(self.frame.run_button.state(), (tk.DISABLED,))
        self.assertEqual(self.frame.save_log_button.state(), (tk.DISABLED,))
        mock_open_file.assert_called_once_with(
            self.frame.file_path, mode="w", encoding="utf-8"
        )
        mock_thread.assert_called()
        mock_thread.return_value.start.assert_called_once()
        mock_check_thread.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.save_presets_to_file")
    def test_save_preset_new(
        self, mock_save_to_file: MagicMock, mock_generate_label: MagicMock
    ) -> None:
        """Test of 'save_preset" function when no preset exists."""
        preset_label = "Script 'script.py --help'"
        mock_generate_label.return_value = preset_label
        self.frame.presets = []
        preset = {
            "mode": "Script",
            "target": "script.py",
            "args": "--help",
            "cwd": "",
        }
        self.frame.save_preset(preset)
        self.assertEqual(1, len(self.frame.presets))
        self.assertEqual(self.frame.presets_combobox.get(), preset_label)
        mock_generate_label.assert_called_once_with(preset)
        mock_save_to_file.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.save_presets_to_file")
    def test_save_preset_add(
        self, mock_save_to_file: MagicMock, mock_generate_label: MagicMock
    ) -> None:
        """Test of 'save_preset" function 12 presets are added."""
        for i in range(12):
            mock_generate_label.return_value = f"Script 'script_{i}.py --opt {i}'"
            self.frame.save_preset(
                {
                    "mode": "Script",
                    "target": f"script_{i}.py",
                    "args": f"--opt {i}",
                    "cwd": "",
                }
            )
        self.assertEqual(10, len(self.frame.presets))
        self.assertEqual("script_11.py", self.frame.presets[0]["target"])
        self.assertEqual("script_2.py", self.frame.presets[-1]["target"])
        self.assertEqual(
            self.frame.presets_combobox.get(), "Script 'script_11.py --opt 11'"
        )
        mock_generate_label.assert_called()
        mock_save_to_file.assert_called()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame.save_presets_to_file")
    def test_save_preset_duplicate(
        self, mock_save_to_file: MagicMock, mock_generate_label: MagicMock
    ) -> None:
        """Test of 'save_preset" function when added preset already exists."""
        preset_label = "Script 'script.py --help'"
        mock_generate_label.return_value = preset_label
        preset = {
            "mode": "Script",
            "target": "script.py",
            "args": "--help",
            "cwd": "",
        }
        self.frame.presets = [
            {"mode": "Program", "target": "program.exe", "args": "arg", "cwd": ""},
            preset,
        ]
        self.frame.save_preset(preset)
        self.assertEqual(2, len(self.frame.presets))
        self.assertEqual(self.frame.presets_combobox.get(), preset_label)
        self.assertDictEqual(self.frame.presets[0], preset)
        mock_generate_label.assert_called()
        mock_save_to_file.assert_called_once()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._insert_text")
    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    def test_apply_preset(
        self, mock_generate_label: MagicMock, mock_insert_text: MagicMock
    ) -> None:
        """Test applying selected preset values to input fields."""
        mock_generate_label.return_value = "Script script.py --arg"
        self.frame.presets = [
            {
                "mode": "Script",
                "target": "script.py",
                "args": "--arg",
                "cwd": "directory",
            }
        ]
        self.frame.presets_combobox["values"] = ["Preset 1"]
        self.frame.presets_combobox.current(0)
        self.frame.apply_preset_cb()
        self.assertEqual("Script", self.frame.mode_combobox.get())
        mock_insert_text.assert_has_calls(
            [
                call(self.frame.target_script_entry, "script.py"),
                call(self.frame.arguments_entry, "--arg"),
                call(self.frame.working_directory_entry, "directory"),
            ]
        )
        self.assertEqual(
            self.frame.status_label.cget("text"),
            "Applied preset 'Script script.py --arg'.",
        )
        self.assertEqual(
            self.frame.indicator_canvas.itemcget(self.frame.oval, "fill"), "lightgrey"
        )

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._insert_text")
    def test_apply_preset_no_selection(self, mock_insert_text: MagicMock) -> None:
        """Test of 'apply_preset_cb' function when no preset is selected."""
        self.frame.presets_combobox.set("")
        self.frame.apply_preset_cb()
        mock_insert_text.assert_not_called()

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._insert_text")
    def test_apply_preset_invalid_index(self, mock_insert_text: MagicMock) -> None:
        """Test of 'apply_preset_cb' function when selected preset has invalid index."""
        self.frame.presets = [
            {
                "mode": "Script",
                "target": "script.py",
                "args": "--arg",
                "cwd": "",
            }
        ]
        self.frame.presets_combobox["values"] = ["Preset 1", "Preset 2"]
        self.frame.presets_combobox.current(1)
        self.frame.apply_preset_cb()
        mock_insert_text.assert_not_called()

    @patch("cli.cmd_gui.frame_run.run_gui.run_script_impl.run_python_script")
    def test_run_selected_command_script(self, mock_run_script: MagicMock) -> None:
        """Test of the '_run_selected_command' function in Script mode."""
        process_result = SubprocessResult(0)
        mock_io = MagicMock()
        mock_run_script.return_value = process_result
        # pylint: disable-next=protected-access
        self.frame._run_selected_command(
            mode="Script",
            command_args=["script.py", "--x"],
            cwd=str(PATH_GUI),
            stdout=mock_io,
            stderr=mock_io,
        )
        mock_run_script.assert_called_once_with(
            python_args=["script.py", "--x"],
            cwd=str(PATH_GUI),
            stdout=mock_io,
            stderr=mock_io,
        )
        self.assertIs(self.frame.queue.get_nowait(), process_result)

    @patch("cli.cmd_gui.frame_run.run_gui.run_program_impl.run_program")
    def test_run_selected_command_program(self, mock_run_program: MagicMock) -> None:
        """Test of the '_run_selected_command' function in Program mode."""
        process_result = SubprocessResult(0)
        mock_io = MagicMock()
        mock_run_program.return_value = process_result
        # pylint: disable-next=protected-access
        self.frame._run_selected_command(
            mode="Program",
            command_args=["app.exe", "--x"],
            cwd=str(PATH_GUI),
            stdout=mock_io,
            stderr=mock_io,
        )
        mock_run_program.assert_called_once_with(
            args=["app.exe", "--x"], cwd=str(PATH_GUI), stdout=mock_io, stderr=mock_io
        )
        self.assertEqual(self.frame.queue.get_nowait(), process_result)


class TestRunFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the RunFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)

    @patch("cli.cmd_gui.frame_run.run_gui.file_name_from_current_time")
    def test_save_log(self, mock_time: MagicMock) -> None:
        """Test 'save_log_cb' function."""
        mock_run_frame = MagicMock()
        # pylint: disable-next=protected-access
        mock_run_frame._parse_args.return_value = ["mode", "target", "args"]
        mock_time.return_value = "new"
        file_name = PROJECT_BUILD_ROOT / "gui" / "mode_target_args_new.txt"
        mock_open_file = mock_open(read_data="log")
        with patch("builtins.open", mock_open_file):
            run_gui.RunFrame.save_log_cb(mock_run_frame)
        mock_time.assert_called_once()
        mock_open_file.assert_any_call(
            mock_run_frame.file_path, encoding="utf-8", errors="ignore"
        )
        mock_open_file.assert_any_call(
            file_name, mode="w", encoding="utf-8", errors="ignore"
        )
        mock_open_stream = mock_open_file()
        mock_open_stream.write.assert_called_once_with("log")
        mock_run_frame.indicator_canvas.itemconfig.assert_called_once_with(
            mock_run_frame.oval, fill="green"
        )
        mock_run_frame.status_label.config.assert_called_once_with(
            text=f"Log saved in the file '{file_name}'."
        )

    def test_parse_args_empty(self) -> None:
        """Test of the function 'parse_args' with blank input."""
        # pylint: disable-next=protected-access
        parse_args = run_gui.RunFrame._parse_args("   ")
        self.assertEqual([], parse_args)

    @patch("cli.cmd_gui.frame_run.run_gui.get_platform")
    def test_parse_args_win32(self, mock_get_platform: MagicMock) -> None:
        """Test of the function 'parse_args' on Windows."""
        mock_get_platform.return_value = "win32"
        # pylint: disable-next=protected-access
        parse_args = run_gui.RunFrame._parse_args(" 'argument' 1 ")
        mock_get_platform.assert_called_once()
        self.assertEqual(["'argument'", "1"], parse_args)

    @patch("cli.cmd_gui.frame_run.run_gui.get_platform")
    def test_parse_args_linux(self, mock_get_platform: MagicMock) -> None:
        """Test of the function 'parse_args' on Linux."""
        mock_get_platform.return_value = "linux"
        # pylint: disable-next=protected-access
        parse_args = run_gui.RunFrame._parse_args(" 'argument' 1 ")
        mock_get_platform.assert_called_once()
        self.assertEqual(["argument", "1"], parse_args)

    def test_generate_label_args(self) -> None:
        """Test of the function '_generate_label' with args given."""
        command = {"mode": "mode", "target": "target", "args": "args "}
        # pylint: disable-next=protected-access
        string = run_gui.RunFrame._generate_label(command)
        self.assertEqual("mode 'target args'", string)

    def test_generate_label_no_args(self) -> None:
        """Test of the function '_generate_label' without args."""
        command = {"mode": "mode", "target": "target", "args": " "}
        # pylint: disable-next=protected-access
        string = run_gui.RunFrame._generate_label(command)
        self.assertEqual("mode 'target'", string)

    def test_generate_label_empty(self) -> None:
        """Test of the function '_generate_label' without input."""
        # pylint: disable-next=protected-access
        string = run_gui.RunFrame._generate_label(None)
        self.assertEqual("Command", string)

    def test_delete_presets_file(self) -> None:
        """Test of the function 'delete_presets_cb' when the file exists"""
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = Path(__file__)
        mock_run_frame.presets = ["preset 1", "preset 2"]
        mock_run_frame.presets_combobox = MagicMock()
        with patch("cli.cmd_gui.frame_run.run_gui.Path.unlink") as mock_unlink:
            run_gui.RunFrame.delete_presets_cb(mock_run_frame)
            mock_unlink.assert_called_once()
        self.assertEqual([], mock_run_frame.presets)
        mock_run_frame.presets_combobox.set.assert_called_once_with("")

    def test_delete_presets_invalid_file(self) -> None:
        """Test of the function 'delete_presets_cb' when the file does not exist"""
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = Path("does/not/exist")
        mock_run_frame.presets = ["preset 1", "preset 2"]
        mock_presets_combobox = MagicMock()
        mock_presets_combobox["values"] = []
        mock_run_frame.presets_combobox = mock_presets_combobox
        with patch("cli.cmd_gui.frame_run.run_gui.Path.unlink") as mock_unlink:
            run_gui.RunFrame.delete_presets_cb(mock_run_frame)
            mock_unlink.assert_not_called()
        self.assertEqual([], mock_run_frame.presets)
        mock_presets_combobox.set.assert_called_once_with("")

    @patch("cli.cmd_gui.frame_run.run_gui.json.dump")
    def test_save_presets_to_file(self, mock_dump: MagicMock) -> None:
        """Test of the function 'save_presets_to_file'"""
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = "path/to/file"
        mock_run_frame.presets = []
        mock_open_file = mock_open()
        with patch("builtins.open", mock_open_file):
            run_gui.RunFrame.save_presets_to_file(mock_run_frame)
        mock_open_file.assert_called_once_with(
            mock_run_frame.preset_file, mode="w", encoding="utf-8"
        )
        mock_open_stream = mock_open_file()
        mock_dump.assert_called_once_with(
            mock_run_frame.presets, mock_open_stream, ensure_ascii=True, indent=2
        )

    def test_insert_text(self) -> None:
        """Test of the function 'insert_text'"""
        mock_entry_obj = MagicMock(spec=ttk.Entry)
        # pylint: disable-next=protected-access
        run_gui.RunFrame._insert_text(mock_entry_obj, "input")
        mock_entry_obj.delete.assert_called_once_with(0, tk.END)
        mock_entry_obj.insert.assert_called_once_with(tk.END, "input")


class TestLoadPresetsNoUiTestableMethods(unittest.TestCase):
    """Test of the 'load_presets' function of the RunFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)

    def test_invalid_path(self) -> None:
        """Given path is not a file"""
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = Path("invalid/file")
        loaded_presets = run_gui.RunFrame.load_presets(mock_run_frame)
        self.assertListEqual([], loaded_presets)

    def test_invalid_file(self) -> None:
        """Loading file causes error"""
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = Path(__file__)
        loaded_presets = run_gui.RunFrame.load_presets(mock_run_frame)
        self.assertListEqual([], loaded_presets)

    @patch("cli.cmd_gui.frame_run.run_gui.json")
    def test_empty_file(self, mock_json: MagicMock) -> None:
        """Given file has no content"""
        mock_json.load.return_value = ""
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = Path(__file__)
        loaded_presets = run_gui.RunFrame.load_presets(mock_run_frame)
        self.assertListEqual([], loaded_presets)

    @patch("cli.cmd_gui.frame_run.run_gui.json")
    def test_valid_presets(self, mock_json: MagicMock) -> None:
        """File contains several valid and invalid presets"""
        presets = [
            {
                "mode": "mode",
                "target": "target",
                "args": "args",
                "cwd": "cwd",
                "content": "content",
            },
            "",
            {"mode": "mode", "content": "content"},
            {"mode": "mode 1", "target": "target 1", "args": "args 1", "cwd": "cwd 1"},
        ]
        mock_json.load.return_value = presets
        mock_run_frame = MagicMock()
        mock_run_frame.preset_file = Path(__file__)
        loaded_presets = run_gui.RunFrame.load_presets(mock_run_frame)
        self.assertEqual(2, len(loaded_presets))
        self.assertListEqual(
            [
                {"mode": "mode", "target": "target", "args": "args", "cwd": "cwd"},
                presets[3],
            ],
            loaded_presets,
        )


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestInit(unittest.TestCase):
    """Test initialization of the BootloaderFrame class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)
        importlib.reload(frame_base)

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    def test_not_dir(self, mock_generate_label: MagicMock) -> None:
        """Initialize the RunFrame class when the directory does not exist"""
        mock_generate_label.return_value = "preset label"
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
        cache_dir = run_gui.CACHE_DIR
        run_gui.CACHE_DIR = Path(__file__).parent.parent / "test_data"
        root = tk.Tk()
        root.withdraw()
        text = tk.Text()
        with patch(
            "cli.cmd_gui.frame_run.run_gui.PROJECT_ROOT", new=Path("does/not/exist")
        ):
            run_frame = run_gui.RunFrame(root, text)
        self.assertEqual(run_frame.working_directory_entry.get().strip(), "")
        self.assertEqual(2, len(run_frame.presets))
        self.assertDictEqual(
            {"mode": "Program", "target": "program.exe", "args": "", "cwd": "D:\\"},
            run_frame.presets[0],
        )
        self.assertDictEqual(
            {
                "mode": "Script",
                "target": "script.py",
                "args": "--help",
                "cwd": "C:\\",
            },
            run_frame.presets[1],
        )
        self.assertEqual(2, len(run_frame.presets_combobox["values"]))
        root.update()
        root.destroy()
        run_gui.CACHE_DIR = cache_dir

    @patch("cli.cmd_gui.frame_run.run_gui.RunFrame._generate_label")
    def test_is_dir(self, mock_generate_label: MagicMock) -> None:
        """Initialize the RunFrame class when directory exists"""
        mock_generate_label.return_value = "preset label"
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
        cache_dir = run_gui.CACHE_DIR
        run_gui.CACHE_DIR = Path(__file__).parent.parent / "test_data"
        root = tk.Tk()
        root.withdraw()
        text = tk.Text()
        with patch("cli.cmd_gui.frame_run.run_gui.PROJECT_ROOT", new=PATH_GUI):
            run_frame = run_gui.RunFrame(root, text)
        self.assertEqual(run_frame.working_directory_entry.get().strip(), str(PATH_GUI))
        self.assertEqual(2, len(run_frame.presets))
        self.assertDictEqual(
            {"mode": "Program", "target": "program.exe", "args": "", "cwd": "D:\\"},
            run_frame.presets[0],
        )
        self.assertDictEqual(
            {
                "mode": "Script",
                "target": "script.py",
                "args": "--help",
                "cwd": "C:\\",
            },
            run_frame.presets[1],
        )
        self.assertEqual(2, len(run_frame.presets_combobox["values"]))
        root.update()
        root.destroy()
        run_gui.CACHE_DIR = cache_dir


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
