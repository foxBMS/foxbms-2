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

"""Testing file 'cli/cmd_embedded_ut/embedded_ut_impl.py'."""

import sys
import unittest
from pathlib import Path

try:
    from cli.cmd_embedded_ut import embedded_ut_impl
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_embedded_ut import embedded_ut_impl


class TestSpR(unittest.TestCase):
    """Test of the main entry point"""

    @unittest.skipIf(not sys.platform.startswith("win"), "Windows specific test")
    def test_win32(self):
        """TODO"""
        self.assertIn(
            "app_project_win32.yml", str(embedded_ut_impl.CEEDLING_PROJECT_FILE_SRC_APP)
        )
        self.assertIn(
            "bootloader_project_win32.yml",
            str(embedded_ut_impl.CEEDLING_PROJECT_FILE_SRC_BL),
        )

    @unittest.skipIf(not sys.platform.startswith("linux"), "Linux specific test")
    def test_linux(self):
        """TODO"""
        self.assertIn(
            "app_project_posix.yml", str(embedded_ut_impl.CEEDLING_PROJECT_FILE_SRC_APP)
        )
        self.assertIn(
            "bootloader_project_posix.yml",
            str(embedded_ut_impl.CEEDLING_PROJECT_FILE_SRC_BL),
        )

    def test_make_unit_test_dir(self):
        """Test directory creation"""
        embedded_ut_impl.make_unit_test_dir("app")
        embedded_ut_impl.make_unit_test_dir("bootloader")
        with self.assertRaises(SystemExit) as cm:
            embedded_ut_impl.make_unit_test_dir("dummy")
        self.assertEqual(
            cm.exception.code,
            "Something went wrong.\nExpect argument from list ('app', "
            "'bootloader'), but got 'dummy'.",
        )

    def test_run_embedded_tests(self):
        """Run the actual test function"""
        with self.assertRaises(SystemExit) as cm:
            embedded_ut_impl.run_embedded_tests(project="dummy")
        self.assertEqual(
            cm.exception.code,
            "Something went wrong.\nExpect argument from list ('app', "
            "'bootloader'), but got 'dummy'.",
        )

        result = embedded_ut_impl.run_embedded_tests()
        self.assertEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
