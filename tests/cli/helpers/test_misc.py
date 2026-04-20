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

"""Testing file 'cli/helpers/misc.py'."""

import importlib
import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.helpers import misc
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import misc


class TestGetProjectRoot(unittest.TestCase):
    """Test of 'get_project_root' function."""

    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).parents[3]

    def test_no_git_available(self):
        """Test no git repository is available"""
        with patch.dict("sys.modules", {"git": None}):
            root = misc.get_project_root()
        self.assertEqual(self.root, root)

    def test_invalid_git_repository(self):
        """Check behavior if foxBMS is used without a git repository, e.g. from an archive."""
        root = misc.get_project_root("foo")
        self.assertEqual(self.root, root)

    def test_git_available(self):
        """Test git is available"""
        root = misc.get_project_root()
        self.assertEqual(self.root, root)

    @patch("cli.helpers.misc.Path.is_file")
    def test_no_file(self, mock_is_file: MagicMock):
        """Test 'fox.py' file does not exist in root"""
        mock_is_file.return_value = False
        root = misc.get_project_root()
        self.assertEqual(self.root / "cli", root)


class TestGetEnv(unittest.TestCase):
    """Test of 'get_env' function."""

    def test_root_in_path(self):
        """Path of the project root is in the file path"""
        ret = misc.get_env()
        self.assertTrue(ret)

    @patch("cli.helpers.misc.PROJECT_ROOT", new=Path("invalid/path"))
    @patch("cli.helpers.misc.Path.is_file")
    def test_root_not_in_path_project(self, mock_is_file: MagicMock):
        """Path of the project root is not in the file path and 'fox.py' is in the root directory"""
        mock_is_file.return_value = True
        with self.assertRaises(SystemExit) as cm:
            misc.get_env()
        self.assertEqual(
            cm.exception.code, "Use 'fox.py' script when in the foxBMS repository."
        )

    @patch("cli.helpers.misc.PROJECT_ROOT", new=Path("invalid/path"))
    @patch("cli.helpers.misc.Path.is_file")
    def test_root_not_in_path_package(self, mock_is_file: MagicMock):
        """Path of the project root is not in the file path
        and 'fox.py' is not in the root directory
        """
        mock_is_file.return_value = False
        with self.assertRaises(SystemExit) as cm:
            misc.get_env()
        self.assertEqual(
            cm.exception.code,
            "Use 'fox-cli' when in the fox CLI package, 'fox.py' is not available.",
        )


class TestGetFilePath(unittest.TestCase):
    """Test of 'get_file_path' function"""

    def setUp(self):
        self.root_is_project = misc.ROOT_IS_PROJECT

    def tearDown(self):
        misc.ROOT_IS_PROJECT = self.root_is_project

    def test_project(self):
        """Test function when in the project"""
        misc.ROOT_IS_PROJECT = True
        result = misc.get_file_path()
        self.assertEqual(result, misc.PROJECT_ROOT)

    def test_package(self):
        """Test function when in the package"""
        misc.ROOT_IS_PROJECT = False
        result = misc.get_file_path()
        self.assertEqual(result, misc.PROJECT_ROOT / "project_data")


@patch("cli.helpers.misc.Path.is_dir")
@patch("cli.helpers.misc.Path.is_file")
@patch("cli.helpers.misc.Path.read_text")
@patch("cli.helpers.misc.Path.write_text")
class TestCreatePreCommitFile(unittest.TestCase):
    """Test of 'create_pre_commit_file' function."""

    def test_dir_does_not_exist(
        self,
        mock_write_text: MagicMock,
        mock_read_text: MagicMock,
        mock_is_file: MagicMock,
        mock_is_dir: MagicMock,
    ):
        """Test directory .git/hooks does not exist."""
        mock_is_dir.return_value = False
        misc.create_pre_commit_file()
        mock_write_text.assert_not_called()
        mock_read_text.assert_not_called()
        mock_is_file.assert_not_called()
        mock_is_dir.assert_called_once()

    def test_file_does_not_exist(
        self,
        mock_write_text: MagicMock,
        mock_read_text: MagicMock,
        mock_is_file: MagicMock,
        mock_is_dir: MagicMock,
    ):
        """Test directory .git/hooks exists, but the .git/hooks/pre-commit file does not."""
        mock_write_text.return_value = None
        mock_is_file.return_value = False
        mock_is_dir.return_value = True
        misc.create_pre_commit_file()
        text = (
            "#!/usr/bin/env bash\n"
            "#\n"
            'SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"\n'
            '"$SCRIPT_DIR/../../fox.sh" pre-commit run\n'
        )
        mock_write_text.assert_called_once_with(text, encoding="utf-8", newline="\n")
        mock_read_text.assert_not_called()
        mock_is_file.assert_called_once()
        mock_is_dir.assert_called_once()

    def test_invalid_content(
        self,
        mock_write_text: MagicMock,
        mock_read_text: MagicMock,
        mock_is_file: MagicMock,
        mock_is_dir: MagicMock,
    ):
        """Test .git/hooks/pre-commit has invalid text."""
        mock_is_dir.return_value = True
        mock_is_file.return_value = True
        mock_read_text.return_value = None
        mock_write_text.return_value = None
        misc.create_pre_commit_file()
        text = (
            "#!/usr/bin/env bash\n"
            "#\n"
            'SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"\n'
            '"$SCRIPT_DIR/../../fox.sh" pre-commit run\n'
        )
        mock_write_text.assert_called_once_with(text, encoding="utf-8", newline="\n")
        mock_read_text.assert_called_once_with(encoding="utf-8")
        mock_is_file.assert_called_once()
        mock_is_dir.assert_called_once()

    def test_valid_content(
        self,
        mock_write_text: MagicMock,
        mock_read_text: MagicMock,
        mock_is_file: MagicMock,
        mock_is_dir: MagicMock,
    ):
        """Test .git/hooks/pre-commit exists and the text is as expected."""
        mock_is_dir.return_value = True
        mock_read_text.return_value = (
            "#!/usr/bin/env bash\n"
            "#\n"
            'SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"\n'
            '"$SCRIPT_DIR/../../fox.sh" pre-commit run\n'
        )
        misc.create_pre_commit_file()
        mock_write_text.assert_not_called()
        mock_read_text.assert_called_once_with(encoding="utf-8")
        mock_is_file.assert_called_once()
        mock_is_dir.assert_called_once()

    def test_not_root_is_project(
        self,
        mock_write_text: MagicMock,
        mock_read_text: MagicMock,
        mock_is_file: MagicMock,
        mock_is_dir: MagicMock,
    ):
        """Test ROOT_IS_PROJECT is False"""
        with patch("cli.helpers.misc.ROOT_IS_PROJECT", new=False):
            misc.create_pre_commit_file()
        mock_is_dir.assert_not_called()
        mock_is_file.assert_not_called()
        mock_read_text.assert_not_called()
        mock_write_text.assert_not_called()


