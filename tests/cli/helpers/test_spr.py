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

"""Testing file 'cli/helpers/spr.py'."""

import importlib
import io
import subprocess
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.helpers import spr
    from cli.helpers.spr import SubprocessResult, prepare_subprocess_output, run_process
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import spr
    from cli.helpers.spr import SubprocessResult, prepare_subprocess_output, run_process


class TestSpR(unittest.TestCase):
    """Test of 'spr.py'."""

    def setUp(self):
        importlib.reload(spr)
        return super().setUp()

    def test_prepare_subprocess_output(self):
        """basic prepare_subprocess_output test"""
        ret = prepare_subprocess_output(-1, b"foxBMS", b"dummy")
        self.assertEqual(-1, ret.returncode)
        self.assertEqual("foxBMS", ret.out)
        self.assertEqual("dummy", ret.err)

        ret = prepare_subprocess_output(1, b"", b"")
        self.assertEqual(1, ret.returncode)
        self.assertEqual("", ret.out)
        self.assertEqual("", ret.err)

    def test_add_sprs(self):
        """test adding of two 'SubprocessResults'"""
        ret = prepare_subprocess_output(
            -1, b"foxBMS", b"dummy"
        ) + prepare_subprocess_output(1, b"1", b"2")
        self.assertEqual(2, ret.returncode)
        self.assertEqual("foxBMS\n\n1", ret.out)
        self.assertEqual("dummy\n\n2", ret.err)

    def test_run_process_no_program(self):
        """test running a program, without providing a program"""
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = run_process([])
        self.assertEqual(1, ret.returncode)
        self.assertEqual("", ret.out)
        self.assertEqual("No program provided.", ret.err)
        self.assertIn("No program provided.", buf.getvalue())

    def test_run_process_invalid_program(self):
        """test running a program, without providing a valid program"""
        cmd = ["does-not-exist"]
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = run_process(cmd)
        self.assertEqual(1, ret.returncode)
        self.assertEqual("", ret.out)
        self.assertEqual(f"Program '{cmd[0]}' does not exist.", ret.err)
        self.assertIn(f"Program '{cmd[0]}' does not exist.", buf.getvalue())

    def test_spr_str(self):
        """Test string-representation of the SubprocessResult class"""
        dummy = SubprocessResult(1, "abc", "def")
        self.assertEqual("return code: 1\n\nout:abc\n\ndef\n", str(dummy))

    @patch("sys.platform", new="linux")
    @patch("shutil.which")
    @patch("cli.helpers.spr.subprocess.Popen")
    def test_run_process_linux(self, mock_popen: MagicMock, mock_which: MagicMock):
        """Test the 'run_process' function for Linux."""
        importlib.reload(spr)
        mock_which.return_value = "some-program"
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 0
        mock_popen.return_value.__enter__.return_value = mock_process

        result = run_process(["some-program", "some-arguments"])

        mock_popen.assert_called_once_with(
            ["some-program", "some-arguments"],
            cwd=Path(__file__).parents[3],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=None,
        )
        self.assertEqual(
            result.out, mock_process.communicate.return_value[0].decode("utf-8")
        )
        self.assertEqual(
            result.err, mock_process.communicate.return_value[1].decode("utf-8")
        )

    @patch("builtins.hasattr")
    @patch("shutil.which")
    @patch("cli.helpers.spr.subprocess.Popen")
    @unittest.skipIf(not sys.platform.startswith("win32"), "Windows specific test")
    def test_run_process_no_isatty(
        self, mock_popen: MagicMock, mock_which: MagicMock, mock_hasattr: MagicMock
    ):
        """Test the 'run_process' function without 'isatty' in 'sys.stdin'."""
        mock_hasattr.return_value = False
        mock_which.return_value = "some-program"
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 0
        mock_popen.return_value.__enter__.return_value = mock_process

        result = run_process(["some-program", "some-arguments"])

        self.assertEqual(
            result.out, mock_process.communicate.return_value[0].decode("utf-8")
        )
        self.assertEqual(
            result.err, mock_process.communicate.return_value[1].decode("utf-8")
        )
        mock_hasattr.assert_called_once()
        mock_popen.assert_called_once_with(
            ["some-program", "some-arguments"],
            cwd=Path(__file__).parents[3],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=None,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )

    @patch("shutil.which")
    @patch("cli.helpers.spr.subprocess.Popen")
    def test_run_process(self, mock_popen, mock_which):
        """Test the 'run_process' function."""
        mock_which.return_value = "some-program"
        mock_process = MagicMock()
        mock_process.communicate.return_value = (b"stdout", b"stderr")
        mock_process.returncode = 0
        mock_popen.return_value.__enter__.return_value = mock_process

        result = run_process(["some-program", "some-arguments"])

        mock_popen.assert_called_once_with(
            ["some-program", "some-arguments"],
            cwd=Path(__file__).parents[3],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=None,
        )
        self.assertEqual(
            result.out, mock_process.communicate.return_value[0].decode("utf-8")
        )
        self.assertEqual(
            result.err, mock_process.communicate.return_value[1].decode("utf-8")
        )


if __name__ == "__main__":
    unittest.main()
