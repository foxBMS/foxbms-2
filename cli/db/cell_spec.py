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

"""Dataclass for cell specification parameters.

This module defines the immutable `CellSpec` dataclass containing the
manufacturer, geometry, chemistry, mass, temperature limits and voltage
limits of a lithium-ion cell. Input validation is performed in `__post_init__`.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class CellSpec:  # pylint: disable=R0902
    """Specification of a lithium-ion cell.

    Attributes:
        name: Model name of the cell.
        manufacturer: Manufacturer of the cell.
        shape: Geometrical shape (e.g., prismatic, pouch, cylindrical).
        chemistry: Chemistry description (e.g., NMC, LFP).
        height: Height of the cell in meters.
        length: Length of the cell in meters.
        width: Width of the cell in meters.
        weight: Mass of the cell in kilograms.
        temperature_max: Maximum allowed cell temperature in degrees Celsius.
        temperature_min: Minimum allowed cell temperature in degrees Celsius.
        voltage_min: Minimum allowed cell voltage in volts.
        voltage_max: Maximum allowed cell voltage in volts.
        voltage_nom: Nominal cell voltage in volts.
    """

    name: str
    manufacturer: str
    shape: str
    chemistry: str
    height: float
    length: float
    width: float
    weight: float
    temperature_max: float
    temperature_min: float
    voltage_min: float
    voltage_max: float
    voltage_nom: float

    def __post_init__(self) -> None:
        """Validate attributes and enforce basic consistency rules.

        - String fields must be instances of ``str``.
        - Numeric fields must be instances of ``float``.
        - For ``shape == 'cylindrical'``, ``width`` must equal ``length``.
        - ``temperature_max`` must be >= ``temperature_min``.
        - ``voltage_max`` must be >= ``voltage_min``.
        - ``voltage_nom`` must be within the voltage limits.

        Raises:
            TypeError
                If any string field is not a ``str`` or any numeric field is not a ``float``.
            ValueError
                If geometric, temperature or voltage consistency checks fail.
        """
        str_attributes = [
            self.name,
            self.manufacturer,
            self.shape,
            self.chemistry,
        ]
        if not all(isinstance(x, str) for x in str_attributes):
            err_txt = (
                "At least one parameter in the cell specifications is "
                "not a str but should be."
            )
            raise TypeError(err_txt)
        float_attributes = [
            self.height,
            self.length,
            self.weight,
            self.temperature_max,
            self.temperature_min,
            self.voltage_max,
            self.voltage_min,
        ]
        if not all(isinstance(x, float) for x in float_attributes):
            err_txt = (
                "At least one parameter in the cell specifications is "
                "not a float but should be."
            )
            raise TypeError(err_txt)
        if self.shape == "cylindrical" and self.width != self.length:
            err_msg = "For a cylindrical cell both width and length should be the same."
            raise ValueError(err_msg)
        if self.temperature_max < self.temperature_min:
            err_txt = "Max. cell temperature is lower than min. cell temperature."
            raise ValueError(err_txt)
        if self.voltage_max < self.voltage_min:
            err_txt = "Max. cell voltage is lower than min. cell voltage."
            raise ValueError(err_txt)
        if self.voltage_nom > self.voltage_max or self.voltage_nom < self.voltage_min:
            err_txt = "Nom. cell voltage is outside the voltage limits."
            raise ValueError(err_txt)
