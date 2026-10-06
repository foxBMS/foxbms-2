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

"""Testing file 'tools/waf_tools/battery_config_validate_utils.py'."""

import sys
import unittest
from pathlib import Path

from pydantic import ValidationError
from pydantic.dataclasses import dataclass

ROOT = Path(__file__).parents[2]

for i in [
    ROOT / "tools/waf_tools",
    ROOT / "tools/waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Windows
    ROOT / "tools/.waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable=wrong-import-position

from battery_config_validate_utils import (  # noqa: E402
    DEFINE_CONFIG,
    BoolDefine,
    FloatDefine,
    IntDefine,
    StrDefine,
    _float_from_number,
    _format_validation_error,
    _int_from_number,
    extract_defines,
    parse_defines_to_dataclass,
)

# pylint: enable


class _ConfigError(RuntimeError):
    """Error class used to verify that the injected 'error_cls' is honored."""


@dataclass(config=DEFINE_CONFIG)
class _MixedTypes:
    """Dataclass covering all supported field types."""

    flag: BoolDefine
    count: IntDefine
    ratio: FloatDefine
    name: StrDefine


@dataclass(config=DEFINE_CONFIG)
class _SomeDefines:
    """Minimal dataclass with some defines."""

    some_random_name_2: IntDefine
    some_random_name_3: FloatDefine


@dataclass(config=DEFINE_CONFIG)
class _Fallback:
    """Dataclass with an unsupported field type."""

    values: list


class IntFromNumberTests(unittest.TestCase):
    """Test for the pre-validator '_int_from_number()'."""

    def test_int_from_number_narrows_integral_float(self) -> None:
        """Narrow integral floats to 'int'."""
        result = _int_from_number(42.0)
        self.assertEqual(result, 42)
        self.assertIsInstance(result, int)

    def test_int_from_number_keeps_fractional_float(self) -> None:
        """Keep floats with a fractional part so that pydantic rejects them."""
        self.assertEqual(_int_from_number(4.2), 4.2)

    def test_int_from_number_keeps_bool(self) -> None:
        """Keep booleans so that pydantic rejects them for integer fields."""
        self.assertIs(_int_from_number(True), True)

    def test_int_from_number_keeps_unrelated_value(self) -> None:
        """Pass through values that are neither 'bool' nor 'float'."""
        self.assertEqual(_int_from_number("someValue"), "someValue")


class FloatFromNumberTests(unittest.TestCase):
    """Test for the pre-validator '_float_from_number()'."""

    def test_float_from_number_widens_int(self) -> None:
        """Widen integers to 'float' so that JSON integers are accepted."""
        result = _float_from_number(42)
        self.assertEqual(result, 42.0)
        self.assertIsInstance(result, float)

    def test_float_from_number_keeps_bool(self) -> None:
        """Keep booleans so that pydantic rejects them for float fields."""
        self.assertIs(_float_from_number(False), False)

    def test_float_from_number_keeps_unrelated_value(self) -> None:
        """Pass through values that are neither 'bool' nor 'int'-"""
        self.assertEqual(_float_from_number("4.2"), "4.2")


class StrictTypeTests(unittest.TestCase):
    """Tests for the strictness enforced by 'DEFINE_CONFIG'."""

    def _codes(self, error: ValidationError) -> list[str]:
        """Return the pydantic error codes of a validation error."""
        return [e["type"] for e in error.errors(include_url=False)]

    def test_int_is_rejected_for_bool(self) -> None:
        """Reject '1' and '0' for boolean defines."""
        with self.assertRaises(ValidationError) as e:
            _MixedTypes(flag=1, count=1, ratio=1.0, name="n")
        self.assertEqual(self._codes(e.exception), ["bool_type"])

    def test_string_is_rejected_for_bool(self) -> None:
        """Reject the JSON strings 'true' and 'false' for boolean defines."""
        with self.assertRaises(ValidationError) as e:
            _MixedTypes(flag="true", count=1, ratio=1.0, name="n")
        self.assertEqual(self._codes(e.exception), ["bool_type"])

    def test_numeric_string_is_rejected_for_int(self) -> None:
        """Reject numeric strings, strict mode must not coerce."""
        with self.assertRaises(ValidationError) as e:
            _MixedTypes(flag=True, count="42", ratio=1.0, name="n")
        self.assertEqual(self._codes(e.exception), ["int_type"])

    def test_fractional_float_is_rejected_for_int(self) -> None:
        """Reject floats with a fractional part for integer defines."""
        with self.assertRaises(ValidationError) as e:
            _MixedTypes(flag=True, count=4.2, ratio=1.0, name="n")
        self.assertEqual(self._codes(e.exception), ["int_type"])

    def test_bool_is_rejected_for_int(self) -> None:
        """Reject booleans, which are 'int' subclasses in Python."""
        with self.assertRaises(ValidationError) as e:
            _MixedTypes(flag=True, count=True, ratio=1.0, name="n")
        self.assertEqual(self._codes(e.exception), ["int_type"])

    def test_int_is_rejected_for_str(self) -> None:
        """Reject non-strings for string defines."""
        with self.assertRaises(ValidationError) as e:
            _MixedTypes(flag=True, count=1, ratio=1.0, name=5)
        self.assertEqual(self._codes(e.exception), ["string_type"])


class ExtractDefinesTests(unittest.TestCase):
    """Tests for 'extract_defines()'."""

    def _make_cfg(self) -> dict:
        return {
            "files": [
                {
                    "file_name": "battery_system_cfg.h",
                    "defines": [
                        {
                            "name": "BS_SOME_RANDOM_NAME_1",
                            "value": True,
                            "value_type": "bool",
                        }
                    ],
                },
                {
                    "file_name": "battery_cell_cfg.h",
                    "defines": [
                        {
                            "name": "BC_SOME_RANDOM_NAME_2",
                            "value": 2,
                            "value_type": "int",
                        },
                        {
                            "name": "BC_SOME_RANDOM_NAME_3",
                            "value": 3.3,
                            "value_type": "float",
                        },
                    ],
                },
            ]
        }

    def test_extract_defines_returns_matching_file_entry(self) -> None:
        """Return a flat name to value mapping for the requested file."""
        result = extract_defines(self._make_cfg(), "battery_cell_cfg.h", _ConfigError)
        file_entry = {"BC_SOME_RANDOM_NAME_2": 2, "BC_SOME_RANDOM_NAME_3": 3.3}
        self.assertEqual(result, file_entry)

    def test_extract_defines_raises_on_unknown_file(self) -> None:
        """Raise the injected error class when the file entry is missing."""
        with self.assertRaises(_ConfigError) as e:
            extract_defines(self._make_cfg(), "does_not_exist.h", _ConfigError)
        err_msg = "File 'does_not_exist.h' not found in configuration"
        self.assertEqual(str(e.exception), err_msg)

    def test_extract_defines_raises_on_missing_files_key(self) -> None:
        """Raise when the configuration does not contain a 'files' list."""
        with self.assertRaises(_ConfigError):
            extract_defines({}, "battery_cell_cfg.h", _ConfigError)


class ParseDefinesToDataclassTests(unittest.TestCase):
    """Tests for 'parse_defines_to_dataclass()'."""

    def test_parse_defines_populates_dataclass(self) -> None:
        """Resolve all fields via '<PREFIX>_<FIELD_NAME>'."""
        defines = {"BC_SOME_RANDOM_NAME_2": 2, "BC_SOME_RANDOM_NAME_3": 3.3}
        result = parse_defines_to_dataclass(defines, _SomeDefines, "BC", _ConfigError)
        self.assertEqual(
            result, _SomeDefines(some_random_name_2=2, some_random_name_3=3.3)
        )

    def test_parse_defines_is_case_insensitive(self) -> None:
        """Match define names irrespective of their case."""
        defines = {"bc_some_random_name_2": 2, "Bc_Some_Random_Name_3": 3.3}
        result = parse_defines_to_dataclass(defines, _SomeDefines, "BC", _ConfigError)
        self.assertEqual(
            result, _SomeDefines(some_random_name_2=2, some_random_name_3=3.3)
        )

    def test_parse_defines_inserts_infix_after_first_field_part(self) -> None:
        """Build '<PREFIX>_<FIRST>_<INFIX>_<REST>' when an infix is given."""
        defines = {"BC_SOME_OTHER_RANDOM_NAME_2": 2, "BC_SOME_OTHER_RANDOM_NAME_3": 3.3}
        result = parse_defines_to_dataclass(
            defines, _SomeDefines, "BC", _ConfigError, infix="other"
        )
        self.assertEqual(
            result, _SomeDefines(some_random_name_2=2, some_random_name_3=3.3)
        )

    def test_parse_defines_without_prefix(self) -> None:
        """Use the plain field name when no prefix is configured."""
        defines = {"SOME_RANDOM_NAME_2": 2, "SOME_RANDOM_NAME_3": 3.3}
        result = parse_defines_to_dataclass(defines, _SomeDefines, "", _ConfigError)
        self.assertEqual(
            result, _SomeDefines(some_random_name_2=2, some_random_name_3=3.3)
        )

    def test_parse_defines_coerces_all_supported_types(self) -> None:
        """Coerce values to the declared field types."""
        defines = {"BS_FLAG": True, "BS_COUNT": 3, "BS_RATIO": 2, "BS_NAME": "SomeName"}
        result = parse_defines_to_dataclass(defines, _MixedTypes, "BS", _ConfigError)
        self.assertEqual(result, _MixedTypes(True, 3, 2.0, "SomeName"))
        self.assertIsInstance(result.flag, bool)
        self.assertIsInstance(result.count, int)
        self.assertIsInstance(result.ratio, float)
        self.assertIsInstance(result.name, str)

    def test_parse_defines_raises_on_missing_define(self) -> None:
        """Raise the injected error class when a required define is missing."""
        define = {"BC_SOME_RANDOM_NAME_2": 2}
        with self.assertRaises(_ConfigError) as e:
            parse_defines_to_dataclass(define, _SomeDefines, "BC", _ConfigError)
        err_msg = "Required define 'BC_SOME_RANDOM_NAME_3' is missing"
        self.assertEqual(str(e.exception), err_msg)

    def test_parse_defines_validates_list_fields(self) -> None:
        """Keep raw values for field types without coercion rules."""
        defines = {"values": [1, 2, 3]}
        result = parse_defines_to_dataclass(defines, _Fallback, "", _ConfigError)
        self.assertEqual(result.values, [1, 2, 3])


class FormatValidationErrorTests(unittest.TestCase):
    """Tests for the error message of 'parse_defines_to_dataclass()'."""

    def _parse(self, defines: dict) -> str:
        """Parse the defines and return the resulting error message."""
        with self.assertRaises(_ConfigError) as e:
            parse_defines_to_dataclass(defines, _MixedTypes, "BS", _ConfigError)
        return str(e.exception)

    def test_all_invalid_defines_are_reported(self) -> None:
        """Report every invalid define."""
        msg = self._parse(
            {"BS_FLAG": 1, "BS_COUNT": 1, "BS_RATIO": "x", "BS_NAME": "n"}
        )
        self.assertTrue(msg.startswith("2 invalid defines for '_MixedTypes':"))
        self.assertIn("BS_FLAG:", msg)
        self.assertIn("(got 1)", msg)
        self.assertIn("BS_RATIO:", msg)
        self.assertIn("(got 'x')", msg)

    def test_single_error_uses_singular(self) -> None:
        """Use the singular noun for exactly one invalid define."""
        msg = self._parse(
            {"BS_FLAG": True, "BS_COUNT": 1, "BS_RATIO": 1.0, "BS_NAME": 5}
        )
        self.assertTrue(msg.startswith("1 invalid define for '_MixedTypes':"))

    def test_define_names_are_reported(self) -> None:
        """Report define names instead of dataclass field names."""
        msg = self._parse(
            {"BS_FLAG": 1, "BS_COUNT": 1, "BS_RATIO": 1.0, "BS_NAME": "n"}
        )
        self.assertIn("BS_FLAG:", msg)
        self.assertNotIn("flag:", msg)

    def test_empty_loc_falls_back_to_class_name(self) -> None:
        """Use class name as name if the error location is empty."""

        class _MockValidationError:
            def error_count(self) -> int:
                return 1

            def errors(self, *_args: object, **_kwargs: object) -> list[dict]:
                return [{"loc": (), "msg": "invalid", "input": 42}]

        mock_error = _MockValidationError()
        result = _format_validation_error(_SomeDefines, mock_error, {})
        expected = (
            "1 invalid define for '_SomeDefines':\n  <_SomeDefines>: invalid (got 42)"
        )
        self.assertEqual(result, expected)


if __name__ == "__main__":
    unittest.main()
