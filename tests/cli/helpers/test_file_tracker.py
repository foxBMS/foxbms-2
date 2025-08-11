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

"""Testing file 'cli/helpers/file_tracker.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

try:
    from cli.helpers.file_tracker import FileTracker
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers.file_tracker import FileTracker


@patch("cli.helpers.file_tracker.FileTracker._update_hash_json")
class TestFileTrackerInit(unittest.TestCase):
    """Class to test the TmpHandler init"""

    def test_tmp_dir_does_not_exist(self, _: Mock) -> None:
        """Tests the init of the FileTracker in case the tmp_dir
        does not exists"""
        mock_tmp_dir = MagicMock()
        mock_tmp_dir.exists.return_value = False
        buf = io.StringIO()
        with (
            redirect_stderr(buf),
            self.assertRaises(SystemExit) as cm,
        ):
            FileTracker(mock_tmp_dir)
        self.assertTrue("hash file does not exist!" in buf.getvalue())
        self.assertEqual(cm.exception.code, 1)

    def test_hash_json_exists(self, mock_update_hash_json: Mock) -> None:
        """Tests the init of the FileTracker in case the tmp_dir
        does not exists"""
        mock_tmp_dir = MagicMock()
        mock_tmp_dir.exists.return_value = True
        mock_tmp_dir.__truediv__().exists.return_value = True  # pylint: disable=C2801
        FileTracker(mock_tmp_dir)
        mock_update_hash_json.assert_not_called()

    def test_hash_json_does_not_exists(self, mock_update_hash_json: Mock) -> None:
        """Tests the init of the FileTracker in case the tmp_dir
        does not exists"""
        mock_tmp_dir = MagicMock()
        mock_tmp_dir.exists.return_value = True
        mock_tmp_dir.__truediv__().exists.return_value = False  # pylint: disable=C2801
        FileTracker(mock_tmp_dir)
        mock_update_hash_json.assert_called_once()


@patch("cli.helpers.file_tracker.get_sha256_file_hash_str")
@patch("cli.helpers.file_tracker.Path.resolve")
class TestFileTrackerCheckFileChange(unittest.TestCase):
    """Class to test the TmpHandler init"""

    def setUp(self) -> None:
        """Creates the FileTracker object for the later tests"""
        self.file_tracker_obj = Mock()
        self.file_tracker_obj._read_hash_json = Mock()  # pylint: disable=protected-access
        self.file_tracker_obj._update_hash_json = Mock()  # pylint: disable=protected-access

    def test_file_has_not_changed(
        self, mock_resolve: Mock, mock_file_hash_str: Mock
    ) -> None:
        """Tests the check_file_changed method in case file has not changed"""
        mock_resolve.return_value = Path("file_tracked.txt")
        # pylint: disable=protected-access
        self.file_tracker_obj._read_hash_json.return_value = {
            "file_tracked.txt": "hash"
        }
        mock_file_hash_str.return_value = "hash"
        changed = FileTracker.check_file_changed(
            self.file_tracker_obj, Path("file_tracked.txt")
        )
        self.assertFalse(changed)
        mock_file_hash_str.assert_called_once_with(
            file_path=Path("file_tracked.txt").resolve()
        )
        # pylint: disable=protected-access
        self.file_tracker_obj._read_hash_json.assert_called_once()
        self.file_tracker_obj._update_hash_json.assert_not_called()

    def test_file_has_changed(
        self, mock_resolve: Mock, mock_file_hash_str: Mock
    ) -> None:
        """Tests the check_file_changed method in case file has changed"""
        mock_resolve.return_value = Path("file_tracked.txt")
        # pylint: disable=protected-access
        self.file_tracker_obj._read_hash_json.return_value = {
            "file_tracked.txt": "different_hash"
        }
        mock_file_hash_str.return_value = "hash"
        changed = FileTracker.check_file_changed(
            self.file_tracker_obj, Path("file_tracked.txt")
        )
        self.assertTrue(changed)
        mock_file_hash_str.assert_called_once_with(
            file_path=Path("file_tracked.txt").resolve()
        )
        # pylint: disable=protected-access
        self.file_tracker_obj._read_hash_json.assert_called_once()
        self.file_tracker_obj._update_hash_json.assert_called_with(
            {"file_tracked.txt": "hash"}
        )

    def test_not_yet_tracked(
        self, mock_resolve: Mock, mock_file_hash_str: Mock
    ) -> None:
        """Tests the check_file_changed method in case file has changed"""
        mock_resolve.return_value = Path("file_tracked.txt")
        self.file_tracker_obj._read_hash_json.return_value = {}  # pylint: disable=protected-access
        mock_file_hash_str.return_value = "hash"
        changed = FileTracker.check_file_changed(
            self.file_tracker_obj, Path("file_tracked.txt")
        )
        self.assertTrue(changed)
        mock_file_hash_str.assert_called_once_with(
            file_path=Path("file_tracked.txt").resolve()
        )
        # pylint: disable=protected-access
        self.file_tracker_obj._read_hash_json.assert_called_once()
        self.file_tracker_obj._update_hash_json.assert_called_with(
            {"file_tracked.txt": "hash"}
        )


@patch("builtins.open")
@patch("cli.helpers.file_tracker.json.load")
class TestFileTrackerReadHashJson(unittest.TestCase):
    """Class to test the TmpHandler read_hash_json method"""

    def setUp(self) -> None:
        """Creates the FileTracker object for the later tests"""
        self.file_tracker_obj = Mock()
        self._path_to_hash_json = Path("test")

    def test_read_json(self, mock_load: Mock, mock_file: Mock) -> None:
        """Tests the reading of the hash json file"""
        FileTracker._read_hash_json(self.file_tracker_obj)  # pylint: disable=protected-access
        # pylint: disable=protected-access
        mock_file.assert_called_once_with(
            self.file_tracker_obj._path_to_hash_json, encoding="utf-8"
        )
        mock_load.assert_called_once_with(mock_file().__enter__())  # pylint: disable=C2801


@patch("builtins.open")
@patch("cli.helpers.file_tracker.json.dump")
class TestFileTrackerUpdateHashJson(unittest.TestCase):
    """Class to test the TmpHandler update_hash_json method"""

    def setUp(self) -> None:
        """Creates the FileTracker object for the later tests"""
        self.file_tracker_obj = Mock()
        self._path_to_hash_json = Path("test")

    def test_update_json(self, mock_dump: Mock, mock_file: Mock) -> None:
        """Tests the updating of the hash json file"""
        FileTracker._update_hash_json(self.file_tracker_obj, {"test": "test"})  # pylint: disable=protected-access
        # pylint: disable=protected-access
        mock_file.assert_called_once_with(
            self.file_tracker_obj._path_to_hash_json, mode="w", encoding="utf-8"
        )
        mock_dump.assert_called_once_with({"test": "test"}, mock_file().__enter__())  # pylint: disable=C2801


if __name__ == "__main__":
    unittest.main()
