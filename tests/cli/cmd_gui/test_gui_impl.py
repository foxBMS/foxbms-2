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

import contextlib
import gc
import importlib
import os
import shutil
import sys
import threading
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui import gui_impl
    from cli.helpers.project_context import PROJECT_BUILD_ROOT, PROJECT_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_gui import gui_impl
    from cli.helpers.project_context import PROJECT_BUILD_ROOT, PROJECT_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "gui"


def destroy_tk_app(app: gui_impl.FoxGui | None) -> None:
    """Tear down a Tk root deterministically, from the main thread."""
    if app is None:
        return
    # assert threading.current_thread() is threading.main_thread()

    # 1. let every tab stop its worker threads (this is what tearDown was missing)
    try:
        for tab_name in app.notebook.tabs():
            with contextlib.suppress(tk.TclError, AttributeError):
                app.notebook.nametowidget(tab_name).on_close()
    except (tk.TclError, AttributeError):
        pass

    # 2. make sure no worker thread is left that could run a Tcl call in __del__
    for thread in threading.enumerate():
        if thread is not threading.main_thread():
            thread.join(timeout=10)
            if thread.is_alive():
                msg = f"thread still running: {thread!r}"
                raise AssertionError(msg)

    # 3. run pending finalizers while the Tcl interpreter is STILL alive
    try:
        app.update_idletasks()
        gc.collect()
        app.destroy()
    except (tk.TclError, RuntimeError):
        pass
    finally:
        del app
        gc.collect()  # collect the widget/Variable cycles now, not at shutdown


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestFoxGui(unittest.TestCase):
    """Test of the 'open_vs_code_*' functions of the FoxGui class"""

    app = None

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        cls.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)
        with patch("cli.cmd_gui.gui_impl.FoxGui.update"):
            cls.app = gui_impl.main()
        cls.app.withdraw()
        cls.app.notebook.unbind("<<NotebookTabChanged>>")

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        app, cls.app = cls.app, None
        destroy_tk_app(app)
        remove_data(cls.start_time)

    def setUp(self) -> None:  # noqa: D102
        self.addCleanup(self._reset_about_state)

    def _reset_about_state(self) -> None:
        """Release About dialog references created by a single test"""
        # pylint: disable=protected-access
        self.app._about_dialog = None
        self.app._about_image = None
        self.app._about_image_label = None

    @patch("tkinter.filedialog.asksaveasfile")
    def test_create_new_file(self, mock_asksaveasfile: MagicMock) -> None:
        """Test 'create_new_file_cb' function of FoxGui class"""
        self.app.create_new_file_cb()
        mock_asksaveasfile.assert_called_once()

    @patch("webbrowser.open")
    def test_view_license(self, mock_open: MagicMock) -> None:
        """Test 'view_licence_cb' function of FoxGui class"""
        self.app.view_license_cb()
        mock_open.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.tk.Toplevel")
    @patch("cli.cmd_gui.gui_impl.tk.PhotoImage")
    def test_show_about_project(
        self, mock_photoimage: MagicMock, mock_toplevel: MagicMock
    ) -> None:
        """Test 'show_about_cb' function of FoxGui class when ROOT_IS_PROJECT is True"""
        with patch("cli.cmd_gui.gui_impl.ROOT_IS_PROJECT", new=True):
            self.app.show_about_cb()
        mock_toplevel.assert_called_once()
        mock_photoimage.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.tk.Toplevel")
    @patch("cli.cmd_gui.gui_impl.tk.PhotoImage")
    def test_show_about_no_project(
        self, mock_photoimage: MagicMock, mock_toplevel: MagicMock
    ) -> None:
        """Test 'show_about_cb' function of FoxGui class when ROOT_IS_PROJECT is False"""
        with patch("cli.cmd_gui.gui_impl.ROOT_IS_PROJECT", new=False):
            self.app.show_about_cb()
        mock_toplevel.assert_called_once()
        mock_photoimage.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.tk.Toplevel")
    def test_show_about_reuses_existing_dialog(self, mock_toplevel: MagicMock) -> None:
        """Test that 'show_about_cb' reuses an already open About dialog."""
        mock_dialog = MagicMock()
        mock_dialog.winfo_exists.return_value = True
        self.app._about_dialog = mock_dialog  # pylint: disable=protected-access

        self.app.show_about_cb()

        mock_toplevel.assert_not_called()
        mock_dialog.lift.assert_called_once()
        mock_dialog.focus_force.assert_called_once()

    def test_tab_changed(self) -> None:
        """Test 'tab_changed_cb' function of FoxGui class"""
        mock_tab = MagicMock()
        with patch.object(
            self.app.notebook, "nametowidget", return_value=mock_tab
        ) as mock_nametowidget:
            self.app.tab_changed_cb("<<NotebookTabChanged>>")
        mock_nametowidget.assert_called_once()
        mock_tab.reset_text.assert_called_once()
        mock_tab.write_text.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.FoxGui.close_tabs")
    @patch("cli.cmd_gui.gui_impl.FoxGui.destroy")
    def test_close_window(
        self, mock_destroy: MagicMock, mock_close_tabs: MagicMock
    ) -> None:
        """Test 'close_window' function of FoxGui class"""
        self.app.close_window()
        mock_close_tabs.assert_called_once()
        mock_destroy.assert_called_once()


