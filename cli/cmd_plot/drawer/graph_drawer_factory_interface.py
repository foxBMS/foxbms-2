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

"""GraphDrawerFactory Interface"""

import sys
from abc import ABC, abstractmethod

from ...helpers.click_helpers import recho
from .graph_types import GraphTypes
from .line_graph_drawer import LineGraphDrawer


class GraphDrawerFactoryInterface(ABC):  # pylint: disable=too-few-public-methods
    """Interface class that creates a LineGraphDrawer object"""

    @abstractmethod
    def get_object(self, graph_config: dict) -> LineGraphDrawer:
        """Creates a LineGraphDrawer object from the given configuration."""

    @staticmethod
    def _get_graph_type(graph_config: dict) -> GraphTypes:
        """Determines the graph type and returns it"""
        try:
            # The plot configuration is defined as a list of dictionaries and
            # the method _get_graph_type gets one of these dictionaries passed
            # as parameter graph_config.
            if not isinstance(graph_config, dict):
                recho(
                    "Plot configuration is not a list of dictionaries. "
                    "Please check the plot configuration format."
                )
                sys.exit(1)
            graph_type = graph_config["type"]
            return GraphTypes[str(graph_type).split("_", maxsplit=1)[0]]
        except KeyError:
            if "graph_type" in locals():
                recho(
                    f"Graph type {str(graph_type).split('_', maxsplit=1)[0]}"
                    "is not valid."
                )
            else:
                recho(
                    "One of the graph configurations does not contain the "
                    "manditory key 'type'."
                )
            sys.exit(1)
