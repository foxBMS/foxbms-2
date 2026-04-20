#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Dataclasses for current limit model parameters.

Defines:
- ``CurrentLimits``: paired arrays of currents and temperatures,
- ``CurrentLimitModel``: a model wrapping charge/discharge limits.
"""

from dataclasses import dataclass

from . import BaseModel


@dataclass
class CurrentLimits:
    """Dataclass for current limits as a function of temperature.

    Attributes:
        currents: Current limits corresponding to the given temperatures.
        temperatures: Temperatures aligned with the currents.
    """

    currents: list[float]
    temperatures: list[float]

    def __post_init__(self) -> None:
        """Validate container types and lengths for limits.

        Ensures both ``currents`` and ``temperatures`` are lists of floats
        and have the same length.

        Raises:
            TypeError
                If containers are not lists or elements are not floats,
                or if lengths differ.
        """
        if not isinstance(self.currents, list):
            err_txt = "The limits of the current are not provided as a list."
            raise TypeError(err_txt)
        if not all(isinstance(x, float) for x in self.currents):
            err_txt = "The limits of the current are not floats."
            raise TypeError(err_txt)
        if not isinstance(self.temperatures, list):
            err_txt = "The limits of the temperatures are not provided as a list."
            raise TypeError(err_txt)
        if not all(isinstance(x, float) for x in self.temperatures):
            err_txt = "The limits of the temperatures are not floats."
            raise TypeError(err_txt)
        if len(self.currents) != len(self.temperatures):
            err_txt = (
                "The same number of currents and temperatures must "
                "be provided as limits."
            )
            raise TypeError(err_txt)


@dataclass
class CurrentLimitModel(BaseModel):
    """Model parameter container for charge and discharge current limits.

    Attributes:
        charge: Current limits for charging
        discharge: Current limits for discharging
    """

    charge: CurrentLimits | dict
    discharge: CurrentLimits | dict

    def __post_init__(self) -> None:
        """Post-initialize by validating base fields and normalizing inputs.

        Converts ``charge`` and ``discharge`` from dicts to ``CurrentLimits`` if needed
        and calls ``BaseModel.__post_init__()`` for common validation.

        Raises:
            TypeError
                If nested ``CurrentLimits`` validation fails.
        """
        super().__post_init__()
        if isinstance(self.charge, dict):
            self.charge: CurrentLimits = CurrentLimits(**self.charge)
        if isinstance(self.discharge, dict):
            self.discharge: CurrentLimits = CurrentLimits(**self.discharge)
