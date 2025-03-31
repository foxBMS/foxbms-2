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

"""Testing file 'cli/commands/c_axivion.py'."""

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


class TestFoxCliMainCommandAxivion(unittest.TestCase):
    """Test of the 'axivion' commands and options."""

    def test_axivion_0(self):
        """Test 'fox.py axivion' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["axivion"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.check_versions")
    def test_axivion_1(self, mock_check_versions: MagicMock):
        """Test 'fox.py axivion --check-versions' command."""
        mock_check_versions.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "--check-versions"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.self_test")
    def test_axivion_2(self, mock_self_test):
        """Test 'fox.py axivion self-test' command."""
        mock_self_test.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "self-test"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.export_architecture")
    def test_axivion_3(self, mock_export_architecture):
        """Test 'fox.py axivion export-architecture' command."""
        mock_export_architecture.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "export-architecture"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.check_if_architecture_up_to_date")
    def test_axivion_4(self, mock_check_if_architecture_up_to_date):
        """Test 'fox.py axivion check-architecture-up-to-date' command."""
        mock_check_if_architecture_up_to_date.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "check-architecture-up-to-date"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.check_violations")
    def test_axivion_5(self, mock_check_violations):
        """Test 'fox.py axivion check-violations' command."""
        mock_check_violations.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "check-violations"])
        self.assertEqual(1, result.exit_code)  # no report provided

    @patch("cli.cmd_axivion.axivion_impl.combine_report_files")
    def test_axivion_6(self, mock_combine_report_files: MagicMock):
        """Test 'fox.py axivion combine-reports' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "combine-reports"])
        self.assertEqual(1, result.exit_code)  # no reports provided

        mock_combine_report_files.return_value = 0
        runner = CliRunner()
        # provide any existing file as we are mocking the function
        result = runner.invoke(main, ["axivion", "combine-reports", __file__])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.run_local_analysis")
    def test_axivion_7(self, mock_local_analysis: MagicMock):
        """Test 'fox.py axivion local-analysis' command."""
        mock_local_analysis.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "local-analysis"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.start_local_dashserver")
    def test_axivion_8(self, mock_start_local_dashserver: MagicMock):
        """Test 'fox.py axivion local-dashserver' command."""
        mock_start_local_dashserver.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "local-dashserver"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.cmd_axivion.axivion_impl.make_race_pdfs")
    def test_axivion_9(self, mock_make_race_pdfs: MagicMock):
        """Test 'fox.py axivion make-race-pdfs' command."""
        mock_make_race_pdfs.return_value = SubprocessResult(0)
        runner = CliRunner()
        result = runner.invoke(main, ["axivion", "make-race-pdfs"])
        self.assertEqual(0, result.exit_code)


if __name__ == "__main__":
    unittest.main()
