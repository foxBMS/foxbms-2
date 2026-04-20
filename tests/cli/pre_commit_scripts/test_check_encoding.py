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

"""Testing file 'cli/pre_commit_scripts/check_encoding.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import patch

try:
    from cli.pre_commit_scripts import check_encoding
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import check_encoding


class TestCheckEncoding(unittest.TestCase):
    """Test of the main function"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_main_ascii(self):
        """Testing ASCII"""
        test = "ascii.txt"
        result = check_encoding.main([str(self.tests_dir / test)])
        self.assertEqual(result, 0)

        test = "utf-8.txt"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_encoding.main([str(self.tests_dir / test)])
        self.assertEqual(result, 0)

        test = "utf-16.txt"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_encoding.main([str(self.tests_dir / test)])
        self.assertEqual(result, 1)
        self.assertEqual(
            err.getvalue(),
            f"{(self.tests_dir / test).as_posix()}: Could not open file in 'utf-8' mode.\n",
        )

    def test_main_utf_8(self):
        """Testing UTF-8"""
        test = "ascii.txt"
        result = check_encoding.main([str(self.tests_dir / test)])
        self.assertEqual(result, 0)

        test = "utf-8.txt"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_encoding.main([str(self.tests_dir / test)])
        self.assertEqual(result, 0)

        test = "utf-16.txt"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_encoding.main([str(self.tests_dir / test)])
        self.assertEqual(result, 1)
        self.assertEqual(
            err.getvalue(),
            f"{(self.tests_dir / test).as_posix()}: Could not open file in 'utf-8' mode.\n",
        )

    def test_main_utf_16self_test(self):
        """Self test"""
        fox = (self.tests_dir / "utf-16.txt").read_text(encoding="utf-16")
        self.assertEqual(fox, "🦊 foxBMS\n")

    @patch("pathlib.Path.read_text")
    def test_main_uses_encoding_map_by_extension(self, mock_read_text):
        """Known suffixes should use ENCODING_MAP values."""
        mock_read_text.return_value = "ok"
        file_c = self.tests_dir / "file.c"
        result = check_encoding.main([str(file_c)])
        self.assertEqual(result, 0)
        mock_read_text.assert_called_once_with(encoding="ascii")

    @patch("pathlib.Path.read_text")
    def test_main_uses_utf8_fallback_for_unknown_suffix(self, mock_read_text):
        """Unknown suffixes should fall back to UTF-8."""
        mock_read_text.return_value = "ok"
        file_txt = self.tests_dir / "file.txt"
        result = check_encoding.main([str(file_txt)])
        self.assertEqual(result, 0)
        mock_read_text.assert_called_once_with(encoding="utf-8")


if __name__ == "__main__":
    unittest.main()
