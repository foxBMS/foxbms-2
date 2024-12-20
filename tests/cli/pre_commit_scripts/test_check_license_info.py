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

"""Testing file 'cli/pre_commit_scripts/check_license_info.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.pre_commit_scripts import check_license_info
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.pre_commit_scripts import check_license_info

TYPE_TO_EXT = {
    "asm": "asm",
    "batch": "bat",
    "c": "c",
    "dot": "dot",
    "pwsh": "ps1",
    "py": "py",
    "shell": "sh",
    "toml": "toml",
    "yaml": "yml",
}


class TestCheckLicenseInfo(unittest.TestCase):
    """Test of the main function"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_valid_files(self):
        """Test with valid files"""
        for i in ["asm", "batch", "c", "dot", "pwsh", "py", "shell", "toml", "yaml"]:
            argv = [
                str(self.tests_dir / f"valid-license.{TYPE_TO_EXT[i]}"),
                "--file-type",
                i,
                "--license-type",
                "BSD-3-Clause",
            ]
            result = check_license_info.main(argv)
            self.assertEqual(result, 0)

    def test_invalid_license(self):
        """Test with an invalid file"""
        test = self.tests_dir / "invalid-license.c"
        argv = [
            str(test),
            "--file-type",
            "c",
            "--license-type",
            "BSD-3-Clause",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_license_info.main(argv)
        self.assertEqual(result, 1)
        self.assertIn(
            "Line 6: Expected: ' * SPDX-License-Identifier: BSD-3-Clause'\n"
            "Line 6: Actual:   ' * some text'",
            buf.getvalue(),
        )

    def test_missing_license_information(self):
        """Test with an file that misses license information"""
        test = self.tests_dir / "no-license.c"
        argv = [
            str(test),
            "--file-type",
            "c",
            "--license-type",
            "BSD-3-Clause",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_license_info.main(argv)
        self.assertEqual(result, 2)


if __name__ == "__main__":
    unittest.main()
