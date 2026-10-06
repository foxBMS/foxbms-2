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
from ...helpers.misc import file_name_from_current_time
from ...helpers.project_context import PROJECT_BUILD_ROOT, PROJECT_ROOT
from ...helpers.spr import SubprocessResult
from ..frame_base import BaseFrame


# pylint: disable-next=too-many-ancestors, too-many-instance-attributes
class RunFrame(BaseFrame):
    """'Run Program/Script' frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent, text_widget, "output_gui_run.txt")

        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)
        self.rowconfigure(3, weight=3)

        self.run_process: Thread
        self.queue: Queue[SubprocessResult] = Queue()
        self.current_command: dict[str, str] | None = None
        CACHE_DIR.mkdir(parents=True, exist_ok=True)
        self.preset_file = CACHE_DIR / "run_frame_presets.json"
        self.presets: list[dict[str, str]] = self.load_presets()

        # Create a frame for all input
        input_frame = ttk.Frame(self)
        input_frame.grid(column=0, row=0, padx=20, pady=(15, 0), sticky="nwe")
        input_frame.columnconfigure(1, weight=1)
        input_frame.rowconfigure((0, 1, 2, 3, 4), weight=1, uniform="a")

        ttk.Label(input_frame, text="Mode").grid(
            column=0, row=0, padx=(0, 10), pady=5, sticky="we"
        )
        self.mode_combobox = ttk.Combobox(
            input_frame, values=["Script", "Program"], state="readonly"
        )
        self.mode_combobox.grid(column=1, row=0, pady=5, sticky="we")
        self.mode_combobox.current(0)

        ttk.Label(input_frame, text="Target File/Program").grid(
            column=0, row=1, padx=(0, 10), pady=5, sticky="we"
        )
        self.target_script_entry = ttk.Entry(input_frame)
        self.target_script_entry.grid(column=1, row=1, pady=5, sticky="we")

        ttk.Button(
            input_frame, text="Select File", command=self.select_target_file_cb
        ).grid(column=2, row=1, pady=5, sticky="we")

        ttk.Label(input_frame, text="Arguments").grid(
            column=0, row=2, padx=(0, 10), pady=5, sticky="we"
        )
        self.arguments_entry = ttk.Entry(input_frame)
        self.arguments_entry.grid(column=1, row=2, pady=5, sticky="we")

        ttk.Label(input_frame, text="Working Directory").grid(
            column=0, row=3, padx=(0, 10), pady=5, sticky="we"
        )
        self.working_directory_entry = ttk.Entry(input_frame)
        self.working_directory_entry.grid(column=1, row=3, pady=5, sticky="we")
        if PROJECT_ROOT.is_dir():
            self.working_directory_entry.insert(tk.END, str(PROJECT_ROOT))
        ttk.Button(
            input_frame,
            text="Select\nDirectory",
            command=self.select_directory_cb,
            style="Multiline.TButton",
        ).grid(column=2, row=3, sticky="we")

        ttk.Label(input_frame, text="Presets").grid(
            column=0, row=4, padx=(0, 10), pady=5, sticky="we"
        )
        self.presets_combobox = ttk.Combobox(
            input_frame,
            values=[self._generate_label(preset) for preset in self.presets],
            state="readonly",
        )
        self.presets_combobox.grid(column=1, row=4, pady=5, sticky="we")
        self.presets_combobox.bind("<<ComboboxSelected>>", self.apply_preset_cb)

        ttk.Button(
            input_frame,
            text="Delete\nPresets",
            command=self.delete_presets_cb,
            style="Multiline.TButton",
        ).grid(column=2, row=4, pady=(1, 0), sticky="we")

        # Create a Frame for the Buttons
        button_frame = ttk.Frame(self)
        button_frame.grid(column=0, row=1, pady=5, sticky="n")

        self.run_button = ttk.Button(
            button_frame, text="Run", command=self.run_cb, padding=(0, 5)
        )
        self.run_button.pack(side=tk.LEFT, padx=(0, 5), fill=tk.Y)

        self.save_log_button = ttk.Button(
            button_frame, text="Save Log", command=self.save_log_cb, padding=(0, 5)
        )
        self.save_log_button.pack(side=tk.LEFT, padx=(5, 0), fill=tk.Y)
        self.save_log_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]

        # Create a Status Frame
        state_frame = ttk.Frame(self)
        state_frame.grid(column=0, row=2, pady=5, sticky="n")

        self.indicator_canvas = tk.Canvas(state_frame, width=30, height=30)
        self.indicator_canvas.pack(side=tk.LEFT)
        self.oval = self.indicator_canvas.create_oval(
            *((5, 5), (25, 25)), fill="lightgrey"
        )

        self.status_label = ttk.Label(
            state_frame, text="No command has been run.", justify=tk.LEFT, anchor=tk.W
        )
        self.status_label.pack(side=tk.LEFT)

    def select_target_file_cb(self) -> None:
        """Open filedialog and write selected item into Entry widget."""
        if self.mode_combobox.get() == "Script":
            file_types = [("Python Files", "*.py"), ("All Files", "*.*")]
        else:
            file_types = [("All Files", "*.*")]
        file_path = fd.askopenfilename(filetypes=file_types)
        if file_path:
            self.target_script_entry.delete(0, tk.END)
            self.target_script_entry.insert(tk.END, file_path)

    def select_directory_cb(self) -> None:
        """Open directory dialog and write selected item into Entry widget."""
        directory = fd.askdirectory(initialdir=str(PROJECT_ROOT))
        if directory:
            self.working_directory_entry.delete(0, tk.END)
            self.working_directory_entry.insert(tk.END, directory)

    def apply_preset_cb(self, event: tk.Event | None = None) -> None:
        """Apply selected preset values to the input fields."""
        selected_preset = self.presets_combobox.current()
        if (selected_preset == -1) or (selected_preset >= len(self.presets)):
            return
        self.mode_combobox.set(self.presets[selected_preset]["mode"])
        self._insert_text(
            self.target_script_entry, self.presets[selected_preset]["target"]
        )
        self._insert_text(self.arguments_entry, self.presets[selected_preset]["args"])
        self._insert_text(
            self.working_directory_entry, self.presets[selected_preset]["cwd"]
        )
        self.status_label.config(
            text=f"Applied preset '{self._generate_label(self.presets[selected_preset])}'."
        )
        self.indicator_canvas.itemconfig(self.oval, fill="lightgrey")

    def delete_presets_cb(self) -> None:
        """Delete preset file and combobox values."""
        if self.preset_file.is_file():
            self.preset_file.unlink()
        self.presets = []
        self.presets_combobox.set("")
        self.presets_combobox["values"] = []

    def run_cb(self) -> None:
        """Run script or program based on selected mode and input fields."""
        self.indicator_canvas.itemconfig(self.oval, fill="darkgrey")
        self.reset_text()

        mode = self.mode_combobox.get().strip()
        target = self.target_script_entry.get().strip()
        cwd = self.working_directory_entry.get().strip()
        args = self.arguments_entry.get().strip()

        if not target or (mode == "Script" and not Path(target).is_file()):
            self.status_label.config(text="Please provide a target file/program.")
            self.indicator_canvas.itemconfig(self.oval, fill="lightgrey")
            return

        if not Path(cwd).is_dir():
            self.status_label.config(text="Please provide a valid working directory.")
            self.indicator_canvas.itemconfig(self.oval, fill="lightgrey")
            return

        self.current_command = {
            "mode": mode,
            "target": target,
            "args": args,
            "cwd": cwd,
        }
        self.status_label.config(
            text=f"Running '{self._generate_label(self.current_command)}'."
        )

        self.run_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        self.save_log_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="w", encoding="utf-8")  # noqa: SIM115
        self.run_process = Thread(
            target=self._run_selected_command,
            kwargs={
                "mode": mode,
                "command_args": [target] + self._parse_args(args),
                "cwd": cwd,
                "stdout": self.file_stream,
                "stderr": self.file_stream,
            },
            daemon=True,
        )
        self.run_process.start()
        self.check_thread()

    def save_log_cb(self) -> None:
        """Save current run log into a timestamped file."""
        gui_dir = PROJECT_BUILD_ROOT / "gui"
        gui_dir.mkdir(parents=True, exist_ok=True)
        timestamp = str(file_name_from_current_time())
        current_command = "_".join(
            self._parse_args(self._generate_label(self.current_command))
        )
        file_name = gui_dir / f"{current_command}_{timestamp}.txt"
        with open(self.file_path, encoding="utf-8", errors="ignore") as log:
            log_content = log.read()
        with open(file_name, mode="w", encoding="utf-8", errors="ignore") as f:
            f.write(log_content)
        self.indicator_canvas.itemconfig(self.oval, fill="green")
        self.status_label.config(text=f"Log saved in the file '{file_name}'.")

    def save_preset(self, new_preset: dict[str, str]) -> None:
        """Save input as preset for quick reuse and add new presets at the top."""
        self.presets = [preset for preset in self.presets if preset != new_preset]
        self.presets.insert(0, new_preset)
        self.presets = self.presets[:10]
        self.presets_combobox["values"] = [
            self._generate_label(preset) for preset in self.presets
        ]
        self.presets_combobox.current(0)
        self.save_presets_to_file()

    def save_presets_to_file(self) -> None:
        """Save presets in cache directory for next sessions."""
        with open(self.preset_file, mode="w", encoding="utf-8") as f:
            json.dump(self.presets, f, ensure_ascii=True, indent=2)

    def load_presets(self) -> list[dict[str, str]]:
        """Load presets from cache file."""
        if not self.preset_file.is_file():
            return []
        try:
            with open(self.preset_file, encoding="utf-8") as f:
                loaded_presets = json.load(f)
        except (json.JSONDecodeError, OSError):
            return []
        valid_presets: list[dict[str, str]] = []
        for preset in loaded_presets:
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
            return []
        return valid_presets[:10]

    def _run_selected_command(
        self,
        mode: str,
        command_args: list[str],
        cwd: str,
        stdout: TextIO,
        stderr: TextIO,
    ) -> None:
        """Run selected mode with provided arguments and place result into queue."""
        if mode == "Script":
            result = run_script_impl.run_python_script(
                python_args=command_args,
                cwd=cwd,
                stdout=stdout,
                stderr=stderr,
            )
        else:
            result = run_program_impl.run_program(
                args=command_args,
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
        self.run_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        self.save_log_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        self.file_stream.close()
        if not self.queue.empty() and (return_value := self.queue.get()):
            if return_value.returncode == 0:
                self.indicator_canvas.itemconfig(self.oval, fill="green")
                self.status_label.config(
                    text=f"{self._generate_label(self.current_command)} was successful."
                )
                if self.current_command is not None:
                    self.save_preset(self.current_command)
            else:
                self.indicator_canvas.itemconfig(self.oval, fill="red")
                self.status_label.config(
                    text=f"{self._generate_label(self.current_command)} "
                    f"failed with exit code {return_value.returncode}."
                )

    @staticmethod
    def _parse_args(args: str) -> list[str]:
        """Parse free-form argument string into a list preserving quoted segments."""
        if not args.strip():
            return []
        # pylint: disable-next=superfluous-parens
        return shlex.split(args, posix=(get_platform() != "win32"))

    @staticmethod
    def _generate_label(command: dict[str, str] | None) -> str:
        """Return a compact label for a command."""
        if not command:
            return "Command"
        mode = command["mode"]
        target = command["target"]
        args = command["args"].strip()
        if args:
            return f"{mode} '{target} {args}'"
        return f"{mode} '{target}'"

    @staticmethod
    def _insert_text(entry_obj: ttk.Entry, input_str: str) -> None:
        """Delete content of Entry widget and insert the input."""
        entry_obj.delete(0, tk.END)
        entry_obj.insert(tk.END, input_str)
