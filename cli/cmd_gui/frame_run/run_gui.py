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

# cspell:ignore initialdir

"""Implements the 'run_program' and 'run_script' frame"""

from __future__ import annotations

import json
import shlex
import tkinter as tk
from pathlib import Path
from queue import Queue
from threading import Thread
from tkinter import filedialog as fd
from tkinter import ttk
from typing import TextIO

from ...cmd_run_program import run_program_impl
from ...cmd_run_script import run_script_impl
from ...helpers.dirs import CACHE_DIR
from ...helpers.host_platform import get_platform
from ...helpers.misc import (
    PROJECT_BUILD_ROOT,
    PROJECT_ROOT,
    file_name_from_current_time,
)


# pylint: disable-next=too-many-ancestors,too-many-instance-attributes
class RunFrame(ttk.Frame):
    """'Run' frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent)

        self.parent = parent
        self.text = text_widget
        self.text_index: int = 0
        self.file_path = PROJECT_BUILD_ROOT / "gui" / "output_gui_run.txt"
        (PROJECT_BUILD_ROOT / "gui").mkdir(parents=True, exist_ok=True)
        self.file_path.touch()

        self.run_process: Thread
        self.queue: Queue = Queue()
        self.file_stream: TextIO
        self.current_command: str = ""
        self.current_preset: dict[str, str] | None = None
        self.presets: list[dict[str, str]] = []
        CACHE_DIR.mkdir(parents=True, exist_ok=True)
        self.preset_file = CACHE_DIR / "run_frame_presets.json"

        self.columnconfigure(0, weight=1)

        input_frame = ttk.Labelframe(self, text="Run Configuration", padding=(10, 5))
        input_frame.grid(column=0, row=0, padx=10, pady=(10, 5), sticky="news")
        input_frame.columnconfigure(1, weight=1)

        mode_label = ttk.Label(self, text="Mode", width=18)
        mode_label.grid(in_=input_frame, column=0, row=0, pady=(0, 5), sticky="news")

        self.mode_combobox = ttk.Combobox(
            self, width=50, values=["Script", "Program"], state="readonly"
        )
        self.mode_combobox.grid(
            in_=input_frame, column=1, row=0, pady=(0, 5), sticky="news"
        )
        self.mode_combobox.current(0)

        target_label = ttk.Label(self, text="Target", width=18)
        target_label.grid(in_=input_frame, column=0, row=1, pady=5, sticky="news")

        self.target_entry = ttk.Entry(self, width=50)
        self.target_entry.grid(in_=input_frame, column=1, row=1, pady=5, sticky="news")

        target_button = ttk.Button(
            self, text="Choose file", command=self.open_target_cb
        )
        target_button.grid(in_=input_frame, column=2, row=1, pady=5, sticky="news")

        args_label = ttk.Label(self, text="Arguments", width=18)
        args_label.grid(in_=input_frame, column=0, row=2, pady=5, sticky="news")

        self.args_entry = ttk.Entry(self, width=50)
        self.args_entry.grid(in_=input_frame, column=1, row=2, pady=5, sticky="news")

        cwd_label = ttk.Label(self, text="Working directory", width=18)
        cwd_label.grid(in_=input_frame, column=0, row=3, pady=5, sticky="news")

        self.cwd_entry = ttk.Entry(self, width=50)
        self.cwd_entry.grid(in_=input_frame, column=1, row=3, pady=5, sticky="news")
        self.cwd_entry.insert(tk.END, str(PROJECT_ROOT))

        cwd_button = ttk.Button(self, text="Choose directory", command=self.open_cwd_cb)
        cwd_button.grid(in_=input_frame, column=2, row=3, pady=5, sticky="news")

        preset_label = ttk.Label(self, text="Recent Presets", width=18)
        preset_label.grid(in_=input_frame, column=0, row=4, pady=5, sticky="news")

        self.preset_combobox = ttk.Combobox(self, width=50, values=[], state="readonly")
        self.preset_combobox.grid(
            in_=input_frame, column=1, row=4, pady=5, sticky="news"
        )
        self.preset_combobox.bind("<<ComboboxSelected>>", self.apply_preset_cb)
        self.load_presets()

        button_frame = ttk.Frame(self)
        button_frame.grid(column=0, row=1, pady=(5, 10), sticky="ns")

        self.run_button = ttk.Button(self, text="Run", command=self.run_command_cb)
        self.run_button.pack(in_=button_frame, side=tk.LEFT, ipadx=1, ipady=8, padx=3)

        self.save_button = ttk.Button(
            self, text="Save Log", command=self.save_command_cb
        )
        self.save_button.pack(in_=button_frame, side=tk.LEFT, ipadx=1, ipady=8, padx=3)
        self.save_button.state(["disabled"])

        state_frame = ttk.Frame(self, padding=(0, 10))
        state_frame.grid(column=0, row=2)

        self.canvas = tk.Canvas(self, width=30, height=30)
        self.canvas.pack(in_=state_frame, side=tk.LEFT)
        self.oval = self.canvas.create_oval(*((5, 5), (25, 25)), fill="darkgrey")

        self.status_text = ttk.Label(self, text="No command has been run.")
        self.status_text.pack(in_=state_frame, side=tk.LEFT)

    def open_target_cb(self) -> None:
        """Open file dialog and write selected target path into target entry."""
        if self.mode_combobox.get() == "Script":
            file_types = [("Python Files", "*.py"), ("All Files", "*.*")]
        else:
            file_types = [("All Files", "*.*")]
        file_path = fd.askopenfilename(filetypes=file_types)
        if file_path:
            self.target_entry.delete(0, tk.END)
            self.target_entry.insert(tk.END, file_path)

    def open_cwd_cb(self) -> None:
        """Open directory dialog and write selected cwd into cwd entry."""
        directory = fd.askdirectory(initialdir=str(PROJECT_ROOT))
        if directory:
            self.cwd_entry.delete(0, tk.END)
            self.cwd_entry.insert(tk.END, directory)

    def _parse_args(self, args: str) -> list[str]:
        """Parse free-form argument string into a list preserving quoted segments."""
        if not args.strip():
            return []
        # pylint: disable-next=superfluous-parens
        return shlex.split(args, posix=(get_platform() != "win32"))

    def is_debug_enabled(self) -> bool:
        """Return whether GUI debug mode is enabled on the top-level window."""
        return bool(getattr(self.winfo_toplevel(), "debug_gui", False))

    def write_debug(self, file_input: str) -> None:
        """Write debug text when --debug-gui is enabled."""
        if self.is_debug_enabled():
            self.write_text(f"[debug] {file_input}\n")

    @staticmethod
    def _preset_label(preset: dict[str, str]) -> str:
        """Return a compact display label for a stored preset."""
        mode = preset["mode"]
        target = preset["target"]
        args = preset["args"].strip()
        if args:
            return f"{mode}: {target} {args}"
        return f"{mode}: {target}"

    def remember_preset(self, preset: dict[str, str]) -> None:
        """Store a run preset for quick reuse and keep newest presets first."""
        self.presets = [existing for existing in self.presets if existing != preset]
        self.presets.insert(0, preset)
        self.presets = self.presets[:10]
        labels = [self._preset_label(existing) for existing in self.presets]
        self.preset_combobox["values"] = labels
        self.preset_combobox.set(labels[0])
        self.save_presets()

    def save_presets(self) -> None:
        """Persist presets in cache directory so they are available next session."""
        with open(self.preset_file, mode="w", encoding="utf-8") as f:
            json.dump(self.presets, f, ensure_ascii=True, indent=2)

    def load_presets(self) -> None:
        """Load presets from cache file and populate combobox values."""
        if not self.preset_file.is_file():
            return
        try:
            with open(self.preset_file, encoding="utf-8") as f:
                loaded = json.load(f)
        except (json.JSONDecodeError, OSError):
            return

        valid_presets: list[dict[str, str]] = []
        for preset in loaded:
            if not isinstance(preset, dict):
                continue
            if not {"mode", "target", "args", "cwd"}.issubset(preset):
                continue
            valid_presets.append(
                {
                    "mode": str(preset["mode"]),
                    "target": str(preset["target"]),
                    "args": str(preset["args"]),
                    "cwd": str(preset["cwd"]),
                }
            )

        if not valid_presets:
            return
        self.presets = valid_presets[:10]
        labels = [self._preset_label(existing) for existing in self.presets]
        self.preset_combobox["values"] = labels
        self.preset_combobox.set(labels[0])

    def apply_preset_cb(self, event: tk.Event | None = None) -> None:
        """Apply selected preset values to the input fields."""
        selected_label = self.preset_combobox.get().strip()
        if not selected_label:
            return
        for preset in self.presets:
            if self._preset_label(preset) != selected_label:
                continue
            self.mode_combobox.set(preset["mode"])
            self.target_entry.delete(0, tk.END)
            self.target_entry.insert(tk.END, preset["target"])
            self.args_entry.delete(0, tk.END)
            self.args_entry.insert(tk.END, preset["args"])
            self.cwd_entry.delete(0, tk.END)
            self.cwd_entry.insert(tk.END, preset["cwd"])
            self.write_debug(f"Applied preset '{selected_label}'")
            return

    def run_command_cb(self) -> None:
        """Run script or program based on selected mode and input fields."""
        self.text_index = 0
        self.text.config(state="normal")
        self.text.delete("1.0", tk.END)
        self.text.config(state="disabled")

        mode = self.mode_combobox.get().strip()
        target = self.target_entry.get().strip()
        cwd = self.cwd_entry.get().strip()

        if not target:
            self.status_text.config(text="Please provide a target file/program.")
            self.canvas.itemconfig(self.oval, fill="red")
            return

        if not Path(cwd).is_dir():
            self.status_text.config(text="Please provide a valid working directory.")
            self.canvas.itemconfig(self.oval, fill="red")
            return

        args = self._parse_args(self.args_entry.get())
        command_list = [target] + args
        self.current_command = f"{mode.lower()}: {' '.join(command_list)}"
        self.current_preset = {
            "mode": mode,
            "target": target,
            "args": self.args_entry.get().strip(),
            "cwd": cwd,
        }
        self.write_debug(f"Running in cwd '{cwd}': {' '.join(command_list)}")
        self.status_text.config(text=f"Running command: {self.current_command}")

        self.run_button.state(["disabled"])
        self.save_button.state(["disabled"])
        self.canvas.itemconfig(self.oval, fill="darkgrey")
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="w", encoding="utf-8")  # noqa: SIM115

        self.run_process = Thread(
            target=self._run_selected_command,
            kwargs={
                "mode": mode,
                "command_args": command_list,
                "cwd": cwd,
                "stdout": self.file_stream,
                "stderr": self.file_stream,
            },
            daemon=True,
        )
        self.run_process.start()
        self.check_thread()

    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def _run_selected_command(
        self,
        mode: str,
        command_args: list[str],
        cwd: str,
        stdout: TextIO,
        stderr: TextIO,
    ) -> None:
        """Run selected command mode and place subprocess result into queue."""
        if mode == "Script":
            result = run_script_impl.run_python_script(
                command_args,
                cwd=cwd,
                stdout=stdout,
                stderr=stderr,
            )
        else:
            result = run_program_impl.run_program(
                command_args,
                cwd=cwd,
                stdout=stdout,
                stderr=stderr,
            )
        self.queue.put(result)

    def check_thread(self) -> None:
        """Poll execution thread and update status when it finishes."""
        self.write_text()
        if self.run_process.is_alive():
            self.after(50, self.check_thread)
            return

        self.run_button.state(["!disabled"])
        self.save_button.state(["!disabled"])
        self.file_stream.close()
        if not self.queue.empty() and (return_value := self.queue.get()):
            if return_value.returncode == 0:
                self.canvas.itemconfig(self.oval, fill="green")
                self.status_text.config(
                    text=f"Command '{self.current_command}' was successful."
                )
                if self.current_preset is not None:
                    self.remember_preset(self.current_preset)
                self.write_debug("Runner command finished successfully")
            else:
                self.canvas.itemconfig(self.oval, fill="red")
                self.status_text.config(
                    text=f"Command '{self.current_command}' failed "
                    f"(exit code {return_value.returncode})."
                )
                self.write_debug(
                    f"Runner command failed with exit code {return_value.returncode}"
                )

    def save_command_cb(self) -> None:
        """Save current run log into a timestamped file."""
        gui_dir = self.file_path.parent
        timestamp = str(file_name_from_current_time())
        file = gui_dir / Path(f"run_{timestamp}.txt")
        with open(self.file_path, encoding="utf-8", errors="ignore") as log:
            log_content = log.read()
        with open(file, mode="w", encoding="utf-8", errors="ignore") as f:
            f.write(log_content)
        self.status_text.config(text=f"Log saved in file '{file}'.")

    def write_text(self, file_input: None | str = None) -> None:
        """Writes the file content in the text box."""
        if file_input is not None:
            with open(self.file_path, mode="a", encoding="utf-8", errors="ignore") as f:
                f.write(file_input)
        if self != self.parent.nametowidget(self.parent.select()):
            return
        self.text.config(state="normal")
        with open(self.file_path, encoding="utf-8", errors="ignore") as f:
            file_content = f.read()
            text_length = len(file_content)
            self.text.insert(tk.END, file_content[self.text_index :])
            if text_length > 0:
                self.text_index = text_length
            self.text.see(tk.END)
        self.text.config(state="disabled")
