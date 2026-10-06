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

# cspell:ignore activestyle, selectmode

"""Implements the 'build' frame"""

import tkinter as tk
from dataclasses import dataclass
from queue import Queue
from threading import Thread
from tkinter import ttk

from ...helpers.misc import file_name_from_current_time
from ...helpers.project_context import PROJECT_BUILD_ROOT, ROOT_IS_PROJECT
from ...helpers.spr import SubprocessResult
from ..frame_base import BaseFrame

if ROOT_IS_PROJECT:
    from ...cmd_build.build_impl import run_top_level_waf
else:

    def dummy(
        args: list[str],  # pylint: disable=unused-argument
        stdout: int | None = None,  # pylint: disable=unused-argument
        stderr: int | None = None,  # pylint: disable=unused-argument
    ) -> SubprocessResult:
        """Do nothing"""
        return SubprocessResult(0)

    run_top_level_waf = dummy


@dataclass
class Command:
    """Container for a Command and its help-string"""

    name: str
    help: str


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class BuildFrame(BaseFrame):
    """'Build' Frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent, text_widget, "output_gui_build.txt")

        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        self.build_process: Thread
        self.result_queue: Queue[SubprocessResult] = Queue()
        self.current_command: str = ""
        self.commands: list[Command] = []
        self.reduced_commands: list[Command] = []

        # Create a listbox for commands
        self.commands_listbox = tk.Listbox(
            self, height=10, width=80, selectmode=tk.SINGLE, activestyle="none"
        )
        self.commands_listbox.grid(column=0, row=0, pady=(10, 5), sticky="ns")

        # Create a frame for searching for a command
        search_frame = ttk.Frame(self)
        search_frame.grid(column=0, row=1)

        ttk.Label(search_frame, text="Search commands:").pack(
            side=tk.LEFT, padx=(0, 10), pady=10
        )
        self.search_command_entry = ttk.Entry(search_frame, width=30)
        self.search_command_entry.pack(side=tk.LEFT, pady=10)
        self.search_command_entry.bind("<KeyRelease>", self.select_command_cb)

        # Create a Frame for the Buttons
        button_frame = ttk.Frame(self)
        button_frame.grid(column=0, row=2, pady=5)

        # Create a "Generate Command List" button
        self.command_list_button = ttk.Button(
            button_frame,
            text="Generate\nCommand List",
            command=self.generate_command_list_cb,
            style="Multiline.TButton",
        )
        self.command_list_button.pack(side=tk.LEFT, padx=(0, 5), fill=tk.Y)

        # Create a "Run" button
        self.run_button = ttk.Button(
            button_frame, text="Run", command=self.run_command_cb
        )
        self.run_button.pack(side=tk.LEFT, padx=5, fill=tk.Y)
        self.run_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]

        # Create a "Save Log" button
        self.save_log_button = ttk.Button(
            button_frame, text="Save Log", command=self.save_log_cb
        )
        self.save_log_button.pack(side=tk.LEFT, padx=(5, 0), fill=tk.Y)
        self.save_log_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]

        # Create a Status Frame
        state_frame = ttk.Frame(self)
        state_frame.grid(column=0, row=3, pady=5)

        # Create Indicator Canvas
        self.indicator_canvas = tk.Canvas(state_frame, width=30, height=30)
        self.indicator_canvas.pack(side=tk.LEFT)
        self.oval = self.indicator_canvas.create_oval(
            *((5, 5), (25, 25)), fill="lightgrey"
        )
        # Create Status Label
        self.status_label = ttk.Label(
            state_frame, text="No command has been run.", justify=tk.LEFT, anchor=tk.W
        )
        self.status_label.pack(side=tk.LEFT)

    def generate_command_list_cb(self) -> None:
        """Run 'waf --help' and use the output as the command list"""
        self.current_command = "Generate Command List"
        self.reset_text()
        self.run_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        self.command_list_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        self.indicator_canvas.itemconfig(self.oval, fill="darkgrey")
        self.status_label.config(text=f"Running command '{self.current_command}'.")

        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="w", encoding="utf-8")  # noqa: SIM115
        self.build_process = Thread(
            target=run_top_level_waf,
            kwargs={
                "args": ["--help", "--color=no"],
                "stdout": self.file_stream,
                "stderr": self.file_stream,
            },
            daemon=True,
        )
        self.build_process.start()
        self.check_thread()

    def run_command_cb(self) -> None:
        """Run the provided build command"""
        if not self.commands_listbox.curselection():  # type: ignore[no-untyped-call]
            self.status_label.config(text="Please select a command.")
            self.indicator_canvas.itemconfig(self.oval, fill="lightgrey")
            return
        self.indicator_canvas.itemconfig(self.oval, fill="darkgrey")
        self.reset_text()
        self.run_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        self.command_list_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        command: int = self.commands_listbox.curselection()[0]  # type: ignore[no-untyped-call]
        self.current_command = self.reduced_commands[command].name
        self.status_label.config(text=f"Running command '{self.current_command}'.")
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="w", encoding="utf-8")  # noqa: SIM115
        self.build_process = Thread(
            target=lambda args, stdout, stderr: self.result_queue.put(
                # Decorator prevents function from being executed if it is not available
                run_top_level_waf(args=args, stdout=stdout, stderr=stderr)
            ),
            kwargs={
                "args": [self.current_command] + ["--color=no"],
                "stdout": self.file_stream,
                "stderr": self.file_stream,
            },
            daemon=True,
        )
        self.build_process.start()
        self.check_thread()

    def check_thread(self) -> None:
        """If the provided thread is not alive the button is activated."""
        if self.current_command != "Generate Command List":
            self.write_text()
        if self.build_process.is_alive():
            self.after(50, self.check_thread)
            return
        if self.current_command == "Generate Command List":
            self.generate_command_list()
        else:
            self.save_log_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        self.command_list_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        self.run_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        self.file_stream.close()
        if not self.result_queue.empty() and (return_value := self.result_queue.get()):
            if return_value.returncode == 0:
                self.indicator_canvas.itemconfig(self.oval, fill="green")
                self.status_label.config(
                    text=f"Command '{self.current_command}' was successful."
                )
            else:
                self.indicator_canvas.itemconfig(self.oval, fill="red")
                self.status_label.config(
                    text=f"Command '{self.current_command}' failed."
                )

    def save_log_cb(self) -> None:
        """Save current log into a timestamped file."""
        gui_dir = PROJECT_BUILD_ROOT / "gui"
        gui_dir.mkdir(parents=True, exist_ok=True)
        timestamp = str(file_name_from_current_time())
        file_name = gui_dir / f"{self.current_command}_{timestamp}.txt"
        with open(self.file_path, encoding="utf-8", errors="ignore") as log:
            log_content = log.read()
        with open(file_name, mode="w", encoding="utf-8", errors="ignore") as f:
            f.write(log_content)
        self.indicator_canvas.itemconfig(self.oval, fill="green")
        self.status_label.config(text=f"Log saved in the file '{file_name}'.")

    def generate_command_list(self) -> None:
        """Generate the command list from the output of 'waf --help'"""
        with open(self.file_path, encoding="utf-8") as f:
            text = f.read()
        self.commands.clear()
        self.reduced_commands.clear()
        self.commands_listbox.delete(0, tk.END)
        lines = text.splitlines()
        in_main_commands = False
        parsed_commands: list[Command] = []
        for line in lines:
            stripped_line = line.strip()
            if not in_main_commands:
                if stripped_line.startswith("Main commands"):
                    in_main_commands = True
                continue
            if stripped_line.lower().startswith("options:"):
                break
            if ":" not in stripped_line:
                continue
            command_data = stripped_line.split(":", maxsplit=1)
            if not command_data[0].strip():
                continue
            parsed_commands.append(
                Command(
                    name=command_data[0].strip(),
                    help=command_data[1].strip().rstrip(":"),
                )
            )
        # Remove command list from file
        open(self.file_path, mode="w", encoding="utf-8").close()  # pylint: disable=consider-using-with
        if not parsed_commands:
            self.indicator_canvas.itemconfig(self.oval, fill="red")
            self.status_label.config(
                text="Could not parse command list from 'waf --help' output."
            )
            return
        for command in parsed_commands:
            self.commands.append(command)
            self.reduced_commands.append(command)
            self.commands_listbox.insert(tk.END, f"{command.name}  ({command.help})")
        self.indicator_canvas.itemconfig(self.oval, fill="lightgrey")
        self.status_label.config(text="Select a command to run.")

    def select_command_cb(self, event: tk.Event) -> None:
        """Reduce the available commands according to the search term"""
        searched_command: str = self.search_command_entry.get()
        self.commands_listbox.delete(0, tk.END)
        self.reduced_commands.clear()

        for command in self.commands:
            if searched_command.lower() in command.name.lower():
                self.commands_listbox.insert(
                    tk.END, f"{command.name}  ({command.help})"
                )
                self.reduced_commands.append(command)
