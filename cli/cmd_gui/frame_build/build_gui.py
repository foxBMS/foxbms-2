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

"""Implements the 'build' frame"""

import tkinter as tk
from dataclasses import dataclass
from pathlib import Path
from queue import Queue
from threading import Thread
from tkinter import ttk
from typing import TextIO

from ...cmd_build.build_impl import run_top_level_waf
from ...helpers.misc import PROJECT_BUILD_ROOT, file_name_from_current_time


@dataclass
class Command:
    """Container for a Command and its help-string"""

    name: str
    help: str


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class BuildFrame(ttk.Frame):
    """'Build' Frame"""

    def __init__(self, parent, text_widget: tk.Text) -> None:
        super().__init__(parent)
        self.parent: ttk.Notebook = parent
        self.text = text_widget
        self.text_index: int = 0
        self.build_process: Thread
        self.queue: Queue = Queue()
        self.current_command: str = ""
        self.commands: list[Command] = []
        self.reduced_commands: list[Command] = []
        self.file_path = PROJECT_BUILD_ROOT / "output_gui_build.txt"
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)
        self.file_path.touch()
        self.file_stream: TextIO

        style = ttk.Style()
        style.configure("Multiline.TButton", justify="center")

        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        # Create a listbox with commands
        self.listbox = tk.Listbox(self, height=10, width=80, selectmode="single")
        self.listbox.grid(column=0, row=0, pady=(10, 0), sticky="ns")

        # Create a frame for searching for a command
        search_frame = ttk.Frame(self, padding=(0, 10))
        search_frame.grid(column=0, row=1)

        ttk.Label(self, text="Search commands:").pack(
            in_=search_frame, side=tk.LEFT, padx=(0, 5), pady=10
        )
        self.search_command_entry = ttk.Entry(self, width=30)
        self.search_command_entry.pack(in_=search_frame, side=tk.LEFT, pady=10)
        self.search_command_entry.bind("<KeyRelease>", self.select_command_cb)

        # Create a frame for the buttons
        button_frame = ttk.Frame(self)
        button_frame.grid(column=0, row=2)

        # Create a "Generate Command List" button
        self.command_list_button = ttk.Button(
            self,
            text="Generate\nCommand List",
            command=self.generate_command_list_command_cb,
            style="Multiline.TButton",
        )
        self.command_list_button.pack(in_=button_frame, side=tk.LEFT, ipadx=1, padx=3)

        # Create a "Run" button
        self.run_button = ttk.Button(
            self,
            text="Run",
            command=self.run_command_cb,
        )
        self.run_button.pack(in_=button_frame, side=tk.LEFT, ipadx=1, ipady=8, padx=3)
        self.run_button.state(["disabled"])

        # Create a "Save Log" button
        self.save_button = ttk.Button(
            self, text="Save Log", command=self.save_command_cb
        )
        self.save_button.pack(in_=button_frame, side=tk.LEFT, ipadx=1, ipady=8, padx=3)
        self.save_button.state(["disabled"])

        # Create a frame for the status
        state_frame = ttk.Frame(self, padding=(0, 10))
        state_frame.grid(column=0, row=3)

        # Create a Canvas to contain an oval
        self.canvas = tk.Canvas(self, width=30, height=30)
        self.canvas.pack(in_=state_frame, side=tk.LEFT)
        self.oval = self.canvas.create_oval(*((5, 5), (25, 25)), fill="darkgrey")

        self.status_text = ttk.Label(self, text="No command has been run.")
        self.status_text.pack(in_=state_frame, side=tk.LEFT)

    def generate_command_list_command_cb(self):
        """Run 'waf --help' and use the output as the command list"""
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="w", encoding="utf-8")
        self.current_command = "Generate Command List"
        self.command_list_button.state(["disabled"])
        self.run_button.state(["disabled"])
        self.text_index = 0
        self.text.config(state="normal")
        self.text.delete("1.0", tk.END)
        self.text.config(state="disabled")

        self.canvas.itemconfig(self.oval, fill="darkgrey")
        self.status_text.config(text=f"Running command: {self.current_command}")

        self.build_process = Thread(
            target=lambda args, stdout, stderr: self.queue.put(
                run_top_level_waf(args=args, stdout=stdout, stderr=stderr)
            ),
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
        self.text_index = 0
        self.text.config(state="normal")
        self.text.delete("1.0", tk.END)
        self.text.config(state="disabled")
        self.run_button.state(["disabled"])
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="w", encoding="utf-8")
        command = self.listbox.curselection()[0]
        self.current_command = self.reduced_commands[command].name
        self.canvas.itemconfig(self.oval, fill="darkgrey")
        self.status_text.config(text=f"Running command: {self.current_command}")
        self.build_process = Thread(
            target=lambda args, stdout, stderr: self.queue.put(
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
        else:
            if self.current_command == "Generate Command List":
                self.command_list_button.state(["!disabled"])
                self.run_button.state(["!disabled"])
                self.generate_command_list()
            else:
                self.run_button.state(["!disabled"])
                self.save_button.state(["!disabled"])
            self.file_stream.close()
            if not self.queue.empty() and (return_value := self.queue.get()):
                if return_value.returncode == 0:
                    self.canvas.itemconfig(self.oval, fill="green")
                    self.status_text.config(
                        text=f"Command '{self.current_command}' was successful."
                    )
                else:
                    self.canvas.itemconfig(self.oval, fill="red")
                    self.status_text.config(
                        text=f"Command '{self.current_command}' was not successful."
                    )

    def save_command_cb(self) -> None:
        """Save the Log into a file"""
        gui_dir = PROJECT_BUILD_ROOT / "gui"
        gui_dir.mkdir(parents=True, exist_ok=True)
        timestamp = str(file_name_from_current_time())
        file = gui_dir / Path(f"{self.current_command}_{timestamp}.txt")
        with open(self.file_path, encoding="utf-8", errors="ignore") as log:
            log_content = log.read()
        with open(file, mode="w", encoding="utf-8", errors="ignore") as f:
            f.write(log_content)

        self.status_text.config(
            text=f"""Log saved in the file
            '{gui_dir / f"{self.current_command}_{timestamp}.txt"}'."""
        )

    def write_text(self) -> None:
        """Writes the file content in the text box"""
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

    def generate_command_list(self) -> None:
        """Generates the command list from the output of 'waf --help'"""
        with open(self.file_path, encoding="utf-8") as f:
            text = f.read()
        commands = text.split("Main commands")[1].split("options:")[0]
        lines_to_process = commands.splitlines()[1:-1]
        for line in lines_to_process:
            command = Command(
                name=line.lstrip().split(":")[0].strip(),
                help=line.split(":")[1].strip(),
            )
            self.commands.append(command)
            self.reduced_commands.append(command)
            self.listbox.insert(tk.END, f"{command.name}  ({command.help})")

    def select_command_cb(self, event: tk.Event) -> None:
        """Reduces the available commands accordingly to the search term"""
        searched_command: str = self.search_command_entry.get()
        self.listbox.delete(0, tk.END)
        self.reduced_commands.clear()

        for command in self.commands:
            if searched_command.lower() in command.name.lower():  # pylint: disable=no-member
                self.listbox.insert(tk.END, f"{command.name}  ({command.help})")
                self.reduced_commands.append(command)
