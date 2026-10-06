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

"""Testing file 'tools/waf_tools/validate_test_json.py'."""

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
from validate_test_json import validate_test_configuration  # noqa: E402


class _Node:  # pylint: disable=too-few-public-methods
    """Minimal Waf node exposing the name used by the validator."""

    def __init__(self, name: str) -> None:
        self.name = name


class _Path:
    """Minimal Waf path node exposing the validator's required methods."""

    def __init__(self, node_path: str, test_files: list[str]) -> None:
        self._path = node_path
        self._test_files = test_files
        self.glob_patterns: list[str] = []

    def abspath(self) -> str:
        """Return the path used to group configurations."""
        return self._path

    def ant_glob(self, pattern: str) -> list[_Node]:
        """Return test nodes and record the requested glob pattern."""
        self.glob_patterns.append(pattern)
        return [_Node(name) for name in self._test_files]


class _ConfigNode:  # pylint: disable=too-few-public-methods
    """Minimal Waf node representing a test.json file."""

    def __init__(self, parent: _Path, node_path: str) -> None:
        self.parent = parent
        self._path = node_path

    def path_from(self, _node: object) -> str:
        """Return the repository-relative configuration path."""
        return self._path


class ValidateTestConfigurationTests(unittest.TestCase):
    """Tests for the Waf-bound test configuration validator."""

    def test_empty_configuration_reports_unreferenced_test(self) -> None:
        """Report a test file when its test.json contains no entries."""
        config_path = _Path("tests/unit/app/application/soa", ["test_soa.c"])
        config_node = _ConfigNode(
            config_path, "tests/unit/app/application/soa/test.json"
        )
        context = SimpleNamespace(
            test_config_nodes=[config_node],
            all_test_configs=[],
            test_configs=[],
            srcnode=object(),
            fatal=MagicMock(side_effect=RuntimeError),
        )

        with self.assertRaises(RuntimeError):
            validate_test_configuration(context)

        context.fatal.assert_called_once_with(
            "The following unit test files are not referenced in "
            "tests/unit/app/application/soa/test.json:\n"
            " - test_soa.c\n"
            "Please add them to tests/unit/app/application/soa/test.json "
            "to ensure they are tested."
        )

    def test_configurations_sharing_a_path_are_merged(self) -> None:
        """Accept test files referenced by separate entries in one directory."""
        config_path = _Path("tests/unit/app/example", ["test_alpha.c", "test_beta.c"])
        context = SimpleNamespace(
            test_configs=[
                {"path": config_path, "source": [_Node("test_alpha.c")]},
                {"path": config_path, "source": [_Node("test_beta.c")]},
            ],
            fatal=MagicMock(),
        )

        validate_test_configuration(context)

        context.fatal.assert_not_called()
        self.assertEqual(config_path.glob_patterns, ["test_*.c", "test_*.c"])

    def test_platform_specific_configuration_is_validated(self) -> None:
        """Validate tests excluded from the current platform build."""
        config_path = _Path("tests/unit/bootloader/example", ["test_windows.c"])
        context = SimpleNamespace(
            all_test_configs=[
                {"path": config_path, "test": _Node("test_windows.c")},
            ],
            test_configs=[],
            fatal=MagicMock(),
        )

        validate_test_configuration(context)

        context.fatal.assert_not_called()

    def test_missing_test_file_is_reported(self) -> None:
        """Report an on-disk test that is absent from the resolved sources."""
        config_path = _Path("tests/unit/app/example", ["test_alpha.c", "test_beta.c"])
        context = SimpleNamespace(
            test_configs=[
                {"path": config_path, "source": [_Node("test_alpha.c")]},
            ],
            fatal=MagicMock(side_effect=RuntimeError),
        )

        with self.assertRaises(RuntimeError):
            validate_test_configuration(context)

        context.fatal.assert_called_once_with(
            "The following unit test files are not referenced in "
            "tests/unit/app/example:\n"
            " - test_beta.c\n"
            "Please add them to tests/unit/app/example "
            "to ensure they are tested."
        )

    def test_validator_ignores_non_test_sources(self) -> None:
        """Ignore source files that do not use the test_ prefix."""
        config_path = _Path("tests/unit/app/example", ["test_alpha.c"])
        context = SimpleNamespace(
            test_configs=[
                {
                    "path": config_path,
                    "source": [_Node("helper_test.c"), _Node("test_alpha.c")],
                },
            ],
            fatal=MagicMock(),
        )

        validate_test_configuration(context)

        context.fatal.assert_not_called()


if __name__ == "__main__":
    unittest.main()
