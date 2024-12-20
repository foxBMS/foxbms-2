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

"""Testing file 'cli/pre_commit_scripts/check_include_guard.py'."""

import sys
import unittest
from pathlib import Path

try:
    from cli.pre_commit_scripts import check_include_guard
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.pre_commit_scripts import check_include_guard


class TestIncludeGuard(unittest.TestCase):
    """Class to test the functions inside check_include_guard.py"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_main_c_file(self):
        """Test main function with only .c-file"""
        # Prepare test parameters
        argv = [str(self.tests_dir / "test_file.c")]
        # Call the function under test
        result = check_include_guard.main(argv)
        # Assertions
        self.assertEqual(result, 0)

    def test_main_invalid_h_file(self):
        """test main function with empty .h-file"""
        # Prepare test parameters
        argv = [str(self.tests_dir / "test_file_empty.h")]
        # Call the function under test
        result = check_include_guard.main(argv)
        # Assertions
        self.assertEqual(result, 4)

    def test_main_valid_h_file(self):
        """test main function with valid .h-file"""
        # Prepare test parameters
        argv = [str(self.tests_dir / "test_file_valid.h")]
        # Call the function under test
        result = check_include_guard.main(argv)
        # Assertions
        self.assertEqual(result, 0)

    def test_main_invalid_h_file_multiple(self):
        """test main function with invalid .h-file with one marker twice"""
        # Prepare test parameters
        argv = [str(self.tests_dir / "test_file_multiple.h")]
        # Call the function under test
        result = check_include_guard.main(argv)
        # Assertions
        self.assertEqual(result, 1)

    def test_main_invalid_h_file_missing(self):
        """test main function with invalid .h-file with missing #define statement"""
        # Prepare test parameters
        argv = [str(self.tests_dir / "test_file_invalid_content.h")]
        # Call the function under test
        result = check_include_guard.main(argv)
        # Assertions
        self.assertEqual(result, 3)


if __name__ == "__main__":
    unittest.main()
