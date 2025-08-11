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

"""Testing file 'cli/helpers/tmp_handler.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

try:
    from cli.helpers.tmp_handler import TmpHandler
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers.tmp_handler import TmpHandler


@patch("cli.helpers.tmp_handler.TmpHandler._create_tmp_directory")
@patch("cli.helpers.tmp_handler.TmpHandler._check_for_tmp_directory")
class TestTmpHandlerInit(unittest.TestCase):
    """Class to test the TmpHandler init"""

    def test_init_tmp_dir_not_none(self, mock_check: Mock, mock_create: Mock) -> None:
        """Tests the init of the TmpHandler with tmp_dir not equal None"""
        TmpHandler(Path("test"))
        mock_check.return_value = Path("test")
        mock_check.assert_called_once()
        mock_create.assert_not_called()

    def test_init_tmp_dir_none(self, mock_check: Mock, mock_create: Mock) -> None:
        """Tests the init of the TmpHandler with tmp_dir equal None"""
        mock_check.return_value = None
        TmpHandler(Path("test"))
        mock_check.assert_called_once()
        mock_create.assert_called_once()


@patch("cli.helpers.tmp_handler.Path.exists")
@patch("cli.helpers.tmp_handler.TmpHandler.get_hash_name")
class TestTmpHandlerCheckForTmpFile(unittest.TestCase):
    """Class to test the TmpHandler check_for_tmp_file"""

    def setUp(self):
        """Setups the TmpHandler object for later tests"""
        self.tmp_handler_obj = Mock()
        self.tmp_handler_obj.tmp_dir = Path("test_dir")

    def test_with_tmp_file_found(
        self, mock_get_hash_name: Mock, mock_exists: Mock
    ) -> None:
        """Tests the check_for_tmp_file with tmp file found"""
        mock_get_hash_name.return_value = Path("test.txt")
        mock_exists.return_value = True
        found_file = TmpHandler.check_for_tmp_file(
            self.tmp_handler_obj, Path("test"), "txt"
        )
        self.assertEqual(found_file, Path("test_dir/test.txt"))
        mock_get_hash_name.assert_called_once_with(Path("test"), "txt")

    def test_with_tmp_file_not_found(
        self, mock_get_hash_name: Mock, mock_exists: Mock
    ) -> None:
        """Tests the check_for_tmp_file with tmp file found"""
        mock_get_hash_name.return_value = Path("test.txt")
        mock_exists.return_value = False
        found_file = TmpHandler.check_for_tmp_file(
            self.tmp_handler_obj, Path("test"), "txt"
        )
        self.assertEqual(found_file, None)
        mock_get_hash_name.assert_called_once_with(Path("test"), "txt")


@patch("cli.helpers.tmp_handler.Path.resolve")
@patch("cli.helpers.tmp_handler.Path.is_dir")
@patch("cli.helpers.tmp_handler.Path.iterdir")
class TestTmpHandlerCheckForTmpDirectory(unittest.TestCase):
    """Class to test the TmpHandler check_for_tmp_directory"""

    def setUp(self):
        self.tmp_handler_obj = Mock()
        self.tmp_handler_obj._tmp_dir_parent = Path("Test")  # pylint: disable=protected-access
        self.tmp_handler_obj._tmp_folder_prefix = "temp_data_foxcli_"  # pylint: disable=protected-access

    def test_tmp_dir_found(
        self, mock_iter_dir: Mock, mock_is_dir: Mock, mock_resolve: Mock
    ) -> None:
        """Tests the check_for_tmp_dir method in case no tmp dir is found"""
        mock_iter_dir.return_value = [Path("wrong_name"), Path("temp_data_foxcli_test")]
        mock_is_dir.return_value = True
        mock_resolve.return_value = Path("temp_data_foxcli_test")
        found_tmp_dir = TmpHandler._check_for_tmp_directory(self.tmp_handler_obj)  # pylint: disable=protected-access
        self.assertEqual(Path("temp_data_foxcli_test"), found_tmp_dir)
        mock_resolve.assert_called_once()

    def test_tmp_dir_not_found(
        self, mock_iter_dir: Mock, mock_is_dir: Mock, mock_resolve: Mock
    ) -> None:
        """Tests the check_for_tmp_dir method in case no tmp dir is found"""
        mock_iter_dir.return_value = [Path("wrong_name"), Path("wong_name_too")]
        mock_is_dir.return_value = True
        found_tmp_dir = TmpHandler._check_for_tmp_directory(self.tmp_handler_obj)  # pylint: disable=protected-access
        self.assertEqual(None, found_tmp_dir)
        mock_resolve.assert_not_called()


@patch("cli.helpers.tmp_handler.tempfile.mkdtemp")
class TestTmpHandlerCreateTmpDirectory(unittest.TestCase):
    """Class to test the TmpHandler check_for_tmp_directory"""

    def setUp(self):
        """Creates the TmpHandler object"""
        self.tmp_handler_obj = Mock()
        self.tmp_handler_obj._tmp_dir_parent = Path("Test")  # pylint: disable=protected-access
        self.tmp_handler_obj._tmp_folder_prefix = "temp_data_foxcli_"  # pylint: disable=protected-access

    def test_create_tmp_dir(self, mock_mkdtemp: Mock) -> None:
        """Tests the creation of a temporary directory"""
        mock_mkdtemp.return_value = "tmp_dir"
        self.tmp_handler_obj._tmp_dir_parent = Path("parent")  # pylint: disable=protected-access
        tmp_dir = TmpHandler._create_tmp_directory(self.tmp_handler_obj)  # pylint: disable=protected-access
        self.assertEqual(tmp_dir, Path("tmp_dir"))
        mock_mkdtemp.assert_called_once_with(
            prefix="temp_data_foxcli_", dir=Path("parent")
        )


@patch("cli.helpers.tmp_handler.hashlib.sha256")
class TestTmpHandlerGetHashName(unittest.TestCase):
    """Class to test the TmpHandler check_for_tmp_directory"""

    def test_get_hash_name(self, mock_sha256: Mock) -> None:
        """Tests the return value of get_hash_name"""
        file_path = Mock()
        file_path.stem = "test"
        mock_hash = Mock()
        mock_hash.hexdigest.return_value = "hash"
        mock_sha256.return_value = mock_hash
        name = TmpHandler.get_hash_name(file_path, file_extension="txt")
        self.assertEqual(Path("hash_test.txt"), name)


if __name__ == "__main__":
    unittest.main()
