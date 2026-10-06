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

"""Testing file 'cli/cmd_gui/frame_bms_configuration/bms_configuration_gui.py'."""

# cspell:ignore scrollregion,subtab

# pylint: disable=protected-access,too-many-lines

import importlib
import json
import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from tkinter import ttk
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_bms_configuration import bms_configuration_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_bms_configuration import bms_configuration_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "build_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class BmsConfigurationGuiTestBase(unittest.TestCase):
    """Provide shared GUI setup and helpers for BMS configuration tests."""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack()
        text = tk.Text()
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)
            importlib.reload(bms_configuration_gui)
            self.frame = bms_configuration_gui.BmsConfigurationFrame(
                self.notebook, text
            )
            self.frame.pack()
            self.notebook.add(self.frame, text="BMS Configuration")

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    def _get_bms_notebook(self) -> ttk.Notebook:
        """Return the nested notebook of the BMS configuration frame."""
        notebook = self.frame.winfo_children()[0]
        self.assertIsInstance(notebook, ttk.Notebook)
        return notebook

    def _get_diagnosis_tab(self) -> bms_configuration_gui.DiagnosisConfigFrame:
        """Return the diagnosis configuration tab instance."""
        notebook = self._get_bms_notebook()
        return notebook.nametowidget(notebook.tabs()[-1])

    @staticmethod
    def _get_comboboxes(
        diagnosis_tab: bms_configuration_gui.DiagnosisConfigFrame,
    ) -> list[ttk.Combobox]:
        """Return all combobox widgets from a diagnosis tab."""
        return [
            child
            for child in diagnosis_tab.entries_frame.winfo_children()
            if isinstance(child, ttk.Combobox)
        ]

    @staticmethod
    def _get_labels(
        diagnosis_tab: bms_configuration_gui.DiagnosisConfigFrame,
    ) -> list[ttk.Label]:
        """Return all label widgets from a diagnosis tab."""
        return [
            child
            for child in diagnosis_tab.entries_frame.winfo_children()
            if isinstance(child, ttk.Label)
        ]


