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

"""Testing file 'cli/cmd_build/build_impl.py'."""

import sys
import unittest
from pathlib import Path
from subprocess import PIPE
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_build import build_impl
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_build import build_impl
    from cli.helpers.spr import SubprocessResult


class TestRunWaf(unittest.TestCase):
    """Test the 'waf' wrapper."""

    @patch("cli.cmd_build.build_impl.run_process")
    def test_run_waf(self, mock_run_process: MagicMock):
        """Test the 'run_waf' function."""
        mock_run_process.return_value = SubprocessResult(0)
        result = build_impl.run_waf(["--help"])
        expected_cmd = [
            sys.executable,
            str(Path(__file__).parents[3] / "tools/waf"),
            "--help",
        ]
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=Path(__file__).parents[3], stdout=PIPE, stderr=PIPE
        )
        self.assertEqual(result.returncode, 0)

    @patch("cli.cmd_build.build_impl.run_process")
    def test_run_top_level_waf(self, mock_run_process: MagicMock):
        """Test the 'run_waf' function."""
        mock_run_process.return_value = SubprocessResult(0)
        result = build_impl.run_top_level_waf(["--help"])
        expected_cmd = [
            sys.executable,
            str(Path(__file__).parents[3] / "tools/waf"),
            "--help",
        ]
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=Path(__file__).parents[3], stdout=PIPE, stderr=PIPE
        )
        self.assertEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
