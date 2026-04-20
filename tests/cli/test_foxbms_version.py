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

"""Testing file 'cli/foxbms_version.py'."""

import re
import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli import foxbms_version
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    from cli import foxbms_version


class TestExtractVersion(unittest.TestCase):
    """Tests the 'extract_version' function"""

    def test_no_match(self):
        """Regex does not match"""
        txt = "Line 1\nLine 2\nLine 3"
        mock_pattern = MagicMock()
        mock_pattern.search.return_value = None
        with self.assertRaises(SystemExit) as cm:
            foxbms_version.extract_version(txt, mock_pattern)
        self.assertEqual(cm.exception.code, "Could not determine foxBMS 2 version.")

    def test_version_found(self):
        """Regex matches"""
        txt = """Text 2\nLine 5\nLine 7 2"""
        result = foxbms_version.extract_version(
            txt, re.compile(r"Line (\d{1,} \d{1,})")
        )
        self.assertEqual(result, "7 2")


@patch("cli.foxbms_version.extract_version")
class TestGetVersion(unittest.TestCase):
    """Tests the 'get_version' function"""

    @patch("cli.foxbms_version.Path.read_text")
    @patch("cli.foxbms_version.re.compile")
    def test_no_error(
        self,
        mock_compile: MagicMock,
        mock_read_text: MagicMock,
        mock_extract_version: MagicMock,
    ):
        """Wscript file exists"""
        txt = '''VERSION = "x.y.z"'''
        mock_read_text.return_value = txt
        regex = "pattern"
        mock_compile.return_value = regex
        foxbms_version.get_version()
        mock_extract_version.assert_called_once_with(txt, regex)

    @patch("cli.foxbms_version.Path.read_text")
    @patch("cli.foxbms_version.re.compile")
    def test_file_not_found_error(
        self,
        mock_compile: MagicMock,
        mock_read_text: MagicMock,
        mock_extract_version: MagicMock,
    ):
        """Wscript file does not exist"""
        txt = '''VERSION = "x.y.z"'''
        mock_read_text.side_effect = [FileNotFoundError, txt]
        regex = "pattern"
        mock_compile.return_value = regex
        foxbms_version.get_version()
        mock_extract_version.assert_called_once_with(txt, regex)


@patch("cli.foxbms_version.get_version")
class TestGetNumericVersion(unittest.TestCase):
    """Tests the 'get_numeric_version' function"""

    def test_letters(self, mock_get_version: MagicMock):
        """Test converting from letters to numbers"""
        mock_get_version.return_value = "x.y.z"
        result = foxbms_version.get_numeric_version()
        self.assertEqual(result, "120.121.122")

    def test_numbers(self, mock_get_version: MagicMock):
        """Test function when version is given with numbers"""
        mock_get_version.return_value = "1.2.3"
        result = foxbms_version.get_numeric_version()
        self.assertEqual(result, "1.2.3")


if __name__ == "__main__":
    unittest.main()
