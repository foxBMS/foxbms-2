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

"""Testing file 'cli/cmd_misc/check_repository_depth.py'.

Test the path length checking and repository depth enforcement logic.
"""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_misc import check_repository_depth
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc import check_repository_depth


class TestPathChecker(unittest.TestCase):
    """Test path length and repository depth checking functions."""

    def test_check_path_length_within_limit(self):
        """Verify that _check_path_length returns 0 when path is within limit."""
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            # pylint:disable=protected-access
            ret = check_repository_depth._check_path_length("short/path.txt", 20)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 0)
        self.assertEqual(err, "")
        self.assertEqual(out, "")

    def test_check_path_length_exceeds_limit(self):
        """Verify that _check_path_length returns 1 and writes an error when
        path exceeds limit.
        """
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            # pylint:disable=protected-access
            ret = check_repository_depth._check_path_length("long/path.txt" * 3, 20)
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "File path 'long/path.txtlong/path.txtlong/path.txt' is too long (39, allowed 20).\n",
        )
        self.assertEqual(out, "")

    @patch("cli.cmd_misc.check_repository_depth.which")
    @patch("cli.cmd_misc.check_repository_depth.run_process")
    @patch("cli.cmd_misc.check_repository_depth.PROJECT_ROOT", "/fake/root")
    @patch("cli.cmd_misc.check_repository_depth._check_path_length", return_value=0)
    def test_check_repository_depth(
        self, mock_cpl: MagicMock, mock_run_process: MagicMock, mock_which: MagicMock
    ):
        """Ensure that check_repository_depth returns 0 when all paths are
        within limits.
        """
        mock_which.return_value = "/usr/bin/git"
        files = [
            "src/module/file1.py",
            "src/os/module/file2.py",
            "docs/README.md",
            "other/file3.txt",
        ]
        mock_run_process.return_value = MagicMock(out="\n".join(files))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = check_repository_depth.check_repository_depth()
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 0)
        self.assertEqual(err, "")
        self.assertEqual(out, "")
        # pylint:disable=protected-access
        self.assertEqual(mock_cpl.call_count, len(check_repository_depth._MAX_CONFIG))

    @patch("cli.cmd_misc.check_repository_depth.which")
    @patch("cli.cmd_misc.check_repository_depth.run_process")
    @patch("cli.cmd_misc.check_repository_depth.PROJECT_ROOT", "/fake/root")
    @patch(
        "cli.cmd_misc.check_repository_depth._check_path_length", side_effect=[1, 1, 1]
    )
    def test_check_repository_depth_exceeds(
        self, _mock_cpl: MagicMock, mock_run_process: MagicMock, mock_which: MagicMock
    ):
        """Check that check_repository_depth returns correct error count for
        long paths.
        """
        mock_which.return_value = "/usr/bin/git"
        files = [
            "src/very/long/path/" + "a" * 101,
            "docs/" + "b" * 126,
            "other/" + "c" * 151,
        ]
        mock_run_process.return_value = MagicMock(out="\n".join(files))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = check_repository_depth.check_repository_depth()
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(ret, 3)
        self.assertEqual(err, "")
        self.assertEqual(out, "")


if __name__ == "__main__":
    unittest.main()
