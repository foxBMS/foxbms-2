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

"""Testing file 'cli/cmd_gui/gui_impl.py'."""

import importlib
import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.cmd_gui import gui_impl
    from cli.helpers.misc import PROJECT_BUILD_ROOT, PROJECT_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_gui import gui_impl
    from cli.helpers.misc import PROJECT_BUILD_ROOT, PROJECT_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "gui"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestFoxGui(unittest.TestCase):
    """Test of the FoxGui class"""

    @classmethod
    def setUpClass(cls):
        importlib.reload(gui_impl)

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)
        self.app = gui_impl.main()
        self.app.withdraw()
        self.app.notebook.unbind("<<NotebookTabChanged>>")

    def tearDown(self):
        self.app.update()
        self.app.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.gui_impl.open_ide_generic")
    def test_fox_gui_open_vs_code_generic(self, mock_ide_generic: MagicMock):
        """Test 'open_vs_code_generic_cb' function of FoxGui class"""
        self.app.open_vs_code_generic_cb()
        mock_ide_generic.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_cli")
    def test_fox_gui_open_vs_code_cli(self, mock_ide_cli: MagicMock):
        """Test 'open_vs_code_cli_cb' function of FoxGui class"""
        self.app.open_vs_code_cli_cb()
        mock_ide_cli.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_app")
    def test_fox_gui_open_vs_code_app(self, mock_ide_app: MagicMock):
        """Test 'open_vs_code_app_cb' function of FoxGui class"""
        self.app.open_vs_code_app_cb()
        mock_ide_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_app")
    def test_fox_gui_open_vs_code_app_unit_test(self, mock_ide_app: MagicMock):
        """Test 'open_vs_code_app_unit_test_cb' function of FoxGui class"""
        self.app.open_vs_code_app_unit_test_cb()
        mock_ide_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_bootloader")
    def test_fox_gui_open_vs_code_bootloader(self, mock_ide_bootloader: MagicMock):
        """Test 'open_vs_code_bootloader_cb' function of FoxGui class"""
        self.app.open_vs_code_bootloader_cb()
        mock_ide_bootloader.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_bootloader")
    def test_fox_gui_open_vs_code_bootloader_unit_test(
        self, mock_ide_bootloader: MagicMock
    ):
        """Test 'open_vs_code_bootloader_unit_test_cb' function of FoxGui class"""
        self.app.open_vs_code_bootloader_unit_test_cb()
        mock_ide_bootloader.assert_called_once()

    @patch("tkinter.filedialog.asksaveasfile")
    def test_fox_gui_new_file(self, mock_asksaveasfile: MagicMock):
        """Test 'create_new_file_cb' function of FoxGui class"""
        self.app.create_new_file_cb()
        mock_asksaveasfile.assert_called_once()

    @patch("webbrowser.open")
    def test_fox_gui_view_license(self, mock_open: MagicMock):
        """Test 'view_licence_cb' function of FoxGui class"""
        self.app.view_license_cb()
        mock_open.assert_called_once()

    def test_fox_gui_show_about(self):
        """Test 'show_about_cb' function of FoxGui class"""
        tk.Toplevel = MagicMock()
        self.app.show_about_cb()

    def test_fox_gui_tab_changed(self):
        """Test 'tab_changed_cb' function of FoxGui class"""
        self.app.text.config(state="normal")
        self.app.text.insert(tk.END, "This will be removed.\n")
        self.app.text.config(state="disabled")
        current_tab = self.app.notebook.nametowidget(self.app.notebook.select())
        current_tab.text_index = 10
        self.app.tab_changed_cb("<<NotebookTabChanged>>")
        self.assertEqual(current_tab.text_index, 0)
        self.assertEqual("\n", self.app.text.get("1.0", tk.END))


