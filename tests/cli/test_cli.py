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

"""Testing file 'cli/cli.py'."""

import importlib
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

from click.testing import CliRunner

try:
    from cli import cli
    from cli.foxbms_version import __version__
    from cli.helpers.package_helpers import PACKAGE_COMMANDS
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    from cli import cli
    from cli.foxbms_version import __version__
    from cli.helpers.package_helpers import PACKAGE_COMMANDS


@patch("cli.helpers.misc.ROOT_IS_PROJECT", new=True)
class TestFoxCliMain(unittest.TestCase):
    """Test of the main entry point, when it is in the foxBMS project"""

    @classmethod
    def setUpClass(cls):
        importlib.reload(cli)

    def test_get_program_config(self):
        """Test '--show-config' option"""
        foxbms_config = cli.get_program_config()
        self.assertEqual({"foxBMS 2": __version__}, foxbms_config)

    def test_main_no_args(self):
        """Test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(cli.main)
        self.assertEqual(0, result.exit_code)

    def test_main_show_config(self):
        """Test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(cli.main, ["--show-config"])
        self.assertEqual(0, result.exit_code)

    def test_install(self):
        """Check installation message"""
        runner = CliRunner()
        result = runner.invoke(cli.main, ["install"])
        self.assertEqual(0, result.exit_code)

    def test_install_check(self):
        """Check installation '--check'-option"""
        runner = CliRunner()
        runner.invoke(cli.main, ["install", "--check"])
        # the exit code may vary depending on the actually installed tools
        # therefore we can not check the exit code of this command; as long as
        # no exception is raised, everything is fine.

    def test_no_project(self):
        """Check main entry point, when it is not a foxBMS project"""
        runner = CliRunner()
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=False):
            importlib.reload(cli)
            result = runner.invoke(cli.main, ["--help"])
            self.assertEqual(0, result.exit_code)
            for command in PACKAGE_COMMANDS["unsupported"]:
                self.assertTrue((" " + command + " ") not in result.output)
            for command in PACKAGE_COMMANDS["supported"]:
                self.assertTrue((" " + command + " ") in result.output)


class TestFoxCliMainProject(unittest.TestCase):
    """Test configuration of the main entry point"""

    def test_no_project(self):
        """Check configuration when ROOT_IS_PROJECT is False"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=False):
            importlib.reload(cli)
            command_names = cli.main.commands
            for command in PACKAGE_COMMANDS["unsupported"]:
                self.assertTrue(command not in command_names)
            for command in PACKAGE_COMMANDS["supported"]:
                self.assertTrue(command in command_names)

    def test_project(self):
        """Check configuration ROOT_IS_PROJECT is True"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=True):
            importlib.reload(cli)
            command_names = cli.main.commands
            for command in PACKAGE_COMMANDS["unsupported"]:
                if command != "ci":  # command is hidden
                    self.assertTrue(command in command_names)
            for command in PACKAGE_COMMANDS["supported"]:
                self.assertTrue(command in command_names)


if __name__ == "__main__":
    unittest.main()
