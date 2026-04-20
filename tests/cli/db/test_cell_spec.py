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

"""Testing file 'cli/db/cell_spec.py'."""

import sys
import unittest
from dataclasses import FrozenInstanceError
from pathlib import Path

try:
    from cli.db.cell_spec import CellSpec
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.db.cell_spec import CellSpec


def make_valid_kwargs():
    """Return a base set of valid kwargs for CellSpec."""
    return {
        "name": "X100",
        "manufacturer": "Acme",
        "shape": "prismatic",
        "chemistry": "NMC",
        "height": 0.120,
        "length": 0.250,
        "width": 0.070,
        "weight": 0.750,
        "temperature_max": 60.0,
        "temperature_min": -20.0,
        "voltage_min": 2.5,
        "voltage_max": 4.2,
        "voltage_nom": 3.7,
    }


class TestCellSpecPostInit(unittest.TestCase):
    """Tests for CellSpec.__post_init__ validation."""

    def test_valid_instance(self) -> None:
        """Should construct without errors for valid input."""
        kwargs = make_valid_kwargs()
        spec = CellSpec(**kwargs)
        # Basic attribute checks
        self.assertEqual(spec.name, "X100")
        self.assertEqual(spec.manufacturer, "Acme")
        self.assertEqual(spec.shape, "prismatic")
        self.assertEqual(spec.voltage_nom, 3.7)

    def test_string_type_validation(self) -> None:
        """Should raise TypeError if any string field is not a str."""
        kwargs = make_valid_kwargs()
        kwargs["manufacturer"] = 123  # not a str
        with self.assertRaises(TypeError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("not a str", str(ctx.exception))

    def test_float_type_validation(self) -> None:
        """Should raise TypeError if any numeric field is not a float."""
        kwargs = make_valid_kwargs()
        kwargs["height"] = 1  # int instead of float
        with self.assertRaises(TypeError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("not a float", str(ctx.exception))

    def test_cylindrical_width_length_mismatch(self) -> None:
        """Should raise ValueError if shape is cylindrical and width != length."""
        kwargs = make_valid_kwargs()
        kwargs["shape"] = "cylindrical"
        kwargs["length"] = 0.018
        kwargs["width"] = 0.017  # mismatch
        with self.assertRaises(ValueError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("cylindrical", str(ctx.exception))

    def test_cylindrical_width_length_match(self) -> None:
        """Should raise ValueError if shape is cylindrical and width != length."""
        kwargs = make_valid_kwargs()
        kwargs["shape"] = "cylindrical"
        kwargs["length"] = 0.018
        kwargs["width"] = 0.018
        CellSpec(**kwargs)

    def test_temperature_range_invalid(self) -> None:
        """Should raise ValueError if temperature_max < temperature_min."""
        kwargs = make_valid_kwargs()
        kwargs["temperature_max"] = -10.0
        kwargs["temperature_min"] = 0.0
        with self.assertRaises(ValueError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("temperature", str(ctx.exception).lower())

    def test_voltage_range_invalid(self) -> None:
        """Should raise ValueError if voltage_max < voltage_min."""
        kwargs = make_valid_kwargs()
        kwargs["voltage_min"] = 3.0
        kwargs["voltage_max"] = 2.9
        with self.assertRaises(ValueError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("voltage", str(ctx.exception).lower())

    def test_nominal_voltage_out_of_range_high(self) -> None:
        """Should raise ValueError if voltage_nom > voltage_max."""
        kwargs = make_valid_kwargs()
        kwargs["voltage_nom"] = 4.3  # above max
        with self.assertRaises(ValueError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("Nom. cell voltage", str(ctx.exception))

    def test_nominal_voltage_out_of_range_low(self) -> None:
        """Should raise ValueError if voltage_nom < voltage_min."""
        kwargs = make_valid_kwargs()
        kwargs["voltage_nom"] = 2.4  # below min
        with self.assertRaises(ValueError) as ctx:
            CellSpec(**kwargs)
        self.assertIn("Nom. cell voltage", str(ctx.exception))

    def test_frozen_dataclass_immutability(self) -> None:
        """Should be immutable: assigning any attribute raises FrozenInstanceError."""
        spec = CellSpec(**make_valid_kwargs())
        with self.assertRaises(FrozenInstanceError):
            spec.name = "NewName"


if __name__ == "__main__":
    unittest.main()