class TestFoxGuiNoUiTestableMethods(unittest.TestCase):
    """Test of the FoxGui class"""

    @classmethod
    def setUpClass(cls):
        importlib.reload(gui_impl)

    @patch("cli.cmd_gui.gui_impl.open_ide_generic")
    def test_fox_gui_open_vs_code_generic(self, mock_ide_generic: MagicMock):
        """Test 'open_vs_code_generic_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_generic_cb(None)
        mock_ide_generic.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_app")
    def test_fox_gui_open_vs_code_app(self, mock_ide_app: MagicMock):
        """Test 'open_vs_code_app_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_app_cb(None)
        mock_ide_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_app")
    def test_fox_gui_open_vs_code_app_unit_test(
        self, mock_open_ide_embedded_unit_test_app: MagicMock
    ):
        """Test 'open_vs_code_app_unit_test_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_app_unit_test_cb(None)
        mock_open_ide_embedded_unit_test_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_bootloader")
    def test_fox_gui_open_vs_code_bootloader(self, mock_ide_bootloader: MagicMock):
        """Test 'open_vs_code_bootloader_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_bootloader_cb(None)
        mock_ide_bootloader.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_bootloader")
    def test_fox_gui_open_vs_code_bootloader_unit_test(
        self, open_ide_embedded_unit_test_bootloader: MagicMock
    ):
        """Test 'open_vs_code_bootloader_unit_test_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_bootloader_unit_test_cb(None)
        open_ide_embedded_unit_test_bootloader.assert_called_once()

    @patch("tkinter.filedialog.asksaveasfile")
    def test_fox_gui_new_file(self, mock_asksaveasfile: MagicMock):
        """Test 'create_new_file_cb' function of FoxGui class"""
        gui_impl.FoxGui.create_new_file_cb(None)
        mock_asksaveasfile.assert_called_once()

    @patch("webbrowser.open")
    def test_fox_gui_view_license(self, mock_open: MagicMock):
        """Test 'view_licence_cb' function of FoxGui class"""
        gui_impl.FoxGui.view_license_cb(MagicMock())
        mock_open.assert_called_once()

    def test_fox_gui_tab_changed(self):
        """Test 'tab_changed_cb' function of FoxGui class"""
        mock_tab = MagicMock()
        mock_tab.text_index = 10
        mock_notebook = MagicMock()
        mock_notebook.nametowidget.return_value = mock_tab
        mock_fox_gui = MagicMock()
        mock_fox_gui.notebook = mock_notebook
        gui_impl.FoxGui.tab_changed_cb(mock_fox_gui, "<<NotebookTabChanged>>")
        self.assertEqual(mock_tab.text_index, 0)
        mock_tab.write_text.assert_called_once()
        mock_fox_gui.text.config.assert_has_calls(
            [call(state="normal"), call(state="disabled")]
        )
        mock_fox_gui.text.delete.assert_called_once()


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestFoxGuiCloseWindow(unittest.TestCase):
    """Test of the 'close_window' function"""

    @classmethod
    def setUpClass(cls):
        importlib.reload(gui_impl)

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)
        self.app = gui_impl.main()
        self.app.withdraw()
        self.app.notebook.unbind("<<NotebookTabChanged>>")

    def tearDown(self):
        self.app.update()
        self.app.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.gui_impl.FoxGui.destroy")
    def test_close_unlink(self, mock_destroy: MagicMock):
        """Test 'close_window' function when file_stream and file exist"""
        tabs = self.app.notebook.tabs()
        self.app.notebook.nametowidget(tabs[0]).file_stream = open(  # pylint: disable=consider-using-with
            self.app.notebook.nametowidget(tabs[0]).file_path, encoding="utf-8"
        )
        self.app.close_window()
        for child in PATH_GUI.iterdir():
            if child.exists():
                self.assertFalse("output_gui" in child.name)
        self.assertTrue(self.app.notebook.nametowidget(tabs[0]).file_stream.closed)
        mock_destroy.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.FoxGui.destroy")
    def test_no_file(self, mock_destroy: MagicMock):
        """Test 'close_window' function when only file_stream exists"""
        tabs = self.app.notebook.tabs()
        self.app.notebook.nametowidget(tabs[0]).file_stream = open(  # pylint: disable=consider-using-with
            self.app.notebook.nametowidget(tabs[0]).file_path, encoding="utf-8"
        )
        with (
            patch("pathlib.Path.exists") as mock_exists,
            patch("pathlib.Path.unlink") as mock_unlink,
        ):
            mock_exists.return_value = False
            self.app.close_window()
            mock_exists.assert_has_calls([call(), call(), call(), call(), call()])
            mock_unlink.assert_not_called()
        for tab_name in self.app.notebook.tabs():
            tab_obj = self.app.notebook.nametowidget(tab_name)
            if hasattr(tab_obj, "file_stream"):
                self.assertTrue(tab_obj.file_stream.closed)
        mock_destroy.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.FoxGui.destroy")
    def test_no_stream(self, mock_destroy: MagicMock):
        """Test 'close_window' function when only file exists"""
        self.app.close_window()
        for tab_name in self.app.notebook.tabs():
            tab_obj = self.app.notebook.nametowidget(tab_name)
            if hasattr(tab_obj, "file_stream"):
                self.assertTrue(tab_obj.file_stream.closed)
        for child in PATH_GUI.iterdir():
            if child.exists():
                self.assertFalse("output_gui" in child.name)
        mock_destroy.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.FoxGui.destroy")
    @patch("cli.cmd_gui.gui_impl.SimulateBmsFrame.start_stop_sim_cb")
    def test_sim_active(self, mock_cb: MagicMock, mock_destroy: MagicMock):
        """Test 'close_window' function when sim_active is True"""
        for tab_name in self.app.notebook.tabs():
            tab_obj = self.app.notebook.nametowidget(tab_name)
            if "simulate" in tab_name:
                tab_obj.sim_active = True
        self.app.close_window()
        mock_cb.assert_called_once()
        mock_destroy.assert_called_once()


class TestFoxGuiNoUiCloseWindow(unittest.TestCase):
    """Test of the 'close_window' function without UI"""

    def test_close_unlink(self):
        """Test 'close_window' function when file_stream and file exist"""
        mock_file_stream = MagicMock()
        mock_file_path = MagicMock()
        mock_file_path.exists.return_value = True
        mock_tab = MagicMock()
        mock_tab.file_stream = mock_file_stream
        mock_tab.file_path = mock_file_path
        mock_fox_gui = MagicMock()
        mock_notebook = MagicMock()
        mock_fox_gui.notebook = mock_notebook
        mock_notebook.tabs.return_value = ["tab_1", "tab_2"]
        mock_notebook.nametowidget.return_value = mock_tab
        gui_impl.FoxGui.close_window(mock_fox_gui)
        mock_file_stream.close.assert_has_calls([call(), call()])
        mock_file_path.exists.assert_has_calls([call(), call()])
        mock_file_path.unlink.assert_has_calls([call(), call()])

    def test_no_file(self):
        """Test 'close_window' function when only file_stream exists"""
        mock_file_stream = MagicMock()
        mock_file_path = MagicMock()
        mock_file_path.exists.return_value = False
        mock_tab = MagicMock()
        mock_tab.file_stream = mock_file_stream
        mock_tab.file_path = mock_file_path
        mock_fox_gui = MagicMock()
        mock_notebook = MagicMock()
        mock_fox_gui.notebook = mock_notebook
        mock_notebook.tabs.return_value = ["tab_1", "tab_2"]
        mock_notebook.nametowidget.return_value = mock_tab
        gui_impl.FoxGui.close_window(mock_fox_gui)
        mock_file_stream.close.assert_has_calls([call(), call()])
        mock_file_path.exists.assert_has_calls([call(), call()])
        mock_file_path.unlink.assert_not_called()

    def test_no_stream(self):
        """Test 'close_window' function when only file exists"""
        mock_file_path = MagicMock()
        mock_file_path.exists.return_value = True
        mock_tab = MagicMock()
        mock_tab.file_path = mock_file_path
        if hasattr(mock_tab, "file_stream"):
            delattr(mock_tab, "file_stream")
        mock_fox_gui = MagicMock()
        mock_notebook = MagicMock()
        mock_fox_gui.notebook = mock_notebook
        mock_notebook.tabs.return_value = ["tab_1", "tab_2"]
        mock_notebook.nametowidget.return_value = mock_tab
        gui_impl.FoxGui.close_window(mock_fox_gui)
        mock_file_path.exists.assert_has_calls([call(), call()])
        mock_file_path.unlink.assert_has_calls([call(), call()])

    def test_sim_active(self):
        """Test 'close_window' function when sim_active is True"""
        mock_file_path = MagicMock()
        mock_file_path.exists.return_value = False
        mock_cb = MagicMock()
        mock_tab = MagicMock()
        mock_tab.file_path = mock_file_path
        mock_tab.sim_active = True
        mock_tab.start_stop_sim_cb = mock_cb
        mock_fox_gui = MagicMock()
        mock_notebook = MagicMock()
        mock_fox_gui.notebook = mock_notebook
        mock_notebook.tabs.return_value = ["simulate_tab", "tab_2"]
        mock_notebook.nametowidget.return_value = mock_tab
        gui_impl.FoxGui.close_window(mock_fox_gui)
        mock_file_path.exists.assert_has_calls([call(), call()])
        mock_file_path.unlink.assert_not_called()
        mock_cb.assert_called_once()


@unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
@patch("sys.platform", new="win32")
class TestFoxGuiWin32(unittest.TestCase):
    """Test of the FoxGui class on Windows"""

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        remove_data(self.start_time)

    def test_project(self):
        """Test when ROOT_IS_PROJECT is True"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=True):
            importlib.reload(gui_impl)
            app = gui_impl.main()
            app.withdraw()
            app.notebook.unbind("<<NotebookTabChanged>>")
            self.assertEqual(app.license_file, PROJECT_ROOT / "LICENSE.md")
            app.destroy()

    @patch("tkinter.Wm.iconbitmap")
    def test_no_project(self, mock_iconbitmap: MagicMock):
        """Test when ROOT_IS_PROJECT is False"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=False):
            with patch("cli.helpers.misc.get_file_path") as mock_path:
                dir_path = PROJECT_ROOT / "project_data"
                mock_path.return_value = dir_path
                importlib.reload(gui_impl)
                app = gui_impl.main()
                app.withdraw()
                app.notebook.unbind("<<NotebookTabChanged>>")
                self.assertEqual(app.license_file, dir_path / "LICENSE.md")
                mock_iconbitmap.assert_called_once_with(
                    True, str(dir_path / "favicon.ico")
                )
                app.destroy()

    def test_no_project_check_ide_patch(self):
        """Test when ROOT_IS_PROJECT is False"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=False):
            importlib.reload(gui_impl)
            self.assertEqual(gui_impl.dummy(), 0)


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("sys.platform", new="linux")
class TestFoxGuiLinux(unittest.TestCase):
    """Test of the FoxGui class on linux"""

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        remove_data(self.start_time)

    def test_project(self):
        """Test when ROOT_IS_PROJECT is True"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=True):
            importlib.reload(gui_impl)
            app = gui_impl.main()
            app.withdraw()
            app.notebook.unbind("<<NotebookTabChanged>>")
            self.assertEqual(app.license_file, PROJECT_ROOT / "LICENSE.md")
            app.destroy()

    @patch("tkinter.Wm.iconbitmap")
    def test_no_project(self, mock_iconbitmap: MagicMock):
        """Test when ROOT_IS_PROJECT is False"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=False):
            with patch("cli.helpers.misc.get_file_path") as mock_path:
                dir_path = PROJECT_ROOT / "project_data"
                mock_path.return_value = dir_path
                importlib.reload(gui_impl)
                app = gui_impl.main()
                app.withdraw()
                app.notebook.unbind("<<NotebookTabChanged>>")
                self.assertEqual(app.license_file, dir_path / "LICENSE.md")
                mock_iconbitmap.assert_called_once_with(
                    True, str(dir_path / "favicon.ico")
                )
                app.destroy()


class TestGuiImpl(unittest.TestCase):
    """Test of the gui implementation."""

    @classmethod
    def setUpClass(cls):
        importlib.reload(gui_impl)

    @unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
    @patch("cli.cmd_gui.gui_impl.FoxGui")
    def test_run_gui(self, mock_fox_gui: MagicMock):
        """Test of the 'run_gui' function."""
        gui_impl.run_gui()
        mock_fox_gui.assert_called_once()

    def test_gui_attributes(self):
        """Test class GuiAttributes"""
        attributes = gui_impl.GuiAttributes("test", (100, 200))
        self.assertEqual(attributes.sx, 100)
        self.assertEqual(attributes.sy, 200)
        self.assertEqual(attributes.bg, "test")


def remove_data(start_time: datetime) -> None:
    """Remove all data from the gui directory if it as been created after start_time"""
    if PATH_GUI.is_dir():
        if get_birthtime(PATH_GUI) >= start_time:
            shutil.rmtree(PATH_GUI)
        else:
            children = PATH_GUI.iterdir()
            for child in children:
                if datetime.fromtimestamp(child.stat().st_mtime, tz=UTC) >= start_time:
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
