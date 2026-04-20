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

# cspell:ignore Targetpath

"""Testing file 'cli/cmd_install/install_impl.py'."""

import importlib
import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_install import install_impl
    from cli.cmd_install.install_impl import (
        _create_shortcut_win32,
        all_software_available,
        check_for_all_softwares,
        install_fox_cli_tools_on_host,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_install import install_impl
    from cli.cmd_install.install_impl import (
        _create_shortcut_win32,
        all_software_available,
        check_for_all_softwares,
        install_fox_cli_tools_on_host,
    )


class TestAllSoftwareAvailable(unittest.TestCase):
    """Test of the function 'all_software_available'"""

    def tearDown(self):
        importlib.reload(install_impl)

    @patch(
        "cli.cmd_install.install_impl.REQUIRED_SOFTWARE",
        {"abc": {"executable": "abc", "path": False}},
    )
    def test_all_software_not_available(self):
        """Test function with some unavailable software"""
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = all_software_available()
        self.assertEqual(result, 1)
        self.assertIn("abc: {'executable': 'abc', 'path': False}", buf.getvalue())

    @patch(
        "cli.cmd_install.install_impl.REQUIRED_SOFTWARE",
        {"git": {"executable": "git", "path": False}},
    )
    @patch("shutil.which")
    def test_all_software_available(self, mock_which: MagicMock):
        """Test function with only Python as required software"""
        mock_which.return_value = "git"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = all_software_available()
        self.assertEqual(result, 0)
        self.assertIn("All required software is installed.", buf.getvalue())

    @unittest.skipIf(not sys.platform.startswith("win32"), "Windows specific test")
    @patch(
        "cli.cmd_install.install_impl.REQUIRED_SOFTWARE",
        {
            "does-not-exist": {
                "executable": "does-not-exist",
                "path": False,
                "availability": ["linux"],
            }
        },
    )
    @patch("shutil.which")
    def test_all_software_available_1(self, mock_which: MagicMock):
        """Test function with only Python as required software"""
        mock_which.return_value = None
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = all_software_available()
        self.assertEqual(result, 0)
        self.assertEqual(
            err.getvalue(),
            "does-not-exist (does-not-exist) is not available on win32.\n",
        )
        self.assertEqual("All required software is installed.\n", out.getvalue())

    @unittest.skipIf(not sys.platform.startswith("linux"), "Linux specific test")
    @patch(
        "cli.cmd_install.install_impl.REQUIRED_SOFTWARE",
        {
            "does-not-exist": {
                "executable": "does-not-exist",
                "path": False,
                "availability": ["win32"],
            }
        },
    )
    @patch("shutil.which")
    def test_all_software_available_2(self, mock_which: MagicMock):
        """Test function with only Python as required software"""
        mock_which.return_value = None
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = all_software_available()
        self.assertEqual(result, 0)
        self.assertEqual(
            err.getvalue(),
            "does-not-exist (does-not-exist) is not available on linux.\n",
        )
        self.assertEqual("All required software is installed.\n", out.getvalue())

    @patch("cli.cmd_install.install_impl.deepcopy")
    def test_check_for_all_softwares(self, mock_deepcopy: MagicMock):
        """Test invalid expected required software definition"""
        mock_deepcopy.return_value = {
            "drawio": {
                "executable": {
                    "win32": 1,
                    "linux": 1,
                },
                "path": False,
            },
        }
        with self.assertRaises(SystemExit) as cm:
            check_for_all_softwares()
        self.assertEqual((cm.exception.code).startswith("Invalid path file "), True)


@unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
class TestCreateShortcutWin32(unittest.TestCase):
    """Test of the function '_create_shortcut_win32'"""

    def tearDown(self):
        importlib.reload(install_impl)

    @unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
    @patch("cli.cmd_install.install_impl.Dispatch")
    def test_failure(self, mock_dispatch: MagicMock):
        """Test creating of shortcut failed"""
        # pylint: disable-next=import-outside-toplevel
        from win32com.universal import com_error  # noqa: PLC0415

        mock_dispatch.side_effect = com_error("Failure")
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = _create_shortcut_win32(Path(), "", [], "")
        self.assertEqual(result, 1)
        self.assertEqual("Failed to create shortcut:\nFailure\n", err.getvalue())
        self.assertEqual("", out.getvalue())

    @unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
    def test_create_shortcut_win32_win32com_not_available(self):
        install_impl.WIN32COM_AVAILABLE = False
        err, out = io.StringIO(), io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # arguments do not matter as the test checks for early exit due to
            # missing package
            result = _create_shortcut_win32(Path(), "", [], "")
        self.assertEqual(result, 1)

    @patch("cli.cmd_install.install_impl.get_numeric_version")
    @patch("cli.cmd_install.install_impl.Path.is_file")
    @patch("cli.cmd_install.install_impl.Dispatch")
    def test_file_exists(
        self, mock_dispatch: MagicMock, mock_is_file: MagicMock, mock_version: MagicMock
    ):
        """Test shortcut already exists"""
        mock_shortcut = MagicMock()
        mock_shell = MagicMock()
        mock_shell.CreateShortCut.return_value = mock_shortcut
        mock_dispatch.return_value = mock_shell
        mock_is_file.return_value = True
        mock_version.return_value = "0.0.0"
        shortcut_path = Path("TOOL new - 0.0.0.lnk")
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            with patch("cli.cmd_install.install_impl.TOOL_NAME", new="TOOL"):
                result = _create_shortcut_win32(
                    Path(), " new", ["arg1", "arg2"], "cmd", "new path"
                )
        self.assertEqual(result, 0)
        mock_is_file.assert_called_once()
        self.assertEqual(
            f"""Shortcut '{shortcut_path}' exists. Updating...
Successfully created shortcut: {shortcut_path}\n""",
            out.getvalue(),
        )
        self.assertEqual("", err.getvalue())
        mock_dispatch.assert_called_once_with("WScript.Shell")
        mock_shell.CreateShortCut.assert_called_once_with(str(shortcut_path))
        self.assertEqual(mock_shortcut.Targetpath, "new path")
        self.assertEqual(mock_shortcut.WindowStyle, 3)
        self.assertEqual(mock_shortcut.Arguments, "arg1 arg2 -Command cmd")
        mock_shortcut.save.assert_called_once()

    @patch("cli.cmd_install.install_impl.get_numeric_version")
    @patch("cli.cmd_install.install_impl.Path.is_file")
    @patch("cli.cmd_install.install_impl.Dispatch")
    def test_no_file(
        self, mock_dispatch: MagicMock, mock_is_file: MagicMock, mock_version: MagicMock
    ):
        """Test shortcut does not exist"""
        mock_shortcut = MagicMock()
        mock_shell = MagicMock()
        mock_shell.CreateShortCut.return_value = mock_shortcut
        mock_dispatch.return_value = mock_shell
        mock_is_file.return_value = False
        mock_version.return_value = "0.0.0"
        shortcut_path = Path("TOOL new - 0.0.0.lnk")
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            with patch("cli.cmd_install.install_impl.TOOL_NAME", new="TOOL"):
                result = _create_shortcut_win32(
                    Path(), " new", ["arg1", "arg2"], "cmd", "new path"
                )
        self.assertEqual(result, 0)
        mock_is_file.assert_called_once()
        self.assertEqual(
            f"Successfully created shortcut: {shortcut_path}\n", out.getvalue()
        )
        self.assertEqual("", err.getvalue())
        mock_dispatch.assert_called_once_with("WScript.Shell")
        mock_shell.CreateShortCut.assert_called_once_with(str(shortcut_path))
        self.assertEqual(mock_shortcut.Targetpath, "new path")
        self.assertEqual(mock_shortcut.WindowStyle, 3)
        self.assertEqual(mock_shortcut.Arguments, "arg1 arg2 -Command cmd")
        mock_shortcut.save.assert_called_once()


@patch("cli.cmd_install.install_impl.Path.exists")
@patch("cli.cmd_install.install_impl.get_numeric_version")
@patch("cli.cmd_install.install_impl.get_platform")
class TestInstallFoxCli(unittest.TestCase):
    """Test of the function 'install_fox_cli_tools_on_host'"""

    def tearDown(self):
        importlib.reload(install_impl)

    @patch("cli.cmd_install.install_impl.PREFIX_LINUX", new=Path("prefix"))
    def test_env_exists(
        self, mock_platform: MagicMock, mock_version: MagicMock, mock_exists: MagicMock
    ):
        """Test function when the environment directory already exists"""
        install_impl.WIN32COM_AVAILABLE = True
        mock_platform.return_value = "linux"
        mock_version.return_value = "0.0.0"
        mock_exists.return_value = True
        env_dir = Path("prefix") / "envs/local/0.0.0"
        with self.assertRaises(SystemExit) as cm:
            install_fox_cli_tools_on_host()
        self.assertEqual(
            cm.exception.code,
            f"Directory '{env_dir}' already exists.\nRemove the directory and re-run the command.",
        )
        mock_exists.assert_called_once()

    @patch("cli.cmd_install.install_impl.python_setup")
    @patch("cli.cmd_install.install_impl.PREFIX_WIN32", new=Path("prefix"))
    def test_setup_failure_win32(
        self,
        mock_setup: MagicMock,
        mock_platform: MagicMock,
        mock_version: MagicMock,
        mock_exists: MagicMock,
    ):
        """Test function when creating the environment was not successful on Windows"""
        install_impl.WIN32COM_AVAILABLE = True
        mock_setup.main.return_value = 1
        mock_platform.return_value = "win32"
        mock_version.return_value = "0.0.0"
        mock_exists.return_value = False
        env_dir = Path("prefix") / "envs/local/0.0.0"
        result = install_fox_cli_tools_on_host()
        self.assertEqual(result, 1)
        mock_setup.main.assert_called_once_with(
            env_dir=str(env_dir), self_install=True, dry_run=False
        )

    @patch("cli.cmd_install.install_impl.python_setup")
    @patch("cli.cmd_install.install_impl.PREFIX_LINUX", new=Path("prefix"))
    def test_setup_failure_linux(
        self,
        mock_setup: MagicMock,
        mock_platform: MagicMock,
        mock_version: MagicMock,
        mock_exists: MagicMock,
    ):
        """Test function when creating the environment was not successful on Linux"""
        install_impl.WIN32COM_AVAILABLE = False
        mock_setup.main.return_value = 1
        mock_platform.return_value = "linux"
        mock_version.return_value = "0.0.0"
        mock_exists.return_value = False
        env_dir = Path("prefix") / "envs/local/0.0.0"
        result = install_fox_cli_tools_on_host()
        self.assertEqual(result, 1)
        mock_setup.main.assert_called_once_with(
            env_dir=str(env_dir), self_install=True, dry_run=False
        )

    @patch("cli.cmd_install.install_impl.python_setup")
    @patch("cli.cmd_install.install_impl.PREFIX_LINUX", new=Path("prefix"))
    def test_success_linux(
        self,
        mock_setup: MagicMock,
        mock_platform: MagicMock,
        mock_version: MagicMock,
        mock_exists: MagicMock,
    ):
        """Test function when creating the environment was successful on Linux"""
        install_impl.WIN32COM_AVAILABLE = False
        mock_setup.main.return_value = 0
        mock_platform.return_value = "linux"
        mock_version.return_value = "0.0.0"
        mock_exists.return_value = False
        env_dir = Path("prefix") / "envs/local/0.0.0"
        result = install_fox_cli_tools_on_host()
        self.assertEqual(result, 0)
        mock_setup.main.assert_called_once_with(
            env_dir=str(env_dir), self_install=True, dry_run=False
        )

    @unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
    @patch("cli.cmd_install.install_impl.python_setup")
    @patch("cli.cmd_install.install_impl.Path.mkdir")
    @patch("cli.cmd_install.install_impl._create_shortcut_win32")
    @patch("cli.cmd_install.install_impl.PREFIX_WIN32", new=Path("prefix"))
    def test_shortcut_failure(  # noqa: PLR0913
        self,
        mock_create_shortcut: MagicMock,
        mock_mkdir: MagicMock,
        mock_setup: MagicMock,
        mock_platform: MagicMock,
        mock_version: MagicMock,
        mock_exists: MagicMock,
    ):
        """Test function when creating the shortcuts was not successful"""
        mock_create_shortcut.return_value = 1
        mock_setup.main.return_value = 0
        mock_platform.return_value = "win32"
        mock_version.return_value = "0.0.0"
        mock_exists.return_value = False
        env_dir = Path("prefix") / "envs/local/0.0.0"
        result = install_fox_cli_tools_on_host()
        self.assertEqual(result, 2)
        mock_setup.main.assert_called_once_with(
            env_dir=str(env_dir), self_install=True, dry_run=False
        )
        mock_mkdir.assert_called_once_with(parents=True, exist_ok=True)

    @unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
    @patch("cli.cmd_install.install_impl.python_setup")
    @patch("cli.cmd_install.install_impl.Path.mkdir")
    @patch("cli.cmd_install.install_impl._create_shortcut_win32")
    @patch("cli.cmd_install.install_impl.PREFIX_WIN32", new=Path("prefix"))
    def test_shortcut_success(  # noqa: PLR0913
        self,
        mock_create_shortcut: MagicMock,
        mock_mkdir: MagicMock,
        mock_setup: MagicMock,
        mock_platform: MagicMock,
        mock_version: MagicMock,
        mock_exists: MagicMock,
    ):
        """Test function when creating the shortcuts was successful"""
        mock_create_shortcut.return_value = 0
        mock_setup.main.return_value = 0
        mock_platform.return_value = "win32"
        mock_version.return_value = "0.0.0"
        mock_exists.return_value = False
        env_dir = Path("prefix") / "envs/local/0.0.0"
        result = install_fox_cli_tools_on_host()
        self.assertEqual(result, 0)
        mock_setup.main.assert_called_once_with(
            env_dir=str(env_dir), self_install=True, dry_run=False
        )
        mock_mkdir.assert_called_once_with(parents=True, exist_ok=True)


class TestImport(unittest.TestCase):
    """Test importing Dispatch"""

    def tearDown(self):
        importlib.reload(install_impl)

    @patch("importlib.util.find_spec")
    def test_unavailable(self, mock_find_spec: MagicMock):
        """Test when 'wind32com' is not available"""
        mock_find_spec.return_value = False
        importlib.reload(install_impl)
        self.assertFalse(install_impl.WIN32COM_AVAILABLE)

    @unittest.skipUnless(sys.platform.startswith("win32"), "Windows only test.")
    @patch("importlib.util.find_spec")
    def test_available(self, mock_find_spec: MagicMock):
        """Test when 'wind32com' is available"""
        mock_find_spec.return_value = True
        importlib.reload(install_impl)
        self.assertTrue(install_impl.WIN32COM_AVAILABLE)


if __name__ == "__main__":
    unittest.main()
