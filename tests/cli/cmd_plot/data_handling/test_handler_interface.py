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

"""Testing file 'cli/cmd_plot/data_handling/handler_interface.py'."""

import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import mock

import pandas as pd

try:
    import cli.cmd_plot.data_handling.handler_interface as interface
    from cli.cmd_plot.data_handling.handler_interface import DataHandlerInterface
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    import cli.cmd_plot.data_handling.handler_interface as interface
    from cli.cmd_plot.data_handling.handler_interface import DataHandlerInterface


class TestinterfaceGetTmpData(unittest.TestCase):
    """Tests for the static get_tmp_data method of interface."""

    def test_returns_dataframe_when_cache_valid(self):
        """Return cached DataFrame when parquet exists, file unchanged, and no_tmp is False."""
        mock_tmp = mock.Mock()
        with TemporaryDirectory() as tmp_dir:
            mock_tmp.tmp_dir = Path(tmp_dir)
            cached_parquet = Path(tmp_dir) / "cache.parquet"
            data_path = Path(tmp_dir) / "data.csv"

            with (
                mock.patch.object(interface, "TmpHandler", return_value=mock_tmp),
                mock.patch.object(interface, "FileTracker") as mock_file_tracker,
                mock.patch.object(interface.pd, "read_parquet") as read_parquet,
            ):
                mock_tmp.check_for_tmp_file.return_value = cached_parquet
                mock_file_tracker.return_value.check_file_changed.return_value = False
                expected_df = pd.DataFrame({"x": [1]})
                read_parquet.return_value = expected_df

                df = DataHandlerInterface.get_tmp_data(data_path, no_tmp=False)
                self.assertIs(df, expected_df)
                read_parquet.assert_called_once_with(cached_parquet, engine="pyarrow")

    def test_returns_none_when_no_tmp_flag(self):
        """Return None when no_tmp=True even if a cached parquet exists."""
        mock_tmp = mock.Mock()
        with TemporaryDirectory() as tmp_dir:
            mock_tmp.tmp_dir = Path(tmp_dir)
            data_path = Path(tmp_dir) / "data.csv"

            with (
                mock.patch.object(interface, "TmpHandler", return_value=mock_tmp),
                mock.patch.object(interface, "FileTracker") as mock_file_tracker,
            ):
                mock_tmp.check_for_tmp_file.return_value = (
                    Path(tmp_dir) / "cache.parquet"
                )
                mock_file_tracker.return_value.check_file_changed.return_value = False

                df = DataHandlerInterface.get_tmp_data(data_path, no_tmp=True)
                self.assertIsNone(df)

    def test_returns_none_when_file_changed(self):
        """Return None when FileTracker indicates the source file has changed."""
        mock_tmp = mock.Mock()
        with TemporaryDirectory() as tmp_dir:
            mock_tmp.tmp_dir = Path(tmp_dir).name
            data_path = Path(tmp_dir) / "data.csv"

        with (
            mock.patch.object(interface, "TmpHandler", return_value=mock_tmp),
            mock.patch.object(interface, "FileTracker") as mock_file_tracker,
        ):
            mock_tmp.check_for_tmp_file.return_value = Path(tmp_dir) / "cache.parquet"
            mock_file_tracker.return_value.check_file_changed.return_value = True

            df = DataHandlerInterface.get_tmp_data(data_path, no_tmp=False)
            self.assertIsNone(df)

    def test_returns_none_when_no_cached_file(self):
        """Return None when no cached parquet file is found."""
        mock_tmp = mock.Mock()
        with TemporaryDirectory() as tmp_dir:
            mock_tmp.tmp_dir = Path(tmp_dir)
            data_path = Path(tmp_dir) / "data.csv"

        with (
            mock.patch.object(interface, "TmpHandler", return_value=mock_tmp),
            mock.patch.object(interface, "FileTracker"),
        ):
            mock_tmp.check_for_tmp_file.return_value = None

            df = DataHandlerInterface.get_tmp_data(data_path, no_tmp=False)
            self.assertIsNone(df)

    def test_arrow_invalid_triggers_log_and_exit(self):
        """Log and exit when pyarrow raises ArrowInvalid while accessing parquet."""
        mock_tmp = mock.Mock()
        with TemporaryDirectory() as tmp_dir:
            mock_tmp.tmp_dir = Path(tmp_dir)
            cached_parquet = Path(tmp_dir) / "cache.parquet"
            data_path = Path(tmp_dir) / "data.csv"

        with (
            mock.patch.object(interface, "TmpHandler", return_value=mock_tmp),
            mock.patch.object(interface, "FileTracker") as mock_file_tracker,
            mock.patch.object(interface, "recho") as mock_recho,
            mock.patch.object(interface.pd, "read_parquet") as read_parquet,
        ):
            mock_tmp.check_for_tmp_file.return_value = cached_parquet
            mock_file_tracker.return_value.check_file_changed.return_value = False
            read_parquet.side_effect = interface.ArrowInvalid("boom")

            with self.assertRaises(SystemExit):
                _ = DataHandlerInterface.get_tmp_data(data_path, no_tmp=False)

            self.assertTrue(mock_recho.called)
            args, _ = mock_recho.call_args
            self.assertIn("Parquet Error:", args[0])


class TestinterfaceWriteTmpFile(unittest.TestCase):
    """Tests for the static write_tmp_file method of interface."""

    def test_writes_parquet_to_tmp_dir(self):
        """Write DataFrame to parquet in the tmp directory using hash-based filename."""
        mock_tmp = mock.Mock()
        with TemporaryDirectory() as tmp_dir:
            mock_tmp.tmp_dir = Path(tmp_dir)
            data_path = Path(tmp_dir) / "data.csv"
            mock_tmp.get_hash_name.return_value = "abc.parquet"

        df = pd.DataFrame({"a": [1]})

        with (
            mock.patch.object(interface, "TmpHandler", return_value=mock_tmp),
            mock.patch.object(pd.DataFrame, "to_parquet") as to_parquet,
        ):
            DataHandlerInterface.write_tmp_file(df, data_path)

            mock_tmp.get_hash_name.assert_called_once_with(data_path, "parquet")
            to_parquet.assert_called_once_with(
                mock_tmp.tmp_dir / "abc.parquet", engine="pyarrow"
            )


if __name__ == "__main__":
    unittest.main()
