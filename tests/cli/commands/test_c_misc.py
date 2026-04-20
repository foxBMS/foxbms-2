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

"""Testing file 'cli/commands/c_misc.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

from click.testing import CliRunner

try:
    from cli.cli import main
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cli import main


class TestFoxCliMainCommandMisc(unittest.TestCase):
    """Test of the 'misc' commands and options."""

    def test_misc_without_subcommand_prints_help(self):
        """Test 'fox.py misc' command shows help text."""
        runner = CliRunner()
        result = runner.invoke(main, ["misc"])
        self.assertEqual(0, result.exit_code)
        self.assertIn("Usage:", result.stdout)

    @patch("cli.commands.c_misc.get_prefixes")
    def test_misc_list_prefixes(self, mock_get_prefixes: MagicMock):
        """Test 'fox.py misc --list-prefixes' command."""
        mock_get_prefixes.return_value = ["ABC", "DEF"]
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "--list-prefixes"])
        self.assertEqual(0, result.exit_code)
        self.assertEqual("ABC\nDEF\n", result.stdout)

    @patch("cli.commands.c_misc.lint_freertos")
    def test_misc_uncrustify_freertos(self, mock_lint_freertos: MagicMock):
        """Build documentation example code"""
        mock_lint_freertos.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "uncrustify-freertos"])
        self.assertEqual(0, result.exit_code)

    def test_misc_verify_checksum_no_files(self):
        """Build documentation example code"""
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "verify-checksum", "xyz"])
        self.assertEqual(1, result.exit_code)
        self.assertIn("No files provided.", result.stderr)

    @patch("cli.commands.c_misc.verify")
    def test_misc_verify_checksum(self, mock_verify: MagicMock):
        """Build documentation example code"""
        mock_verify.return_value = 0
        runner = CliRunner()
        result = runner.invoke(
            main, ["misc", "verify-checksum", str(Path(__file__).parent), "xyz"]
        )
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_misc.check_repository_depth")
    def test_check_repository_depth(self, mock_check_repository_depth: MagicMock):
        """Test 'fox.py misc check-repository-depth' command."""
        mock_check_repository_depth.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "check-repository-depth"])
        self.assertEqual(0, result.exit_code)


if __name__ == "__main__":
    unittest.main()
