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

"""Testing file 'cli/cmd_plot/execution.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import Mock, call, patch

try:
    from cli.cmd_plot.data_handling.data_source_types import DataSourceTypes
    from cli.cmd_plot.drawer.graph_types import GraphTypes
    from cli.cmd_plot.execution import Executor
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_plot.data_handling.data_source_types import DataSourceTypes
    from cli.cmd_plot.drawer.graph_types import GraphTypes
    from cli.cmd_plot.execution import Executor
PATH_DATA = Path(__file__).parent / "test_data"
PATH_EXECUTION = Path(__file__).parent / "test_execution"


class TestInit(unittest.TestCase):
    """Class to test the init method of the Executor class"""

    def setUp(self) -> None:
        self.config = {
            "input_data": [PATH_DATA],
            "data_config": PATH_EXECUTION / "test_data_source_config.yaml",
            "plot_config": PATH_EXECUTION / "test_plot_config.yaml",
            "data_source_type": "CSV",
            "output": None,
        }

    def test_init_valid_config(self) -> None:
        """Tests the init with valid config"""
        Executor(**self.config)

    def test_read_config_invalid_yaml(self) -> None:
        """Tests the read_config method with a valid yaml"""
        self.config["plot_config"] = PATH_EXECUTION / "test_yaml_error.yaml"
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Executor(**self.config)
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("Invalid configuration file" in buf.getvalue())

    def test_data_source_type_none(self) -> None:
        """Test the get_data_source_type method with data_source_type as None"""
        self.config["data_source_type"] = None
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Executor(**self.config)
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue(
            "Data source type is required when a directory" in buf.getvalue()
        )

    def test_data_source_type_not_valid(self) -> None:
        """Test the get_data_source_type method with invalid data_source_type"""
        self.config["data_source_type"] = "TEST"
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Executor(**self.config)
        self.assertEqual(cm.exception.code, 1)
        self.assertTrue("is not valid for input" in buf.getvalue())

    def test_data_source_type_with_single_file(self) -> None:
        """Test the get_data_source_type method with single file"""
        self.config["data_source_type"] = None
        self.config["input_data"] = [PATH_DATA / "input_data.csv"]
        exe = Executor(**self.config)
        self.assertEqual(exe.data_source_type, DataSourceTypes["CSV"])


class TestCreatePlots(unittest.TestCase):
    """Class to test the create_plots method of the Executor class"""

    def setUp(self) -> None:
        self.config = {
            "input_data": [PATH_DATA],
            "data_config": PATH_EXECUTION / "test_data_source_config.yaml",
            "plot_config": PATH_EXECUTION / "test_plot_config.yaml",
            "data_source_type": "CSV",
            "output": None,
        }
        self.executor = Executor(**self.config)

    @patch("pathlib.Path.mkdir")
    @patch("cli.cmd_plot.drawer.graph_drawer_factory.GraphDrawerFactory.get_object")
    @patch(
        "cli.cmd_plot.data_handling.data_handler_factory.DataHandlerFactory.get_object"
    )
    def test_create_plots(
        self, mock_data_get: Mock, mock_drawer_get: Mock, mock_mkdir: Mock
    ) -> None:
        """Tests the create_plots method with valid executor object"""
        self.executor.create_plots()  # pylint: disable=protected-access
        mock_data_get.assert_called_once_with(
            self.executor.data_source_type, self.executor.data_config
        )
        mock_mkdir.assert_called_once()
        data_files = self.executor._get_data_files()  # pylint: disable=protected-access
        for file in data_files:
            plot_dir = Path(self.executor.output) / Path(file).stem
            for key, value in self.executor.plot_config.items():
                grap_type = Executor._get_graph_type(key)  # pylint: disable=protected-access
                mock_drawer_get.assert_has_calls([call(grap_type, value)])
                mock_drawer_get().draw.assert_has_calls(
                    [call(data=mock_data_get().get_data())]
                )
                mock_drawer_get().save.assert_has_calls([call(plot_dir)])
                mock_drawer_get().show.assert_called_with()


class TestGetDataFiles(unittest.TestCase):
    """Class to test the get_data_files method of the Executor class"""

    def setUp(self) -> None:
        self.config = {
            "input_data": [PATH_DATA],
            "data_config": PATH_EXECUTION / "test_data_source_config.yaml",
            "plot_config": PATH_EXECUTION / "test_plot_config.yaml",
            "data_source_type": "CSV",
            "output": None,
        }

    def test_input_as_dir(self) -> None:
        """Test the get_data_files method with a directory as input"""
        files = Executor(**self.config)._get_data_files()  # pylint: disable=protected-access
        self.assertEqual(files[0], PATH_DATA / "input_data.csv")

    def test_input_as_file(self) -> None:
        """Test the get_data_files method with file as input"""
        self.config["input_data"] = [PATH_DATA / "input_data.csv"]
        files = Executor(**self.config)._get_data_files()  # pylint: disable=protected-access
        self.assertEqual(files[0], PATH_DATA / "input_data.csv")

    def test_input_no_file_or_dir(self) -> None:
        """Test the get_data_files method with file as input"""
        self.config["input_data"] = [PATH_DATA / "data.csv"]
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Executor(**self.config)._get_data_files()  # pylint: disable=protected-access
        self.assertEqual(cm.exception.code, 1)
        self.assertIn(
            "Input data has to contain files or directories only.", buf.getvalue()
        )


class TestGetGraphType(unittest.TestCase):
    """Class to test the get_graph_type method of the Executor class"""

    def test_valid_graph_type(self) -> None:
        """Tests the get_graph_type method with a valid graph type"""
        graph_type = Executor._get_graph_type("LINE_1")  # pylint: disable=protected-access
        self.assertEqual(graph_type, GraphTypes["LINE"])

    def test_invalid_graph_type(self) -> None:
        """Tests the get_graph_type method with a invalid graph type"""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            Executor._get_graph_type("test")  # pylint: disable=protected-access
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("is not valid", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
