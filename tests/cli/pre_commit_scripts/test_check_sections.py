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

"""Testing file 'cli/pre_commit_scripts/check_sections.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.pre_commit_scripts import check_sections
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.pre_commit_scripts import check_sections

# pylint: disable=line-too-long


class TestCheckSections(unittest.TestCase):
    """Test of the main function"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_valid_file_src_c(self):
        """Test with a valid file"""
        argv = [
            str(self.tests_dir / "valid-sections-src.c"),
            "--file-type",
            "src",
        ]
        result = check_sections.main(argv)
        self.assertEqual(result, 0)

    def test_valid_file_src_h(self):
        """Test with a valid file"""
        argv = [
            str(self.tests_dir / "valid-sections-src.h"),
            "--file-type",
            "src",
        ]
        result = check_sections.main(argv)
        self.assertEqual(result, 0)

    def test_valid_file_test_c(self):
        """Test with a valid file"""
        argv = [
            str(self.tests_dir / "valid-sections-test.c"),
            "--file-type",
            "test",
        ]
        result = check_sections.main(argv)
        self.assertEqual(result, 0)

    def test_valid_file_test_h(self):
        """Test with a valid file"""
        argv = [
            str(self.tests_dir / "valid-sections-test.h"),
            "--file-type",
            "test",
        ]
        result = check_sections.main(argv)
        self.assertEqual(result, 0)

    def test_unknown_file_extension(self):
        """Test with an unknown file extension"""
        argv = [
            str(self.tests_dir / "unknown-file-extension.abc"),
            "--file-type",
            "src",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 1)
        self.assertIn(
            "unknown-file-extension.abc: Unkown file extension '.abc'.", buf.getvalue()
        )

        argv = [
            str(self.tests_dir / "unknown-file-extension.abc"),
            "--file-type",
            "test",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 1)
        self.assertIn(
            "unknown-file-extension.abc: Unkown file extension '.abc'.", buf.getvalue()
        )

    def test_invalid_0(self):
        """TODO"""
        argv = [
            str(self.tests_dir / "invalid-sections-test-0.h"),
            "--file-type",
            "test",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 6)

        for i in [
            r".*invalid-sections-test-0\.h: \/\*========== Includes =======================================================\*\/ is missing\.",
            r".*invalid-sections-test-0\.h: \/\*========== Includes =======================================================\*\/ requires a blank line before the marker\.",
            r".*invalid-sections-test-0\.h: \/\*========== Unit Testing Framework Directives ==============================\*\/ is missing\.",
            r".*invalid-sections-test-0\.h: \/\*========== Unit Testing Framework Directives ==============================\*\/ requires a blank line before the marker\.",
            r".*invalid-sections-test-0\.h: \/\*========== Macros and Definitions =========================================\*\/ is missing\.",
            r".*invalid-sections-test-0\.h: \/\*========== Macros and Definitions =========================================\*\/ requires a blank line before the marker\.",
        ]:
            self.assertRegex(buf.getvalue(), i)

    def test_invalid_1(self):
        """TODO"""
        argv = [
            str(self.tests_dir / "invalid-sections-test-1.h"),
            "--file-type",
            "test",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 2)
        self.assertRegex(
            buf.getvalue(),
            r".*invalid-sections-test-1.h: markers are not in the correct order\.",
        )
        self.assertRegex(
            buf.getvalue(),
            r"invalid-sections-test-1\.h: \/\*========== Includes =======================================================\*\/ occurs more than once\.",
        )

    def test_invalid_c_0(self):
        """TODO"""
        argv = [
            str(self.tests_dir / "invalid-sections-test-0.c"),
            "--file-type",
            "src",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 1)
        self.assertRegex(
            buf.getvalue(),
            r".*invalid-sections-test-0\.c: '#ifdef UNITY_UNIT_TEST' is missing after \/\*========== Externalized Static Function Implementations \(Unit Test\) =======\*\/\.",
        )

    def test_invalid_c_1(self):
        """TODO"""
        argv = [
            str(self.tests_dir / "invalid-sections-test-1.c"),
            "--file-type",
            "src",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 4)

        for i in [
            r"invalid-sections-test-1\.c: \/\*========== Externalized Static Function Implementations \(Unit Test\) =======\*\/ is missing\.",
            r"invalid-sections-test-1\.c: \/\*========== Externalized Static Function Implementations \(Unit Test\) =======\*\/ requires a blank line before the marker\.",
            r"invalid-sections-test-1\.c: markers are not in the correct order.",
        ]:
            self.assertRegex(buf.getvalue(), i)

    def test_invalid_c_2(self):
        """TODO"""
        argv = [
            str(self.tests_dir / "invalid-sections-test-2.h"),
            "--file-type",
            "src",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_sections.main(argv)
        self.assertEqual(result, 1)

        self.assertRegex(
            buf.getvalue(),
            r"invalid-sections-test-2\.h: '#ifdef UNITY_UNIT_TEST' is missing after \/\*========== Externalized Static Functions Prototypes \(Unit Test\) ===========\*\/\.",
        )

    def test_special_case(self):
        """TODO"""
        argv = [
            "src/app/application/config/battery_system_cfg.h",
            "--file-type",
            "src",
        ]
        result = check_sections.main(argv)
        self.assertEqual(result, 0)


if __name__ == "__main__":
    unittest.main()
