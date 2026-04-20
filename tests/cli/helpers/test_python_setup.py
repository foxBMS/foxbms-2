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

"""Testing file 'cli/helpers/python_setup.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.helpers import python_setup
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import python_setup


class TestGetPythonVersion(unittest.TestCase):
    """Test determining required Python version"""

    @patch("sys.platform", new="linux")
    @patch("cli.helpers.python_setup.Popen")
    def test_get_python_version_linux_raises(self, mock_popen: MagicMock):
        """Test env creation"""
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 1
        mock_popen.return_value.__enter__.return_value = mock_process

        err = io.StringIO()
        out = io.StringIO()
        with (
            self.assertRaises(SystemExit) as cm,
            redirect_stderr(err),
            redirect_stdout(out),
        ):
            python_setup.get_python_version(dry_run=False)
        self.assertEqual(cm.exception.code, 1)
        self.assertEqual(
            err.getvalue(),
            "Go To python.org and download the latest version of Python 3.12.\n"
            "Then re-run the command.\n",
        )
        self.assertEqual(out.getvalue(), "\nRunning:\n -> cmd: python3.12 --version\n")
        self.assertEqual(mock_process.communicate.call_count, 1)

    @patch("sys.platform", new="win32")
    @patch("cli.helpers.python_setup.Popen")
    def test_get_python_version_win32_raises(self, mock_popen: MagicMock):
        """Test env creation"""
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 1
        mock_popen.return_value.__enter__.return_value = mock_process

        err = io.StringIO()
        out = io.StringIO()
        with (
            self.assertRaises(SystemExit) as cm,
            redirect_stderr(err),
            redirect_stdout(out),
        ):
            python_setup.get_python_version(dry_run=False)
        self.assertEqual(cm.exception.code, 1)
        self.assertEqual(
            err.getvalue(),
            "Go To python.org and download the latest version of Python 3.12.\n"
            "Then re-run the command.\n",
        )
        self.assertEqual(out.getvalue(), "\nRunning:\n -> cmd: py -3.12 --version\n")
        self.assertEqual(mock_process.communicate.call_count, 1)

    @patch("sys.platform", new="win32")
    @patch("cli.helpers.python_setup.Popen")
    def test_get_python_version_succeeds(self, mock_popen: MagicMock):
        """Test env creation"""
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"Python 3.12.10", b"")
        mock_process.returncode = 0
        mock_popen.return_value.__enter__.return_value = mock_process

        err = io.StringIO()
        out = io.StringIO()

        with redirect_stderr(err), redirect_stdout(out):
            ret = python_setup.get_python_version(dry_run=False)
        self.assertEqual(ret, ("py", "-3.12"))
        self.assertEqual(err.getvalue(), "")
        self.assertEqual(
            out.getvalue(),
            "\nRunning:\n -> cmd: py -3.12 --version\n -> out: Python version: Python 3.12.10\n\n",
        )
        self.assertEqual(mock_process.communicate.call_count, 1)

    @patch("sys.platform", new="win32")
    def test_get_python_version_succeeds_dry_run(self):
        """Test env creation"""
        out = io.StringIO()
        with redirect_stdout(out):
            ret = python_setup.get_python_version()
        self.assertEqual(ret, ("py", "-3.12"))
        self.assertEqual(
            out.getvalue(),
            "\nRunning (dry run):\n"
            " -> cmd: py -3.12 --version\n"
            " -> out: Python version: Python 3.12.10\n\n",
        )


class TestInstallConfirmation(unittest.TestCase):
    """Test different options for install confirmation"""

    def test_install_confirmation_conformation_as_arg(self):
        """Test user confirmation through command line argument"""
        self.assertIsNone(python_setup.install_confirmation(True))

    @patch("builtins.input")
    def test_install_confirmation_answers_invalid_then_no(self, mock_input: MagicMock):
        """Test main, 'no' choice"""
        mock_input.side_effect = ["blu", "n"]
        err = io.StringIO()
        out = io.StringIO()
        with (
            self.assertRaises(SystemExit) as cm,
            redirect_stderr(err),
            redirect_stdout(out),
        ):
            python_setup.install_confirmation(False)
        self.assertEqual(cm.exception.code, 1)
        self.assertEqual(
            err.getvalue(),
            "yes or no answer required.\n"
            "This Python environment is required.\n"
            "You can also install it manually as shown in the documentation.\n"
            "Exiting...\n",
        )
        self.assertEqual(
            out.getvalue(),
            "The foxBMS Python environment is missing.\n"
            "It can be installed automatically.\n"
            "Answer 'Yes' to automatically install the environment or 'No' to "
            "abort installation process.\n",
        )
        self.assertEqual(mock_input.call_count, 2)

    @patch("builtins.input")
    def test_install_confirmation_answer_yes(self, mock_input: MagicMock):
        """Test main, 'yes' choice"""
        mock_input.return_value = "y"
        out = io.StringIO()
        with redirect_stdout(out):
            python_setup.install_confirmation(False)
        self.assertEqual(
            out.getvalue(),
            "The foxBMS Python environment is missing.\n"
            "It can be installed automatically.\n"
            "Answer 'Yes' to automatically install the environment or 'No' to "
            "abort installation process.\n",
        )
        self.assertEqual(mock_input.call_count, 1)


class TestRunCmd(unittest.TestCase):
    """Test (dry-running) commands (i.e., processes)"""

    def test_run_cmd_dry_run(self):
        """Test dry-running a command"""
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            python_setup.run_cmd(["foo", "bar"], cwd="foo/bar", dry_run=True)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(err, "")
        self.assertRegex(
            out, r"Running \(dry run\):\n -> cwd: .*foo(\\|\/)bar\n -> cmd: foo bar\n"
        )

    @patch("cli.helpers.python_setup.Popen")
    def test_run_cmd(self, mock_popen: MagicMock):
        """Test a command"""
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            python_setup.run_cmd(["foo", "bar"], cwd="foo/bar", dry_run=False)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(err, "")
        self.assertRegex(out, r"Running:\n -> cwd: .*foo(\\|\/)bar\n -> cmd: foo bar\n")
        mock_popen.assert_called_once_with(
            ["foo", "bar"], cwd="foo/bar", stdout=-1, stderr=-1
        )


class TestCreateEnv(unittest.TestCase):
    """Test env creation command"""

    @patch("cli.helpers.python_setup.run_cmd")
    def test_create_env(self, _mock_run_cmd: MagicMock):
        """Test env creation"""
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            python_setup.create_env(py="py", env_dir="foo", ver="-3.12")
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(out, "")
        self.assertEqual(err, "")


class TestCreateEnvProcess(unittest.TestCase):
    """Test venv creation process"""

    @patch("cli.helpers.python_setup.MAX_SLEEP_TIME", 1)
    @patch("cli.helpers.python_setup.time.sleep")
    @patch("cli.helpers.python_setup.Process")
    def test_create_env_process_fails(
        self, mock_process: MagicMock, _mock_sleep: MagicMock
    ):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = python_setup.create_env_process(py="py", env_dir="foo", ver="")

        self.assertEqual(ret, 1)
        self.assertEqual(err.getvalue(), "Could not create virtual environment\n")
        self.assertEqual(out.getvalue(), "..\n")

    @patch("cli.helpers.python_setup.MAX_SLEEP_TIME", 3)
    @patch("cli.helpers.python_setup.time.sleep")
    @patch("cli.helpers.python_setup.Process")
    def test_create_env_process_succeeds(
        self, mock_process: MagicMock, _mock_sleep: MagicMock
    ):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = False
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = python_setup.create_env_process(py="py", env_dir="foo", ver="")

        self.assertEqual(ret, 0)
        self.assertEqual(out.getvalue(), "\n")


class TestInstallPackages(unittest.TestCase):
    """Test packages installation (i.e., dependencies)"""

    @patch("sys.platform", new="win32")
    @patch("cli.helpers.python_setup.Popen")
    def test_install_packages_win32(self, mock_popen: MagicMock):
        """Test env creation"""
        out = io.StringIO()
        with redirect_stdout(out):
            python_setup.install_packages(py="python.exe", cwd="foo/bar", dry_run=False)
        self.assertRegex(
            out.getvalue(),
            r"Running:\n"
            r" -> cwd: .*foo(\\|\/)bar\n"
            r" -> cmd: python.exe -m pip install -r .*.*requirements\.txt",
        )
        mock_popen.assert_called_once_with(
            [
                "python.exe",
                "-m",
                "pip",
                "install",
                "-r",
                str(Path(__file__).parents[3] / "requirements.txt"),
            ],
            cwd="foo/bar",
            stdout=-1,
            stderr=-1,
        )

    @patch("sys.platform", new="linux")
    @patch("cli.helpers.python_setup.Popen")
    def test_install_packages_linux(self, mock_popen: MagicMock):
        """Test env creation"""
        out = io.StringIO()
        with redirect_stdout(out):
            python_setup.install_packages("python.exe", "foo/Scripts", False)
        self.assertRegex(
            out.getvalue(),
            r"python.exe -m pip install -r .*requirements\.txt",
        )
        py = "python.exe"
        mock_popen.assert_called_once_with(
            [py]
            + [
                "-m",
                "pip",
                "install",
                "-r",
                str(Path(__file__).parents[3] / "requirements.txt"),
            ],
            cwd="foo/Scripts",
            stdout=-1,
            stderr=-1,
        )


class TestInstallPackagesProcess(unittest.TestCase):
    """Test package installation process"""

    @patch("cli.helpers.python_setup.MAX_SLEEP_TIME", 1)
    @patch("cli.helpers.python_setup.time.sleep")
    @patch("cli.helpers.python_setup.Process")
    def test_install_packages_process_fails(
        self, mock_process: MagicMock, _mock_sleep: MagicMock
    ):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = python_setup.install_packages_process(
                "python.exe", "foo/Scripts", False
            )

        self.assertEqual(ret, 1)
        self.assertEqual(
            err.getvalue(), "Could not install packages into the environment\n"
        )
        self.assertEqual(out.getvalue(), "..\n")

    @patch("cli.helpers.python_setup.MAX_SLEEP_TIME", 3)
    @patch("cli.helpers.python_setup.Process")
    def test_install_packages_process_succeeds(self, mock_process: MagicMock):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = False
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = python_setup.install_packages_process(
                "python.exe", "foo/Scripts", False
            )

        self.assertEqual(ret, 0)
        self.assertEqual(out.getvalue(), "\n")


class TestInstallFoxCliPackage(unittest.TestCase):
    """Test fox_cli package installation"""

    @patch("cli.helpers.python_setup.run_cmd")
    def test_install_fox_cli_package(self, mock_run_cmd: MagicMock):
        """Test main, 'no' choice"""
        python_setup.install_fox_cli_package("py", cwd="foo")
        mock_run_cmd.assert_called_once_with(
            ["py", "-m", "pip", "install", "."], "foo", True
        )


class TestInstallFoxCliPackageProcess(unittest.TestCase):
    """Test fox_cli package installation process"""

    @patch("cli.helpers.python_setup.MAX_SLEEP_TIME", 1)
    @patch("cli.helpers.python_setup.time.sleep")
    @patch("cli.helpers.python_setup.Process")
    def test_install_fox_cli_package_process_fails(
        self, mock_process: MagicMock, _mock_sleep: MagicMock
    ):
        """Test main, 'no' choice"""
        # installing co_cli package in the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_process.terminate.return_value = None

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            # all arguments are don't care, as the process is mocked
            ret = python_setup.install_fox_cli_package_process("foo", "bar", "dir")
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 1)
        self.assertEqual(
            err, "Could not install the 'fox CLI' package into the environment 'dir'\n"
        )
        self.assertEqual(out, "..\n")

    @patch("cli.helpers.python_setup.MAX_SLEEP_TIME", 1)
    @patch("cli.helpers.python_setup.Process")
    def test_install_fox_cli_package_process_succeeds(self, mock_process: MagicMock):
        """Test main, 'no' choice"""
        # installing co_cli package in the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = False
        mock_instance_process.terminate.return_value = None

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            # all arguments are don't care, as the process is mocked
            ret = python_setup.install_fox_cli_package_process("foo", "bar", "dir")
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 0)
        self.assertEqual(err, "")
        self.assertEqual(out, "\n")


class TestMain(unittest.TestCase):
    """Test python_setup.py as script"""

    def setUp(self):
        self.out_header = "foxBMS 2 Install Helper\n"
        self.python_determined = "--> Successfully determined Python version\n"
        self.env_installed = "--> Successfully installed the Python environment\n"
        self.deps_installed = (
            "--> Successfully installed the dependencies in the Python environment\n"
        )
        return super().setUp()

    @patch("sys.platform", new="win32")
    @patch("cli.helpers.python_setup.get_python_version")
    @patch("cli.helpers.python_setup.install_confirmation")
    @patch("cli.helpers.python_setup.create_env_process")
    def test_main_create_env_process_fails(
        self,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing the environment fails."""
        mock_gpv.side_effect = [("py", "-3.12")]
        mock_ic.return_value = 0
        mock_cep.return_value = 1
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = python_setup.main("foo")
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 1)
        self.assertEqual(err, "")
        self.assertEqual(out, f"{self.out_header}{self.python_determined}")

    @patch("sys.platform", new="win32")
    @patch("cli.helpers.python_setup.get_python_version")
    @patch("cli.helpers.python_setup.install_confirmation")
    @patch("cli.helpers.python_setup.create_env_process")
    def test_main_create_env_process_fails_self_install(
        self,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing the environment fails."""
        mock_gpv.side_effect = [("py", "-3.12")]
        mock_ic.return_value = 0
        mock_cep.return_value = 1
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = python_setup.main("foo", self_install=True)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 1)
        # as we mock the failing process, we do not see its error message
        self.assertEqual(err, "")
        self.assertEqual(out, f"{self.out_header}{self.python_determined}")

    @patch("sys.platform", new="linux")
    @patch("cli.helpers.python_setup.get_python_version")
    @patch("cli.helpers.python_setup.install_confirmation")
    @patch("cli.helpers.python_setup.create_env_process")
    @patch("cli.helpers.python_setup.install_packages_process")
    def test_main_install_packages_process_fails(
        self,
        mock_ipp: MagicMock,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing packages in the environment fails."""
        mock_gpv.side_effect = [("python3.12", "")]
        mock_ic.return_value = 0
        mock_cep.return_value = 0
        mock_ipp.return_value = 1
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = python_setup.main("foo")
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 1)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            self.out_header + self.python_determined + self.env_installed,
        )

    @patch("cli.helpers.python_setup.get_python_version")
    @patch("cli.helpers.python_setup.install_confirmation")
    @patch("cli.helpers.python_setup.create_env_process")
    @patch("cli.helpers.python_setup.install_packages_process")
    def test_main_succeeds(
        self,
        mock_ipp: MagicMock,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing the environment and its packages succeeds."""
        mock_gpv.side_effect = [("py", "-3.12")]
        mock_ic.return_value = 0
        mock_cep.return_value = 0
        mock_ipp.return_value = 0
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = python_setup.main("foo")
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 0)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            self.out_header
            + self.python_determined
            + self.env_installed
            + self.deps_installed,
        )

    @patch("cli.helpers.python_setup.get_python_version")
    @patch("cli.helpers.python_setup.install_confirmation")
    @patch("cli.helpers.python_setup.create_env_process")
    @patch("cli.helpers.python_setup.install_packages_process")
    @patch("cli.helpers.python_setup.install_fox_cli_package_process")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_main_self_install_fails(
        self,
        mock_ifcpp: MagicMock,
        mock_ipp: MagicMock,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing the environment and its packages succeeds."""
        mock_gpv.side_effect = [("py", "-3.12")]
        mock_ic.return_value = 0
        mock_cep.return_value = 0
        mock_ipp.return_value = 0
        mock_ifcpp.return_value = 1
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = python_setup.main("foo", self_install=True)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 1)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            self.out_header
            + self.python_determined
            + self.env_installed
            + self.deps_installed,
        )

    @patch("cli.helpers.python_setup.get_python_version")
    @patch("cli.helpers.python_setup.install_confirmation")
    @patch("cli.helpers.python_setup.create_env_process")
    @patch("cli.helpers.python_setup.install_packages_process")
    @patch("cli.helpers.python_setup.install_fox_cli_package_process")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_main_self_install_succeeds(
        self,
        mock_ifcpp: MagicMock,
        mock_ipp: MagicMock,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing the environment and its packages succeeds."""
        mock_gpv.side_effect = [("py", "-3.12")]
        mock_ic.return_value = 0
        mock_cep.return_value = 0
        mock_ipp.return_value = 0
        mock_ifcpp.return_value = 0
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = python_setup.main("foo", self_install=True)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 0)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            self.out_header
            + self.python_determined
            + self.env_installed
            + self.deps_installed
            + "--> Successfully installed 'fox CLI' package to 'foo'\n",
        )


class TestAsScript(unittest.TestCase):
    """Test invocation as script/module"""

    @patch("cli.helpers.python_setup.main")
    def test_as_script_no_args(self, mock_main: MagicMock):
        """Test invocation as script/module with defaults"""
        with patch("sys.argv", new=["python", "env-name"]):
            python_setup._as_script()  # pylint: disable=protected-access
        mock_main.assert_called_once_with(
            env_dir="env-name", confirm=False, self_install=False, dry_run=False
        )

    @patch("cli.helpers.python_setup.main")
    def test_as_script_args(self, mock_main: MagicMock):
        """Test invocation as script/module with command line arguments/options"""
        with patch(
            "sys.argv",
            new=["python", "env-name", "--confirm", "--dry-run", "--self-install"],
        ):
            python_setup._as_script()  # pylint: disable=protected-access
        mock_main.assert_called_once_with(
            env_dir="env-name", confirm=True, self_install=True, dry_run=True
        )


if __name__ == "__main__":
    unittest.main()
