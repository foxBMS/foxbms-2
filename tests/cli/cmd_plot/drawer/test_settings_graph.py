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

"""Testing file 'cli/cmd_plot/drawer/settings_graph.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import Mock, patch

try:
    from cli.cmd_plot.drawer.settings_graph import LinesSettings, Mapping
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_plot.drawer.settings_graph import LinesSettings, Mapping


class TestMappingPostInit(unittest.TestCase):
    """Class to test the post_init method of the Mapping dataclass"""

    def test_post_init_valid_config(self) -> None:
        """Tests the post_init with valid config"""
        config = {"x": "test", "y2": {"input": "test"}}
        mapping = Mapping(**config)
        # The attributes need to be checked one by one because
        # the labels in LinesSettings are an iterator, which
        # can't be easily compared
        self.assertEqual(mapping.x, "test")
        self.assertEqual(mapping.y1, None)
        self.assertEqual(mapping.y3, None)
        self.assertEqual(mapping.date_format, None)
        self.assertEqual(mapping.x_ticks_count, 2)
        self.assertTrue(isinstance(mapping.y2, LinesSettings))

    def test_post_init_dateformat_invalid(self) -> None:
        """Tests the post_init with invalid dateformat"""
        config = {"x": "test", "y2": {"input": "test"}, "date_format": "%23"}
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Mapping(**config)
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("Date format is invalid" in buf.getvalue())

    @patch("cli.cmd_plot.drawer.settings_graph.datetime")
    def test_post_init_strftime_return_values(self, mock_datetime: Mock) -> None:
        """Tests the post_init with invalid dateformat and problematic strftime
        return value (Linux related unittest)"""
        config = {"x": "test", "y2": {"input": "test"}, "date_format": "%23"}
        mock_datetime.now().strftime.return_value = "%2 3"
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Mapping(**config)
        mock_datetime.now().strftime.assert_called_once_with("%23")
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("Date format is invalid" in buf.getvalue())


class TestLinesSettingsPostInit(unittest.TestCase):
    """Class to test the post_init method of the LinesSettings dataclass"""

    def test_post_init_with_label_as_string(self) -> None:
        """Tests the post_init with label as string"""
        lines_setting = LinesSettings(["input"], labels=["input"])
        self.assertEqual(list(lines_setting.labels), ["input"])

    def test_post_init_with_label_as_true(self) -> None:
        """Tests the post_init with label true and input has one element"""
        lines_setting = LinesSettings(["input"])
        self.assertEqual(list(lines_setting.labels), ["input"])

    def test_post_init_with_label_input_multiple_elements(self) -> None:
        """Tests the post_init with label true and input has multiple elements"""
        lines_setting = LinesSettings(["input_1", "input_2", "input_3"])
        self.assertEqual(list(lines_setting.labels), ["input_1", "input_2", "input_3"])


if __name__ == "__main__":
    unittest.main()
