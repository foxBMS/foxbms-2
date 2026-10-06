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

"""Testing file 'cli/cmd_misc/open_unit_test_reports.py'."""

import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_misc.open_unit_test_reports import (
        _worker,
        open_cli_unit_test_report,
        open_embedded_unit_test_report,
    )
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc.open_unit_test_reports import (
        _worker,
        open_cli_unit_test_report,
        open_embedded_unit_test_report,
    )
    from cli.helpers.project_context import PROJECT_BUILD_ROOT


class Test_Worker(unittest.TestCase):  # noqa: N801
    """Test the _worker function."""

    def setUp(self) -> None:
        """Set up a temporary directory for testing."""
        self.temp_dir = tempfile.mkdtemp()
        self.index_file = Path(self.temp_dir) / "index.html"

    def tearDown(self) -> None:
        """Clean up the temporary directory after testing."""
        shutil.rmtree(self.temp_dir)

    @patch("webbrowser.open")
    def test_worker_with_existing_file(self, mock_webbrowser_open: MagicMock) -> None:
        """Test _worker with an existing index file."""
        self.index_file.write_text("<html><body>Test Report</body></html>")
        _worker(index_file=self.index_file, _type="embedded")
        mock_webbrowser_open.assert_called_once_with(str(self.index_file), new=1)

    @patch("webbrowser.open")
    def test_worker_with_non_existing_file(
        self, mock_webbrowser_open: MagicMock
    ) -> None:
        """Test _worker with a non-existing index file."""
        non_existing_file = Path(self.temp_dir) / "non_existing_index.html"
        _worker(index_file=non_existing_file, _type="embedded")
        mock_webbrowser_open.assert_called_once_with(
            str(non_existing_file.parent / "error.html"), new=1
        )


class TestOpenEmbeddedUnitTestReport(unittest.TestCase):
    """Test the open_embedded_unit_test_report function."""

    @patch("cli.cmd_misc.open_unit_test_reports._worker")
    def test_open_embedded_unit_test_report(self, mock_worker: MagicMock) -> None:
        """Test that open_embedded_unit_test_report calls _worker with correct arguments."""
        open_embedded_unit_test_report()
        mock_worker.assert_called_once_with(
            PROJECT_BUILD_ROOT / "app_unit_test_gcc/coverage/index.html",
            "Embedded",
        )


class TestOpenCliUnitTestReport(unittest.TestCase):
    """Test the open_cli_unit_test_report function."""

    @patch("cli.cmd_misc.open_unit_test_reports._worker")
    def test_open_cli_unit_test_report(self, mock_worker: MagicMock) -> None:
        """Test that open_cli_unit_test_report calls _worker with correct arguments."""
        open_cli_unit_test_report()
        mock_worker.assert_called_once_with(
            PROJECT_BUILD_ROOT / "cli-selftest/index.html", "CLI"
        )


if __name__ == "__main__":
    unittest.main()
