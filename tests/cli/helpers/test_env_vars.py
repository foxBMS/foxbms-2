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

"""Testing file 'cli/helpers/env_vars.py'."""

import importlib
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

try:
    from cli.helpers import env_vars, host_platform
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.helpers import env_vars, host_platform


class TestWinVars(unittest.TestCase):
    """Test setting of default values for environment variables."""

    @patch("sys.platform", "linux")
    @patch("os.environ.get", return_value="USER")
    def test_platform_linux(self, _):
        """test setting of values on Linux."""
        importlib.reload(host_platform)
        importlib.reload(env_vars)

        # Test that parameters have been set to the patched value
        self.assertEqual(env_vars.USERNAME, "USER")
        self.assertEqual(env_vars.USERPROFILE, "USER")
        self.assertEqual(env_vars.PROGRAMFILES, "USER")
        self.assertEqual(env_vars.PROGRAMFILESX86, "USER")
        self.assertEqual(env_vars.LOCALAPPDATA, "USER")

    @patch("sys.platform", "linux")
    @patch("os.environ.get", return_value="")
    def test_linux_username(self, _):
        """test setting of 'USERNAME' if os.environ.get("USER", "") returns empty."""
        importlib.reload(host_platform)
        importlib.reload(env_vars)

        self.assertEqual(env_vars.USERNAME, "")

    @patch("sys.platform", "win32")
    @patch("os.environ.get", return_value="USER")
    def test_platform_win32(self, _):
        """test setting of values on Windows."""
        # Patch return value of os.environ.get with a default value
        importlib.reload(host_platform)
        importlib.reload(env_vars)

        # Test that parameters have been set to the patched value
        self.assertEqual(env_vars.USERNAME, "USER")
        self.assertEqual(env_vars.HOMEDRIVE, "USER\\")
        self.assertEqual(env_vars.USERPROFILE, "USER")
        self.assertEqual(env_vars.PROGRAMFILES, "USER")
        self.assertEqual(env_vars.PROGRAMFILESX86, "USER")
        self.assertEqual(env_vars.LOCALAPPDATA, "USER")


if __name__ == "__main__":
    unittest.main()
