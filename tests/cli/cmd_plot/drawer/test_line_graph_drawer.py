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

"""Testing file 'cli/cmd_plot/drawer/line_graph_drawer.py'."""

import io
import shutil
import sys
import unittest
from contextlib import redirect_stderr
from datetime import datetime
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

try:
    from cli.cmd_plot.drawer.line_graph_drawer import LineGraphDrawer
    from cli.cmd_plot.drawer.settings_graph import (
        Description,
        GraphSettings,
        LinesSettings,
        Mapping,
    )
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_plot.drawer.line_graph_drawer import LineGraphDrawer
    from cli.cmd_plot.drawer.settings_graph import (
        Description,
        GraphSettings,
        LinesSettings,
        Mapping,
    )
    from cli.helpers.misc import PROJECT_BUILD_ROOT

PATH_EXECUTION = Path(__file__).parent.parent / "test_execution"
PATH_DATA = Path(__file__).parent.parent / "test_data"


class TestDraw(unittest.TestCase):
    """Class to test the draw method of the LineGraphDrawer class"""

    def setUp(self) -> None:
        self.data = pd.read_csv((PATH_DATA / "input_data.csv"), skiprows=4)
        self.line = LinesSettings(input=["Voltage(V)"], labels=["Cell Voltage"])
        _, self.axes = plt.subplots()

    @patch("matplotlib.dates.DateFormatter")
    @patch("matplotlib.ticker.LinearLocator")
    @patch("cli.cmd_plot.drawer.line_graph_drawer.LineGraphDrawer._draw_line")
    def test_one_y_axis(
        self, mock_draw_line: Mock, mock_locator: Mock, mock_formatter: Mock
    ) -> None:
        """Tests draw method with only one y axis"""
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Cell Voltage (V)"],
            ),
            mapping=Mapping(
                x="Time",
                date_format="%S",
                x_ticks_count=2,
                y1=self.line,
                y2=None,
                y3=None,
            ),
            axes=self.axes,
            name="test_1",
        )
        line_graph_drawer._axes = Mock()  # pylint: disable=protected-access
        mock_draw_line.return_value = ["draw"]
        line_graph_drawer.draw(data=self.data)
        line_graph_drawer._axes.legend.assert_called_once_with(handles=["draw"])  # pylint: disable=protected-access
        line_graph_drawer._axes.set_title.assert_called_once_with("Test Plot")  # pylint: disable=protected-access
        line_graph_drawer._axes.set_xlabel.assert_called_once_with("Date")  # pylint: disable=protected-access
        mock_locator.assert_called_once_with(2)
        mock_formatter.assert_called_once_with("%S")

    @patch("cli.cmd_plot.drawer.line_graph_drawer.LineGraphDrawer._draw_line")
    def test_two_y_axes(self, mock_draw_line: Mock) -> None:
        """Tests draw method with two y axis"""
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Cell Voltage (V)", "Cell Voltage (V)"],
            ),
            mapping=Mapping(
                x="Time", x_ticks_count=2, y1=self.line, y2=self.line, y3=None
            ),
            axes=self.axes,
            name="test_1",
        )
        line_graph_drawer._axes = Mock()  # pylint: disable=protected-access
        mock_draw_line.return_value = ["draw"]
        line_graph_drawer.draw(data=self.data)
        line_graph_drawer._axes.twinx.assert_called_once()  # pylint: disable=protected-access

    @patch("cli.cmd_plot.drawer.line_graph_drawer.plt")
    @patch("cli.cmd_plot.drawer.line_graph_drawer.LineGraphDrawer._draw_line")
    def test_three_y_axes(self, mock_draw_line: Mock, mock_plt: Mock) -> None:
        """Tests draw method with only one y axis"""
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Cell Voltage (V)", "Cell Voltage (V)", "Cell Voltage (V)"],
            ),
            mapping=Mapping(
                x="Time", x_ticks_count=2, y1=self.line, y2=self.line, y3=self.line
            ),
            axes=self.axes,
            name="test_1",
        )
        line_graph_drawer._axes = Mock()  # pylint: disable=protected-access
        mock_axis = Mock()
        line_graph_drawer._axes.twinx.return_value = mock_axis  # pylint: disable=protected-access
        mock_draw_line.return_value = ["draw"]
        line_graph_drawer.draw(data=self.data)
        mock_axis.spines.right.set_position.assert_called_once_with(("axes", 1.2))
        mock_plt.subplots_adjust.assert_called_once_with(right=0.75)

    def test_key_error(self) -> None:
        """Tests draw method with KeyError"""
        line = LinesSettings(input=["test"], labels=["Cell Voltage"])
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["test"],
            ),
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line),
            axes=self.axes,
            name="test_1",
        )
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            line_graph_drawer.draw(data=self.data)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Column 'test' is not known", buf.getvalue())

    def test_index_error(self) -> None:
        """Tests draw method with IndexError"""
        line = LinesSettings(input=["Voltage(V)"], labels=["Cell Voltage"])
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=[],
            ),
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line),
            axes=self.axes,
            name="test_1",
        )
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            line_graph_drawer.draw(data=self.data)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Number of y_axes descriptions", buf.getvalue())

    def test_stop_iteration_error(self) -> None:
        """Tests draw method with StopIterationErrors error"""
        line = LinesSettings(
            input=["Voltage(V)", "Voltage(V)"], labels=["Cell Voltage"]
        )
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Voltage (V)"],
            ),
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line),
            axes=self.axes,
            name="test_1",
        )
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            line_graph_drawer.draw(data=self.data)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Number of labels does not", buf.getvalue())

    @patch("cli.cmd_plot.drawer.line_graph_drawer.LineGraphDrawer._draw_line")
    def test_x_value_as_string_warning(self, _: Mock) -> None:
        """Tests draw method with x_values as string warning"""
        line = LinesSettings(input=["Voltage(V)"], labels=["Cell Voltage"])
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Voltage (V)"],
            ),
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line),
            axes=self.axes,
            name="test_1",
        )
        buf = io.StringIO()
        mock_data = MagicMock()
        # pylint: disable=C2801
        mock_data.loc.__getitem__().to_numpy().dtype.type = np.object_
        with redirect_stderr(buf):
            line_graph_drawer.draw(data=mock_data)
        self.assertIn(
            "The data type of the x values is not numerical or "
            "datetime. The number of x-ticks can vary!",
            buf.getvalue(),
        )


