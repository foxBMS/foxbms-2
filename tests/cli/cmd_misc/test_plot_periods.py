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

"""Testing file 'cli/cmd_misc/plot_periods.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

try:
    from cli.cmd_misc.plot_periods import (
        _parse_line,
        extract_timestamps_for_ids,
        plot_stats,
        plot_time_differences_for_ids,
        run_plot_periods,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc.plot_periods import (
        _parse_line,
        extract_timestamps_for_ids,
        plot_stats,
        plot_time_differences_for_ids,
        run_plot_periods,
    )


class TestParseLine(unittest.TestCase):
    """Class to test '_parse_line' function."""

    def test_id_timestamp_format(self) -> None:
        """A line in 'ID: <id> Timestamp: <ts>' format is parsed correctly."""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line("ID: 0x123 Timestamp: 1.500")
        self.assertEqual("0x123", id_val)
        self.assertEqual(1.5, ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_trc_format(self) -> None:
        """A line in TRC format is parsed with ms-to-seconds conversion."""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line(
                "1) 1500.0 DT 1 0x123 Rx d 8 00 00 00 00 00 00 00 00"
            )
        self.assertEqual("0x123", id_val)
        self.assertEqual(1.5, ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_id_timestamp_format_value_error(self) -> None:
        """A non-numeric timestamp in 'ID:/Timestamp:' format"""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line("ID: 0x123 Timestamp: abc")
        self.assertIsNone(id_val)
        self.assertIsNone(ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_id_timestamp_format_index_error(self) -> None:
        """'ID: Timestamp:' without trailing values."""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line("ID: Timestamp:")
        self.assertIsNone(id_val)
        self.assertIsNone(ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_trc_format_invalid_timestamp(self) -> None:
        """A TRC line with a non-numeric time field"""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line("1) abc DT 1 0x123 Rx")
        self.assertIsNone(id_val)
        self.assertIsNone(ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_unrecognized_format(self) -> None:
        """An unrecognized line with fewer than 5 parts"""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line("some random text")
        self.assertIsNone(id_val)
        self.assertIsNone(ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_empty_string(self) -> None:
        """An empty string returns"""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            id_val, ts = _parse_line("")
        self.assertIsNone(id_val)
        self.assertIsNone(ts)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())


@patch("cli.cmd_misc.plot_periods._parse_line")
@patch("cli.cmd_misc.plot_periods.logger")
class TestExtractTimestampsForIds(unittest.TestCase):
    """Test 'extract_timestamps_for_ids' function."""

    def setUp(self) -> None:  # noqa: D102
        self.log_file = MagicMock()
        self.target_ids = ["0x123", "0x456"]

    def _setup_file(self, lines: list[str]) -> None:
        """Configure the mock file to yield the given lines."""
        self.log_file.exists.return_value = True
        mock_file = MagicMock()
        mock_file.__enter__.return_value = iter(lines)
        self.log_file.open.return_value = mock_file

    def test_file_does_not_exist(
        self, mock_logger: Mock, mock_parse_line: Mock
    ) -> None:
        """A FileNotFoundError is raised when the file does not exist."""
        self.log_file.exists.return_value = False
        err = io.StringIO()
        out = io.StringIO()
        with (
            redirect_stderr(err),
            redirect_stdout(out),
            self.assertRaises(FileNotFoundError),
        ):
            extract_timestamps_for_ids(self.log_file, self.target_ids)
        mock_logger.warning.assert_called_once()
        mock_parse_line.assert_not_called()
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_valid_file_with_matching_ids(self, _: Mock, mock_parse_line: Mock) -> None:
        """Timestamps are collected for matching target IDs."""
        self._setup_file(["line1\n", "line2\n"])
        mock_parse_line.side_effect = [("0x123", 1.0), ("0x456", 2.0)]
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = extract_timestamps_for_ids(self.log_file, self.target_ids)
        self.assertEqual({"0x123": [1.0], "0x456": [2.0]}, ret)
        self.assertEqual(2, mock_parse_line.call_count)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_empty_lines_are_skipped(self, _: Mock, mock_parse_line: Mock) -> None:
        """Empty lines are skipped and not passed to _parse_line."""
        self._setup_file(["\n", "line1\n"])
        mock_parse_line.return_value = ("0x123", 1.0)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = extract_timestamps_for_ids(self.log_file, self.target_ids)
        self.assertEqual(1, mock_parse_line.call_count)
        self.assertEqual(1, mock_parse_line.call_count)
        self.assertIn(1.0, ret["0x123"])
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_comment_lines_are_skipped(
        self, _: Mock, mock_parse_line: MagicMock
    ) -> None:
        """Lines starting with ';' are skipped."""
        self._setup_file(["; comment\n", "line1\n"])
        mock_parse_line.return_value = ("0x123", 1.0)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            extract_timestamps_for_ids(self.log_file, self.target_ids)
        self.assertEqual(1, mock_parse_line.call_count)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_no_matching_ids(self, _: Mock, mock_parse_line: MagicMock) -> None:
        """Empty lists are returned for IDs not present in the file."""
        self._setup_file(["line1\n"])
        mock_parse_line.return_value = ("0x999", 1.0)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = extract_timestamps_for_ids(self.log_file, self.target_ids)
        self.assertEqual({"0x123": [], "0x456": []}, ret)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_unparseable_line(self, _: Mock, mock_parse_line: Mock) -> None:
        """Lines returning (None, None) from _parse_line are ignored."""
        self._setup_file(["invalid\n"])
        mock_parse_line.return_value = (None, None)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = extract_timestamps_for_ids(self.log_file, self.target_ids)
        self.assertEqual({"0x123": [], "0x456": []}, ret)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())


class TestPlotStats(unittest.TestCase):
    """Test 'plot_stats' function."""

    def setUp(self) -> None:  # noqa: D102
        self.differences = [0.1, 0.2, 0.3]
        self.target_id = "0x123"

    def _setup_plt(self, mock_plt: Mock) -> tuple[MagicMock, MagicMock]:
        """Configure mocked matplotlib objects."""
        mock_fig = MagicMock()
        mock_ax = MagicMock()
        mock_ax.get_legend_handles_labels.return_value = ([], [])
        mock_plt.subplots.return_value = (mock_fig, mock_ax)
        return mock_fig, mock_ax

    @patch("cli.cmd_misc.plot_periods.logger")
    @patch("cli.cmd_misc.plot_periods.plt")
    def test_show_true(self, mock_plt: Mock, _mock_logger: Mock) -> None:
        """When show is True, plt.show() is called and no file is saved."""
        mock_fig, _ = self._setup_plt(mock_plt)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            plot_stats(self.differences, self.target_id, show=True)
        mock_plt.show.assert_called_once()
        mock_fig.savefig.assert_not_called()
        mock_plt.close.assert_called_once_with(mock_fig)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    @patch("cli.cmd_misc.plot_periods.logger")
    @patch("cli.cmd_misc.plot_periods.plt")
    def test_show_false_saves_file(self, mock_plt: Mock, mock_logger: Mock) -> None:
        """When show is False, the plot is saved and plt.show() is not called."""
        mock_fig, _ = self._setup_plt(mock_plt)
        output_path = MagicMock()
        output_path.resolve.return_value = Path("output")
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            plot_stats(
                self.differences,
                self.target_id,
                show=False,
                output_path=output_path,
            )
        output_path.mkdir.assert_called_once_with(parents=True, exist_ok=True)
        mock_fig.savefig.assert_called_once()
        mock_plt.show.assert_not_called()
        mock_plt.close.assert_called_once_with(mock_fig)
        mock_logger.info.assert_called_once()
        self.assertIn("Plot saved in", mock_logger.info.call_args[0][0])
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())


class TestPlotTimeDifferencesForIds(unittest.TestCase):
    """Test 'plot_time_differences_for_ids' function."""

    def setUp(self) -> None:  # noqa: D102
        self.log_file = MagicMock()

    @patch("cli.cmd_misc.plot_periods.logger")
    @patch("cli.cmd_misc.plot_periods.plot_stats")
    @patch("cli.cmd_misc.plot_periods.extract_timestamps_for_ids")
    def test_valid_timestamps(
        self, mock_extract: Mock, mock_plot_stats: Mock, _mock_logger: Mock
    ) -> None:
        """plot_stats is called with computed differences for valid timestamps."""
        mock_extract.return_value = {"0x123": [1.0, 1.1, 1.2]}
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            plot_time_differences_for_ids(self.log_file, ["0x123"])
        mock_plot_stats.assert_called_once()
        args, _ = mock_plot_stats.call_args
        self.assertEqual([0.1, 0.1], args[0])
        self.assertEqual("0x123", args[1])
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    @patch("cli.cmd_misc.plot_periods.logger")
    @patch("cli.cmd_misc.plot_periods.plot_stats")
    @patch("cli.cmd_misc.plot_periods.extract_timestamps_for_ids")
    def test_no_timestamps(
        self, mock_extract: Mock, mock_plot_stats: Mock, mock_logger: Mock
    ) -> None:
        """A warning is logged when no timestamps are found."""
        mock_extract.return_value = {"0x123": []}
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            plot_time_differences_for_ids(self.log_file, ["0x123"])
        mock_logger.warning.assert_called_once()
        self.assertIn("No timestamps found", mock_logger.warning.call_args[0][0])
        mock_plot_stats.assert_not_called()
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    @patch("cli.cmd_misc.plot_periods.logger")
    @patch("cli.cmd_misc.plot_periods.plot_stats")
    @patch("cli.cmd_misc.plot_periods.extract_timestamps_for_ids")
    def test_single_timestamp(
        self, mock_extract: Mock, mock_plot_stats: Mock, mock_logger: Mock
    ) -> None:
        """A warning is logged when only one timestamp exists."""
        mock_extract.return_value = {"0x123": [1.0]}
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            plot_time_differences_for_ids(self.log_file, ["0x123"])
        mock_logger.warning.assert_called_once()
        self.assertIn("Only a single occurrence", mock_logger.warning.call_args[0][0])
        mock_plot_stats.assert_not_called()
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())


@patch("cli.cmd_misc.plot_periods.plot_time_differences_for_ids")
class TestRunPlotPeriods(unittest.TestCase):
    """Test 'run_plot_periods' function."""

    def setUp(self) -> None:  # noqa: D102
        self.log_file = MagicMock()

    def test_success(self, mock_plot: Mock) -> None:
        """Returns 0 when plot_time_differences_for_ids succeeds."""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = run_plot_periods(self.log_file, ["0x123"])
        self.assertEqual(0, ret)
        mock_plot.assert_called_once_with(
            self.log_file, ["0x123"], show=False, output_path=Path()
        )
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())

    def test_file_not_found(self, mock_plot: Mock) -> None:
        """Returns 1 when a FileNotFoundError is raised."""
        mock_plot.side_effect = FileNotFoundError
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = run_plot_periods(self.log_file, ["0x123"])
        self.assertEqual(1, ret)
        self.assertEqual("", err.getvalue())
        self.assertEqual("", out.getvalue())


if __name__ == "__main__":
    unittest.main()
