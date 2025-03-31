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

"""Testing file 'cli/cli.py'."""

import sys
import unittest
from pathlib import Path

from click.testing import CliRunner

try:
    from cli.cli import get_program_config, main
    from cli.foxbms_version import __version__
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    from cli.cli import get_program_config, main
    from cli.foxbms_version import __version__


class TestFoxCliMain(unittest.TestCase):
    """Test of the main entry point"""

    def test_get_program_config(self):
        """test '--show-config' option"""
        foxbms_config = get_program_config()
        self.assertEqual({"foxBMS 2": __version__}, foxbms_config)

    def test_main_no_args(self):
        """test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main)
        self.assertEqual(0, result.exit_code)

    def test_main_show_config(self):
        """test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main, ["--show-config"])
        self.assertEqual(0, result.exit_code)

    def test_install(self):
        """check installation message"""
        runner = CliRunner()
        result = runner.invoke(main, ["install"])
        self.assertEqual(0, result.exit_code)

    def test_install_check(self):
        """check installation '--check'-option"""
        runner = CliRunner()
        runner.invoke(main, ["install", "--check"])
        # the exit code may vary depending on the actually installed tools
        # therefore we can not check the exit code of this command; as long as
        # no exception is raised, everything is fine.


if __name__ == "__main__":
    unittest.main()
