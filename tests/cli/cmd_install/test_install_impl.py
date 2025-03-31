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

"""Testing file 'cli/cmd_install/install_impl.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_install.install_impl import (
        all_software_available,
        check_for_all_softwares,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_install.install_impl import (
        all_software_available,
        check_for_all_softwares,
    )


class TestInstallImpl(unittest.TestCase):
    """Test of the function all_software_available"""

    @patch(
        "cli.cmd_install.install_impl.REQUIRED_SOFTWARE",
        {"abc": {"executable": "abc", "path": False}},
    )
    def test_all_software_not_available(self):
        """test function with some unavailable software"""
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
        """test function with only Python as required software"""
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
        """test function with only Python as required software"""
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
        """test function with only Python as required software"""
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
        """Test invalid expected required software defintion"""
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


if __name__ == "__main__":
    unittest.main()
