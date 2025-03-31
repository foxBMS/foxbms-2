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

"""Testing file 'cli/fallback/fallback.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from subprocess import PIPE
from unittest.mock import MagicMock, patch

sys.path.insert(0, str(Path(__file__).parents[3] / "cli/fallback"))
import fallback  # pylint: disable=wrong-import-position


class TestFoxCliMain(unittest.TestCase):
    """Test of the main entry point"""

    @patch("sys.platform", new="linux")
    @patch("fallback.Popen")
    def test_get_python_version_linux_raises(self, mock_popen: MagicMock):
        """Test env creation"""
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 1
        mock_popen.return_value.__enter__.return_value = mock_process

        err = io.StringIO()
        out = io.StringIO()
        with self.assertRaises(SystemExit) as cm:
            with redirect_stderr(err), redirect_stdout(out):
                fallback.get_python_version()  # pylint: disable=c-extension-no-member
        self.assertEqual(cm.exception.code, 1)
        self.assertEqual(
            err.getvalue(),
            "Go To python.org and download the latest version of Python 3.12.\n"
            "Then re-run the command.\n",
        )
        self.assertEqual(out.getvalue(), "Running: python3.12 --version\n")
        self.assertEqual(mock_process.communicate.call_count, 1)

    @patch("sys.platform", new="win32")
    @patch("fallback.Popen")
    def test_get_python_version_win32_raises(self, mock_popen: MagicMock):
        """Test env creation"""
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 1
        mock_popen.return_value.__enter__.return_value = mock_process

        err = io.StringIO()
        out = io.StringIO()
        with self.assertRaises(SystemExit) as cm:
            with redirect_stderr(err), redirect_stdout(out):
                fallback.get_python_version()  # pylint: disable=c-extension-no-member
        self.assertEqual(cm.exception.code, 1)
        self.assertEqual(
            err.getvalue(),
            "Go To python.org and download the latest version of Python 3.12.\n"
            "Then re-run the command.\n",
        )
        self.assertEqual(out.getvalue(), "Running: py -3.12 --version\n")
        self.assertEqual(mock_process.communicate.call_count, 1)

    @patch("sys.platform", new="win32")
    @patch("fallback.Popen")
    def test_get_python_version_succeeds(self, mock_popen: MagicMock):
        """Test env creation"""
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 0
        mock_popen.return_value.__enter__.return_value = mock_process

        out = io.StringIO()
        with redirect_stdout(out):
            ret = fallback.get_python_version()  # pylint: disable=c-extension-no-member
        self.assertEqual(ret, ("py", "-3.12"))
        self.assertEqual(
            out.getvalue(), "Running: py -3.12 --version\nPython version: stdout\n"
        )
        self.assertEqual(mock_process.communicate.call_count, 1)

    @patch("builtins.input")
    def test_install_confirmation_answers_invalid_then_no(self, mock_input: MagicMock):
        """Test main, 'no' choice"""
        mock_input.side_effect = ["blu", "n"]
        err = io.StringIO()
        out = io.StringIO()
        with self.assertRaises(SystemExit) as cm:
            with redirect_stderr(err), redirect_stdout(out):
                fallback.install_confirmation()  # pylint: disable=c-extension-no-member
        self.assertEqual(cm.exception.code, 1)
        self.assertEqual(
            err.getvalue(),
            "yes or no answer required.\n"
            "This Python environment is required.\n"
            "You can also intall it manually as shown in the documentation.\n"
            "Exiting.\n",
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
            fallback.install_confirmation()  # pylint: disable=c-extension-no-member
        self.assertEqual(
            out.getvalue(),
            "The foxBMS Python environment is missing.\n"
            "It can be installed automatically.\n"
            "Answer 'Yes' to automatically install the environment or 'No' to "
            "abort installation process.\n",
        )
        self.assertEqual(mock_input.call_count, 1)

    @patch("fallback.Popen")
    def test_create_env(self, mock_popen: MagicMock):
        """Test env creation"""
        out = io.StringIO()
        with redirect_stdout(out):
            fallback.create_env("py", "foo", "-3.12")  # pylint: disable=c-extension-no-member
        self.assertEqual(out.getvalue(), "Running: py -3.12 -m venv foo\n")
        mock_popen.assert_called_once_with(
            ["py", "-3.12", "-m", "venv", "foo"], stdout=PIPE, stderr=PIPE
        )

    @patch("fallback.MAX_SLEEP_TIME", 1)
    @patch("fallback.Process")
    def test_create_env_process_fails(self, mock_process: MagicMock):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=c-extension-no-member
            ret = fallback.create_env_process(py="py", env_dir="foo", ver="")

        self.assertEqual(ret, 1)
        self.assertEqual(err.getvalue(), "Could not create virtual environment.\n")
        self.assertEqual(out.getvalue(), "..\n\n")

    @patch("fallback.MAX_SLEEP_TIME", 3)
    @patch("fallback.Process")
    def test_create_env_process_succeeds(self, mock_process: MagicMock):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = False
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=c-extension-no-member
            ret = fallback.create_env_process(py="py", env_dir="foo", ver="")

        self.assertEqual(ret, 0)
        self.assertEqual(out.getvalue(), "\n\n")

    @patch("sys.platform", new="win32")
    @patch("fallback.Popen")
    def test_install_packages_win32(self, mock_popen: MagicMock):
        """Test env creation"""
        out = io.StringIO()
        with redirect_stdout(out):
            fallback.install_packages("foo")  # pylint: disable=c-extension-no-member
        self.assertRegex(
            out.getvalue(),
            r"Running: foo\\Scripts\\python\.exe -m pip install -r .*requirements\.txt",
        )
        py = "foo\\Scripts\\python.exe"
        mock_popen.assert_called_once_with(
            [py]
            + [
                "-m",
                "pip",
                "install",
                "-r",
                str(Path(__file__).parents[3] / "requirements.txt"),
            ]
        )

    @patch("sys.platform", new="linux")
    @patch("fallback.Popen")
    def test_install_packages_linux(self, mock_popen: MagicMock):
        """Test env creation"""
        out = io.StringIO()
        with redirect_stdout(out):
            fallback.install_packages("foo")  # pylint: disable=c-extension-no-member
        self.assertRegex(
            out.getvalue(),
            r"foo\/bin\/python -m pip install -r .*requirements\.txt",
        )
        py = "foo/bin/python"
        mock_popen.assert_called_once_with(
            [py]
            + [
                "-m",
                "pip",
                "install",
                "-r",
                str(Path(__file__).parents[3] / "requirements.txt"),
            ]
        )

    @patch("fallback.MAX_SLEEP_TIME", 1)
    @patch("fallback.Process")
    def test_install_packages_process_fails(self, mock_process: MagicMock):
        """Test main, 'no' choice"""
        # installing the virtual environment takes too long
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_process.terminate.return_value = None

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=c-extension-no-member
            ret = fallback.install_packages_process(env_dir="foo")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err.getvalue(), "Could not intall packages into the environment.\n"
        )
        self.assertEqual(out.getvalue(), "..\n\n")

    @patch("fallback.MAX_SLEEP_TIME", 3)
    @patch("fallback.Process")
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
            # pylint: disable-next=c-extension-no-member
            ret = fallback.install_packages_process(env_dir="foo")

        self.assertEqual(ret, 0)
        self.assertEqual(out.getvalue(), "\n\n")

    @patch("fallback.get_python_version")
    @patch("fallback.install_confirmation")
    @patch("fallback.create_env_process")
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
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            with patch("sys.argv", new=sys.argv + ["env-name"]):
                ret = fallback.main()  # pylint: disable=c-extension-no-member
        self.assertEqual(ret, 1)
        self.assertEqual(out.getvalue(), "foxBMS 2 Install Helper\n")

    @patch("fallback.get_python_version")
    @patch("fallback.install_confirmation")
    @patch("fallback.create_env_process")
    @patch("fallback.install_packages_process")
    def test_main_install_packages_process_fails(
        self,
        mock_ipp: MagicMock,
        mock_cep: MagicMock,
        mock_ic: MagicMock,
        mock_gpv: MagicMock,
    ):
        """Installing packages in the environment fails."""
        mock_gpv.side_effect = [("py", "-3.12")]
        mock_ic.return_value = 0
        mock_cep.return_value = 0
        mock_ipp.return_value = 1
        out = io.StringIO()
        with redirect_stdout(out):
            with patch("sys.argv", new=sys.argv + ["env-name"]):
                ret = fallback.main()  # pylint: disable=c-extension-no-member
        self.assertEqual(ret, 1)
        self.assertEqual(out.getvalue(), "foxBMS 2 Install Helper\n")

    @patch("fallback.get_python_version")
    @patch("fallback.install_confirmation")
    @patch("fallback.create_env_process")
    @patch("fallback.install_packages_process")
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
        out = io.StringIO()
        with redirect_stdout(out):
            with patch("sys.argv", new=sys.argv + ["env-name"]):
                ret = fallback.main()  # pylint: disable=c-extension-no-member
        self.assertEqual(ret, 0)
        self.assertEqual(
            out.getvalue(),
            "foxBMS 2 Install Helper\nSuccessfully installed the Python environment.\n",
        )


if __name__ == "__main__":
    unittest.main()
