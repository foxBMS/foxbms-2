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

"""Testing file 'cli/cmd_plot/data_handling/data_handler_factory.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

from yaml import YAMLError, safe_load

try:
    from cli.cmd_plot.data_handling.csv_handler import CSVHandler
    from cli.cmd_plot.data_handling.data_handler_factory import DataHandlerFactory
    from cli.cmd_plot.data_handling.data_source_types import DataSourceTypes
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_plot.data_handling.csv_handler import CSVHandler
    from cli.cmd_plot.data_handling.data_handler_factory import DataHandlerFactory
    from cli.cmd_plot.data_handling.data_source_types import DataSourceTypes

PATH_EXECUTION = Path(__file__).parent.parent / "test_execution"


class TestGetObject(unittest.TestCase):
    """Class to test the get_object method of the DataHandlerFactory class"""

    def setUp(self) -> None:
        """Setups the factory object and the config"""
        with open(
            PATH_EXECUTION / "test_data_source_config.yaml", encoding="utf-8"
        ) as stream:
            try:
                self.test_data_source_config = safe_load(stream)
            except YAMLError as exc:
                raise exc
        self.factory = DataHandlerFactory()

    def test_get_object_csv(self) -> None:
        """Tests the function get_object with CSV as data type"""
        expected = CSVHandler(
            {
                "Time": "string",
                "Index": "float",
                "Voltage(V)": "float",
                "Current(A)": "float",
                "Capacity(Ah)": "float",
                "Power(W)": "float",
            },
            4,
            3,
        )
        actual = self.factory.get_object(
            DataSourceTypes.CSV, self.test_data_source_config
        )
        self.assertIsInstance(actual, CSVHandler)
        self.assertEqual(actual.skip, expected.skip)
        self.assertEqual(actual.precision, expected.precision)
        self.assertDictEqual(actual.columns, expected.columns)

    def test_get_object_unkown_data_type(self) -> None:
        """Tests the function get_object with unkown data type"""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            self.factory.get_object("not_csv", self.test_data_source_config)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Data source type not known", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
