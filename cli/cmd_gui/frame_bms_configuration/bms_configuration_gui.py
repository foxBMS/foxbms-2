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

# cspell:ignore xscrollcommand,yscrollcommand,scrollregion,textvariable,wraplength
# cspell:ignore subtab,subtabs

"""Implements the 'BMS Configuration' frame"""

import json
import tkinter as tk
from pathlib import Path
from tkinter import ttk
from typing import Any, ClassVar

from ...helpers.project_context import PROJECT_ROOT
from ...helpers.spr import run_process
from ..frame_base import BaseFrame


# pylint: disable-next=too-many-ancestors
class BmsConfigurationSubFrame(BaseFrame):
    """Provide a notebook sub-frame for the BMS configuration frame."""


# pylint: disable-next=too-many-ancestors,too-many-instance-attributes
class DiagnosisConfigFrame(BaseFrame):
    """Display and configure diagnosis entries from diag_array_cfg.json."""

    SENSITIVITY_VALUES: ClassVar[tuple[int, ...]] = (
        1,
        3,
        5,
        10,
        20,
        50,
        100,
        500,
    )
    SEVERITY_VALUES: ClassVar[tuple[str, ...]] = ("INFO", "WARNING", "FATAL_ERROR")
    DELAY_VALUES: ClassVar[tuple[int, ...]] = (
        -1,
        0,
        100,
        200,
        1000,
        2000,
    )
    ENABLED_VALUES: ClassVar[tuple[bool, ...]] = (True, False)

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        """Initialize the diagnosis configuration frame.

        Args:
            parent: Parent notebook widget.
            text_widget: Shared output text widget.

        """
        super().__init__(
            parent, text_widget, "output_gui_bms_configuration_diagnosis.txt"
        )
        self.diag_config_path = PROJECT_ROOT / "conf/bms/diag_array_cfg.json"
        self.diag_data: dict[str, Any] | None = None
        self._active_combobox: ttk.Combobox | None = None
        self._pending_canvas_width: int = 0
        self._resize_after_id: str | None = None
        self._scrollregion_update_scheduled: bool = False
        self.dropdown_vars: list[
            tuple[dict[str, Any], tk.IntVar, tk.StringVar, tk.IntVar, tk.StringVar]
        ] = []

        self.columnconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)

        file_frame = ttk.Frame(self)
        file_frame.grid(column=0, row=0, padx=10, pady=(10, 5), sticky="ew")
        file_frame.columnconfigure(1, weight=1)

        button_frame = ttk.Frame(file_frame)
        button_frame.grid(column=0, row=0, padx=(0, 10), sticky="w")

        ttk.Button(button_frame, text="Load", command=self.load_config_cb).pack(
            side=tk.LEFT, padx=(0, 5)
        )
        ttk.Button(button_frame, text="Save", command=self.save_config_cb).pack(
            side=tk.LEFT, padx=(5, 0)
        )

        self.file_path_var = tk.StringVar(value=str(self.diag_config_path))
        self.file_path_entry = ttk.Entry(
            file_frame, textvariable=self.file_path_var, state="readonly"
        )
        self.file_path_entry.grid(column=1, row=0, sticky="ew")

        table_frame = ttk.Frame(self)
        table_frame.grid(column=0, row=1, padx=10, pady=5, sticky="nsew")
        table_frame.columnconfigure(0, weight=1)
        table_frame.rowconfigure(0, weight=1)

        self.canvas = tk.Canvas(table_frame)
        self.canvas.grid(column=0, row=0, sticky="nsew")

        y_scrollbar = ttk.Scrollbar(
            table_frame, orient=tk.VERTICAL, command=self.canvas.yview
        )
        y_scrollbar.grid(column=1, row=0, sticky="ns")
        x_scrollbar = ttk.Scrollbar(
            table_frame, orient=tk.HORIZONTAL, command=self.canvas.xview
        )
        x_scrollbar.grid(column=0, row=1, sticky="ew")
        self.canvas.config(
            yscrollcommand=y_scrollbar.set, xscrollcommand=x_scrollbar.set
        )

        self.entries_frame = ttk.Frame(self.canvas)
        self.canvas_window = self.canvas.create_window(
            (0, 0), window=self.entries_frame, anchor="nw"
        )
        self.entries_frame.bind("<Configure>", self._on_frame_configure)
        self.canvas.bind("<Configure>", self._on_canvas_configure)
        self.canvas.bind("<Enter>", self._bind_mousewheel)
        self.canvas.bind("<Leave>", self._unbind_mousewheel)
        self.entries_frame.bind("<Enter>", self._bind_mousewheel)
        self.entries_frame.bind("<Leave>", self._unbind_mousewheel)
        self.bind_all("<Button-1>", self._clear_selection_cb, add="+")

        self.load_config_cb(log_output=False)

    def _get_path(self) -> Path:
        """Return the config file path from the entry widget.


        Returns:
            The selected config file path.
        """
        return Path(self.file_path_var.get().strip())

    def load_config_cb(self, log_output: bool = True) -> None:
        """Load diagnosis configuration and rebuild the table.

        Args:
            log_output: Enable status logging to the output widget.
        """
        file_path = self._get_path()
        if not file_path.is_file():
            if log_output:
                self.write_text(f"Diagnosis config file not found: '{file_path}'.\n")
            return
        with open(file_path, encoding="utf-8") as f:
            self.diag_data = json.load(f)

        entries = self._collect_array_entries(self.diag_data)
        if not entries:
            if log_output:
                self.write_text(
                    f"No entries found in 'files[].array_entries' in '{file_path}'.\n"
                )
            return
        self._build_entries_table(entries)
        if log_output:
            self.write_text(f"Loaded diagnosis config from '{file_path}'.\n")

    def save_config_cb(self) -> None:
        """Save selected values back to the diagnosis config file."""
        file_path = self._get_path()
        if not file_path.is_file():
            self.write_text(f"Diagnosis config file not found: '{file_path}'.\n")
            return

        if self.diag_data is None:
            self.write_text("No diagnosis data loaded.\n")
            return

        if not self.dropdown_vars:
            self.write_text("No diagnosis entries loaded.\n")
            return

        for (
            entry,
            sensitivity_var,
            severity_var,
            delay_var,
            enabled_var,
        ) in self.dropdown_vars:
            entry["sensitivity"] = sensitivity_var.get()
            entry["severity"] = severity_var.get()
            entry["delay"] = delay_var.get()
            entry["enabled"] = enabled_var.get() == "True"

        file_path.parent.mkdir(parents=True, exist_ok=True)
        with open(file_path, mode="w", encoding="utf-8") as f:
            json.dump(self.diag_data, f, ensure_ascii=True, indent=2)
            f.write("\n")
        self.write_text(f"Saved diagnosis config to '{file_path}'.\n")
        self._write_git_diff(file_path)

    def _write_git_diff(self, file_path: Path) -> None:
        """Write the file git diff into the output log.

        Args:
            file_path: Path of the saved configuration file.
        """
        try:
            repo_root = PROJECT_ROOT.resolve()
            relative_path = file_path.resolve().relative_to(repo_root)
        except ValueError:
            self.write_text(
                f"No git diff generated, file is outside repository: '{file_path}'.\n"
            )
            return

        normalized_path = str(relative_path).replace("\\", "/")
        result = run_process(cmd=["git", "diff", "--", normalized_path], cwd=repo_root)

        if result.returncode != 0:
            self.write_text(
                f"git diff failed for '{normalized_path}': {result.err.strip()}\n"
            )
            return

        diff_content = result.out.strip()
        if not diff_content:
            self.write_text(f"No git diff for '{normalized_path}'.\n")
            return

        self.write_text(f"Git diff for '{normalized_path}':\n")
        self.write_text(diff_content + "\n")

    @staticmethod
    def _collect_array_entries(diag_data: dict[str, Any]) -> list[dict[str, Any]]:
        """Collect all entries from files[].array_entries.

        Args:
            diag_data: Loaded diagnosis JSON content.

        Returns:
            Flattened list of array entry dictionaries.
        """
        entries: list[dict[str, Any]] = []
        for file_entry in diag_data.get("files", []):
            entries.extend(file_entry.get("array_entries", []))
        return entries

    def _build_entries_table(self, entries: list[dict[str, Any]]) -> None:
        """Build table rows from diagnosis entries.

        Args:
            entries: Diagnosis entries to render.

        """
        for child in self.entries_frame.winfo_children():
            child.destroy()

        self.dropdown_vars = []

        headers = [
            "event",
            "sensitivity",
            "severity",
            "delay",
            "enabled",
            "callback",
            "description",
        ]
        for column, title in enumerate(headers):
            ttk.Label(self.entries_frame, text=title).grid(
                column=column, row=0, padx=4, pady=(2, 6), sticky="w"
            )

        for row, entry in enumerate(entries, start=1):
            ttk.Label(self.entries_frame, text=str(entry.get("event", ""))).grid(
                column=0, row=row, padx=4, pady=2, sticky="w"
            )

            sensitivity_var = tk.IntVar(value=entry.get("sensitivity", 0))
            sensitivity_combo = ttk.Combobox(
                self.entries_frame,
                state="readonly",
                values=[str(i) for i in self.SENSITIVITY_VALUES],
                textvariable=sensitivity_var,
                width=10,
            )
            sensitivity_combo.grid(column=1, row=row, padx=4, pady=2, sticky="ew")
            self._bind_combobox_mousewheel(sensitivity_combo)

            severity_var = tk.StringVar(value=str(entry.get("severity", "")))
            severity_combo = ttk.Combobox(
                self.entries_frame,
                state="readonly",
                values=self.SEVERITY_VALUES,
                textvariable=severity_var,
                width=12,
            )
            severity_combo.grid(column=2, row=row, padx=4, pady=2, sticky="ew")
            self._bind_combobox_mousewheel(severity_combo)

            delay_var = tk.IntVar(value=entry.get("delay", 0))
            delay_combo = ttk.Combobox(
                self.entries_frame,
                state="readonly",
                values=[str(i) for i in self.DELAY_VALUES],
                textvariable=delay_var,
                width=10,
            )
            delay_combo.grid(column=3, row=row, padx=4, pady=2, sticky="ew")
            self._bind_combobox_mousewheel(delay_combo)

            enabled_value = entry.get("enabled", True)
            enabled_var = tk.StringVar(value=str(enabled_value))  # "True"/"False"
            enabled_combo = ttk.Combobox(
                self.entries_frame,
                state="readonly",
                values=[str(i) for i in self.ENABLED_VALUES],  # ["True", "False"]
                textvariable=enabled_var,
                width=14,
            )
            enabled_combo.grid(column=4, row=row, padx=4, pady=2, sticky="ew")
            self._bind_combobox_mousewheel(enabled_combo)

            ttk.Label(self.entries_frame, text=str(entry.get("callback", ""))).grid(
                column=5, row=row, padx=4, pady=2, sticky="w"
            )
            ttk.Label(
                self.entries_frame,
                text=str(entry.get("description", "")),
                wraplength=520,
                justify=tk.LEFT,
            ).grid(column=6, row=row, padx=4, pady=2, sticky="w")

            self.dropdown_vars.append(
                (entry, sensitivity_var, severity_var, delay_var, enabled_var)
            )

    def _on_frame_configure(self, _event: tk.Event) -> None:
        """Schedule a scrollregion update after frame geometry changes.

        Args:
            _event: Tkinter configure event.
        """
        if self._scrollregion_update_scheduled:
            return
        self._scrollregion_update_scheduled = True
        self.after_idle(self._update_scrollregion)

    def _on_canvas_configure(self, event: tk.Event) -> None:
        """Schedule a debounced resize of the inner canvas window.

        Args:
            event: Tkinter configure event.
        """
        self._pending_canvas_width = event.width
        if self._resize_after_id is not None:
            self.after_cancel(self._resize_after_id)
        self._resize_after_id = self.after(10, self._apply_canvas_width)

    def _update_scrollregion(self) -> None:
        """Apply the delayed scrollregion update."""
        self.canvas.configure(scrollregion=self.canvas.bbox("all"))
        self._scrollregion_update_scheduled = False

    def _apply_canvas_width(self) -> None:
        """Apply the delayed canvas window width update.

        Keep at least the viewport width but do not shrink below content width,
        otherwise horizontal scrolling would be clipped.
        """
        content_width = self.entries_frame.winfo_reqwidth()
        target_width = max(self._pending_canvas_width, content_width)
        self.canvas.itemconfigure(self.canvas_window, width=target_width)
        self._resize_after_id = None

    def _bind_combobox_mousewheel(self, combobox: ttk.Combobox) -> None:
        """Bind combobox events for selection and wheel handling.

        Args:
            combobox: Combobox widget to configure.
        """
        combobox.bind("<Button-1>", self._set_active_combobox_cb)
        combobox.bind("<FocusIn>", self._set_active_combobox_cb)
        combobox.bind("<<ComboboxSelected>>", self._clear_after_selection_cb)
        combobox.bind("<FocusOut>", self._clear_active_combobox_cb)
        combobox.bind("<MouseWheel>", self._on_combobox_mousewheel_cb)
        combobox.bind("<Button-4>", self._on_combobox_mousewheel_cb)
        combobox.bind("<Button-5>", self._on_combobox_mousewheel_cb)

    def _set_active_combobox_cb(self, event: tk.Event) -> None:
        """Mark the event source combobox as active.

        Args:
            event: Tkinter event from the combobox.
        """
        if isinstance(event.widget, ttk.Combobox):
            self._set_active_combobox(event.widget)

    def _clear_after_selection_cb(self, event: tk.Event) -> None:
        """Clear active state after a combobox selection.

        Args:
            event: Tkinter event from the combobox.
        """
        if isinstance(event.widget, ttk.Combobox):
            self._clear_after_selection(event.widget)

    def _clear_active_combobox_cb(self, event: tk.Event) -> None:
        """Clear active state after combobox focus leaves.

        Args:
            event: Tkinter event from the combobox.
        """
        if isinstance(event.widget, ttk.Combobox):
            self._clear_active_combobox(event.widget)

    def _on_combobox_mousewheel_cb(self, event: tk.Event) -> str | None:
        """Route combobox wheel events through the wheel handler.

        Args:
            event: Tkinter wheel event from the combobox.

        Returns:
            "break" to stop propagation, None to keep default behavior.
        """
        if isinstance(event.widget, ttk.Combobox):
            return self._on_combobox_mousewheel(event, event.widget)
        return None

    def _on_combobox_mousewheel(
        self, event: tk.Event, widget: ttk.Combobox
    ) -> str | None:
        """Handle mousewheel events on a combobox.

        Args:
            event: Mousewheel event.
            widget: Combobox receiving the event.

        Returns:
            "break" to stop propagation, None to keep default behavior.
        """
        if widget.instate(["focus"]):
            return None
        self._on_mousewheel(event)
        return "break"

    def _set_active_combobox(self, widget: ttk.Combobox) -> None:
        """Set the currently active combobox.

        Args:
            widget: Combobox to mark active.
        """
        self._active_combobox = widget

    def _clear_after_selection(self, widget: ttk.Combobox) -> None:
        """Clear active combobox after a value selection.

        Args:
            widget: Combobox that emitted the selection event.

        """
        if self._active_combobox == widget:
            self._active_combobox = None
        self.canvas.focus_set()

    def _clear_selection_cb(self, event: tk.Event) -> None:
        """Clear active combobox when clicking outside combobox widgets.

        Args:
            event: Mouse click event.
        """
        if not isinstance(event.widget, tk.Misc):
            return
        current_widget: tk.Misc | None = event.widget
        is_descendant = False
        while current_widget is not None:
            if current_widget == self:
                is_descendant = True
                break
            parent_path = current_widget.winfo_parent()
            if not parent_path:
                break
            try:
                current_widget = current_widget.nametowidget(parent_path)
            except KeyError:
                break
        if not is_descendant:
            return
        if isinstance(event.widget, ttk.Combobox):
            return
        self._active_combobox = None
        self.canvas.focus_set()

    def _clear_active_combobox(self, widget: ttk.Combobox) -> None:
        """Clear active combobox when combobox focus leaves.

        Args:
            widget: Combobox that lost focus.
        """
        if self._is_popdown_focused(widget):
            self.after(50, self._clear_after_popdown_closed, widget)
            return
        if self._active_combobox == widget:
            self._active_combobox = None

    @staticmethod
    def _is_popdown_focused(widget: ttk.Combobox) -> bool:
        """Return whether the combobox popdown currently owns focus.

        Args:
            widget: Combobox to inspect.

        Returns:
            True when popdown owns focus, otherwise False.
        """
        try:
            popdown_path = str(
                widget.tk.call("ttk::combobox::PopdownWindow", str(widget))
            )
            focus_path = str(widget.tk.call("focus"))
            return bool(popdown_path and focus_path.startswith(popdown_path))
        except tk.TclError:
            return False

    def _clear_after_popdown_closed(self, widget: ttk.Combobox) -> None:
        """Clear active combobox after popdown closes without selection.

        Args:
            widget: Combobox whose popdown was open.
        """
        if self._active_combobox != widget:
            return
        if not widget.winfo_exists():
            return
        if self._is_popdown_focused(widget):
            self.after(50, self._clear_after_popdown_closed, widget)
            return
        self._active_combobox = None
        self.canvas.focus_set()

    def _is_combobox_interaction_active(self) -> bool:
        """Return whether combobox interaction is currently active.

        Returns:
            True when combobox or popdown is active, otherwise False.
        """
        if (self._active_combobox is not None) and self._active_combobox.winfo_exists():
            return True
        try:
            focus_path = str(self.tk.call("focus"))
            return "popdown" in focus_path.lower()
        except tk.TclError:
            return False

    def _bind_mousewheel(self, _event: tk.Event) -> None:
        """Bind global mousewheel handlers for the diagnosis table.

        Args:
            _event: Tkinter enter event.
        """
        self.canvas.bind_all("<MouseWheel>", self._on_mousewheel)
        self.canvas.bind_all("<Button-4>", self._on_mousewheel)
        self.canvas.bind_all("<Button-5>", self._on_mousewheel)

    def _unbind_mousewheel(self, _event: tk.Event) -> None:
        """Unbind global mousewheel handlers for the diagnosis table.

        Args:
            _event: Tkinter leave event.
        """
        self.canvas.unbind_all("<MouseWheel>")
        self.canvas.unbind_all("<Button-4>")
        self.canvas.unbind_all("<Button-5>")

    def _on_mousewheel(self, event: tk.Event) -> str:
        """Handle mousewheel events for diagnosis table scrolling.

        Args:
            event: Mousewheel event.

        Returns:
            "break" when event is handled, otherwise empty string.
        """
        if getattr(event, "num", None) == 4:
            delta_units = -1
        elif getattr(event, "num", None) == 5:
            delta_units = 1
        else:
            delta_units = -1 if getattr(event, "delta", 0) > 0 else 1
        if isinstance(event.widget, ttk.Combobox) and event.widget.instate(["focus"]):
            return ""
        if self._is_combobox_interaction_active():
            return "break"
        self.canvas.yview_scroll(delta_units, "units")
        return "break"


