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

"""Implementation of the classes that contain data for LineGraphDrawer."""

import sys
from dataclasses import dataclass
from datetime import datetime
from typing import Iterator

from ...helpers.click_helpers import recho


@dataclass(frozen=False, slots=True)
class LinesSettings:
    """Class that contains all settings for the Lines"""

    input: list[str]
    labels: Iterator[str] | None = None
    max: float | None = None
    min: float | None = None
    factor: float = 1

    def __post_init__(self) -> None:
        if self.labels is None:
            self.labels = iter(self.input)
        else:
            self.labels = iter(self.labels)


@dataclass(frozen=False, slots=True)
class GraphSettings:
    """Class that contains all information about the graph."""

    height_px: float = 700
    width_px: float = 700
    dpi: int = 100
    show: bool = True
    save: bool = False
    format: str = "png"


@dataclass(frozen=False, slots=True)
class Mapping:
    """Class that contains all information about the Mapping of the plot."""

    x: str
    y1: LinesSettings | None = None
    y2: LinesSettings | None = None
    y3: LinesSettings | None = None
    date_format: str | None = None
    x_ticks_count: int = 2

    def __post_init__(self) -> None:
        """Init the LineSettings for the y axes"""
        for attr in ["y1", "y2", "y3"]:
            if isinstance(self.__getattribute__(attr), dict):
                setattr(self, attr, LinesSettings(**self.__getattribute__(attr)))
        try:
            # This check is needed, because invalid date format
            # will cause Tkinter exception, which can not be
            # catched easily.
            if self.date_format is not None:
                date_str = datetime.now().strftime(self.date_format)
                # This if clause is needed on Linux, because strftime will
                # silently accept wrong date format strings. If the date
                # format string is wrong strftime returns the date format
                # string with additional whitespace on Linux instead of
                # the current datetime
                if date_str.replace(" ", "") == self.date_format:
                    raise ValueError
        except ValueError:
            recho("Error plot config file: Date format is invalid.")
            sys.exit(1)


@dataclass(frozen=True, slots=True)
class Description:
    """Class that contains all information of the Description of the plot."""

    title: str
    x_axis: str
    y_axes: list[str]
