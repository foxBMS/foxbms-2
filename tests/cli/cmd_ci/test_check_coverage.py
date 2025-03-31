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

"""Testing file 'cli/cmd_ci/check_coverage.py'."""

import importlib
import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import patch

try:
    from cli.cmd_ci import check_coverage
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_ci import check_coverage


class TestCheckCoverage(unittest.TestCase):
    """Test of 'check_coverage.py'"""

    def setUp(self):
        importlib.reload(check_coverage)
        return super().setUp()

    def test_platform_linux(self):
        """test setting of Certificate_File on Linux."""
        with patch("sys.platform", new="linux"):
            importlib.reload(check_coverage)
            self.assertEqual(
                check_coverage.CERTIFICATE_FILE, "/etc/ssl/certs/ca-bundle.crt"
            )

    def test_platform_win32(self):
        """test setting of Certificate_File on Linux."""
        with patch("sys.platform", new="win32"):
            importlib.reload(check_coverage)
            self.assertEqual(check_coverage.CERTIFICATE_FILE, "")

    def test_check_coverage_no_project(self):
        """test comparing reports, without providing a project"""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            check_coverage.check_coverage()
        self.assertEqual(cm.exception.code, 1)
        self.assertRegex(
            buf.getvalue(),
            r"File '.*GcovCoverageCobertura\.xml' does not exist.",
        )

    def test_check_coverage_invalid_project(self):
        """test comparing reports, without providing a valid project"""
        project = "asdf"
        with self.assertRaises(SystemExit) as cm:
            check_coverage.check_coverage(project)
        self.assertEqual(
            cm.exception.code,
            "Something went wrong.\n"
            "Expect argument ('app', 'bootloader') but got 'asdf'.",
        )


if __name__ == "__main__":
    unittest.main()
