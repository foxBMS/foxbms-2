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

"""Testing file 'cli/cmd_plot/data_handling/parquet_handler.py'."""

import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import mock

import pandas as pd

try:
    import cli.cmd_plot.data_handling.parquet_handler as ph
    from cli.cmd_plot.data_handling.parquet_handler import PARQUETHandler
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    import cli.cmd_plot.data_handling.parquet_handler as ph
    from cli.cmd_plot.data_handling.parquet_handler import PARQUETHandler


class TestPARQUETHandlerInit(unittest.TestCase):
    """Tests for the __init__ method of PARQUETHandler."""

    def test_can_instantiate(self):
        """Ensure the handler can be instantiated without arguments."""
        handler = PARQUETHandler()
        self.assertIsInstance(handler, PARQUETHandler)


class TestPARQUETHandlerGetData(unittest.TestCase):
    """Tests for the get_data method of PARQUETHandler."""

    def setUp(self):
        """Create shared fixtures."""
        self.handler = PARQUETHandler()

    def test_reads_parquet_and_writes_cache_when_no_tmp(self):
        """Read from disk when no cached data is available and write tmp file."""
        with TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "data.parquet"
            fresh_df = pd.DataFrame({"y": [3, 4]})

            with mock.patch.object(
                ph.pd, "read_parquet", return_value=fresh_df
            ) as m_read:
                df = self.handler.get_data(file_path, no_tmp=False)

            self.assertIs(df, fresh_df)
            m_read.assert_called_once_with(file_path, engine="pyarrow")

    def test_arrow_error_logs_and_exits(self):
        """Log ArrowInvalid errors and exit with SystemExit."""
        with TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "data.parquet"
            with (
                mock.patch.object(PARQUETHandler, "get_tmp_data", return_value=None),
                mock.patch.object(
                    ph.pd, "read_parquet", side_effect=ph.ArrowInvalid("boom")
                ) as m_read,
                mock.patch.object(ph, "recho") as m_recho,
                self.assertRaises(SystemExit),
            ):
                _ = self.handler.get_data(file_path, no_tmp=False)

            m_read.assert_called_once()
            self.assertTrue(m_recho.called)
            args, _ = m_recho.call_args
            self.assertIn("Parquet Error", args[0])

    def test_oserror_logs_and_exits(self):
        """Log OSError errors and exit with SystemExit."""
        with TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "data.parquet"
            with (
                mock.patch.object(PARQUETHandler, "get_tmp_data", return_value=None),
                mock.patch.object(
                    ph.pd, "read_parquet", side_effect=OSError("no access")
                ) as m_read,
                mock.patch.object(ph, "recho") as m_recho,
                self.assertRaises(SystemExit),
            ):
                _ = self.handler.get_data(file_path, no_tmp=False)

            m_read.assert_called_once()
            self.assertTrue(m_recho.called)
            args, _ = m_recho.call_args
            self.assertIn("Can not access file:", args[0])


if __name__ == "__main__":
    unittest.main()
