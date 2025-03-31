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

"""Testing file 'cli/cmd_ide/ide_impl.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import patch

try:
    from cli.cmd_ide import ide_impl
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_ide import ide_impl
    from cli.helpers.spr import SubprocessResult


class TestIdeImpl(unittest.TestCase):
    """Test"""

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    def test_no_code(self, mock_which):
        """code not available"""
        mock_which.return_value = None
        with self.assertLogs(level="INFO") as cm:
            result = ide_impl.open_ide()
        mock_which.assert_called_once_with("code")
        self.assertEqual(result, -1)
        self.assertEqual(cm.output[0], "ERROR:root:Could not find 'code' binary.")

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    def test_wd_does_not_exist(self, mock_which):
        """code not available"""
        mock_which.return_value = "/path/to/code"
        with self.assertLogs(level="INFO") as cm:
            result = ide_impl.open_ide(wd=Path("a/b/c/d/e/f"))
        mock_which.assert_called_once_with("code")
        self.assertEqual(result, -1)
        self.assertRegex(
            cm.output[0], r"ERROR\:root\:Working directory \'.*f\' does not exist\."
        )

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    @patch("cli.cmd_ide.ide_impl.run_process")
    def test_open_ide_generic(self, mock_run_process, mock_which):
        """open generic"""
        mock_which.return_value = "/path/to/code"
        mock_run_process.return_value = SubprocessResult(0)
        result = ide_impl.open_ide_generic()
        mock_which.assert_called_once_with("code")
        mock_run_process.assert_called_once_with(
            cmd=["/path/to/code", str(Path(__file__).parents[3])],
            cwd=Path(__file__).parents[3],
        )
        self.assertEqual(result, mock_run_process.return_value.returncode)

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    @patch("cli.cmd_ide.ide_impl.run_process")
    def test_open_ide_app(self, mock_run_process, mock_which):
        """open app"""
        mock_which.return_value = "/path/to/code"
        mock_run_process.return_value = SubprocessResult(0)
        result = ide_impl.open_ide_app()
        mock_which.assert_called_once_with("code")
        mock_run_process.assert_called_once_with(
            cmd=[
                "/path/to/code",
                str(Path(__file__).parents[3] / "src/app"),
            ],
            cwd=Path(__file__).parents[3] / "src/app",
        )
        self.assertEqual(result, mock_run_process.return_value.returncode)

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    @patch("cli.cmd_ide.ide_impl.run_process")
    def test_open_ide_bootloader(self, mock_run_process, mock_which):
        """open bootloader"""
        mock_which.return_value = "/path/to/code"
        mock_run_process.return_value = SubprocessResult(0)
        result = ide_impl.open_ide_bootloader()
        mock_which.assert_called_once_with("code")
        mock_run_process.assert_called_once_with(
            cmd=[
                "/path/to/code",
                str(Path(__file__).parents[3] / "src/bootloader"),
            ],
            cwd=Path(__file__).parents[3] / "src/bootloader",
        )
        self.assertEqual(result, mock_run_process.return_value.returncode)

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    @patch("cli.cmd_ide.ide_impl.run_process")
    def test_open_open_ide_embedded_unit_test_app(self, mock_run_process, mock_which):
        """open app unit tests"""
        mock_which.return_value = "/path/to/code"
        mock_run_process.return_value = SubprocessResult(0)
        result = ide_impl.open_ide_embedded_unit_test_app()
        mock_which.assert_called_once_with("code")
        mock_run_process.assert_called_once_with(
            cmd=[
                "/path/to/code",
                str(Path(__file__).parents[3] / "tests/unit/app"),
            ],
            cwd=Path(__file__).parents[3] / "tests/unit/app",
        )
        self.assertEqual(result, mock_run_process.return_value.returncode)

    @patch("cli.cmd_ide.ide_impl.shutil.which")
    @patch("cli.cmd_ide.ide_impl.run_process")
    def test_open_ide_embedded_unit_test_bootloader(self, mock_run_process, mock_which):
        """open bootloader unit tests"""
        mock_which.return_value = "/path/to/code"
        mock_run_process.return_value = SubprocessResult(0)
        result = ide_impl.open_ide_embedded_unit_test_bootloader()
        mock_which.assert_called_once_with("code")
        mock_run_process.assert_called_once_with(
            cmd=[
                "/path/to/code",
                str(Path(__file__).parents[3] / "tests/unit/bootloader"),
            ],
            cwd=Path(__file__).parents[3] / "tests/unit/bootloader",
        )
        self.assertEqual(result, mock_run_process.return_value.returncode)


if __name__ == "__main__":
    unittest.main()
