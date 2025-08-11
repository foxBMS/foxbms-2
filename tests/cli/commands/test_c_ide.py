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

"""Testing file 'cli/commands/c_ide.py'."""

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


class TestFoxCliMainCommandIde(unittest.TestCase):
    """Test of the 'ide' commands and options."""

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_0(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide' command."""
        mock_ide_impl.open_ide_generic.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ide"])
        self.assertEqual(0, result.exit_code)
        mock_ide_impl.open_ide_generic.assert_called_once()

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_1(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide --no-generic' command."""
        mock_ide_impl.mock_ide_impl.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ide", "--no-generic"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_2(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide --app --no-generic' command."""
        mock_ide_impl.open_ide_app.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ide", "--app", "--no-generic"])
        self.assertEqual(0, result.exit_code)
        mock_ide_impl.open_ide_app.assert_called_once()

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_cli(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide --cli --no-generic' command."""
        mock_ide_impl.open_ide_cli.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ide", "--cli", "--no-generic"])
        self.assertEqual(0, result.exit_code)
        mock_ide_impl.open_ide_cli.assert_called_once()

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_3(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide --bootloader --no-generic' command."""
        mock_ide_impl.open_ide_bootloader.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["ide", "--bootloader", "--no-generic"])
        self.assertEqual(0, result.exit_code)
        mock_ide_impl.open_ide_bootloader.assert_called_once()

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_4(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide --embedded-unit-test-app --no-generic' command."""
        mock_ide_impl.open_ide_embedded_unit_test_app.return_value = 0
        runner = CliRunner()
        result = runner.invoke(
            main, ["ide", "--embedded-unit-test-app", "--no-generic"]
        )
        self.assertEqual(0, result.exit_code)
        mock_ide_impl.open_ide_embedded_unit_test_app.assert_called_once()

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide_5(self, mock_ide_impl: MagicMock):
        """Test 'fox.py ide --embedded-unit-test-bootloader --no-generic' command."""
        mock_ide_impl.open_ide_embedded_unit_test_bootloader.return_value = 0
        runner = CliRunner()
        result = runner.invoke(
            main, ["ide", "--embedded-unit-test-bootloader", "--no-generic"]
        )
        self.assertEqual(0, result.exit_code)
        mock_ide_impl.open_ide_embedded_unit_test_bootloader.assert_called_once()


if __name__ == "__main__":
    unittest.main()
