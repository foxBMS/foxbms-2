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

"""Implementation of the GraphDrawerFactory class to create
and return a LineGraphDrawer object for the given data."""

import sys

import matplotlib
import matplotlib.pyplot as plt
import scienceplots  # pylint: disable=import-error,unused-import # noqa: F401

from ...helpers.click_helpers import recho
from .graph_drawer_factory_interface import (
    GraphDrawerFactoryInterface,
)
from .graph_types import GraphTypes
from .line_graph_drawer import LineGraphDrawer
from .settings_graph import (
    Description,
    GraphSettings,
    Mapping,
)


class GraphDrawerFactory(GraphDrawerFactoryInterface):  # pylint: disable=too-few-public-methods
    """Class that implements the interface GraphDrawerFactory"""

    def get_object(self, graph_type: GraphTypes, config: dict) -> LineGraphDrawer:
        """Creates a LineGraphDrawer object from the given configuration."""
        match graph_type:
            case GraphTypes.LINE:
                # Create all relevant properties for GraphDrawer
                LineGraphDrawer.validate_config(config)
                mapping = Mapping(**config["mapping"])
                graph_settings = GraphSettings(**config["graph"])
                description = Description(**config["description"])
                plt.style.use(["science", "ieee", "no-latex"])
                matplotlib.rc("font", size=16)
                # avoid overlap of x and y-axis values
                matplotlib.rcParams["xtick.major.pad"] = 12
                _, axes = plt.subplots(
                    figsize=(
                        graph_settings.width_px / graph_settings.dpi,
                        graph_settings.height_px / graph_settings.dpi,
                    ),
                    dpi=graph_settings.dpi,
                )
                line_graph_drawer = LineGraphDrawer(
                    graph=graph_settings,
                    descriptions=description,
                    mapping=mapping,
                    axes=axes,
                    name=config["name"],
                )
                return line_graph_drawer
            case _:
                recho("The given graph type hasn't been implemented.")
                sys.exit(1)