class TestDrawLine(unittest.TestCase):
    """Class to test the draw_line method of the LineGraphDrawer class"""

    def setUp(self) -> None:
        self.start_time = datetime.now()
        _, axes = plt.subplots()
        self.axis = axes
        self.description = Description(
            title="Test Plot",
            x_axis="Date",
            y_axes=["Cell Voltage (V)"],
        )
        self.x_values = np.array([1, 2, 3])
        self.y_values = np.array([2, 4, 6])

    @patch("cli.cmd_plot.drawer.line_graph_drawer.Axes.plot")
    def test_with_min_max(self, mock_plot: Mock) -> None:
        """Tests the function draw_line with provided min max values"""
        line = LinesSettings(
            input=["column 1", "column 2"],
            labels=["test 1", "test 2"],
            factor=2,
            min=2,
            max=15,
        )
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=False, save=True),
            descriptions=self.description,
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line, y2=None, y3=None),
            axes=self.axis,
            name="test_1",
        )
        line_graph_drawer._draw_line(self.axis, self.x_values, self.y_values, line)  # pylint: disable=protected-access
        call_args, call_kwargs = mock_plot.call_args
        self.assertEqual(list(call_args[0]), list(self.x_values))
        self.assertEqual(list(call_args[1]), list(self.y_values * 2))
        self.assertEqual(call_kwargs["label"], "test 1")
        self.assertEqual(call_kwargs["scaley"], False)
        self.assertEqual(self.axis.get_ylim(), (np.float64(2.0), np.float64(15.0)))
        # test for color is not possible because of the iterator

    @patch("cli.cmd_plot.drawer.line_graph_drawer.Axes.plot")
    def test_witout_min_max(self, mock_plot: Mock) -> None:
        """Tests the function draw_line without min max values"""
        line = LinesSettings(
            input=["column 1", "column 2"], labels=["test 1", "test 2"], factor=10
        )
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=False, save=True),
            descriptions=self.description,
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line, y2=None, y3=None),
            axes=self.axis,
            name="test_1",
        )
        line_graph_drawer._draw_line(self.axis, self.x_values, self.y_values, line)  # pylint: disable=protected-access
        call_args, call_kwargs = mock_plot.call_args
        self.assertEqual(list(call_args[0]), list(self.x_values))
        self.assertEqual(list(call_args[1]), list(self.y_values * 10))
        self.assertEqual(call_kwargs["label"], "test 1")
        self.assertEqual(call_kwargs["scaley"], True)


