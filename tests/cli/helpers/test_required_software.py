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


# cspell:ignore creationflags

"""Testing file 'cli/helpers/required_software.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.helpers import required_software
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import required_software


class TestRequiredSoftware(unittest.TestCase):
    """Test of 'required_software.py'."""

    def test_get_tool_executable_name_universal_name(self) -> None:
        """Basic get_tool_executable_name test"""
        test_dict = {
            "foo": required_software.ToolDefinition(
                {"executable": "foo", "path": False}
            )
        }
        ret = required_software.get_tool_executable_name(test_dict["foo"])
        self.assertEqual("foo", ret)

    @patch("cli.helpers.required_software.get_platform")
    def test_get_tool_executable_name_platform_specific_name(
        self, mock_get_platform: MagicMock
    ) -> None:
        """Basic get_tool_executable_name test"""
        mock_get_platform.return_value = "win32"
        test_dict = {
            "foo": required_software.ToolDefinition(
                {"executable": {"win32": "foo", "linux": "bar"}, "path": False}
            )
        }
        ret = required_software.get_tool_executable_name(test_dict["foo"])
        self.assertEqual("foo", ret)


if __name__ == "__main__":
    unittest.main()