# pylint: disable-next=too-many-ancestors
class BmsConfigurationFrame(BaseFrame):
    """'BMS Configuration' Frame"""

    def __init__(self, parent: ttk.Notebook, text_widget: tk.Text) -> None:
        """Initialize the BMS configuration frame and subtabs.

        Args:
            parent: Parent notebook widget.
            text_widget: Shared output text widget.
        """
        super().__init__(parent, text_widget, "output_gui_bms_configuration.txt")
        self._closing = False

        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        bms_configuration_notebook = ttk.Notebook(self)
        bms_configuration_notebook.grid(column=0, row=0, sticky="news")
        self.bms_configuration_notebook = bms_configuration_notebook
        self.bms_configuration_notebook.bind(
            "<<NotebookTabChanged>>", self._subtab_changed_cb
        )

        bms_tab = BmsConfigurationSubFrame(
            bms_configuration_notebook,
            text_widget,
            "output_gui_bms_configuration_bms.txt",
        )
        bms_tab.pack(fill=tk.BOTH, expand=True)
        bms_configuration_notebook.add(bms_tab, text="BMS")

        battery_system_tab = BmsConfigurationSubFrame(
            bms_configuration_notebook,
            text_widget,
            "output_gui_bms_configuration_battery_system.txt",
        )
        battery_system_tab.pack(fill=tk.BOTH, expand=True)
        bms_configuration_notebook.add(battery_system_tab, text="Battery System")

        battery_cell_tab = BmsConfigurationSubFrame(
            bms_configuration_notebook,
            text_widget,
            "output_gui_bms_configuration_battery_cell.txt",
        )
        battery_cell_tab.pack(fill=tk.BOTH, expand=True)
        bms_configuration_notebook.add(battery_cell_tab, text="Battery Cell")

        diagnosis_tab = DiagnosisConfigFrame(bms_configuration_notebook, text_widget)
        diagnosis_tab.pack(fill=tk.BOTH, expand=True)
        bms_configuration_notebook.add(diagnosis_tab, text="Diagnosis")

    def _get_active_subtab(self) -> BaseFrame:
        """Return the currently selected BMS configuration subtab.

        Returns:
            Active subtab frame.
        """
        _select = self.bms_configuration_notebook.select()  # type: ignore[no-untyped-call]
        return self.bms_configuration_notebook.nametowidget(_select)

    def _subtab_changed_cb(self, _event: tk.Event) -> None:
        """Refresh output text when switching BMS configuration subtabs.

        Args:
            _event: Notebook tab change event.
        """
        if self._closing or not self.winfo_exists():
            return
        active_subtab = self._get_active_subtab()
        active_subtab.reset_text()
        active_subtab.write_text()

    def reset_text(self) -> None:
        """Reset output text using the active subtab context."""
        self._get_active_subtab().reset_text()

    def write_text(self, file_input: str | None = None) -> None:
        """Write output text using the active subtab context.

        Args:
            file_input: Optional text to append before refresh.
        """
        self._get_active_subtab().write_text(file_input)

    def on_close(self) -> None:
        """Run cleanup hooks for all BMS configuration subtabs."""
        self._closing = True
        self.bms_configuration_notebook.unbind("<<NotebookTabChanged>>")
        for tab_name in self.bms_configuration_notebook.tabs():  # type: ignore[no-untyped-call]
            tab_obj = self.bms_configuration_notebook.nametowidget(tab_name)
            tab_obj.on_close()
        super().on_close()
