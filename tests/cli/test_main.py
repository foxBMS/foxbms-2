#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""CLI testing"""

import sys
import unittest
from pathlib import Path

from click.testing import CliRunner

sys.path.insert(0, str(Path(__file__).parent.parent.parent))
# pylint: disable=wrong-import-position
from cli.cli import main
from cli.helpers.misc import PROJECT_ROOT

# pylint: enable=wrong-import-position


class TestFoxCliMain(unittest.TestCase):
    """Test of the main entry point"""

    def test_main_no_args(self):
        """test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main)
        self.assertEqual(0, result.exit_code)

    def test_main_waf(self):
        """test main entry point, when 'waf' commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main, ["waf", "--help"])
        self.assertEqual(0, result.exit_code)

        runner = CliRunner()
        result = runner.invoke(main, ["waf", "--cwd", str(PROJECT_ROOT), "--help"])
        self.assertEqual(0, result.exit_code)

    def test_misc_build_crc_code(self):
        """build CRC example code"""
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "build-crc-code"])
        self.assertEqual(0, result.exit_code)

    def test_misc_build_doc_code(self):
        """build CRC example code"""
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "build-doc-code"])
        self.assertEqual(0, result.exit_code)

    def test_install(self):
        """check installation message"""
        runner = CliRunner()
        result = runner.invoke(main, ["install"])
        self.assertEqual(0, result.exit_code)

    def test_install_check(self):
        """check installation '--check'-option"""
        runner = CliRunner()
        result = runner.invoke(main, ["install", "--check"])
        self.assertEqual(0, result.exit_code)

    def test_ci_create_readme(self):
        """check CI readme creation"""
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "create-readme"])
        self.assertEqual(0, result.exit_code)

    def test_ci_check_ci_config(self):
        """check CI configuration style"""
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "check-ci-config"])
        self.assertEqual(0, result.exit_code)

    def test_ci_check_coverage(self):
        """check CI coverage checker"""
        runner = CliRunner()
        runner.invoke(main, ["ci", "check-coverage"])
        # ignore the result (i.e., no assert), as the function will be tested
        # anyway


if __name__ == "__main__":
    unittest.main()
