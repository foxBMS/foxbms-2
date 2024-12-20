#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
import shutil
import sys
import unittest
from contextlib import redirect_stderr
from importlib import reload
from pathlib import Path
from unittest.mock import patch

try:
    from cli.cmd_misc import run_uncrustify
    from cli.helpers import host_platform
    from cli.helpers.misc import PROJECT_ROOT
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.cmd_misc import run_uncrustify
    from cli.helpers import host_platform
    from cli.helpers.misc import PROJECT_ROOT
    from cli.helpers.spr import SubprocessResult


PATH_TEMP = Path(__file__).parent / "temp"


class TestRunUncrustify(unittest.TestCase):
    """Class to test the run uncrustify script"""

    def setUp(self):
        # Create temporal directory for uncrustify.exe
        PATH_TEMP.mkdir(parents=True, exist_ok=True)
        self.path_uncrustify = PATH_TEMP / "uncrustify.exe"

    def tearDown(self):
        # Delete the temporal directory
        shutil.rmtree(PATH_TEMP)

    @patch("cli.cmd_misc.run_uncrustify.run_process")
    def test_run_uncrustify_process(self, mock_run_process):
        """test function run uncrustify process"""
        # Prepare test arguments
        mock_run_process.return_value = SubprocessResult(0)
        uncrustify = ""
        args = [""]
        _file = ""
        expected_cmd = [uncrustify] + args + [_file]

        # Call the function under test
        result = run_uncrustify.run_uncrustify_process(uncrustify, args, _file)

        # Assertions
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value)

    @patch("shutil.which")
    def test_lint_freertos_without_files(self, mock_which):
        """test run uncrustify without files"""
        reload(run_uncrustify)
        # Prepare test arguments
        mock_which.return_value = shutil.which("uncrustify", path=PATH_TEMP)
        run_uncrustify.FREERTOS_FILES = []

        # Call the function under test
        result = run_uncrustify.lint_freertos()

        # Assertions
        self.assertEqual(result, 0)

    @patch("shutil.which")
    def test_lint_freertos_argument(self, mock_which):
        """test run uncrustify with an argument"""
        reload(run_uncrustify)
        # Prepare test arguments
        mock_which.return_value = shutil.which("uncrustify", path=PATH_TEMP)
        run_uncrustify.FREERTOS_FILES = []

        # Call the function under test
        result = run_uncrustify.lint_freertos(False)

        # Assertions
        self.assertEqual(result, 0)

    @patch("shutil.which")
    def test_lint_freertos_invalid_file(self, mock_which):
        """test run uncrustify with an invalid file"""
        reload(run_uncrustify)
        # Prepare test arguments
        mock_which.return_value = shutil.which("uncrustify", path=PATH_TEMP)
        run_uncrustify.FREERTOS_FILES = ["does/not/exist"]

        buf = io.StringIO()
        with redirect_stderr(buf):
            result = run_uncrustify.lint_freertos()
        self.assertEqual(result, 1)
        self.assertRegex(
            buf.getvalue(), r"'.*does[\/|\\]not[\/|\\]exist' is not a file."
        )

    @patch("sys.platform", "linux")
    @patch("shutil.which", return_value=None)
    def test_lint_freertos_linux_no_uncrustify(self, _):
        """test could not find uncrustify on Linux"""
        # Reload to update sys.platform
        reload(host_platform)
        reload(run_uncrustify)

        buf = io.StringIO()
        with redirect_stderr(buf):
            result = run_uncrustify.lint_freertos()
        self.assertEqual(result, 1)
        self.assertIn("Could not find uncrustify.", buf.getvalue())

    @patch("sys.platform", "win32")
    @patch("shutil.which", return_value=None)
    def test_lint_freertos_windows_no_uncrustify(self, _):
        """test could not find uncrustify on Windows"""
        # Reload to update sys.platform
        reload(host_platform)
        reload(run_uncrustify)

        buf = io.StringIO()
        with redirect_stderr(buf):
            result = run_uncrustify.lint_freertos()
        self.assertEqual(result, 1)
        self.assertIn("Could not find uncrustify.", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
