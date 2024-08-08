#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""unit-test for plot_periods"""

import logging
import pathlib
import unittest
from io import StringIO
from unittest.mock import patch

import plot_periods  # pylint: disable=import-error

TEST_FILE = pathlib.Path(__file__).parent / "test_log.txt"


class TestPlotPeriods(unittest.TestCase):
    """some tests for plot_periods"""

    def setUp(self):
        """Ensure logging is configured properly before each test"""
        logging.basicConfig(level=logging.NOTSET)
        self.logger = logging.getLogger()
        self.logger.handlers = []  # Clear existing handlers

    @patch("sys.stderr", new_callable=StringIO)
    def test_no_args(self, mock_stderr):
        """Check the main help option works when no args are provided"""
        with self.assertRaises(SystemExit) as sys_exit:
            plot_periods.main()
        self.assertEqual(sys_exit.exception.code, 2)
        output = mock_stderr.getvalue()
        self.assertIn("usage: ", output)

    @patch("sys.stderr", new_callable=StringIO)
    def test_read_from_file(self, mock_stderr):
        """Check if the warning message is generated on entering a wrong file"""
        # Simulate command line args
        with patch("sys.argv", ["plot_periods.py", "some.txt", "0220"]):
            with self.assertRaises(SystemExit) as sys_exit:
                plot_periods.main()
        # Check the exit code
        self.assertEqual(sys_exit.exception.code, 1)
        # Capture output
        output = mock_stderr.getvalue()
        self.assertEqual("WARNING:root:The file 'some.txt' does not exist.\n", output)

    @patch("sys.stderr", new_callable=StringIO)
    def test_wrong_id(self, mock_stderr):
        """Check if the warning message is generated when no timestamps are found"""
        with patch("sys.argv", ["plot_periods.py", str(TEST_FILE), "0222"]):
            plot_periods.main()
        output = mock_stderr.getvalue()
        self.assertIn("No timestamps found for the target id: 0222", output)

    def test_timestamp_is_correct(self) -> None:
        """Reading CAN message log from log file"""
        timestamps = plot_periods.extract_timestamps_for_ids(TEST_FILE, ["024e"])
        # timestamps extracted value should be equal to this manually extracted values i have
        self.assertEqual(
            timestamps,
            {"024e": [132.37569, 133.675478]},
            "Returned timestamps do not match expected timestamps",
        )

    @patch("sys.stderr", new_callable=StringIO)
    def test_timestamps_length(self, mock_stderr):
        """Check if the warning message is generated in case of a single timestamp"""
        with patch("sys.argv", ["plot_periods.py", str(TEST_FILE), "0221"]):
            plot_periods.main()
        output = mock_stderr.getvalue()
        self.assertEqual(
            "WARNING:root:Only a single occurrence for the target id: 0221\n", output
        )

    @patch("matplotlib.figure.Figure.savefig")
    def test_figure_saving(self, mock_savefig):
        """Test if the figure is saved as expected."""
        with patch("sys.argv", ["plot_periods.py", str(TEST_FILE), "035c"]):
            plot_periods.main()
        # Assert that savefig was called once
        self.assertTrue(mock_savefig.called)
        self.assertEqual(mock_savefig.call_count, 1)
        # Assert it was called with the expected filename
        mock_savefig.assert_called_with("035c.png", dpi=100.0)


if __name__ == "__main__":
    unittest.main()
