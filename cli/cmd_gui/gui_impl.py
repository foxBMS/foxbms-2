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

# cspell:ignore initialdir,xscrollcommand,yscrollcommand

"""Implements the functionalities behind the 'gui' command"""

import contextlib
import ctypes
import tkinter as tk
import webbrowser
from dataclasses import dataclass
from tkinter import filedialog, ttk

from ..helpers.host_platform import get_platform
from ..helpers.project_context import PROJECT_ROOT, ROOT_IS_PROJECT, get_file_path
from .frame_base import BaseFrame
from .frame_bms_configuration.bms_configuration_gui import BmsConfigurationFrame
from .frame_bootloader.bootloader_gui import BootloaderFrame
from .frame_build.build_gui import BuildFrame
from .frame_cli_unittest.cli_unittest_gui import CliUnittestFrame
from .frame_embedded_ut.embedded_ut_gui import EmbeddedUtFrame
from .frame_plot.plot_gui import PlotFrame
from .frame_run.run_gui import RunFrame
from .frame_sim.sim_gui import SimulateBmsFrame
from .style_config import configure_styles

if ROOT_IS_PROJECT:
    from ..cmd_ide.ide_impl import (
        open_ide_app,
        open_ide_bootloader,
        open_ide_cli,
        open_ide_embedded_unit_test_app,
        open_ide_embedded_unit_test_bootloader,
        open_ide_generic,
    )
else:

    def dummy() -> int:
        """Do nothing"""
        return 0

    open_ide_app = dummy
    open_ide_bootloader = dummy
    open_ide_cli = dummy
    open_ide_embedded_unit_test_app = dummy
    open_ide_embedded_unit_test_bootloader = dummy
    open_ide_generic = dummy


@dataclass
class GuiAttributes:
    """Container for re-usable GUI attributes"""

    bg: str
    s: tuple[int, int]

    def __post_init__(self) -> None:
        """Create readable names"""
        self.sx = self.s[0]
        self.sy = self.s[1]


