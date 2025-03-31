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

"""Testing file 'cli/pre_commit_scripts/check_include_guard.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.pre_commit_scripts import check_include_guard
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import check_include_guard


class TestIncludeGuard(unittest.TestCase):
    """Class to test the functions inside check_include_guard.py"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_main_c_file(self):
        """Test main function with only .c-file"""
        argv = [str(self.tests_dir / "test_file.c")]
        result = check_include_guard.main(argv)
        self.assertEqual(result, 0)

    def test_main_invalid_h_file(self):
        """test main function with empty .h-file"""
        argv = [str(self.tests_dir / "test_file_empty.h")]
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_include_guard.main(argv)
        self.assertEqual(result, 4)
        file_name = (
            "tests/cli/pre_commit_scripts/test_check_include_guard/test_file_empty.h"
        )
        expected_errs = [
            f"{file_name}: #ifndef FOXBMS__TEST_FILE_EMPTY_H_ is missing.",
            f"{file_name}: #define FOXBMS__TEST_FILE_EMPTY_H_ is missing.",
            f"{file_name}: #endif /* FOXBMS__TEST_FILE_EMPTY_H_ */ is missing.",
            f"{file_name}: The pattern needs to be:",
        ]
        for line, expected_err in zip(err.getvalue().splitlines(), expected_errs):
            self.assertIn(expected_err, line)

    def test_main_valid_h_file(self):
        """test main function with valid .h-file"""
        argv = [str(self.tests_dir / "test_file_valid.h")]
        result = check_include_guard.main(argv)
        self.assertEqual(result, 0)

    def test_main_invalid_h_file_multiple(self):
        """test main function with invalid .h-file with one marker twice"""
        argv = [str(self.tests_dir / "test_file_multiple.h")]
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_include_guard.main(argv)
        self.assertEqual(result, 1)
        file_name = (
            "tests/cli/pre_commit_scripts/test_check_include_guard/test_file_multiple.h"
        )
        expected_errs = [
            f"{file_name}: #endif /* FOXBMS__TEST_FILE_MULTIPLE_H_ */ occurs more than once.",
        ]
        for line, expected_err in zip(err.getvalue().splitlines(), expected_errs):
            self.assertIn(expected_err, line)

    def test_main_invalid_h_file_missing(self):
        """test main function with invalid .h-file with missing #define statement"""
        argv = [str(self.tests_dir / "test_file_invalid_content.h")]
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_include_guard.main(argv)
        self.assertEqual(result, 3)
        # pylint: disable-next=line-too-long
        file_name = "tests/cli/pre_commit_scripts/test_check_include_guard/test_file_invalid_content.h"
        expected_errs = [
            f"{file_name}: #define FOXBMS__TEST_FILE_INVALID_CONTENT_H_ is missing.",
            f"{file_name}: markers are not in the correct order.",
            f"{file_name}: The pattern needs to be:",
            "",
            "#ifndef FOXBMS__TEST_FILE_INVALID_CONTENT_H_",
            "#define FOXBMS__TEST_FILE_INVALID_CONTENT_H_",
        ]
        for line, expected_err in zip(err.getvalue().splitlines(), expected_errs):
            self.assertIn(expected_err, line)


if __name__ == "__main__":
    unittest.main()
