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

"""Implements the functionalities behind the 'gui' command"""

import ctypes
import tkinter as tk
import webbrowser
from dataclasses import dataclass
from tkinter import filedialog, ttk

from ..cmd_ide.ide_impl import (
    open_ide_app,
    open_ide_bootloader,
    open_ide_cli,
    open_ide_embedded_unit_test_app,
    open_ide_embedded_unit_test_bootloader,
    open_ide_generic,
)
from ..helpers.host_platform import get_platform
from ..helpers.misc import PROJECT_ROOT
from .frame_bootloader.bootloader_gui import BootloaderFrame
from .frame_build.build_gui import BuildFrame
from .frame_cli_unittest.cli_unittest_gui import CliUnittestFrame
from .frame_embedded_ut.embedded_ut_gui import EmbeddedUtFrame
from .frame_plot.plot_gui import PlotFrame
from .frame_run.run_gui import RunFrame


@dataclass
class GuiAttributes:
    """Container for re-usable GUI attributes"""

    bg: str
    s: tuple[int, int]

    def __post_init__(self) -> None:
        self.sx = self.s[0]
        self.sy = self.s[1]


class FoxGui(tk.Tk):
    """Implementation of a GUI to interact with the foxBMS 2 repository"""

    def __init__(self) -> None:
        if get_platform() == "win32":
            myappid = "fraunhofer-iisb.foxbms"
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)
        super().__init__()
        self.cattrs = GuiAttributes("darkgrey", (768, 576))
        self.configure(background=self.cattrs.bg)
        self.minsize(self.cattrs.sx, self.cattrs.sy)
        self.title("foxBMS 2")
        self.license_file = PROJECT_ROOT / "LICENSE.md"

        self.iconbitmap(True, "docs/_static/favicon.ico")

        # File menu
        menu = tk.Menu(self, tearoff=0)
        menu_file = tk.Menu(menu, tearoff=0)
        menu.add_cascade(label="File", menu=menu_file)
        menu_file.add_command(
            label="New", accelerator="Ctrl+N", command=self.create_new_file_cb
        )
        menu_file.add_separator()
        menu_file.add_command(label="Exit", command=self.quit)

        # Tools menu
        menu_tools = tk.Menu(menu, tearoff=0)
        menu.add_cascade(label="Tools", menu=menu_tools)
        sub_menu = tk.Menu(menu_tools, tearoff=False)
        sub_menu.add_command(label="Generic", command=self.open_vs_code_generic_cb)
        sub_menu.add_command(label="App", command=self.open_vs_code_app_cb)
        sub_menu.add_command(label="fox CLI", command=self.open_vs_code_cli_cb)
        sub_menu.add_command(
            label="App Unit Tests", command=self.open_vs_code_app_unit_test_cb
        )
        sub_menu.add_command(
            label="Bootloader", command=self.open_vs_code_bootloader_cb
        )
        sub_menu.add_command(
            label="Bootloader Unit Tests",
            command=self.open_vs_code_bootloader_unit_test_cb,
        )
        menu_tools.add_cascade(label="VS Code", menu=sub_menu)

        # Help menu
        menu_help = tk.Menu(menu, tearoff=0)
        menu.add_cascade(label="Help", menu=menu_help)
        menu_help.add_command(
            label="View License information", command=self.view_license_cb
        )
        menu_help.add_command(label="About", command=self.show_about_cb)

        self.config(menu=menu)
        self.bind_all("<Control-n>", self.create_new_file_cb)

        # Add a 'Notebook' (i.e., tab support)
        style = ttk.Style()
        style.map("TNotebook.Tab", foreground=[("selected", "darkblue")])
        style.configure("TNotebook.Tab", padding=[5, 0])

        self.notebook = ttk.Notebook(self)
        # Pack the Notebook widget
        self.notebook.pack(expand=True, fill="both")

        text_frame = ttk.Frame(self)
        text_frame.pack(expand=True, fill=tk.BOTH)
        self.text = tk.Text(self, wrap="none", height=30)
        self.text.pack(
            in_=text_frame,
            side=tk.LEFT,
            expand=True,
            fill=tk.BOTH,
        )
        scrollbar_y = ttk.Scrollbar(self, command=self.text.yview, orient="vertical")
        scrollbar_y.pack(in_=text_frame, padx=(0, 5), pady=0, side=tk.RIGHT, fill=tk.Y)
        scrollbar_x = ttk.Scrollbar(self, command=self.text.xview, orient="horizontal")
        scrollbar_x.pack(padx=(0, 20), pady=(0, 0), side=tk.TOP, fill=tk.X)
        self.text.configure(
            xscrollcommand=scrollbar_x.set,
            yscrollcommand=scrollbar_y.set,
        )
        self.text.config(state="disabled")

        tab_build = BuildFrame(self.notebook, self.text)
        self.notebook.add(tab_build, text="Build")

        tab_bootloader = BootloaderFrame(self.notebook, self.text)
        self.notebook.add(tab_bootloader, text="Bootloader")

        tab_plot = PlotFrame(self.notebook, self.text)
        self.notebook.add(tab_plot, text="Plot")

        tab_embedded_ut = EmbeddedUtFrame(self.notebook, self.text)
        self.notebook.add(tab_embedded_ut, text="Embedded Unit Tests")

        tab_cli_unittest = CliUnittestFrame(self.notebook, self.text)
        self.notebook.add(tab_cli_unittest, text="fox CLI Unit Tests")

        tab_run = RunFrame(self.notebook, self.text)
        self.notebook.add(tab_run, text="Run Program/Script")

        self.notebook.bind("<<NotebookTabChanged>>", self.tab_changed_cb)

    def tab_changed_cb(self, event: tk.Event) -> None:
        """Reset text widget when tab is changed"""
        current_tab = self.notebook.nametowidget(self.notebook.select())
        self.text.config(state="normal")
        self.text.delete("1.0", tk.END)
        self.text.config(state="disabled")
        current_tab.text_index = 0
        current_tab.write_text()

    def close_window(self) -> None:
        """Close all open file-streams"""
        for tab_name in self.notebook.tabs():
            tab_obj = self.notebook.nametowidget(tab_name)
            if hasattr(tab_obj, "file_stream"):
                tab_obj.file_stream.close()
            log_file = tab_obj.file_path
            if log_file.exists():
                log_file.unlink()
        self.destroy()

    def create_new_file_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Create a new file"""
        filedialog.asksaveasfile(mode="w", initialdir=str(PROJECT_ROOT))

    def view_license_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """View the foxBMS 2 license"""
        webbrowser.open(str(self.license_file), new=1)

    def show_about_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Show the About dialog"""
        dialog = tk.Toplevel(self, bg=self.cattrs.bg)

        size = (350, 100)
        dialog.minsize(*size)
        dialog.resizable(False, False)
        dialog.title("foxBMS 2 - Fraunhofer IISB")
        tk.Label(dialog, text="foxBMS 2", bg=self.cattrs.bg).place(x=5, y=5)
        tk.Label(dialog, text="Developed by Fraunhofer IISB", bg=self.cattrs.bg).place(
            x=5, y=28
        )
        overlay = tk.Label(
            dialog, text="For license information click", bg=self.cattrs.bg
        )
        overlay.place(x=5, y=51)
        click_license_text = tk.Label(
            dialog,
            text=str(self.license_file),
            bg=self.cattrs.bg,
            fg="blue",
            cursor="hand2",
        )
        click_license_text.bind("<Button-1>", lambda e: self.view_license_cb())
        click_license_text.place(x=158, y=51)
        image = tk.PhotoImage(file="docs/_static/foxbms-with-claim250px.png")
        label = tk.Label(dialog, image=image, bg=self.cattrs.bg)
        label.image = image  # type: ignore[attr-defined]
        label.place(x=int(-image.width() / 2 + size[0] / 2), y=74)

    def open_vs_code_generic_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Open VS Code in the repository root"""
        open_ide_generic()

    def open_vs_code_app_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Open VS Code in the 'src/app' directory"""
        open_ide_app()

    def open_vs_code_cli_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Open VS Code in the 'cli' directory"""
        open_ide_cli()

    def open_vs_code_app_unit_test_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Open VS Code in the 'tests/unit/app' directory"""
        open_ide_embedded_unit_test_app()

    def open_vs_code_bootloader_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Open VS Code in the 'src/bootloader' directory"""
        open_ide_bootloader()

    def open_vs_code_bootloader_unit_test_cb(
        self,
        event: tk.Event | None = None,  # type: ignore[type-arg]
    ) -> None:
        """Open VS Code in the 'tests/unit/bootloader' directory"""
        open_ide_embedded_unit_test_bootloader()


def main() -> FoxGui:  # pragma: no cover
    """For testing purposes"""
    return FoxGui()


def run_gui() -> None:
    """Run the GUI"""
    root = FoxGui()
    root.protocol("WM_DELETE_WINDOW", root.close_window)
    root.mainloop()