class FoxGui(tk.Tk):
    """Implementation of a GUI to interact with the foxBMS 2 repository"""

    # pylint: disable-next=too-many-statements
    def __init__(self) -> None:
        if get_platform() == "win32":
            my_app_id = "fraunhofer-iisb.foxbms"
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(my_app_id)
        super().__init__()
        self.cattrs = GuiAttributes("darkgrey", (868, 576))
        self._about_dialog: tk.Toplevel | None = None
        self._about_image: tk.PhotoImage | None = None
        self._about_image_label: tk.Label | None = None
        self.configure(background=self.cattrs.bg)
        self.geometry(f"{self.cattrs.sx}x{self.cattrs.sy}")
        self.minsize(self.cattrs.sx, self.cattrs.sy)
        self.title("foxBMS 2")
        self.license_file = get_file_path() / "LICENSE.md"
        if ROOT_IS_PROJECT:
            icon = get_file_path() / "docs/_static/fav_icon.ico"
        else:
            icon = get_file_path() / "fav_icon.ico"
        self.iconbitmap(True, str(icon))  # type: ignore[no-untyped-call]
        configure_styles()
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
        if ROOT_IS_PROJECT:
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

        # Add a 'PanedWindow' (allows the user to adjust layout of the GUI)
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)
        paned_window = tk.PanedWindow(self, orient=tk.VERTICAL)
        paned_window.grid(column=0, row=0, sticky="news")

        # Add a Notebook widget (i.e., tab support)
        self.notebook = ttk.Notebook(self)
        paned_window.add(self.notebook, minsize=int(self.cattrs.sy * 0.3))

        text_frame = ttk.Frame(self)
        paned_window.add(text_frame)
        self.text_widget = tk.Text(text_frame, wrap=tk.NONE, height=20)
        self.text_widget.pack(side=tk.LEFT, expand=True, fill=tk.BOTH)
        scrollbar_y = ttk.Scrollbar(
            text_frame, command=self.text_widget.yview, orient=tk.VERTICAL
        )
        scrollbar_y.pack(side=tk.RIGHT, fill=tk.Y)
        scrollbar_x = ttk.Scrollbar(
            self, command=self.text_widget.xview, orient=tk.HORIZONTAL
        )
        scrollbar_x.grid(column=0, row=1, sticky="we")
        self.text_widget.configure(
            xscrollcommand=scrollbar_x.set,
            yscrollcommand=scrollbar_y.set,
        )
        self.text_widget.config(state=tk.DISABLED)

        if ROOT_IS_PROJECT:
            tab_bms_configuration = BmsConfigurationFrame(
                self.notebook, self.text_widget
            )
            self.notebook.add(tab_bms_configuration, text="BMS Configuration")

        if ROOT_IS_PROJECT:
            tab_build = BuildFrame(self.notebook, self.text_widget)
            self.notebook.add(tab_build, text="Build")

        tab_bootloader = BootloaderFrame(self.notebook, self.text_widget)
        self.notebook.add(tab_bootloader, text="Bootloader")

        if ROOT_IS_PROJECT:
            tab_embedded_ut = EmbeddedUtFrame(self.notebook, self.text_widget)
            self.notebook.add(tab_embedded_ut, text="Embedded Unit Tests")

        if ROOT_IS_PROJECT:
            tab_cli_unittest = CliUnittestFrame(self.notebook, self.text_widget)
            self.notebook.add(tab_cli_unittest, text="fox CLI Unit Tests")

        tab_plot = PlotFrame(self.notebook, self.text_widget)
        self.notebook.add(tab_plot, text="Plot")

        tab_run = RunFrame(self.notebook, self.text_widget)
        self.notebook.add(tab_run, text="Run Program/Script")

        tab_sim_bms = SimulateBmsFrame(self.notebook, self.text_widget)
        self.notebook.add(tab_sim_bms, text="Simulate BMS")

        self.notebook.select(0)  # type: ignore[no-untyped-call]

        self.notebook.bind("<<NotebookTabChanged>>", self.tab_changed_cb)
        self.update_idletasks()
        self.geometry(f"{self.cattrs.sx}x{self.cattrs.sy}")
        self.update()
        scrollbar_x.grid_configure(padx=(0, int(scrollbar_y.winfo_width())))

    def _close_about_dialog(self) -> None:
        """Destroy the About dialog and release image references deterministically."""
        if self._about_image_label is not None:
            self._about_image_label.configure(image="")
            self._about_image_label.image = None  # type: ignore[attr-defined]
            self._about_image_label = None

        if self._about_image is not None:
            try:  # noqa: SIM105
                self.tk.call("image", "delete", str(self._about_image))
            except (tk.TclError, RuntimeError):
                pass
            self._about_image = None

        if self._about_dialog is not None:
            try:
                if self._about_dialog.winfo_exists():
                    self._about_dialog.destroy()
            except (tk.TclError, RuntimeError):
                pass
            self._about_dialog = None

    def destroy(self) -> None:
        """Release transient GUI resources before destroying the root window."""
        self._close_about_dialog()
        super().destroy()

    def tab_changed_cb(self, event: tk.Event) -> None:
        """Reset text widget when tab is changed"""
        _select = self.notebook.select()  # type: ignore[no-untyped-call]
        current_tab: BaseFrame = self.notebook.nametowidget(_select)
        current_tab.reset_text()
        current_tab.write_text()

    def close_tabs(self) -> None:
        """Stop the background work of all tabs.

        Must be called from the main thread while the Tcl interpreter is alive.
        """
        try:
            tabs = self.notebook.tabs()  # type: ignore[no-untyped-call]
        except (tk.TclError, RuntimeError, AttributeError):
            return
        for tab_name in tabs:
            with contextlib.suppress(tk.TclError, RuntimeError, AttributeError):
                self.notebook.nametowidget(tab_name).on_close()

    def close_window(self) -> None:
        """Run the 'on_close' function for all tabs before closing the GUI"""
        self.close_tabs()
        self.destroy()

    def create_new_file_cb(self, event: tk.Event | None = None) -> None:
        """Create a new file"""
        filedialog.asksaveasfile(mode="w", initialdir=str(PROJECT_ROOT))

    def view_license_cb(self, event: tk.Event | None = None) -> None:
        """View the foxBMS 2 license"""
        webbrowser.open(str(self.license_file), new=1)

    def show_about_cb(self, event: tk.Event | None = None) -> None:
        """Show the About dialog"""
        if self._about_dialog is not None:
            try:
                if self._about_dialog.winfo_exists():
                    self._about_dialog.lift()
                    self._about_dialog.focus_force()
                    return
            except tk.TclError:
                pass
            self._about_dialog = None

        dialog = tk.Toplevel(self, bg=self.cattrs.bg)
        self._about_dialog = dialog
        dialog.protocol("WM_DELETE_WINDOW", self._close_about_dialog)
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
        click_license_text.bind("<Button-1>", lambda _: self.view_license_cb())
        click_license_text.place(x=158, y=51)
        if ROOT_IS_PROJECT:
            image_file = (
                get_file_path() / "docs" / "_static" / "foxbms-with-claim250px.png"
            )
        else:
            image_file = get_file_path() / "foxbms-with-claim250px.png"
        image = tk.PhotoImage(file=str(image_file))
        self._about_image = image
        label = tk.Label(dialog, image=image, bg=self.cattrs.bg)
        self._about_image_label = label
        label.image = image  # type: ignore[attr-defined]
        label.place(x=int(-image.width() / 2 + size[0] / 2), y=74)

    def open_vs_code_generic_cb(self, event: tk.Event | None = None) -> None:
        """Open VS Code in the repository root"""
        open_ide_generic()

    def open_vs_code_app_cb(self, event: tk.Event | None = None) -> None:
        """Open VS Code in the 'src/app' directory"""
        open_ide_app()

    def open_vs_code_cli_cb(self, event: tk.Event | None = None) -> None:
        """Open VS Code in the 'cli' directory"""
        open_ide_cli()

    def open_vs_code_app_unit_test_cb(self, event: tk.Event | None = None) -> None:
        """Open VS Code in the 'tests/unit/app' directory"""
        open_ide_embedded_unit_test_app()

    def open_vs_code_bootloader_cb(self, event: tk.Event | None = None) -> None:
        """Open VS Code in the 'src/bootloader' directory"""
        open_ide_bootloader()

    def open_vs_code_bootloader_unit_test_cb(
        self, event: tk.Event | None = None
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
