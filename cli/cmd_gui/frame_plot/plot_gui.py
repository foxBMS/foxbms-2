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

"""Implements the 'plot' frame"""

import sys
import tkinter as tk
from pathlib import Path
from queue import Queue
from threading import Thread
from tkinter import ttk
from typing import TextIO

from ...helpers.misc import PROJECT_BUILD_ROOT
from ...helpers.spr import run_process
from .frame_data_config import DataConfigFrame
from .frame_plot_config import PlotConfigFrame
from .frame_run_plot import RunPlotFrame


# pylint: disable-next=too-many-instance-attributes, too-many-ancestors
class PlotFrame(ttk.Frame):
    """Plot Frame"""

    # pylint: disable-next=too-many-statements
    def __init__(self, parent, text_widget: tk.Text) -> None:
        super().__init__(parent)
        self.parent: ttk.Notebook = parent
        self.queue: Queue = Queue()
        self.plot_process: Thread
        self.file_stream: TextIO
        self.text = text_widget
        self.text_index: int = 0
        self.file_path = PROJECT_BUILD_ROOT / "output_gui_plot.txt"
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)
        self.file_path.touch()

        # Set Styles for Headings and for Notebook
        font_heading = ("TkDefaultFont", 10, "bold")
        ttk.Style().configure("heading.TButton", font=font_heading)

        # Configure a 'Notebook'
        plot_notebook = ttk.Notebook(self)
        plot_notebook.pack(fill=tk.BOTH, expand=True)

        self.tab_plot = RunPlotFrame(plot_notebook, self)
        self.tab_plot.pack(fill=tk.X, expand=True)
        plot_notebook.add(self.tab_plot, text="Run Plot")

        tab_data_config = DataConfigFrame(plot_notebook, self)
        tab_data_config.pack(fill=tk.X, expand=True)
        plot_notebook.add(tab_data_config, text="Data Config")

        tab_plot_config = PlotConfigFrame(plot_notebook, self)
        tab_plot_config.pack(fill=tk.X, expand=True)
        plot_notebook.add(tab_plot_config, text="Plot Config")

        self.plot_button = ttk.Button(
            self, text="Plot", command=self.plot_command_cb, style="heading.TButton"
        )
        self.plot_button.pack(pady=5)

    def plot_command_cb(self) -> None:
        """Start the plot-process"""
        self.text_index = 0
        self.text.config(state="normal")
        self.text.delete("1.0", tk.END)
        self.text.config(state="disabled")
        data_config = self.tab_plot.data_config_entry.get().strip()
        plot_config = self.tab_plot.plot_config_entry.get().strip()
        output_dir = Path(self.tab_plot.output_entry.get().strip())
        data_source = self.tab_plot.data_source_entry.get().strip()
        data_type = self.tab_plot.data_type_entry.get().strip()
        if (
            (not Path(data_config).is_file())
            or (not Path(plot_config).is_file())
            or (not Path(data_source).is_file())
        ):
            self.write_text(
                "Configuration files and Data Source have to be given as valid file-paths\n"
            )
            return
        if (output_dir == Path(".")) or (not output_dir.is_dir()):
            output_dir = PROJECT_BUILD_ROOT / "plot"
            output_dir.mkdir(parents=True, exist_ok=True)
            self.tab_plot.output_entry.delete(0, tk.END)
            self.tab_plot.output_entry.insert(tk.END, str(output_dir))
            self.write_text(
                f"Output directory has been set to '{PROJECT_BUILD_ROOT / 'plot'}'\n"
            )
            return
        self.plot_button.state(["disabled"])
        self.write_text("Plotting the given graphs\n")

        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="a", encoding="utf-8")
        cmd = [
            sys.executable,
            "fox.py",
            "plot",
            "--data-config",
            data_config,
            "--plot-config",
            plot_config,
            "--output",
            output_dir,
            "--data-type",
            data_type,
            data_source,
        ]
        self.plot_process = Thread(
            target=lambda cmd, stdout, stderr: self.queue.put(
                run_process(cmd=cmd, stdout=stdout, stderr=stderr)
            ),
            kwargs={"cmd": cmd, "stdout": self.file_stream, "stderr": self.file_stream},
            daemon=True,
        )
        self.plot_process.start()
        self.check_thread()

    def check_thread(self) -> None:
        """If the provided thread is not alive the button is activated"""
        if self.plot_process.is_alive():
            self.after(50, self.check_thread)
        else:
            self.plot_button.state(["!disabled"])
            if not self.queue.empty() and (return_value := self.queue.get()):
                if return_value.returncode == 0:
                    self.file_stream.write("Finished Plotting\n")
                else:
                    self.file_stream.write("Plotting was not successful\n")
            self.file_stream.close()
        self.write_text()

    def write_text(self, file_input: None | str = None) -> None:
        """Writes the file content in the text box"""
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
