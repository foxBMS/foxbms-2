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


"""Testing file 'cli/foxbms_version.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch


try:
    from cli import foxbms_version
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    from cli import foxbms_version


class TestFoxbmsVersion(unittest.TestCase):
    """TODO"""

    @patch("cli.foxbms_version.re.compile")
    def test_get_version_regex_does_not_match(self, mock_compile):
        """Regex does not match, test 0"""
        mock_regex = MagicMock()
        mock_compile.return_value = mock_regex
        mock_regex.search.return_value = False
        with self.assertRaises(SystemExit) as cm:
            foxbms_version.get_version()
        self.assertEqual(cm.exception.code, "Could not determine foxBMS 2 version.")

    @patch("cli.foxbms_version.Path.read_text")
    def test_get_version_expected_test_found(self, mock_read_text):
        """Regex does match"""
        mock_read_text.return_value = '''VERSION = "x.y.z"'''
        result = foxbms_version.get_version()
        self.assertEqual(result, "x.y.z")

    @patch("cli.foxbms_version.Path.read_text")
    def test_get_version_bad_text(self, mock_read_text):
        """Regex does match"""
        mock_read_text.return_value = '''VERSION = "asdasd"'''
        with self.assertRaises(SystemExit) as cm:
            foxbms_version.get_version()
        self.assertEqual(cm.exception.code, "Could not determine foxBMS 2 version.")


if __name__ == "__main__":
    unittest.main()
