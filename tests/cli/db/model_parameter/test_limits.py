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

"""Testing file 'cli/db/model_parameter/limits.py'."""

# cspell:ignore zpath

import io
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

try:
    from cli.db.model_parameter.limits import CurrentLimitModel, CurrentLimits
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.db.model_parameter.limits import CurrentLimitModel, CurrentLimits


def valid_limits_dict():
    """Return a valid limits dict for constructing CurrentLimits via kwargs."""
    return {
        "currents": [1.0, 2.0, 3.5],
        "temperatures": [-10.0, 25.0, 45.0],
    }


class TestCurrentLimitsPostInit(unittest.TestCase):
    """Tests for CurrentLimits.__post_init__ validation."""

    def test_valid_lists_of_floats_same_length(self):
        """Should construct when both lists are floats and lengths match."""
        cl = CurrentLimits(**valid_limits_dict())
        self.assertEqual(cl.currents, [1.0, 2.0, 3.5])
        self.assertEqual(cl.temperatures, [-10.0, 25.0, 45.0])

    def test_currents_not_list_raises_typeerror(self):
        """Should raise TypeError if currents is not a list."""
        with self.assertRaises(TypeError) as ctx:
            CurrentLimits(currents=("1.0", "2.0"), temperatures=[0.0, 25.0])
        self.assertIn("current are not provided as a list", str(ctx.exception))

    def test_currents_contains_non_float_raises_typeerror(self):
        """Should raise TypeError if any current is not a float."""
        with self.assertRaises(TypeError) as ctx:
            CurrentLimits(currents=[1.0, 2], temperatures=[0.0, 25.0])
        self.assertIn("current are not floats", str(ctx.exception))

    def test_temperatures_not_list_raises_typeerror(self):
        """Should raise TypeError if temperatures is not a list."""
        with self.assertRaises(TypeError) as ctx:
            CurrentLimits(currents=[1.0, 2.0], temperatures=(0.0, 25.0))
        self.assertIn("temperatures are not provided as a list", str(ctx.exception))

    def test_temperatures_contains_non_float_raises_typeerror(self):
        """Should raise TypeError if any temperature is not a float."""
        with self.assertRaises(TypeError) as ctx:
            CurrentLimits(currents=[1.0, 2.0], temperatures=[0.0, "25.0"])
        self.assertIn("temperatures are not floats", str(ctx.exception))

    def test_length_mismatch_raises_typeerror(self):
        """Should raise TypeError if list lengths do not match."""
        with self.assertRaises(TypeError) as ctx:
            CurrentLimits(currents=[1.0], temperatures=[0.0, 25.0])
        self.assertIn("same number of currents and temperatures", str(ctx.exception))


class TestCurrentLimitModelPostInit(unittest.TestCase):
    """Tests for CurrentLimitModel.__post_init__ including BaseModel behavior."""

    def test_dicts_are_converted_and_sources_normalized(self):
        """Should convert charge/discharge dicts to CurrentLimits and normalize sources."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            src = directory / "src.txt"
            src.write_text("ok", encoding="utf-8")

            model = CurrentLimitModel(
                name="m",
                sources=["src.txt"],  # string must be resolved against _directory
                _directory=directory,
                charge=valid_limits_dict(),
                discharge=valid_limits_dict(),
            )

            # Dicts converted to CurrentLimits
            self.assertIsInstance(model.charge, CurrentLimits)
            self.assertIsInstance(model.discharge, CurrentLimits)

            # BaseModel normalized sources
            self.assertEqual(len(model.sources), 1)
            self.assertEqual(model.sources[0], src)
            self.assertTrue(model.sources[0].exists())

    def test_nested_validation_error_in_charge(self):
        """Should raise TypeError if nested CurrentLimits in 'charge' is invalid."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            (directory / "ok.txt").write_text("ok", encoding="utf-8")

            bad_charge = {
                "currents": [1.0, 2],
                "temperatures": [0.0, 25.0],
            }  # int present
            with self.assertRaises(TypeError) as ctx:
                CurrentLimitModel(
                    name="m",
                    sources=["ok.txt"],
                    _directory=directory,
                    charge=bad_charge,
                    discharge=valid_limits_dict(),
                )
            self.assertIn("current are not floats", str(ctx.exception))

    def test_existing_instances_pass_through(self):
        """Should keep provided CurrentLimits instances and validate BaseModel fields."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            p = directory / "data.txt"
            p.write_text("ok", encoding="utf-8")

            charge = CurrentLimits(**valid_limits_dict())
            discharge = CurrentLimits(**valid_limits_dict())

            model = CurrentLimitModel(
                name="ok",
                sources=[p],  # Path passed directly
                _directory=directory,
                charge=charge,
                discharge=discharge,
            )

            # Instances preserved (no re-wrapping)
            self.assertIs(model.charge, charge)
            self.assertIs(model.discharge, discharge)
            # Path identity preserved by BaseModel for non-string sources
            self.assertIs(model.sources[0], p)

    def test_base_validation_error_name_type(self):
        """Should raise TypeError when name is not a string."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            (directory / "ok.txt").write_text("ok", encoding="utf-8")

            with self.assertRaises(TypeError) as ctx:
                CurrentLimitModel(
                    name=123,  # not a string
                    sources=["ok.txt"],
                    _directory=directory,
                    charge=valid_limits_dict(),
                    discharge=valid_limits_dict(),
                )
            self.assertIn("Modelname", str(ctx.exception))

    def test_base_validation_error_missing_source(self):
        """Should raise ValueError when a resolved source path does not exist."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            with self.assertRaises(ValueError) as ctx:
                CurrentLimitModel(
                    name="m",
                    sources=["missing.txt"],
                    _directory=directory,
                    charge=valid_limits_dict(),
                    discharge=valid_limits_dict(),
                )
            self.assertIn("At least one source", str(ctx.exception))

    def test_zipfile_directory_and_source_resolution(self):
        """Should resolve string sources against a zipfile.Path _directory and keep zip paths."""
        with tempfile.TemporaryDirectory() as tmp:
            zpath = Path(tmp) / "db.zip"
            # Create a zip with a file at root/data/src.txt
            with zipfile.ZipFile(zpath, "w") as zf:
                zf.writestr("root/data/src.txt", io.BytesIO(b"ok").getvalue())

            # Read and use zipfile.Path as _directory
            with zipfile.ZipFile(zpath) as zf:
                root = zipfile.Path(zf) / "root" / "data"

                model = CurrentLimitModel(
                    name="zip",
                    sources=["src.txt"],
                    _directory=root,
                    charge=valid_limits_dict(),
                    discharge=valid_limits_dict(),
                )

                self.assertEqual(len(model.sources), 1)
                src_path = model.sources[0]
                self.assertIsInstance(src_path, zipfile.Path)
                self.assertTrue(src_path.exists())
                self.assertEqual(src_path.name, "src.txt")


if __name__ == "__main__":
    unittest.main()