class TestShow(unittest.TestCase):
    """Class to test the show method of the LineGraphDrawer class"""

    def setUp(self) -> None:
        """Setup test objects"""
        line_y1 = LinesSettings(input=["Voltage(V)"], labels=["Cell Voltage"])
        _, axes = plt.subplots(figsize=(7, 7), dpi=100)
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=True, save=False),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Cell Voltage (V)"],
            ),
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line_y1, y2=None, y3=None),
            axes=axes,
            name="test_1",
        )
        self.line_graph_drawer = line_graph_drawer

    @patch("matplotlib.pyplot.show")
    def test_set_true(self, mock_show: Mock) -> None:
        """Tests the function show with parameter set true"""
        self.line_graph_drawer.show()
        mock_show.assert_called_once_with()
        plt.clf()
        plt.close("all")

    @patch("matplotlib.pyplot.show")
    def test_set_false(self, mock_show: Mock) -> None:
        """Tests the function show with parameter set false"""
        self.line_graph_drawer._graph.show = False  # pylint: disable=protected-access
        self.line_graph_drawer.show()
        mock_show.assert_not_called()


class TestSave(unittest.TestCase):
    """Class to test the save method of the LineGraphDrawer class"""

    def setUp(self) -> None:
        self.start_time = datetime.now()
        line_y1 = LinesSettings(input=["Values 1"], labels=["Values 1"])
        _, axes = plt.subplots(figsize=(7, 7), dpi=100)
        line_graph_drawer = LineGraphDrawer(
            graph=GraphSettings(show=False, save=True),
            descriptions=Description(
                title="Test Plot",
                x_axis="Date",
                y_axes=["Cell Voltage (V)"],
            ),
            mapping=Mapping(x="Time", x_ticks_count=2, y1=line_y1, y2=None, y3=None),
            axes=axes,
            name="test_1",
        )
        self.line_graph_drawer = line_graph_drawer

    def tearDown(self) -> None:
        """Clean up the test files"""
        remove_test_files(self.start_time)

    @patch("matplotlib.pyplot.savefig")
    def test_save_set_true(self, mock_savefig: Mock) -> None:
        """Tests the function save with parameter set true"""
        self.line_graph_drawer.save(PATH_DATA)
        mock_savefig.assert_called_once_with(
            fname=Path(PATH_DATA) / Path("test_1.png"), dpi=100, format="png"
        )
        plt.clf()
        plt.close("all")

    @patch("matplotlib.pyplot.savefig")
    def test_save_set_false(self, mock_savefig: Mock) -> None:
        """Tests the function save with parameter set false."""
        self.line_graph_drawer._graph.save = False  # pylint: disable=protected-access
        self.line_graph_drawer.save(PATH_DATA)
        mock_savefig.assert_not_called()


class TestValidateConfig(unittest.TestCase):
    """Class to test the validate_config method of the LineGraphDrawer class"""

    def setUp(self):
        self.config = {
            "name": "hypo_test",
            "mapping": {
                "y1": {"input": ["420_DCDC_PSU_Voltage_V", "420_DCDC_TN_Voltage_V"]},
                "x": "Date",
                "x_ticks_count": 8,
                "y2": {"input": ["420_DCDC_PSU_Current_A"]},
                "date_format": "%d %H:%M:%S",
            },
            "description": {
                "x_axis": "Date",
                "y_axes": ["DCDC Voltage", "Current"],
                "title": "Hyprotrade Test",
            },
            "graph": {"show": True, "save": True, "format": "png"},
        }

    def test_validate_success(self):
        """Tests the validate_config method with valid config"""
        LineGraphDrawer.validate_config(self.config)

    def test_validate_fail(self):
        """Tests the validate_config method with invalid config"""
        self.config["name"] = 2
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            LineGraphDrawer.validate_config(self.config)
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("Line graph config validation error", buf.getvalue())


def remove_test_files(start_time: datetime) -> None:
    """Remove all data from the given directory except for the
    file "input_data.csv"."""
    for root, dirs, files in (PATH_DATA).walk(top_down=False):
        for name in files:
            if "input_data.csv" not in name:
                (root / name).unlink()
        for name in dirs:
            (root / name).rmdir()

    if (PROJECT_BUILD_ROOT / "plots").is_dir():
        if (
            datetime.fromtimestamp((PROJECT_BUILD_ROOT / "plots").stat().st_birthtime)
            > start_time
        ):
            shutil.rmtree(PROJECT_BUILD_ROOT / "plots")
        else:
            dirs = [d for d in (PROJECT_BUILD_ROOT / "plots").iterdir() if d.is_dir()]
            for d in dirs:
                if datetime.fromtimestamp(d.stat().st_birthtime) > start_time:
                    shutil.rmtree(d)


if __name__ == "__main__":
    unittest.main()