@patch("cli.cmd_gui.frame_bms_configuration.bms_configuration_gui.BaseFrame.write_text")
class TestBmsConfigurationTabsAndPersistence(BmsConfigurationGuiTestBase):
    """Test tab structure and diagnosis load/save behavior."""

    def test_bms_configuration_tabs_exist(self, _mock_write_text: MagicMock) -> None:
        """Test that all expected BMS configuration tabs are available."""
        notebook = self._get_bms_notebook()
        tab_names = [notebook.tab(tab, option="text") for tab in notebook.tabs()]
        self.assertEqual(
            tab_names,
            ["BMS", "Battery System", "Battery Cell", "Diagnosis"],
        )

    def test_diagnosis_rows_are_loaded(self, _mock_write_text: MagicMock) -> None:
        """Test that diagnosis entries from files[].array_entries are loaded."""
        diagnosis_tab = self._get_diagnosis_tab()

        self.assertGreater(len(diagnosis_tab.dropdown_vars), 0)
        first_entry = diagnosis_tab.dropdown_vars[0][0]
        self.assertIn("event", first_entry)
        self.assertIn("sensitivity", first_entry)
        self.assertIn("severity", first_entry)
        self.assertIn("delay", first_entry)
        self.assertIn("enabled", first_entry)
        self.assertIn("callback", first_entry)
        self.assertIn("description", first_entry)

    def test_diagnosis_save_writes_dropdown_values(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test saving diagnosis data from dropdown values into JSON file."""
        diagnosis_tab = self._get_diagnosis_tab()

        test_config = {
            "files": [
                {
                    "array_entries": [
                        {
                            "event": "DIAG_ID_TEST_EVENT",
                            "description": "Description text",
                            "sensitivity": "1",
                            "severity": "WARNING",
                            "delay": "DISCARD",
                            "enabled": True,
                            "callback": "DIAG_TestCallback",
                        }
                    ]
                }
            ]
        }
        test_file = PATH_GUI / "diag_array_cfg_test.json"
        test_file.parent.mkdir(parents=True, exist_ok=True)
        with open(test_file, mode="w", encoding="utf-8") as f:
            json.dump(test_config, f, ensure_ascii=True, indent=2)
            f.write("\n")

        diagnosis_tab.file_path_var.set(str(test_file))
        diagnosis_tab.load_config_cb(log_output=False)

        entry, sensitivity_var, severity_var, delay_var, enabled_var = (
            diagnosis_tab.dropdown_vars[0]
        )
        self.assertEqual(entry["callback"], "DIAG_TestCallback")
        self.assertEqual(entry["description"], "Description text")

        sensitivity_var.set("10")
        severity_var.set("FATAL_ERROR")
        delay_var.set("200")
        enabled_var.set("false")
        diagnosis_tab.save_config_cb()

        with open(test_file, encoding="utf-8") as f:
            saved = json.load(f)
        saved_entry = saved["files"][0]["array_entries"][0]

        self.assertEqual(saved_entry["sensitivity"], 10)
        self.assertEqual(saved_entry["severity"], "FATAL_ERROR")
        self.assertEqual(saved_entry["delay"], 200)
        self.assertFalse(saved_entry["enabled"])
        self.assertEqual(saved_entry["callback"], "DIAG_TestCallback")
        self.assertEqual(saved_entry["description"], "Description text")

    def test_load_config_missing_file_logs_when_enabled(
        self, mock_write_text: MagicMock
    ) -> None:
        """Test missing config file logs an error when logging is enabled."""
        diagnosis_tab = self._get_diagnosis_tab()
        missing_file = PATH_GUI / "missing_diag_cfg.json"
        diagnosis_tab.file_path_var.set(str(missing_file))

        diagnosis_tab.load_config_cb(log_output=True)

        mock_write_text.assert_any_call(
            f"Diagnosis config file not found: '{missing_file}'.\n"
        )

    def test_load_config_missing_file_without_logging(
        self, mock_write_text: MagicMock
    ) -> None:
        """Test missing config file returns silently when logging is disabled."""
        diagnosis_tab = self._get_diagnosis_tab()
        diagnosis_tab.file_path_var.set(str(PATH_GUI / "missing_no_log.json"))

        diagnosis_tab.load_config_cb(log_output=False)

        mock_write_text.assert_not_called()

    def test_load_config_missing_entries_logs_when_enabled(
        self, mock_write_text: MagicMock
    ) -> None:
        """Test empty files[].array_entries logs a dedicated message."""
        diagnosis_tab = self._get_diagnosis_tab()

        test_file = PATH_GUI / "diag_array_cfg_no_entries.json"
        test_file.parent.mkdir(parents=True, exist_ok=True)
        with open(test_file, mode="w", encoding="utf-8") as f:
            json.dump(
                {"files": [{"array_entries": []}]}, f, ensure_ascii=True, indent=2
            )
            f.write("\n")

        diagnosis_tab.file_path_var.set(str(test_file))
        diagnosis_tab.load_config_cb(log_output=True)

        mock_write_text.assert_any_call(
            f"No entries found in 'files[].array_entries' in '{test_file}'.\n"
        )

    def test_load_config_missing_entries_without_logging(
        self, mock_write_text: MagicMock
    ) -> None:
        """Test empty entries return silently when logging is disabled."""
        diagnosis_tab = self._get_diagnosis_tab()

        test_file = PATH_GUI / "diag_array_cfg_no_entries_no_log.json"
        test_file.parent.mkdir(parents=True, exist_ok=True)
        with open(test_file, mode="w", encoding="utf-8") as f:
            json.dump(
                {"files": [{"array_entries": []}]}, f, ensure_ascii=True, indent=2
            )
            f.write("\n")

        diagnosis_tab.file_path_var.set(str(test_file))
        diagnosis_tab.load_config_cb(log_output=False)

        mock_write_text.assert_not_called()

    def test_load_config_logs_success_when_enabled(
        self, mock_write_text: MagicMock
    ) -> None:
        """Test successful config load writes a status message."""
        diagnosis_tab = self._get_diagnosis_tab()

        test_file = PATH_GUI / "diag_array_cfg_load_success.json"
        test_file.parent.mkdir(parents=True, exist_ok=True)
        with open(test_file, mode="w", encoding="utf-8") as f:
            json.dump(
                {
                    "files": [
                        {
                            "array_entries": [
                                {
                                    "event": "DIAG_ID_TEST_EVENT",
                                    "sensitivity": "1",
                                    "severity": "INFO",
                                    "delay": "0",
                                    "enabled": True,
                                    "callback": "DIAG_TestCallback",
                                    "description": "desc",
                                }
                            ]
                        }
                    ]
                },
                f,
                ensure_ascii=True,
                indent=2,
            )
            f.write("\n")

        diagnosis_tab.file_path_var.set(str(test_file))
        diagnosis_tab.load_config_cb(log_output=True)

        mock_write_text.assert_any_call(
            f"Loaded diagnosis config from '{test_file}'.\n"
        )

    def test_save_config_missing_file_logs(self, mock_write_text: MagicMock) -> None:
        """Test save logs a clear message when config file is missing."""
        diagnosis_tab = self._get_diagnosis_tab()
        missing_file = PATH_GUI / "missing_save_cfg.json"
        diagnosis_tab.file_path_var.set(str(missing_file))

        diagnosis_tab.save_config_cb()

        mock_write_text.assert_any_call(
            f"Diagnosis config file not found: '{missing_file}'.\n"
        )

    def test_save_config_no_data_loaded_logs(self, mock_write_text: MagicMock) -> None:
        """Test save aborts when no diagnosis data is loaded."""
        diagnosis_tab = self._get_diagnosis_tab()

        existing_file = PATH_GUI / "diag_array_cfg_no_data.json"
        existing_file.parent.mkdir(parents=True, exist_ok=True)
        existing_file.write_text("{}\n", encoding="utf-8")

        diagnosis_tab.file_path_var.set(str(existing_file))
        diagnosis_tab.diag_data = None
        diagnosis_tab.save_config_cb()

        mock_write_text.assert_any_call("No diagnosis data loaded.\n")

    def test_save_config_no_entries_loaded_logs(
        self, mock_write_text: MagicMock
    ) -> None:
        """Test save aborts when no dropdown entries are available."""
        diagnosis_tab = self._get_diagnosis_tab()

        existing_file = PATH_GUI / "diag_array_cfg_no_dropdowns.json"
        existing_file.parent.mkdir(parents=True, exist_ok=True)
        existing_file.write_text("{}\n", encoding="utf-8")

        diagnosis_tab.file_path_var.set(str(existing_file))
        diagnosis_tab.diag_data = {"files": []}
        diagnosis_tab.dropdown_vars = []
        diagnosis_tab.save_config_cb()

        mock_write_text.assert_any_call("No diagnosis entries loaded.\n")

    def test_save_config_enabled_true_and_other_value(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test save converts enabled 'true' to bool and keeps unknown string values."""
        diagnosis_tab = self._get_diagnosis_tab()

        test_file = PATH_GUI / "diag_array_cfg_enabled_branches.json"
        test_file.parent.mkdir(parents=True, exist_ok=True)
        with open(test_file, mode="w", encoding="utf-8") as f:
            json.dump({"files": []}, f, ensure_ascii=True, indent=2)
            f.write("\n")

        diagnosis_tab.file_path_var.set(str(test_file))
        diagnosis_tab.diag_data = {
            "files": [
                {
                    "array_entries": [
                        {
                            "event": "E1",
                            "sensitivity": "1",
                            "severity": "INFO",
                            "delay": "0",
                            "enabled": False,
                            "callback": "CB1",
                            "description": "D1",
                        },
                        {
                            "event": "E2",
                            "sensitivity": "1",
                            "severity": "INFO",
                            "delay": "0",
                            "enabled": False,
                            "callback": "CB2",
                            "description": "D2",
                        },
                    ]
                }
            ]
        }
        entries = diagnosis_tab.diag_data["files"][0]["array_entries"]
        diagnosis_tab.dropdown_vars = [
            (
                entries[0],
                tk.StringVar(value="5"),
                tk.StringVar(value="WARNING"),
                tk.StringVar(value="100"),
                tk.StringVar(value="True"),
            ),
            (
                entries[1],
                tk.StringVar(value="10"),
                tk.StringVar(value="FATAL_ERROR"),
                tk.StringVar(value="200"),
                tk.StringVar(value="False"),
            ),
        ]

        with patch.object(diagnosis_tab, "_write_git_diff"):
            diagnosis_tab.save_config_cb()

        self.assertTrue(entries[0]["enabled"])
        self.assertFalse(entries[1]["enabled"])

    def test_write_git_diff_outside_repo_logs(self, mock_write_text: MagicMock) -> None:
        """Test git diff is skipped for files outside the repository root."""
        diagnosis_tab = self._get_diagnosis_tab()

        outside_file = Path("c:/tmp/diag_array_cfg.json")
        diagnosis_tab._write_git_diff(outside_file)

        mock_write_text.assert_any_call(
            f"No git diff generated, file is outside repository: '{outside_file}'.\n"
        )

    @patch("cli.cmd_gui.frame_bms_configuration.bms_configuration_gui.run_process")
    def test_write_git_diff_failure_logs_error(
        self, mock_run_process: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test git diff execution failures are logged."""
        diagnosis_tab = self._get_diagnosis_tab()

        mock_run_process.return_value = SimpleNamespace(
            returncode=1, out="", err="fatal: not a git repository"
        )
        repo_file = bms_configuration_gui.PROJECT_ROOT / "conf/bms/diag_array_cfg.json"
        diagnosis_tab._write_git_diff(repo_file)

        mock_write_text.assert_any_call(
            "git diff failed for 'conf/bms/diag_array_cfg.json': "
            "fatal: not a git repository\n"
        )

    @patch("cli.cmd_gui.frame_bms_configuration.bms_configuration_gui.run_process")
    def test_write_git_diff_with_diff_logs_content(
        self, mock_run_process: MagicMock, mock_write_text: MagicMock
    ) -> None:
        """Test git diff output is written when differences exist."""
        diagnosis_tab = self._get_diagnosis_tab()
        mock_run_process.return_value = SimpleNamespace(
            returncode=0,
            out="diff --git a/conf/bms/diag_array_cfg.json b/conf/bms/diag_array_cfg.json",
            err="",
        )
        repo_file = bms_configuration_gui.PROJECT_ROOT / "conf/bms/diag_array_cfg.json"
        diagnosis_tab._write_git_diff(repo_file)

        mock_write_text.assert_any_call(
            "Git diff for 'conf/bms/diag_array_cfg.json':\n"
        )
        mock_write_text.assert_any_call(
            "diff --git a/conf/bms/diag_array_cfg.json b/conf/bms/diag_array_cfg.json\n"
        )


@patch("cli.cmd_gui.frame_bms_configuration.bms_configuration_gui.BaseFrame.write_text")
# pylint: disable-next=too-many-public-methods
class TestDiagnosisWheelAndCanvasLogic(BmsConfigurationGuiTestBase):
    """Test diagnosis mouse wheel routing and canvas sizing logic."""

    def test_mousewheel_on_unfocused_combobox_scrolls_tab(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test wheel on unfocused combobox scrolls the diagnosis table."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]

        with (
            patch.object(combobox, "instate", return_value=False),
            patch.object(diagnosis_tab, "_on_mousewheel") as mock_on_mousewheel,
        ):
            event = MagicMock()
            handler = diagnosis_tab._on_combobox_mousewheel
            result = handler(event, combobox)

        mock_on_mousewheel.assert_called_once_with(event)
        self.assertEqual(result, "break")

    def test_mousewheel_on_focused_combobox_keeps_default(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test wheel on focused combobox keeps combobox default behavior."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]

        with (
            patch.object(combobox, "instate", return_value=True),
            patch.object(diagnosis_tab, "_on_mousewheel") as mock_on_mousewheel,
        ):
            event = MagicMock()
            handler = diagnosis_tab._on_combobox_mousewheel
            result = handler(event, combobox)

        mock_on_mousewheel.assert_not_called()
        self.assertIsNone(result)

    def test_mousewheel_direction(self, _mock_write_text: MagicMock) -> None:
        """Test wheel direction is translated to canvas scrolling units."""
        diagnosis_tab = self._get_diagnosis_tab()

        with patch.object(diagnosis_tab.canvas, "yview_scroll") as mock_scroll:
            event_up = MagicMock()
            event_up.delta = 120
            event_up.num = None
            wheel_handler = diagnosis_tab._on_mousewheel
            wheel_handler(event_up)

            event_down = MagicMock()
            event_down.delta = -120
            event_down.num = None
            wheel_handler(event_down)

        self.assertEqual(mock_scroll.call_count, 2)
        mock_scroll.assert_any_call(-1, "units")
        mock_scroll.assert_any_call(1, "units")

    def test_apply_canvas_width_uses_content_width(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test that horizontal range is not clipped by viewport width."""
        diagnosis_tab = self._get_diagnosis_tab()

        diagnosis_tab._pending_canvas_width = 600
        with (
            patch.object(
                diagnosis_tab.entries_frame, "winfo_reqwidth", return_value=1200
            ),
            patch.object(diagnosis_tab.canvas, "itemconfigure") as mock_itemconfigure,
        ):
            diagnosis_tab._apply_canvas_width()

        mock_itemconfigure.assert_called_once_with(
            diagnosis_tab.canvas_window, width=1200
        )

    def test_mousewheel_on_focused_combobox_does_not_scroll_canvas(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test focused combobox wheel leaves scrolling to combobox behavior."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]

        with (
            patch.object(combobox, "instate", return_value=True),
            patch.object(diagnosis_tab.canvas, "yview_scroll") as mock_scroll,
        ):
            event = MagicMock()
            event.widget = combobox
            event.delta = 120
            event.num = None
            result = diagnosis_tab._on_mousewheel(event)

        mock_scroll.assert_not_called()
        self.assertEqual(result, "")

    def test_mousewheel_on_canvas_with_active_combobox_disables_tab_scrolling(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test selected combobox disables tab wheel scrolling on canvas."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]

        diagnosis_tab._active_combobox = combobox
        with (
            patch.object(combobox, "instate", return_value=True),
            patch.object(diagnosis_tab.canvas, "yview_scroll") as mock_canvas_scroll,
        ):
            event = MagicMock()
            event.widget = diagnosis_tab.canvas
            event.delta = -120
            event.num = None
            result = diagnosis_tab._on_mousewheel(event)

        self.assertEqual(result, "break")
        mock_canvas_scroll.assert_not_called()

    def test_mousewheel_on_canvas_with_active_combobox_blocks_without_focus(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test selected combobox still blocks tab scrolling without combobox focus."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]

        diagnosis_tab._active_combobox = combobox
        with (
            patch.object(combobox, "winfo_exists", return_value=True),
            patch.object(combobox, "instate", return_value=False),
            patch.object(diagnosis_tab.canvas, "yview_scroll") as mock_canvas_scroll,
        ):
            event = MagicMock()
            event.widget = diagnosis_tab.canvas
            event.delta = -120
            event.num = None
            result = diagnosis_tab._on_mousewheel(event)

        self.assertEqual(result, "break")
        mock_canvas_scroll.assert_not_called()

    def test_background_click_clears_active_combobox(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clicking canvas background clears active combobox selection."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        diagnosis_tab._active_combobox = comboboxes[0]

        event = MagicMock()
        event.widget = diagnosis_tab.canvas
        diagnosis_tab._clear_selection_cb(event)

        self.assertIsNone(diagnosis_tab._active_combobox)

    def test_label_click_clears_active_combobox(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clicking a non-combobox child widget clears active combobox."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        labels = self._get_labels(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        self.assertGreater(len(labels), 0)
        diagnosis_tab._active_combobox = comboboxes[0]

        event = MagicMock()
        event.widget = labels[0]
        diagnosis_tab._clear_selection_cb(event)

        self.assertIsNone(diagnosis_tab._active_combobox)

    def test_combobox_selected_event_clears_active_combobox(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test selecting a combobox value auto-clears active selection state."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]

        diagnosis_tab._active_combobox = combobox
        diagnosis_tab._clear_after_selection(combobox)

        self.assertIsNone(diagnosis_tab._active_combobox)

    def test_clear_after_popdown_closed_retries_while_popdown_focused(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test popdown-close cleanup retries until popdown focus is gone."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreater(len(comboboxes), 0)
        combobox = comboboxes[0]
        diagnosis_tab._active_combobox = combobox

        with (
            patch.object(combobox, "winfo_exists", return_value=True),
            patch.object(diagnosis_tab, "_is_popdown_focused", return_value=True),
            patch.object(diagnosis_tab, "after") as mock_after,
        ):
            diagnosis_tab._clear_after_popdown_closed(combobox)

        mock_after.assert_called_once()
        self.assertEqual(mock_after.call_args[0][0], 50)
        self.assertIs(diagnosis_tab._active_combobox, combobox)

    def test_combobox_event_wrappers_route_combobox_events(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test the event wrapper methods forward combobox events correctly."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]

        event = MagicMock()
        event.widget = combobox
        with (
            patch.object(diagnosis_tab, "_set_active_combobox") as mock_set_active,
            patch.object(
                diagnosis_tab, "_clear_after_selection"
            ) as mock_clear_selection,
            patch.object(diagnosis_tab, "_clear_active_combobox") as mock_clear_active,
            patch.object(
                diagnosis_tab, "_on_combobox_mousewheel", return_value="break"
            ) as mock_mousewheel,
        ):
            diagnosis_tab._set_active_combobox_cb(event)
            diagnosis_tab._clear_after_selection_cb(event)
            diagnosis_tab._clear_active_combobox_cb(event)
            result = diagnosis_tab._on_combobox_mousewheel_cb(event)

        mock_set_active.assert_called_once_with(combobox)
        mock_clear_selection.assert_called_once_with(combobox)
        mock_clear_active.assert_called_once_with(combobox)
        mock_mousewheel.assert_called_once_with(event, combobox)
        self.assertEqual(result, "break")

    def test_combobox_event_wrappers_ignore_non_combobox_widget(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test the event wrapper methods ignore unrelated widgets."""
        diagnosis_tab = self._get_diagnosis_tab()
        event = MagicMock()
        event.widget = object()

        with (
            patch.object(diagnosis_tab, "_set_active_combobox") as mock_set_active,
            patch.object(
                diagnosis_tab, "_clear_after_selection"
            ) as mock_clear_selection,
            patch.object(diagnosis_tab, "_clear_active_combobox") as mock_clear_active,
        ):
            diagnosis_tab._set_active_combobox_cb(event)
            diagnosis_tab._clear_after_selection_cb(event)
            diagnosis_tab._clear_active_combobox_cb(event)

        mock_set_active.assert_not_called()
        mock_clear_selection.assert_not_called()
        mock_clear_active.assert_not_called()

    def test_on_combobox_mousewheel_cb_ignores_non_combobox_widget(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test the wheel wrapper returns None for non-combobox widgets."""
        diagnosis_tab = self._get_diagnosis_tab()
        event = MagicMock()
        event.widget = object()

        with patch.object(diagnosis_tab, "_on_combobox_mousewheel") as mock_mousewheel:
            result = diagnosis_tab._on_combobox_mousewheel_cb(event)

        mock_mousewheel.assert_not_called()
        self.assertIsNone(result)

    def test_on_frame_configure_returns_when_already_scheduled(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test repeated frame configure event returns when update is pending."""
        diagnosis_tab = self._get_diagnosis_tab()

        diagnosis_tab._scrollregion_update_scheduled = True
        with patch.object(diagnosis_tab, "after_idle") as mock_after_idle:
            diagnosis_tab._on_frame_configure(MagicMock())

        mock_after_idle.assert_not_called()

    def test_on_frame_configure_schedules_update_when_not_pending(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test frame configure schedules scrollregion update when idle."""
        diagnosis_tab = self._get_diagnosis_tab()

        diagnosis_tab._scrollregion_update_scheduled = False
        with patch.object(diagnosis_tab, "after_idle") as mock_after_idle:
            diagnosis_tab._on_frame_configure(MagicMock())

        mock_after_idle.assert_called_once_with(diagnosis_tab._update_scrollregion)

    def test_on_canvas_configure_cancels_pending_resize(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test canvas configure cancels pending resize timer before scheduling."""
        diagnosis_tab = self._get_diagnosis_tab()

        diagnosis_tab._resize_after_id = "after-id"
        event = MagicMock()
        event.width = 777
        with (
            patch.object(diagnosis_tab, "after_cancel") as mock_after_cancel,
            patch.object(diagnosis_tab, "after", return_value="new-id") as mock_after,
        ):
            diagnosis_tab._on_canvas_configure(event)

        mock_after_cancel.assert_called_once_with("after-id")
        mock_after.assert_called_once_with(10, diagnosis_tab._apply_canvas_width)
        self.assertEqual(diagnosis_tab._pending_canvas_width, 777)
        self.assertEqual(diagnosis_tab._resize_after_id, "new-id")

    def test_on_canvas_configure_without_pending_resize(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test canvas configure schedules resize when no timer is pending."""
        diagnosis_tab = self._get_diagnosis_tab()

        diagnosis_tab._resize_after_id = None
        event = MagicMock()
        event.width = 500
        with (
            patch.object(diagnosis_tab, "after_cancel") as mock_after_cancel,
            patch.object(diagnosis_tab, "after", return_value="next-id") as mock_after,
        ):
            diagnosis_tab._on_canvas_configure(event)

        mock_after_cancel.assert_not_called()
        mock_after.assert_called_once_with(10, diagnosis_tab._apply_canvas_width)
        self.assertEqual(diagnosis_tab._resize_after_id, "next-id")

    def test_set_active_combobox_assigns_widget(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test set_active_combobox stores the given widget."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]

        diagnosis_tab._set_active_combobox(combobox)
        self.assertIs(diagnosis_tab._active_combobox, combobox)

    def test_clear_after_selection_with_other_widget_keeps_active(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test selecting a different combobox keeps current active combobox."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreaterEqual(len(comboboxes), 2)

        diagnosis_tab._active_combobox = comboboxes[0]
        with patch.object(diagnosis_tab.canvas, "focus_set") as mock_focus_set:
            diagnosis_tab._clear_after_selection(comboboxes[1])

        self.assertIs(diagnosis_tab._active_combobox, comboboxes[0])
        mock_focus_set.assert_called_once()

    def test_clear_selection_ignores_non_tk_widget(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clear selection returns for events without Tk widgets."""
        diagnosis_tab = self._get_diagnosis_tab()
        event = MagicMock()
        event.widget = object()
        diagnosis_tab._active_combobox = MagicMock()

        diagnosis_tab._clear_selection_cb(event)

        self.assertIsNotNone(diagnosis_tab._active_combobox)

    def test_clear_selection_returns_when_widget_not_descendant(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clear selection exits when clicked widget is outside the frame."""
        diagnosis_tab = self._get_diagnosis_tab()
        event = MagicMock()
        event.widget = self.root

        diagnosis_tab._active_combobox = MagicMock()
        diagnosis_tab._clear_selection_cb(event)

        self.assertIsNotNone(diagnosis_tab._active_combobox)

    def test_clear_selection_handles_key_error_when_walking_parents(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clear selection handles stale parent references gracefully."""
        diagnosis_tab = self._get_diagnosis_tab()
        label = self._get_labels(diagnosis_tab)[0]
        diagnosis_tab._active_combobox = MagicMock()

        event = MagicMock()
        event.widget = label
        with (
            patch.object(label, "winfo_parent", return_value="bad-parent"),
            patch.object(label, "nametowidget", side_effect=KeyError),
        ):
            diagnosis_tab._clear_selection_cb(event)

        self.assertIsNotNone(diagnosis_tab._active_combobox)

    def test_clear_selection_handles_none_parent_widget(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test parent walk handles unresolved parent widgets safely."""
        diagnosis_tab = self._get_diagnosis_tab()
        label = self._get_labels(diagnosis_tab)[0]
        diagnosis_tab._active_combobox = MagicMock()

        event = MagicMock()
        event.widget = label
        with (
            patch.object(label, "winfo_parent", return_value="missing-parent"),
            patch.object(label, "nametowidget", return_value=None),
        ):
            diagnosis_tab._clear_selection_cb(event)

        self.assertIsNotNone(diagnosis_tab._active_combobox)

    def test_clear_selection_keeps_active_when_clicking_combobox(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clear selection does not clear when click target is combobox."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]
        diagnosis_tab._active_combobox = combobox

        event = MagicMock()
        event.widget = combobox
        diagnosis_tab._clear_selection_cb(event)

        self.assertIs(diagnosis_tab._active_combobox, combobox)

    def test_clear_active_combobox_schedules_popdown_cleanup(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test focus-out schedules delayed cleanup while popdown is focused."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]

        with (
            patch.object(diagnosis_tab, "_is_popdown_focused", return_value=True),
            patch.object(diagnosis_tab, "after") as mock_after,
        ):
            diagnosis_tab._clear_active_combobox(combobox)

        mock_after.assert_called_once()
        self.assertEqual(mock_after.call_args[0][0], 50)

    def test_clear_active_combobox_clears_directly_when_not_popdown_focused(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test focus-out clears active combobox directly when popdown is closed."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]
        diagnosis_tab._active_combobox = combobox

        with patch.object(diagnosis_tab, "_is_popdown_focused", return_value=False):
            diagnosis_tab._clear_active_combobox(combobox)

        self.assertIsNone(diagnosis_tab._active_combobox)

    def test_clear_active_combobox_keeps_other_active_widget(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test focus-out does not clear active state for a different combobox."""
        diagnosis_tab = self._get_diagnosis_tab()
        comboboxes = self._get_comboboxes(diagnosis_tab)
        self.assertGreaterEqual(len(comboboxes), 2)
        diagnosis_tab._active_combobox = comboboxes[0]

        with patch.object(diagnosis_tab, "_is_popdown_focused", return_value=False):
            diagnosis_tab._clear_active_combobox(comboboxes[1])

        self.assertIs(diagnosis_tab._active_combobox, comboboxes[0])

    def test_is_popdown_focused_true_and_tcl_error(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test popdown focus helper for both normal and TclError code paths."""
        widget = MagicMock()
        widget.tk.call = MagicMock(side_effect=[".c.popdown", ".c.popdown.f"])
        self.assertTrue(
            bms_configuration_gui.DiagnosisConfigFrame._is_popdown_focused(widget)
        )

        widget_error = MagicMock()
        widget_error.tk.call = MagicMock(side_effect=tk.TclError("bad"))
        self.assertFalse(
            bms_configuration_gui.DiagnosisConfigFrame._is_popdown_focused(widget_error)
        )

    def test_clear_after_popdown_closed_early_return_paths(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clear-after-popdown returns early on inactive or destroyed widgets."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]

        diagnosis_tab._active_combobox = None
        diagnosis_tab._clear_after_popdown_closed(combobox)

        diagnosis_tab._active_combobox = combobox
        with patch.object(combobox, "winfo_exists", return_value=False):
            diagnosis_tab._clear_after_popdown_closed(combobox)

    def test_clear_after_popdown_closed_clears_when_closed(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test clear-after-popdown clears active combobox when popdown closes."""
        diagnosis_tab = self._get_diagnosis_tab()
        combobox = self._get_comboboxes(diagnosis_tab)[0]
        diagnosis_tab._active_combobox = combobox

        with (
            patch.object(combobox, "winfo_exists", return_value=True),
            patch.object(diagnosis_tab, "_is_popdown_focused", return_value=False),
            patch.object(diagnosis_tab.canvas, "focus_set") as mock_focus_set,
        ):
            diagnosis_tab._clear_after_popdown_closed(combobox)

        self.assertIsNone(diagnosis_tab._active_combobox)
        mock_focus_set.assert_called_once()

    def test_is_combobox_interaction_active_handles_tcl_error(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test interaction-active helper handles TclError from focus query."""
        dummy = SimpleNamespace(
            _active_combobox=None,
            tk=SimpleNamespace(call=MagicMock(side_effect=tk.TclError("bad"))),
        )

        self.assertFalse(
            bms_configuration_gui.DiagnosisConfigFrame._is_combobox_interaction_active(
                dummy
            )
        )

    def test_is_combobox_interaction_active_true_on_popdown_focus(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test interaction-active helper detects popdown in focus path."""
        dummy = SimpleNamespace(
            _active_combobox=None,
            tk=SimpleNamespace(call=MagicMock(return_value=".foo.popdown.child")),
        )
        self.assertTrue(
            bms_configuration_gui.DiagnosisConfigFrame._is_combobox_interaction_active(
                dummy
            )
        )

    def test_bind_and_unbind_mousewheel(self, _mock_write_text: MagicMock) -> None:
        """Test global wheel event binding and unbinding for diagnosis canvas."""
        diagnosis_tab = self._get_diagnosis_tab()

        with (
            patch.object(diagnosis_tab.canvas, "bind_all") as mock_bind_all,
            patch.object(diagnosis_tab.canvas, "unbind_all") as mock_unbind_all,
        ):
            diagnosis_tab._bind_mousewheel(MagicMock())
            diagnosis_tab._unbind_mousewheel(MagicMock())

        self.assertEqual(mock_bind_all.call_count, 3)
        self.assertEqual(mock_unbind_all.call_count, 3)

    def test_mousewheel_button4_and_button5_branches(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test Linux wheel button events map to expected scroll units."""
        diagnosis_tab = self._get_diagnosis_tab()

        with (
            patch.object(
                diagnosis_tab, "_is_combobox_interaction_active", return_value=False
            ),
            patch.object(diagnosis_tab.canvas, "yview_scroll") as mock_scroll,
        ):
            event_up = MagicMock()
            event_up.num = 4
            event_up.widget = diagnosis_tab.canvas
            diagnosis_tab._on_mousewheel(event_up)

            event_down = MagicMock()
            event_down.num = 5
            event_down.widget = diagnosis_tab.canvas
            diagnosis_tab._on_mousewheel(event_down)

        mock_scroll.assert_any_call(-1, "units")
        mock_scroll.assert_any_call(1, "units")

    def test_build_entries_table_with_non_bool_enabled(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test non-boolean enabled values are kept as strings in combobox vars."""
        diagnosis_tab = self._get_diagnosis_tab()
        diagnosis_tab._build_entries_table(
            [
                {
                    "event": "E",
                    "sensitivity": "1",
                    "severity": "INFO",
                    "delay": "0",
                    "enabled": "maybe",
                    "callback": "CB",
                    "description": "D",
                }
            ]
        )
        _entry, _s, _sev, _delay, enabled_var = diagnosis_tab.dropdown_vars[0]
        self.assertEqual(enabled_var.get(), "maybe")


@patch("cli.cmd_gui.frame_bms_configuration.bms_configuration_gui.BaseFrame.write_text")
class TestBmsConfigurationFrameDelegation(BmsConfigurationGuiTestBase):
    """Test BMS configuration frame subtab delegation and lifecycle cleanup."""

    def test_bms_frame_delegates_reset_write_and_subtab_change(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test BMS frame delegates output actions to the active subtab."""
        active_subtab = MagicMock()
        with patch.object(self.frame, "_get_active_subtab", return_value=active_subtab):
            self.frame.reset_text()
            self.frame.write_text("hello")
            self.frame._subtab_changed_cb(MagicMock())

        active_subtab.reset_text.assert_called()
        active_subtab.write_text.assert_any_call("hello")
        active_subtab.write_text.assert_any_call()

    def test_bms_subtab_changed_cb_skips_while_closing(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test tab-change refresh is skipped while the frame is closing."""
        self.frame._closing = True
        with patch.object(self.frame, "_get_active_subtab") as mock_get_active_subtab:
            self.frame._subtab_changed_cb(MagicMock())

        mock_get_active_subtab.assert_not_called()

    def test_bms_subtab_changed_cb_skips_when_widget_is_gone(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test tab-change refresh is skipped after the widget is destroyed."""
        self.frame._closing = False
        with (
            patch.object(self.frame, "winfo_exists", return_value=False),
            patch.object(self.frame, "_get_active_subtab") as mock_get_active_subtab,
        ):
            self.frame._subtab_changed_cb(MagicMock())

        mock_get_active_subtab.assert_not_called()

    def test_bms_frame_on_close_calls_all_tab_on_close(
        self, _mock_write_text: MagicMock
    ) -> None:
        """Test BMS frame close triggers tab cleanup and base close cleanup."""
        tabs = list(self.frame.bms_configuration_notebook.tabs())
        tab_frames = [
            self.frame.bms_configuration_notebook.nametowidget(tab) for tab in tabs
        ]

        for tab_frame in tab_frames:
            tab_frame.on_close = MagicMock()

        with patch("cli.cmd_gui.frame_base.BaseFrame.on_close") as mock_base_close:
            self.frame.on_close()

        for tab_frame in tab_frames:
            tab_frame.on_close.assert_called_once()
        mock_base_close.assert_called_once()


def remove_data(start_time: datetime) -> None:
    """Remove all data from the gui directory if it as been created after start_time"""
    if PATH_GUI.is_dir():
        if get_birthtime(PATH_GUI) >= start_time:
            shutil.rmtree(PATH_GUI)
        else:
            children = PATH_GUI.iterdir()
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
