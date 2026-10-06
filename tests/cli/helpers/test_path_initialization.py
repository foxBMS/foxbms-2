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

"""Testing file 'cli/helpers/path_initialization.py'."""

# pylint: disable=protected-access

import importlib
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.helpers import host_platform, path_initialization, project_context

except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import host_platform, path_initialization, project_context


class TestPathInitialization(unittest.TestCase):
    """Tests for functions in 'path_initialization.py'."""

    def setUp(self) -> None:
        """Patch logger to avoid side effects from log output in unit tests."""
        self._logger_patcher = patch.object(path_initialization, "logger")
        self._logger_patcher.start()
        self.addCleanup(self._logger_patcher.stop)

    def test_remove_duplicates_preserves_order(self) -> None:
        """Remove duplicates while preserving first-seen order."""
        result = path_initialization._remove_duplicates(["a", "b", "a", "c", "b"])
        self.assertEqual(result, ["a", "b", "c"])

    def test_filter_and_normalize_path_entries_filters_and_strips(self) -> None:
        """Filter excluded and empty entries and strip trailing separators."""
        keep_with_sep = f"keep{path_initialization.os.sep}"
        result = path_initialization._filter_and_normalize_path_entries(
            ["", "some/WindowsApps", "my-conda-bin", keep_with_sep, "other"]
        )
        self.assertEqual(result, ["keep", "other"])

    @patch.dict(
        path_initialization.os.environ, {"PATH": f"one{os.pathsep}two"}, clear=True
    )
    def test_get_environment_path_entries_uses_environment(self) -> None:
        """Split PATH from the environment using the platform separator."""
        result = path_initialization._get_environment_path_entries()
        self.assertEqual(result, ["one", "two"])

    @patch.object(path_initialization, "PATH_FILE")
    def test_prepend_existing_foxbms_path_entries_only_existing_dirs(
        self, mock_path_file: MagicMock
    ) -> None:
        """Prepend only existing directories from the configured path file."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            existing_1 = tmp / "existing_1"
            existing_2 = tmp / "existing_2"
            missing = tmp / "missing"
            existing_1.mkdir()
            existing_2.mkdir()

            paths_file = tmp / "paths.txt"
            paths_file.write_text(
                "\n".join([str(existing_1), str(missing), str(existing_2)]),
                encoding="utf-8",
            )
            mock_path_file.read_text.return_value = paths_file.read_text(
                encoding="utf-8"
            )
            result = path_initialization._prepend_existing_foxbms_path_entries(["base"])

        self.assertEqual(result, [str(existing_1), str(existing_2), "base"])

    @patch.object(path_initialization, "which", return_value="C:/tool/python.exe")
    @patch.object(
        path_initialization, "get_tool_executable_name", return_value="python"
    )
    @patch.object(path_initialization, "REQUIRED_SOFTWARE", {"python": {}})
    def test_remove_conflicting_required_tool_entries_keeps_never_remove(
        self, *_mocks: MagicMock
    ) -> None:
        """Keep entries that include never-remove prefixes even if tools are found."""
        path_entries = ["C:/Windows/System32", "C:/toolchain/bin"]
        result = path_initialization._remove_conflicting_required_tool_entries(
            path_entries
        )

        self.assertEqual(result, ["C:/Windows/System32"])

    @patch.object(
        path_initialization,
        "which",
        return_value="C:/foxbms/envs/current/Scripts/python.exe",
    )
    @patch.object(
        path_initialization, "get_tool_executable_name", return_value="python"
    )
    @patch.object(path_initialization, "REQUIRED_SOFTWARE", {"python": {}})
    @patch.object(path_initialization, "ENV_DIR", Path("C:/foxbms/envs/current"))
    def test_remove_conflicting_required_tool_entries_keeps_env_python(
        self, *_mocks: MagicMock
    ) -> None:
        """Keep python entries that point into the configured foxBMS environment."""
        path_entries = ["C:/foxbms/envs/current/Scripts"]
        result = path_initialization._remove_conflicting_required_tool_entries(
            path_entries
        )

        self.assertEqual(result, path_entries)

    @patch.object(path_initialization, "which", return_value="C:/toolchain/bin/foo.exe")
    @patch.object(path_initialization, "get_tool_executable_name", return_value="foo")
    @patch.object(path_initialization, "REQUIRED_SOFTWARE", {"foo": {"relaxed": True}})
    def test_remove_conflicting_required_tool_entries_keeps_relaxed_tool(
        self, *_mocks: MagicMock
    ) -> None:
        """Keep entries when only relaxed tools are found."""
        path_entries = ["C:/toolchain/bin"]
        result = path_initialization._remove_conflicting_required_tool_entries(
            path_entries
        )

        self.assertEqual(result, path_entries)

    @patch.dict(path_initialization.os.environ, {"PATH": ""}, clear=True)
    @patch.object(path_initialization, "PATH_FILE")
    def test_initialize_path_variable_for_foxbms_raises_for_missing_paths_file(
        self, mock_path_file: MagicMock
    ) -> None:
        """Raise if the configured path file is missing."""
        mock_path_file.read_text.side_effect = FileNotFoundError
        with self.assertRaises(FileNotFoundError):
            path_initialization.initialize_path_variable_for_foxbms()

    @patch.object(
        path_initialization,
        "_filter_and_normalize_path_entries",
        return_value=["final"],
    )
    @patch.object(path_initialization, "_remove_duplicates", return_value=["unique"])
    @patch.object(
        path_initialization,
        "_prepend_existing_foxbms_path_entries",
        return_value=["prepended"],
    )
    @patch.object(
        path_initialization,
        "_remove_conflicting_required_tool_entries",
        return_value=["filtered"],
    )
    @patch.object(
        path_initialization, "_get_environment_path_entries", return_value=["existing"]
    )
    @patch.dict(path_initialization.os.environ, {"PATH": ""}, clear=True)
    def test_initialize_path_variable_for_foxbms_updates_path(
        self,
        mock_get_environment_path_entries: MagicMock,
        mock_remove_conflicting_required_tool_entries: MagicMock,
        mock_prepend_existing_foxbms_path_entries: MagicMock,
        mock_remove_duplicates: MagicMock,
        mock_filter_and_normalize_path_entries: MagicMock,
    ) -> None:
        """Call helpers in order and use each result as input for the next step."""
        helper_calls = MagicMock()
        helper_calls.attach_mock(mock_get_environment_path_entries, "get_existing")
        helper_calls.attach_mock(
            mock_remove_conflicting_required_tool_entries, "remove_ignored"
        )
        helper_calls.attach_mock(mock_prepend_existing_foxbms_path_entries, "prepend")
        helper_calls.attach_mock(mock_remove_duplicates, "deduplicate")
        helper_calls.attach_mock(mock_filter_and_normalize_path_entries, "create")

        path_initialization.initialize_path_variable_for_foxbms()

        self.assertEqual(
            helper_calls.mock_calls,
            [
                call.get_existing(),
                call.remove_ignored(["existing"]),
                call.prepend(["filtered"]),
                call.deduplicate(["prepended"]),
                call.create(["unique"]),
            ],
        )
        mock_get_environment_path_entries.assert_called_once_with()
        mock_remove_conflicting_required_tool_entries.assert_called_once_with(
            ["existing"]
        )
        mock_prepend_existing_foxbms_path_entries.assert_called_once_with(["filtered"])
        mock_remove_duplicates.assert_called_once_with(["prepended"])
        mock_filter_and_normalize_path_entries.assert_called_once_with(["unique"])
        self.assertEqual(path_initialization.os.environ["PATH"], "final")

    def test_path_file_uses_installation_layout_when_root_is_not_project(self) -> None:
        """Build PATH_FILE from 'env/' when repository root is not a project root."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            base_dir = Path(tmp_dir)
            with (
                patch.object(project_context, "ROOT_IS_PROJECT", False),
                patch.object(project_context, "get_file_path", return_value=base_dir),
                patch.object(host_platform, "get_platform", return_value="win32"),
            ):
                importlib.reload(path_initialization)
                self.assertEqual(
                    path_initialization.PATH_FILE,
                    base_dir / "env/paths_win32.txt",
                )

        # Restore module globals for following tests/importers.
        importlib.reload(path_initialization)

    def test_path_file_uses_installation_layout_when_root_is_project(self) -> None:
        """Build PATH_FILE from 'conf/env/' when repository root is a project root."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            base_dir = Path(tmp_dir)
            with (
                patch.object(project_context, "ROOT_IS_PROJECT", True),
                patch.object(project_context, "get_file_path", return_value=base_dir),
                patch.object(host_platform, "get_platform", return_value="win32"),
            ):
                importlib.reload(path_initialization)
                self.assertEqual(
                    path_initialization.PATH_FILE,
                    base_dir / "conf/env/paths_win32.txt",
                )

        # Restore module globals for following tests/importers.
        importlib.reload(path_initialization)


if __name__ == "__main__":
    unittest.main()
