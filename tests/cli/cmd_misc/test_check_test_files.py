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

"""Testing file 'cli/cmd_misc/check_test_files.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_misc import check_test_files
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc import check_test_files


class TestCheckTestFilesPrivateGetCliFiles(unittest.TestCase):
    """Test of '_get_cli_files.py' function."""

    @patch("cli.cmd_misc.check_test_files.PROJECT_ROOT")
    def test__get_cli_files(self, m_project_root: MagicMock):
        """dummy test"""
        m_project_root.return_value = Path("foo")
        ret = check_test_files._get_cli_files()  # pylint: disable=protected-access
        self.assertEqual(ret, [])


class TestCheckTestFilesPrivateGetTestFiles(unittest.TestCase):
    """Test of '_get_test_files.py' function."""

    @patch("cli.cmd_misc.check_test_files.PROJECT_ROOT")
    def test__get_cli_files(self, m_project_root: MagicMock):
        """dummy test"""
        m_project_root.return_value = Path("foo")
        ret = check_test_files._get_test_files()  # pylint: disable=protected-access
        self.assertEqual(ret, [])


class TestCheckTestFiles(unittest.TestCase):
    """Test of 'check_test_files.py'."""

    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).parents[3]
        return super().setUpClass()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files.Path.read_text")
    def test_check_for_test_files_test_ok(
        self, mock_read_text: MagicMock, mock_gtf: MagicMock, mock_gcf: MagicMock
    ):
        """TODO"""
        mock_gcf.return_value = [self.root / "cli/cli.py"]
        mock_gtf.return_value = [Path("tests/cli/test_cli.py")]
        mock_read_text.side_effect = ['"""Testing file \'cli/cli.py\'."""']
        out = io.StringIO()
        with redirect_stdout(out):
            ret = check_test_files.check_for_test_files(verbose=1)
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.out, "")
        self.assertEqual(ret.err, "")
        self.assertEqual(out.getvalue(), "Found all expected test files.\n")
        mock_read_text.assert_called_once_with(encoding="utf-8")
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files.Path.read_text")
    def test_check_for_test_files_test_invalid_docstring(
        self, mock_read_text: MagicMock, mock_gtf: MagicMock, mock_gcf: MagicMock
    ):
        """TODO"""
        mock_gcf.return_value = [
            self.root / "cli/cli.py",
            self.root / "cli/__init__.py",  # ignored
        ]
        mock_gtf.return_value = [Path("tests/cli/test_cli.py")]
        mock_read_text.side_effect = ['"""Testing file \'cli/foo.py\'."""']
        err = io.StringIO()
        with redirect_stderr(err):
            ret = check_test_files.check_for_test_files()
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertRegex(
            err.getvalue(),
            r"tests[\\\/]cli[\\\/]test_cli\.py is missing a docstring "
            r'starting with """Testing file \'cli\/cli\.py\'\."""',
        )
        mock_read_text.assert_called_once_with(encoding="utf-8")
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files.Path.read_text")
    def test_check_for_test_files_test_test_file_missing(
        self, mock_read_text: MagicMock, mock_gtf: MagicMock, mock_gcf: MagicMock
    ):
        """TODO"""
        mock_gcf.return_value = [self.root / "cli/cli.py"]
        mock_gtf.return_value = []
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = check_test_files.check_for_test_files()
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertRegex(
            err.getvalue(),
            r"\'.*cli[\\\/]cli\.py\' expects a test file in "
            r"\'.*tests[\\\/]cli[\\\/]test_cli\.py\'.\n"
            r"Expected test files are missing\.",
        )
        mock_read_text.assert_not_called()
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()


if __name__ == "__main__":
    unittest.main()
