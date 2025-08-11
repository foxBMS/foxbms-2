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

"""Implementation of the Executor class."""

import sys
import warnings
from pathlib import Path
from typing import TypedDict, Unpack

from yaml import YAMLError, safe_load

from ..helpers.click_helpers import recho
from ..helpers.misc import PROJECT_BUILD_ROOT, file_name_from_current_time
from .data_handling.data_handler_factory import DataHandlerFactory
from .data_handling.data_source_types import DataSourceTypes
from .drawer.graph_drawer_factory import GraphDrawerFactory
from .drawer.line_graph_drawer import LineGraphDrawer


class KwargsTyping(TypedDict):
    """Types of the Executor kwargs argument"""

    data_source_type: str | None
    no_tmp: bool


class Executor:  # pylint: disable=too-few-public-methods
    """Class to execute the plot software"""

    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def __init__(
        self,
        input_data: list[Path],
        data_config: Path,
        plot_config: Path,
        output: Path | None = None,
        **kwargs: Unpack[KwargsTyping],
    ) -> None:
        """Initialise the Executor"""
        self.input_data = input_data
        data_source_type = kwargs.get("data_source_type", None)
        self.data_source_type = Executor._get_data_source_type(
            data_source_type, input_data
        )
        self.data_config = self._read_config(data_config)
        self.plot_config = self._read_config(plot_config)
        if (output is None) or (Path(output).is_file()):
            output = PROJECT_BUILD_ROOT / "plots" / file_name_from_current_time()
        self.output = output
        self.no_tmp: bool = kwargs.get("no_tmp", False)

    def create_plots(self) -> None:
        """Create plots from the given data."""
        try:
            handler = DataHandlerFactory().get_object(
                self.data_source_type, self.data_config
            )
            data_files = self._get_data_files()
            for file in data_files:
                data = handler.get_data(Path(file), self.no_tmp)
                # Create directory for the images of the plots
                plot_dir = Path(self.output) / Path(file).stem
                plot_dir.mkdir(parents=True, exist_ok=True)
                # Create and show/save a plot for each graph in plot_config
                for graph_config in self.plot_config:
                    # reinitialize the context manager to avoid checking old warnings
                    with warnings.catch_warnings(record=True) as w:
                        graph_drawer = GraphDrawerFactory().get_object(graph_config)
                        graph_drawer.draw(data=data)
                        graph_drawer.save(plot_dir)
                        graph_drawer.show()
                        Executor._handle_pyplot_warnings(w, graph_drawer)
        except TypeError:
            # Handles the case where self.plot_config is not a list
            recho(
                "Plot configuration is not a list of dictionaries. "
                "Please check the plot configuration format."
            )
            sys.exit(1)

    def _get_data_files(self) -> list[Path]:
        """Get all files from input_data attribute"""
        files = []
        for data in self.input_data:
            if Path(data).is_file():
                files.append(data)
            elif Path(data).is_dir():
                ext = str(self.data_source_type.name).lower()
                files = files + [
                    i for i in sorted(Path(data).glob(f"*.{ext}")) if i.is_file()
                ]
            else:
                recho("Input data has to contain files or directories only.")
                sys.exit(1)
        return files

    def _read_config(self, config_path: Path) -> dict:
        """Read the given configuration file and save all contained data"""
        try:
            with open(config_path, encoding="utf-8") as stream:
                config_content = safe_load(stream=stream)
            return config_content
        except (UnicodeDecodeError, YAMLError) as err:
            recho(f"Invalid configuration file {config_path}: {err}")
            sys.exit(1)

    @staticmethod
    def _get_data_source_type(
        data_source_type: str | None, input_data: list[Path]
    ) -> DataSourceTypes:
        """Determines the data source type depending on the passed input data
        and the type string"""
        if data_source_type is None:
            num_dirs = len([d for d in input_data if d.is_dir()])
            if num_dirs > 0:
                recho(
                    "Data source type is required when a directory is passed as input data."
                )
                sys.exit(1)
            else:
                data_source_type = input_data[0].suffix[1:].upper()
        try:
            return DataSourceTypes[data_source_type]
        except KeyError:
            recho(f"Data Type {data_source_type} is not valid for input data.")
            sys.exit(1)

    @staticmethod
    def _handle_pyplot_warnings(
        warning_handle: list, graph_drawer: LineGraphDrawer
    ) -> None:
        """This method should handle all warning coming from pyplot"""
        if warning_handle:
            warning_texts = [str(x.message) for x in warning_handle]
            if any("Tight layout not applied" in x for x in warning_texts):
                recho(
                    f"Plot layout of {graph_drawer.name} seems too "
                    "small to generate the plot correctly. "
                    "It is recommended to increase the pixel width "
                    "and height.",
                    "yellow",
                )
