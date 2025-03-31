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

"""Testing file 'cli/cmd_misc/run_uncrustify.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from dataclasses import dataclass
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_misc import run_uncrustify
    from cli.helpers.misc import PROJECT_ROOT
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc import run_uncrustify
    from cli.helpers.misc import PROJECT_ROOT
    from cli.helpers.spr import SubprocessResult


@dataclass
class DummyFuture:
    """Mock return value of ProcessPoolExecutor.submit"""

    def __init__(self, x):
        self.returncode = x
        self.out = ""
        self.err = ""


class TestRunUncrustify(unittest.TestCase):
    """Class to test the run uncrustify script"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem
        cls.FREERTOS_FILES = run_uncrustify.FREERTOS_FILES

    def tearDown(self):
        run_uncrustify.FREERTOS_FILES = self.FREERTOS_FILES

    @patch("cli.cmd_misc.run_uncrustify.run_process")
    def test_run_uncrustify_process(self, mock_run_process):
        """test function run uncrustify process"""
        mock_run_process.return_value = SubprocessResult(0)
        uncrustify = ""
        args = [""]
        _file = ""
        expected_cmd = [uncrustify] + args + [_file]
        result = run_uncrustify.run_uncrustify_process(uncrustify, args, _file)
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None
        )
        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_misc.run_uncrustify.which", return_value="uncrustify")
    def test_lint_freertos_without_files(self, _):
        """test run uncrustify without files"""
        run_uncrustify.FREERTOS_FILES = []
        result = run_uncrustify.lint_freertos()
        self.assertEqual(result, 0)

    @patch("cli.cmd_misc.run_uncrustify.which", return_value="uncrustify")
    def test_lint_freertos_argument(self, _):
        """test run uncrustify with an argument"""
        run_uncrustify.FREERTOS_FILES = []
        result = run_uncrustify.lint_freertos(False)
        self.assertEqual(result, 0)

    @patch("sys.platform", new="linux")
    @patch("cli.cmd_misc.run_uncrustify.which", return_value=None)
    def test_lint_freertos_no_uncrustify_linux(self, _):
        """test could not find uncrustify"""
        err = io.StringIO()
        with redirect_stderr(err):
            result = run_uncrustify.lint_freertos()
        self.assertEqual(result, 1)
        self.assertEqual(err.getvalue(), "Could not find uncrustify.\n")

    @patch("sys.platform", new="win32")
    @patch("cli.cmd_misc.run_uncrustify.which", return_value=None)
    def test_lint_freertos_no_uncrustify_win32(self, _):
        """test could not find uncrustify"""
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = run_uncrustify.lint_freertos()
        self.assertEqual(result, 1)
        self.assertEqual(buf.getvalue(), "Could not find uncrustify.\n")

    @patch("cli.cmd_misc.run_uncrustify.FREERTOS_FILES", new=["foo", "bar"])
    @patch("cli.cmd_misc.run_uncrustify.which", return_value="uncrustify")
    @patch("cli.cmd_misc.run_uncrustify.ProcessPoolExecutor")
    def test_lint_freertos_no_error(self, mock_ppe: MagicMock, _):
        """No error occurs when linting"""
        mock_executor = MagicMock()
        mock_ppe.return_value.__enter__.return_value = mock_executor

        mock_f1 = MagicMock()
        mock_f1.result.return_value = DummyFuture(0)
        mock_f2 = MagicMock()
        mock_f2.result.return_value = DummyFuture(0)
        mock_executor.submit.side_effect = [mock_f1, mock_f2]
        with self.assertLogs("root", level="DEBUG") as log:
            ret = run_uncrustify.lint_freertos()
        self.assertEqual(ret, 0)
        self.assertEqual(
            [
                "DEBUG:root:Start worker for file 'foo'",
                "DEBUG:root:Start worker for file 'bar'",
                "DEBUG:root:exitcode: 0",
                "DEBUG:root:stdout: ",
                "DEBUG:root:stderr: ",
                "DEBUG:root:exitcode: 0",
                "DEBUG:root:stdout: ",
                "DEBUG:root:stderr: ",
            ],
            log.output,
        )

    @patch("cli.cmd_misc.run_uncrustify.FREERTOS_FILES", new=["foo", "bar"])
    @patch("cli.cmd_misc.run_uncrustify.which", return_value="uncrustify")
    @patch("cli.cmd_misc.run_uncrustify.ProcessPoolExecutor")
    def test_lint_freertos_error(self, mock_ppe: MagicMock, _):
        """An error occurs when linting"""
        mock_executor = MagicMock()
        mock_ppe.return_value.__enter__.return_value = mock_executor

        mock_f1 = MagicMock()
        mock_f1.result.return_value = DummyFuture(1)
        mock_f2 = MagicMock()
        mock_f2.result.return_value = DummyFuture(1)
        mock_executor.submit.side_effect = [mock_f1, mock_f2]
        with self.assertLogs("root", level="ERROR") as log:
            ret = run_uncrustify.lint_freertos()
        self.assertEqual(ret, 2)
        self.assertEqual(
            [
                "ERROR:root:exitcode: 1",
                "ERROR:root:stdout: ",
                "ERROR:root:stderr: ",
                "ERROR:root:exitcode: 1",
                "ERROR:root:stdout: ",
                "ERROR:root:stderr: ",
            ],
            log.output,
        )


if __name__ == "__main__":
    unittest.main()
