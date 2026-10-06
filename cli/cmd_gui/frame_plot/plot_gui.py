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

"""Implements the 'plot' frame"""

import sys
import tkinter as tk
from pathlib import Path
from queue import Queue
from threading import Thread
from tkinter import ttk

from ...helpers.project_context import PROJECT_BUILD_ROOT, PROJECT_ROOT, ROOT_IS_PROJECT
from ...helpers.spr import SubprocessResult, run_process
from ..frame_base import BaseFrame
from ..style_config import get_heading_font
from .frame_data_config import DataConfigFrame
from .frame_plot_config import PlotConfigFrame
from .frame_run_plot import RunPlotFrame


# pylint: disable-next=too-many-ancestors
class PlotFrame(BaseFrame):
    """'Plot' Frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        super().__init__(parent, text_widget, "output_gui_plot.txt")
        self.queue: Queue[SubprocessResult] = Queue()
        self.plot_process: Thread

        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        # Font has to exist for style to work
        heading_font = get_heading_font()  # noqa: F841 pylint: disable=unused-variable

        # Configure a 'Notebook'
        plot_notebook = ttk.Notebook(self)
        plot_notebook.grid(column=0, row=0, sticky="news")

        self.run_plot_tab = RunPlotFrame(plot_notebook, self)
        self.run_plot_tab.pack(fill=tk.X, expand=True)
        plot_notebook.add(self.run_plot_tab, text="Run Plot")

        data_config_tab = DataConfigFrame(plot_notebook, self)
        data_config_tab.pack(fill=tk.X, expand=True)
        plot_notebook.add(data_config_tab, text="Data Config")

        plot_config_tab = PlotConfigFrame(plot_notebook, self)
        plot_config_tab.pack(fill=tk.X, expand=True)
        plot_notebook.add(plot_config_tab, text="Plot Config")

        self.plot_button = ttk.Button(
            self, text="Plot", command=self.plot_command_cb, style="Heading.TButton"
        )
        self.plot_button.grid(column=0, row=1, pady=5)

    def plot_command_cb(self) -> None:
        """Start the plot-process"""
        self.reset_text()
        data_config = self.run_plot_tab.data_config_entry.get().strip()
        plot_config = self.run_plot_tab.plot_config_entry.get().strip()
        output_dir = Path(self.run_plot_tab.output_directory_entry.get().strip())
        data_source = self.run_plot_tab.data_source_entry.get().strip()
        data_type = self.run_plot_tab.data_type_entry.get().strip()
        if (
            ((data_type != "PARQUET") and (not Path(data_config).is_file()))
            or (not Path(plot_config).is_file())
            or (not Path(data_source).is_file())
        ):
            self.write_text(
                "Configuration files and Data Source have to be given as valid file-paths.\n"
            )
            return
        if (output_dir == Path()) or (not output_dir.is_dir()):
            output_dir = PROJECT_BUILD_ROOT / "plot"
            output_dir.mkdir(parents=True, exist_ok=True)
            self.run_plot_tab.output_directory_entry.delete(0, tk.END)
            self.run_plot_tab.output_directory_entry.insert(tk.END, str(output_dir))
            self.write_text(f"Output directory has been set to '{output_dir}'.\n")
            return
        self.plot_button.state([tk.DISABLED])  # type: ignore[no-untyped-call]
        open(self.file_path, mode="w", encoding="utf-8").close()  # pylint: disable=consider-using-with

        if ROOT_IS_PROJECT:
            plot_command = [sys.executable, PROJECT_ROOT / "fox.py", "plot"]
        else:
            plot_command = [sys.executable, "-m", "fox_cli", "plot"]
        cmd = [
            *plot_command,
            "--plot-config",
            plot_config,
            "--output",
            output_dir,
            "--data-type",
            data_type,
        ]
        if data_type != "PARQUET":
            cmd = cmd + ["--data-config", data_config]
        cmd.append(data_source)
        self.write_text("Running plot command.\n")
        # pylint: disable-next=consider-using-with
        self.file_stream = open(self.file_path, mode="a", encoding="utf-8")  # noqa: SIM115
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
        self.write_text()
        if self.plot_process.is_alive():
            self.after(50, self.check_thread)
            return
        self.plot_button.state(["!disabled"])  # type: ignore[no-untyped-call]
        self.file_stream.close()
        if not self.queue.empty() and (return_value := self.queue.get()):
            if return_value.returncode == 0:
                self.write_text("Plotting was successful.\n")
            else:
                self.write_text(
                    f"Plotting failed with exit code {return_value.returncode}.\n"
                )
