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

"""Testing file 'cli/helpers/project_context.py'."""

import importlib
import sys
import unittest
from pathlib import Path
from typing import ClassVar
from unittest.mock import MagicMock, patch

try:
    from cli.helpers import project_context
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import project_context


class TestGetProjectRoot(unittest.TestCase):
    """Test of 'get_project_root' function."""

    root: ClassVar[Path]

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        cls.root = Path(__file__).parents[3]

    def test_no_git_available(self) -> None:
        """Test no git repository is available"""
        with patch.dict("sys.modules", {"git": None}):
            root = project_context.get_project_root()
        self.assertEqual(self.root, root)

    def test_invalid_git_repository(self) -> None:
        """Check behavior if foxBMS is used without a git repository, e.g. from an archive."""
        root = project_context.get_project_root("foo")
        self.assertEqual(self.root, root)

    def test_git_available(self) -> None:
        """Test git is available"""
        root = project_context.get_project_root()
        self.assertEqual(self.root, root)

    @patch("cli.helpers.project_context.Path.is_file")
    def test_no_file(self, mock_is_file: MagicMock) -> None:
        """Test 'fox.py' file does not exist in root"""
        mock_is_file.return_value = False
        root = project_context.get_project_root()
        self.assertEqual(self.root / "cli", root)


class TestGetEnv(unittest.TestCase):
    """Test of 'get_env' function."""

    def test_root_in_path(self) -> None:
        """Path of the project root is in the file path"""
        ret = project_context.get_env()
        self.assertTrue(ret)

    @patch("cli.helpers.project_context.PROJECT_ROOT", new=Path("invalid/path"))
    @patch("cli.helpers.project_context.Path.is_file")
    def test_root_not_in_path_project(self, mock_is_file: MagicMock) -> None:
        """Path of the project root is not in the file path and 'fox.py' is in the root directory"""
        mock_is_file.return_value = True
        with self.assertRaises(SystemExit) as cm:
            project_context.get_env()
        self.assertEqual(
            cm.exception.code, "Use 'fox.py' script when in the foxBMS repository."
        )

    @patch("cli.helpers.project_context.PROJECT_ROOT", new=Path("invalid/path"))
    @patch("cli.helpers.project_context.Path.is_file")
    def test_root_not_in_path_package(self, mock_is_file: MagicMock) -> None:
        """Path of the project root is not in the file path
        and 'fox.py' is not in the root directory
        """
        mock_is_file.return_value = False
        with self.assertRaises(SystemExit) as cm:
            project_context.get_env()
        self.assertEqual(
            cm.exception.code,
            "Use 'fox-cli' when in the fox CLI package, 'fox.py' is not available.",
        )


class TestGetFilePath(unittest.TestCase):
    """Test of 'get_file_path' function"""

    def setUp(self) -> None:  # noqa: D102
        self.root_is_project = project_context.ROOT_IS_PROJECT

    def tearDown(self) -> None:  # noqa: D102
        project_context.ROOT_IS_PROJECT = self.root_is_project

    def test_project(self) -> None:
        """Test function when in the project"""
        project_context.ROOT_IS_PROJECT = True
        result = project_context.get_file_path()
        self.assertEqual(result, project_context.PROJECT_ROOT)

    def test_package(self) -> None:
        """Test function when in the package"""
        project_context.ROOT_IS_PROJECT = False
        result = project_context.get_file_path()
        self.assertEqual(result, project_context.PROJECT_ROOT / "project_data")


class TestVariables(unittest.TestCase):
    """Test setting of module-level project context variables."""

    def tearDown(self) -> None:  # noqa: D102
        importlib.reload(project_context)

    def test_root_is_project_false(self) -> None:
        """PROJECT_BUILD_ROOT points to user documents when not in project."""
        with (
            patch("pathlib.Path.is_file", return_value=False),
            patch("platformdirs.user_documents_dir", return_value=Path()),
        ):
            importlib.reload(project_context)
            self.assertFalse(project_context.ROOT_IS_PROJECT)
            self.assertEqual(project_context.PROJECT_BUILD_ROOT, Path("fox_cli"))

    def test_root_is_project_true(self) -> None:
        """PROJECT_BUILD_ROOT points to repository build directory in project mode."""
        with patch("pathlib.Path.is_file", return_value=True):
            importlib.reload(project_context)
            self.assertTrue(project_context.ROOT_IS_PROJECT)
            self.assertEqual(
                project_context.PROJECT_BUILD_ROOT,
                project_context.PROJECT_ROOT / "build",
            )


if __name__ == "__main__":
    unittest.main()
