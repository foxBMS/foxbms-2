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

"""Implementation of the LineGraphDrawer class to take given data and draw the graph."""

import sys
from copy import deepcopy
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from jsonschema import exceptions, validate
from matplotlib import dates, ticker
from matplotlib.axes import Axes
from yaml import safe_load

from ...helpers.click_helpers import echo, recho
from .line_graph_drawer_interface import LineGraphDrawerInterface
from .settings_graph import Description, GraphSettings, LinesSettings, Mapping


class LineGraphDrawer(LineGraphDrawerInterface):
    """Class that implements the interface LineGraphDrawer"""

    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def __init__(
        self,
        graph: GraphSettings,
        descriptions: Description,
        mapping: Mapping,
        axes: Axes,
        name: str,
    ) -> None:
        self._graph = graph
        self._descriptions = descriptions
        self._mapping = mapping
        self._axes = axes
        self.name = name
        self._cycler = plt.rcParams["axes.prop_cycle"]()

    def draw(self, data: pd.DataFrame) -> None:
        """Public method which draws the specified graph with error handling"""
        try:
            self._draw(data)
        except KeyError as e:
            recho(f"Error plot config file: Column {str(e)} is not known")
            sys.exit(1)
        except IndexError:
            recho(
                "Error plot config file: Number of y_axes descriptions/labels"
                " does not match number of y-axes."
            )
            sys.exit(1)
        except StopIteration:
            recho(
                "Error plot config file: Number of labels does not "
                "match with number of lines."
            )
            sys.exit(1)

    def show(self) -> None:
        """Shows the plot if wanted"""
        if self._graph.show:
            plt.show()
        else:
            plt.clf()
            plt.close("all")

    def save(self, output_dir: Path) -> None:
        """Saves the plot if wanted"""
        if self._graph.save:
            fname = Path(output_dir) / Path(self.name + "." + self._graph.format)
            echo(f"Save plot at {fname}")
            plt.savefig(
                fname=fname,
                dpi=self._graph.dpi,
                format=self._graph.format,
            )

    def _draw(self, data: pd.DataFrame) -> None:
        """Private method which draws the specified graph, without
        error handling"""
        # x_values should be numerical or date values because many string
        # values will cause problems with the x-axis of the plot
        x_values = data.loc[:, self._mapping.x].to_numpy()
        if self._mapping.date_format is not None:
            self._axes.xaxis.set_major_formatter(
                dates.DateFormatter(self._mapping.date_format)
            )
        elif x_values.dtype.type is np.object_:
            recho(
                "The data type of the x values is not numerical or "
                "datetime. The number of x-ticks can vary!",
                "yellow",
            )
        # Draw all lines in Mapping
        plots: list[matplotlib.lines.Line2D] = []
        for i, ax_name in enumerate(["y1", "y2", "y3"]):
            line_settings = getattr(self._mapping, ax_name)
            if isinstance(line_settings, LinesSettings):
                if i >= 1:
                    axis = self._axes.twinx()
                    if ax_name == "y3":
                        axis.spines.right.set_position(("axes", 1.2))
                        plt.subplots_adjust(right=0.75)
                else:
                    axis = self._axes
                axis.set_ylabel(self._descriptions.y_axes[i])
                # deepcopy needed otherwise iterator immediately
                # goes to last element
                for column in deepcopy(line_settings).input:
                    y_values = data.loc[:, column].to_numpy()
                    plots.extend(
                        self._draw_line(axis, x_values, y_values, line_settings)
                    )
        self._axes.legend(handles=plots)
        self._axes.set_title(self._descriptions.title)
        self._axes.set_xlabel(self._descriptions.x_axis)
        self._axes.xaxis.set_major_locator(
            ticker.LinearLocator(self._mapping.x_ticks_count)
        )

    def _draw_line(
        self,
        axis: Axes,
        x_values: np.ndarray,
        y_values: np.ndarray,
        settings: LinesSettings,
    ) -> list[matplotlib.lines.Line2D]:
        """Draws the specified line"""
        if not (y_values.dtype.type is np.str_ or y_values.dtype.type is np.object_):
            x_values = x_values[~np.isnan(y_values)]
            y_values = y_values[~np.isnan(y_values)]
        scaley = True
        if (settings.min is not None) and (settings.max is not None):
            if y_values.dtype.type is np.str_ or y_values.dtype.type is np.object_:
                recho(
                    "Min/Max axis limits for string y-values in plot "
                    f"'{self._descriptions.title}' are not allowed"
                )
                sys.exit(1)
            axis.set_ylim(settings.min, settings.max)
            scaley = False
        if y_values.dtype.type is np.str_ or y_values.dtype.type is np.object_:
            scaled_y_values = y_values
        else:
            scaled_y_values = y_values * settings.factor
        label = next(settings.labels)  # type: ignore
        return axis.plot(
            x_values,
            scaled_y_values,
            scaley=scaley,
            label=label,
            **next(self._cycler),
        )

    @staticmethod
    def validate_config(config: dict) -> None:
        """Validates the CSVHandler configuration"""
        schema_path = Path(__file__).parent / "schemas" / "line_graph_drawer.json"
        with open(schema_path, encoding="utf-8") as f:
            schema = safe_load(f)
        try:
            validate(config, schema=schema)
        except exceptions.ValidationError as e:
            error_text = str(e).splitlines()[0]
            recho(f"Line graph config validation error: {error_text}")
            sys.exit(1)
