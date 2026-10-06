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

"""Testing file 'tools/waf_tools/config_validate_utils.py'."""

import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import MagicMock

ROOT = Path(__file__).parents[2]
for path in [
    ROOT / "tools/waf_tools",
    ROOT / "tools/waf3-2.1.9-beba77c244731800bf15a003232e7040",
    ROOT / "tools/.waf3-2.1.9-beba77c244731800bf15a003232e7040",
]:
    if path.exists():
        sys.path.insert(0, str(path))

# pylint: disable-next=wrong-import-position
from config_validate_utils import read_json_from_node  # noqa: E402


class ReadJsonFromNodeTests(unittest.TestCase):
    """Tests for 'read_json_from_node()'."""

    def test_reads_json_from_node(self) -> None:
        """Return JSON read from a supplied Waf node."""
        node = MagicMock()
        node.read_json.return_value = {"test": "value"}
        build = SimpleNamespace()

        self.assertEqual(read_json_from_node(build, node), {"test": "value"})
        node.read_json.assert_called_once_with()

    def test_resolves_string_path_before_reading(self) -> None:
        """Resolve a string path through the build source path."""
        node = MagicMock()
        node.read_json.return_value = {"test": "value"}
        build = SimpleNamespace(
            path=SimpleNamespace(find_node=MagicMock(return_value=node))
        )

        self.assertEqual(read_json_from_node(build, "test.json"), {"test": "value"})
        build.path.find_node.assert_called_once_with("test.json")

    def test_missing_file_is_reported(self) -> None:
        """Call the build fatal handler when a file cannot be resolved."""
        fatal = MagicMock(side_effect=RuntimeError)
        build = SimpleNamespace(
            path=SimpleNamespace(find_node=MagicMock(return_value=None)), fatal=fatal
        )

        with self.assertRaises(RuntimeError):
            read_json_from_node(build, "missing.json")
        fatal.assert_called_once_with("Could not find file missing.json")


if __name__ == "__main__":
    unittest.main()
