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

"""Testing file 'cli/commands/c_ci.py'."""

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


class TestFoxCliMainCommandCi(unittest.TestCase):
    """Test of the 'ci' commands and options."""

    @patch("cli.commands.c_ci.create_readme")
    def test_ci_create_readme(self, mock_readme: MagicMock):
        """Test 'fox.py ci create-readme' command."""
        mock_readme.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "create-readme"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_ci.check_ci_config")
    def test_ci_check_ci_config(self, mock_check_ci_config: MagicMock):
        """Test 'fox.py ci check-ci-config' command."""
        mock_check_ci_config.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "check-ci-config"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_ci.check_coverage")
    def test_ci_check_coverage(self, mock_check_coverage: MagicMock):
        """Test 'fox.py ci check-coverage' command."""
        mock_check_coverage.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "check-coverage"])
        self.assertEqual(0, result.exit_code)

    def test_ci_path_shall_not_exist(self):
        """Test 'fox.py ci path-shall-not-exist' command."""
        # path exists
        runner = CliRunner(mix_stderr=False)
        result = runner.invoke(main, ["ci", "path-shall-not-exist", __file__])
        self.assertEqual(1, result.exit_code)
        self.assertRegex(result.stderr, r"Path '.*test_c_ci.py' exists.")

        # path does not exist
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "path-shall-not-exist", "abc/def"])
        self.assertEqual(0, result.exit_code)


if __name__ == "__main__":
    unittest.main()
