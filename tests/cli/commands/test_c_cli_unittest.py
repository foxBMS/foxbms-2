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

"""Testing file 'cli/commands/c_cli_unittest.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

from click.testing import CliRunner

try:
    from cli.cli import main
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cli import main
    from cli.helpers.spr import SubprocessResult


class TestFoxCliMainCommandCliUnittest(unittest.TestCase):
    """Test of the 'cli-unittest' commands and options."""

    def test_cli_unittest_0(self):
        """Test 'fox.py cli-unittest --coverage-report' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["cli-unittest", "--coverage-report"])
        self.assertEqual(result.exit_code, 1)

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_script_tests")
    def test_cli_unittest_1(self, mock_run_script_tests: MagicMock):
        """Test 'fox.py cli-unittest -s --coverage-report' command."""
        runner = CliRunner()
        mock_run_script_tests.return_value = SubprocessResult(0)
        result = runner.invoke(main, ["cli-unittest", "-s", "--coverage-report"])
        self.assertEqual(result.exit_code, 0)

    def test_cli_unittest_2(self):
        """Test 'fox.py cli-unittest' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["cli-unittest"])
        self.assertEqual(result.exit_code, 0)
        self.assertIn("Run unit-tests on the CLI tool itself.", result.stdout)

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_unittest_module")
    def test_cli_unittest_3(self, mock_run_unittest_module: MagicMock):
        """Test 'fox.py cli-unittest discover -s tests/cli' command, i.e.
        arbitrary unittest command."""
        mock_run_unittest_module.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["cli-unittest", "discover", "-s", "tests/cli"])
        self.assertEqual(result.exit_code, 0)
        mock_run_unittest_module.assert_called_once_with(
            ["discover", "-s", "tests/cli"]
        )

    def test_cli_unittest_4(self):
        """Test 'fox.py cli-unittest' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["cli-unittest"])
        self.assertEqual(result.exit_code, 0)


if __name__ == "__main__":
    unittest.main()