@patch("cli.helpers.misc.Path.read_text")
class TestInitializePathVariable(unittest.TestCase):
    """Test of 'initialize_path_variable_for_foxbms' function."""

    def test_dir_does_not_exist(self, mock_read_text: MagicMock):
        """Check path initialization when path does not exist."""
        mock_read_text.return_value = "/opt\nC:\\Windows\ndoes/not/exists"
        misc.initialize_path_variable_for_foxbms()

    @patch("cli.helpers.misc.Path.is_dir")
    def test_dir_exists(self, mock_is_dir: MagicMock, mock_read_text: MagicMock):
        """Check path initialization when path exists and is a directory."""
        mock_read_text.return_value = "C:\\foo\nC:\\WindowsApps"
        mock_is_dir.return_value = True
        misc.initialize_path_variable_for_foxbms()


class TestMisc(unittest.TestCase):
    """Tests functions."""

    def test_terminal_link_print(self):
        """Test clickable links"""
        result = misc.terminal_link_print("www.foxbms.org")
        self.assertEqual(
            "\033]8;;www.foxbms.org\033\\www.foxbms.org\033]8;;\033\\", result
        )

    def test_get_sha256_file_hash_str(self):
        """Test file hashing helper"""
        test = Path(__file__).parent / Path(__file__).stem / "checksum_test.txt"
        result = misc.get_sha256_file_hash_str(test)
        self.assertEqual(
            result, "bc6924dfde4fb58a5c92ecd2d08c66aa3b58f64582a1af431ac4b3dd109e92de"
        )

    def test_get_multiple_files_hash_str(self):
        """Test file hashing helper"""
        test = [Path(__file__).parent / Path(__file__).stem / "checksum_test.txt"]
        result = misc.get_multiple_files_hash_str(test)
        self.assertEqual(
            result, "bc6924dfde4fb58a5c92ecd2d08c66aa3b58f64582a1af431ac4b3dd109e92de"
        )

    @patch("cli.helpers.misc.Path.write_text")
    @patch("cli.helpers.misc.json.loads")
    @patch("cli.helpers.misc.os.environ")
    def test_set_other_environment_variables_for_foxbms(
        self,
        mock_environ: MagicMock,
        mock_json_loads: MagicMock,
        mock_read_text: MagicMock,
    ):
        """Check setting variables."""
        mock_read_text.return_value = ""
        mock_json_loads.return_value = {
            "foo": 1,
            "bar": {
                "linux": "2",
                "win32": "3",
            },
        }
        mock_environ.return_value = {}
        misc.set_other_environment_variables_for_foxbms()


class TestVariables(unittest.TestCase):
    """Test setting of 'PROJECT_BUILD_ROOT' and 'PATH_FILE' variables"""

    def tearDown(self):
        importlib.reload(misc)

    def test_root_is_project_false(self):
        """ROOT_IS_PROJECT is False"""
        with (
            patch("pathlib.Path.is_file", return_value=False),
            patch("cli.helpers.host_platform.get_platform", return_value="platform"),
            patch("platformdirs.user_documents_dir", return_value=Path()),
        ):
            importlib.reload(misc)
            self.assertFalse(misc.ROOT_IS_PROJECT)
            self.assertEqual(misc.PROJECT_BUILD_ROOT, Path("fox_cli"))
            self.assertEqual(
                misc.PATH_FILE,
                misc.PROJECT_ROOT / "project_data/env/paths_platform.txt",
            )

    def test_root_is_project_true(self):
        """ROOT_IS_PROJECT is True"""
        with (
            patch("pathlib.Path.is_file", return_value=True),
            patch("cli.helpers.host_platform.get_platform", return_value="platform"),
        ):
            importlib.reload(misc)
            self.assertTrue(misc.ROOT_IS_PROJECT)
            self.assertEqual(misc.PROJECT_BUILD_ROOT, misc.PROJECT_ROOT / "build")
            self.assertEqual(
                misc.PATH_FILE, misc.PROJECT_ROOT / "conf/env/paths_platform.txt"
            )


if __name__ == "__main__":
    unittest.main()