class TestFoxGuiNoUiTestableMethods(unittest.TestCase):
    """Test of the FoxGui class without creating any tkinter objects"""

    @patch("tkinter.filedialog.asksaveasfile")
    def test_create_new_file(self, mock_asksaveasfile: MagicMock) -> None:
        """Test 'create_new_file_cb' function of FoxGui class"""
        gui_impl.FoxGui.create_new_file_cb(None)
        mock_asksaveasfile.assert_called_once()

    @patch("webbrowser.open")
    def test_view_license(self, mock_open: MagicMock) -> None:
        """Test 'view_licence_cb' function of FoxGui class"""
        gui_impl.FoxGui.view_license_cb(MagicMock())
        mock_open.assert_called_once()

    def test_tab_changed(self) -> None:
        """Test 'tab_changed_cb' function of FoxGui class"""
        mock_tab = MagicMock()
        mock_nametowidget = MagicMock()
        mock_nametowidget.return_value = mock_tab
        mock_fox_gui = MagicMock()
        mock_fox_gui.notebook.nametowidget = mock_nametowidget
        gui_impl.FoxGui.tab_changed_cb(mock_fox_gui, "<<NotebookTabChanged>>")
        mock_nametowidget.assert_called_once()
        mock_tab.reset_text.assert_called_once()
        mock_tab.write_text.assert_called_once()

    def test_close_window(self) -> None:
        """Test 'close_window' function of FoxGui class"""
        mock_tab = MagicMock()
        mock_fox_gui = MagicMock()
        mock_notebook = MagicMock()
        mock_fox_gui.notebook = mock_notebook
        mock_notebook.tabs.return_value = ["tab_1", "tab_2"]
        mock_notebook.nametowidget.return_value = mock_tab
        gui_impl.FoxGui.close_window(mock_fox_gui)
        mock_fox_gui.destroy.assert_called_once()

    def test_close_tabs(self) -> None:
        """Test 'close_tabs' stops all tabs and survives errors of a single tab"""
        mock_tab = MagicMock()
        mock_broken_tab = MagicMock()
        mock_broken_tab.on_close.side_effect = gui_impl.tk.TclError("tab is gone")
        mock_fox_gui = MagicMock()
        mock_fox_gui.notebook.tabs.return_value = ["tab_1", "tab_2"]
        mock_fox_gui.notebook.nametowidget.side_effect = [mock_broken_tab, mock_tab]

        gui_impl.FoxGui.close_tabs(mock_fox_gui)

        mock_broken_tab.on_close.assert_called_once()
        mock_tab.on_close.assert_called_once()

    def test_close_tabs_without_notebook(self) -> None:
        """Test 'close_tabs' returns silently if the notebook no longer exists"""
        mock_fox_gui = MagicMock()
        mock_fox_gui.notebook.tabs.side_effect = gui_impl.tk.TclError("no notebook")

        gui_impl.FoxGui.close_tabs(mock_fox_gui)

        mock_fox_gui.notebook.nametowidget.assert_not_called()

    def test_close_about_dialog_ignores_tclerror_on_image_delete(self) -> None:
        """Test '_close_about_dialog' keeps cleaning up when image delete raises TclError."""
        mock_label = MagicMock()
        mock_dialog = MagicMock()
        mock_dialog.winfo_exists.return_value = True

        mock_fox_gui = MagicMock()
        mock_fox_gui._about_image_label = mock_label  # pylint: disable=protected-access
        mock_fox_gui._about_image = "img-id"  # pylint: disable=protected-access
        mock_fox_gui._about_dialog = mock_dialog  # pylint: disable=protected-access
        mock_fox_gui.tk.call.side_effect = gui_impl.tk.TclError("image delete failed")

        gui_impl.FoxGui._close_about_dialog(mock_fox_gui)  # pylint: disable=protected-access

        mock_label.configure.assert_called_once_with(image="")
        mock_dialog.destroy.assert_called_once()
        self.assertIsNone(mock_fox_gui._about_image_label)  # pylint: disable=protected-access
        self.assertIsNone(mock_fox_gui._about_image)  # pylint: disable=protected-access
        self.assertIsNone(mock_fox_gui._about_dialog)  # pylint: disable=protected-access

    def test_close_about_dialog_ignores_tclerror_on_dialog_query(self) -> None:
        """Test '_close_about_dialog' handles TclError while querying dialog state."""
        mock_dialog = MagicMock()
        mock_dialog.winfo_exists.side_effect = gui_impl.tk.TclError("dialog invalid")

        mock_fox_gui = MagicMock()
        mock_fox_gui._about_image_label = None  # pylint: disable=protected-access
        mock_fox_gui._about_image = None  # pylint: disable=protected-access
        mock_fox_gui._about_dialog = mock_dialog  # pylint: disable=protected-access

        gui_impl.FoxGui._close_about_dialog(mock_fox_gui)  # pylint: disable=protected-access

        self.assertIsNone(mock_fox_gui._about_dialog)  # pylint: disable=protected-access

    def test_close_about_dialog_skips_destroy_for_non_existing_dialog(self) -> None:
        """Test '_close_about_dialog' does not destroy dialog if it no longer exists."""
        mock_dialog = MagicMock()
        mock_dialog.winfo_exists.return_value = False

        mock_fox_gui = MagicMock()
        mock_fox_gui._about_image_label = None  # pylint: disable=protected-access
        mock_fox_gui._about_image = None  # pylint: disable=protected-access
        mock_fox_gui._about_dialog = mock_dialog  # pylint: disable=protected-access

        gui_impl.FoxGui._close_about_dialog(mock_fox_gui)  # pylint: disable=protected-access

        mock_dialog.destroy.assert_not_called()
        self.assertIsNone(mock_fox_gui._about_dialog)  # pylint: disable=protected-access

    @patch("cli.cmd_gui.gui_impl.tk.PhotoImage")
    @patch("cli.cmd_gui.gui_impl.tk.Label")
    @patch("cli.cmd_gui.gui_impl.tk.Toplevel")
    def test_show_about_recreates_dialog_after_tclerror(
        self,
        mock_toplevel: MagicMock,
        _mock_label: MagicMock,
        _mock_photoimage: MagicMock,
    ) -> None:
        """Test 'show_about_cb' recreates dialog when existing dialog raises TclError."""
        mock_existing_dialog = MagicMock()
        mock_existing_dialog.winfo_exists.side_effect = gui_impl.tk.TclError(
            "stale dialog"
        )

        mock_fox_gui = MagicMock()
        mock_fox_gui._about_dialog = mock_existing_dialog  # pylint: disable=protected-access
        mock_fox_gui.cattrs = gui_impl.GuiAttributes("darkgrey", (768, 576))
        mock_fox_gui.license_file = Path("LICENSE.md")
        mock_fox_gui.view_license_cb = MagicMock()
        mock_fox_gui._close_about_dialog = MagicMock()  # pylint: disable=protected-access

        with patch("cli.cmd_gui.gui_impl.ROOT_IS_PROJECT", new=False):
            gui_impl.FoxGui.show_about_cb(mock_fox_gui)

        mock_toplevel.assert_called_once()
        # pylint: disable-next=protected-access
        self.assertIs(mock_fox_gui._about_dialog, mock_toplevel.return_value)

    @patch("cli.cmd_gui.gui_impl.tk.PhotoImage")
    @patch("cli.cmd_gui.gui_impl.tk.Label")
    @patch("cli.cmd_gui.gui_impl.tk.Toplevel")
    def test_show_about_recreates_dialog_after_non_existing_dialog(
        self,
        mock_toplevel: MagicMock,
        _mock_label: MagicMock,
        _mock_photoimage: MagicMock,
    ) -> None:
        """Test 'show_about_cb' creates a new dialog when current one does not exist."""
        mock_existing_dialog = MagicMock()
        mock_existing_dialog.winfo_exists.return_value = False

        mock_fox_gui = MagicMock()
        mock_fox_gui._about_dialog = mock_existing_dialog  # pylint: disable=protected-access
        mock_fox_gui.cattrs = gui_impl.GuiAttributes("darkgrey", (768, 576))
        mock_fox_gui.license_file = Path("LICENSE.md")
        mock_fox_gui.view_license_cb = MagicMock()
        mock_fox_gui._close_about_dialog = MagicMock()  # pylint: disable=protected-access

        with patch("cli.cmd_gui.gui_impl.ROOT_IS_PROJECT", new=False):
            gui_impl.FoxGui.show_about_cb(mock_fox_gui)

        mock_existing_dialog.lift.assert_not_called()
        mock_existing_dialog.focus_force.assert_not_called()
        mock_toplevel.assert_called_once()
        # pylint: disable-next=protected-access
        self.assertIs(mock_fox_gui._about_dialog, mock_toplevel.return_value)


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestFoxGuiVsCode(unittest.TestCase):
    """Test of the 'open_vs_code_*' functions of the FoxGui class"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)
        with patch("cli.cmd_gui.gui_impl.FoxGui.update"):
            self.app = gui_impl.main()
        self.addCleanup(remove_data, self.start_time)
        self.addCleanup(self._cleanup_app)
        self.app.withdraw()
        self.app.notebook.unbind("<<NotebookTabChanged>>")

    def _cleanup_app(self) -> None:
        app, self.app = self.app, None  # drop the TestCase reference first
        destroy_tk_app(app)

    @patch("cli.cmd_gui.gui_impl.open_ide_generic")
    def test_open_vs_code_generic(self, mock_ide_generic: MagicMock) -> None:
        """Test 'open_vs_code_generic_cb' function of FoxGui class"""
        self.app.open_vs_code_generic_cb()
        mock_ide_generic.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_cli")
    def test_open_vs_code_cli(self, mock_ide_cli: MagicMock) -> None:
        """Test 'open_vs_code_cli_cb' function of FoxGui class"""
        self.app.open_vs_code_cli_cb()
        mock_ide_cli.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_app")
    def test_open_vs_code_app(self, mock_ide_app: MagicMock) -> None:
        """Test 'open_vs_code_app_cb' function of FoxGui class"""
        self.app.open_vs_code_app_cb()
        mock_ide_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_app")
    def test_open_vs_code_app_unit_test(self, mock_ide_app: MagicMock) -> None:
        """Test 'open_vs_code_app_unit_test_cb' function of FoxGui class"""
        self.app.open_vs_code_app_unit_test_cb()
        mock_ide_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_bootloader")
    def test_open_vs_code_bootloader(self, mock_ide_bootloader: MagicMock) -> None:
        """Test 'open_vs_code_bootloader_cb' function of FoxGui class"""
        self.app.open_vs_code_bootloader_cb()
        mock_ide_bootloader.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_bootloader")
    def test_open_vs_code_bootloader_unit_test(
        self, mock_ide_bootloader: MagicMock
    ) -> None:
        """Test 'open_vs_code_bootloader_unit_test_cb' function of FoxGui class"""
        self.app.open_vs_code_bootloader_unit_test_cb()
        mock_ide_bootloader.assert_called_once()


class TestFoxGuiVsCodeNoUiTestableMethods(unittest.TestCase):
    """Test of the 'open_vs_code_*' functions of the FoxGui class
    without creating any tkinter objects
    """

    @patch("cli.cmd_gui.gui_impl.open_ide_generic")
    def test_open_vs_code_generic(self, mock_ide_generic: MagicMock) -> None:
        """Test 'open_vs_code_generic_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_generic_cb(None)
        mock_ide_generic.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_app")
    def test_open_vs_code_app(self, mock_ide_app: MagicMock) -> None:
        """Test 'open_vs_code_app_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_app_cb(None)
        mock_ide_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_app")
    def test_open_vs_code_app_unit_test(
        self, mock_open_ide_embedded_unit_test_app: MagicMock
    ) -> None:
        """Test 'open_vs_code_app_unit_test_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_app_unit_test_cb(None)
        mock_open_ide_embedded_unit_test_app.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_bootloader")
    def test_open_vs_code_bootloader(self, mock_ide_bootloader: MagicMock) -> None:
        """Test 'open_vs_code_bootloader_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_bootloader_cb(None)
        mock_ide_bootloader.assert_called_once()

    @patch("cli.cmd_gui.gui_impl.open_ide_embedded_unit_test_bootloader")
    def test_open_vs_code_bootloader_unit_test(
        self, open_ide_embedded_unit_test_bootloader: MagicMock
    ) -> None:
        """Test 'open_vs_code_bootloader_unit_test_cb' function of FoxGui class"""
        gui_impl.FoxGui.open_vs_code_bootloader_unit_test_cb(None)
        open_ide_embedded_unit_test_bootloader.assert_called_once()


@unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
@patch("sys.platform", new="win32")
class TestFoxGuiWin32(unittest.TestCase):
    """Test of the FoxGui class on Windows"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)
        # cleanups run LIFO: 'destroy_tk_app' first, 'remove_data' next, reload last
        self.addCleanup(self._reload_gui_impl)
        self.addCleanup(remove_data, self.start_time)

    @staticmethod
    def _reload_gui_impl() -> None:
        """Drop stale Tk objects before reloading the module under test"""
        gc.collect()
        importlib.reload(gui_impl)

    def tearDown(self) -> None:  # noqa: D102
        remove_data(self.start_time)
        importlib.reload(gui_impl)

    def test_project(self) -> None:
        """Test when ROOT_IS_PROJECT is True"""
        with patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=True):
            importlib.reload(gui_impl)
            with patch("cli.cmd_gui.gui_impl.FoxGui.update"):
                app = gui_impl.main()
            self.addCleanup(destroy_tk_app, app)
            app.withdraw()
            app.notebook.unbind("<<NotebookTabChanged>>")
            self.assertEqual(app.license_file, PROJECT_ROOT / "LICENSE.md")

    @patch("tkinter.Wm.iconbitmap")
    def test_no_project(self, mock_iconbitmap: MagicMock) -> None:
        """Test when ROOT_IS_PROJECT is False"""
        with (
            patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=False),
            patch("cli.helpers.project_context.get_file_path") as mock_path,
        ):
            dir_path = PROJECT_ROOT / "project_data"
            mock_path.return_value = dir_path
            importlib.reload(gui_impl)
            with patch("cli.cmd_gui.gui_impl.FoxGui.update"):
                app = gui_impl.main()
            self.addCleanup(destroy_tk_app, app)
            app.withdraw()
            app.notebook.unbind("<<NotebookTabChanged>>")
            self.assertEqual(app.license_file, dir_path / "LICENSE.md")
            mock_iconbitmap.assert_called_once_with(
                True, str(dir_path / "fav_icon.ico")
            )

    def test_no_project_check_ide_patch(self) -> None:
        """Test 'dummy' function when ROOT_IS_PROJECT is False"""
        with patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=False):
            importlib.reload(gui_impl)
            self.assertEqual(gui_impl.dummy(), 0)


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("sys.platform", new="linux")
class TestFoxGuiLinux(unittest.TestCase):
    """Test of the FoxGui class on linux"""

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)
        # cleanups run LIFO: 'destroy_tk_app' first, 'remove_data' next, reload last
        self.addCleanup(self._reload_gui_impl)
        self.addCleanup(remove_data, self.start_time)

    @staticmethod
    def _reload_gui_impl() -> None:
        """Drop stale Tk objects before reloading the module under test"""
        gc.collect()
        importlib.reload(gui_impl)

    def test_project(self) -> None:
        """Test when ROOT_IS_PROJECT is True"""
        with patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=True):
            importlib.reload(gui_impl)
            with patch("cli.cmd_gui.gui_impl.FoxGui.update"):
                app = gui_impl.main()
            self.addCleanup(destroy_tk_app, app)
            app.withdraw()
            app.notebook.unbind("<<NotebookTabChanged>>")
            self.assertEqual(app.license_file, PROJECT_ROOT / "LICENSE.md")

    @patch("tkinter.Wm.iconbitmap")
    def test_no_project(self, mock_iconbitmap: MagicMock) -> None:
        """Test when ROOT_IS_PROJECT is False"""
        with (
            patch("cli.helpers.project_context.ROOT_IS_PROJECT", new=False),
            patch("cli.helpers.project_context.get_file_path") as mock_path,
        ):
            dir_path = PROJECT_ROOT / "project_data"
            mock_path.return_value = dir_path
            importlib.reload(gui_impl)
            with patch("cli.cmd_gui.gui_impl.FoxGui.update"):
                app = gui_impl.main()
            self.addCleanup(destroy_tk_app, app)
            app.withdraw()
            app.notebook.unbind("<<NotebookTabChanged>>")
            self.assertEqual(app.license_file, dir_path / "LICENSE.md")
            mock_iconbitmap.assert_called_once_with(
                True, str(dir_path / "fav_icon.ico")
            )


class TestGuiImpl(unittest.TestCase):
    """Test of the Gui implementation"""

    @unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
    @patch("cli.cmd_gui.gui_impl.FoxGui")
    def test_run_gui(self, mock_fox_gui: MagicMock) -> None:
        """Test of the 'run_gui' function."""
        gui_impl.run_gui()
        mock_fox_gui.assert_called_once()

    def test_gui_attributes(self) -> None:
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


def tearDownModule() -> None:
    """Report Tk worker threads that survived the tests"""
    gc.collect()
    leftover = [
        thread
        for thread in threading.enumerate()
        if thread is not threading.main_thread()
    ]
    if leftover:
        msg = f"WARNING: GUI tests left threads running: {leftover}"
        print(msg, file=sys.stderr)  # noqa: T201


if __name__ == "__main__":
    unittest.main()
