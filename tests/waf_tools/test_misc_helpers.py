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

"""Testing file 'tools/waf_tools/misc_helper.py'."""

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).parents[2]

sys.path.insert(0, str(ROOT / "tools/waf_tools"))


# pylint: disable-next=wrong-import-position
from misc_helpers import map_python_bool_to_c_bool, to_define  # noqa: E402


class TestMapPythonBoolToCBool(unittest.TestCase):
    """Tests for mapping Python booleans to C boolean strings."""

    def test_true_maps_to_true(self) -> None:
        """Verifies that True maps to the string 'true'."""
        self.assertEqual(map_python_bool_to_c_bool(True), "true")

    def test_false_maps_to_false(self) -> None:
        """Verifies that False maps to the string 'false'."""
        self.assertEqual(map_python_bool_to_c_bool(False), "false")

    def test_output_is_string(self) -> None:
        """Ensures the conversion result is always a string."""
        self.assertIsInstance(map_python_bool_to_c_bool(True), str)
        self.assertIsInstance(map_python_bool_to_c_bool(False), str)


class ToDefineAsciiTest(unittest.TestCase):
    """Tests for the successful conversion of ASCII input."""

    def test_plain_word_is_converted_to_uppercase(self) -> None:
        """A lowercase word is converted to upper case."""
        self.assertEqual(to_define("count"), "COUNT")

    def test_already_uppercase_is_unchanged(self) -> None:
        """Input that is already a valid macro is returned unchanged."""
        self.assertEqual(to_define("COUNT"), "COUNT")

    def test_is_idempotent(self) -> None:
        """Converting an already converted string changes nothing."""
        once = to_define("my header.h")
        self.assertEqual(to_define(once), once)

    def test_empty_string(self) -> None:
        """An empty string is valid ASCII and maps to an empty string."""
        with self.assertRaises(ValueError):
            to_define("")

    def test_digits_are_preserved(self) -> None:
        """Digits are alphanumeric and therefore kept as they are."""
        self.assertEqual(to_define("utf8"), "UTF8")
        self.assertEqual(to_define("123"), "123")

    def test_underscores_are_preserved_as_underscores(self) -> None:
        """An underscore is not alphanumeric but maps back to an underscore."""
        self.assertEqual(to_define("my_var"), "MY_VAR")

    def test_separators_are_replaced(self) -> None:
        """Typical path and whitespace separators become underscores."""
        cases = {
            "my var": "MY_VAR",
            "my-var": "MY_VAR",
            "my.var": "MY_VAR",
            "my/var": "MY_VAR",
            "my\\var": "MY_VAR",
            "my:var": "MY_VAR",
            "my\tvar": "MY_VAR",
            "my\nvar": "MY_VAR",
        }
        for name, expected in cases.items():
            with self.subTest(name=name):
                self.assertEqual(to_define(name), expected)

    def test_each_non_alnum_char_yields_exactly_one_underscore(self) -> None:
        """Runs of separators are neither collapsed nor trimmed."""
        self.assertEqual(to_define("a  b"), "A__B")
        self.assertEqual(to_define(" a "), "_A_")
        self.assertEqual(to_define("---"), "___")

    def test_leading_digit_is_kept(self) -> None:
        """A leading digit is documented behaviour and is not prefixed away."""
        self.assertEqual(to_define("3d.h"), "3D_H")

    def test_realistic_filenames(self) -> None:
        """Header-guard style inputs are converted as expected."""
        cases = {
            "config.h": "CONFIG_H",
            "include/my_lib/api.hpp": "INCLUDE_MY_LIB_API_HPP",
            "foo-bar_baz.2.h": "FOO_BAR_BAZ_2_H",
        }
        for name, expected in cases.items():
            with self.subTest(name=name):
                self.assertEqual(to_define(name), expected)

    def test_result_charset_is_restricted(self) -> None:
        """The result only contains upper-case letters, digits and underscores."""
        result = to_define("Mixed Case-123/x!")
        self.assertEqual(result, "MIXED_CASE_123_X_")
        allowed = set("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_")
        self.assertTrue(set(result) <= allowed, result)

    def test_length_is_preserved(self) -> None:
        """For ASCII input the macro has the same length as the input."""
        name = "some name.h"
        self.assertEqual(len(to_define(name)), len(name))

    def test_all_printable_ascii_is_accepted(self) -> None:
        """Every printable ASCII character is valid input."""
        name = "".join(chr(code) for code in range(0x20, 0x7F))
        result = to_define(name)
        self.assertEqual(len(result), len(name))


class ToDefineNonAsciiTest(unittest.TestCase):
    """Tests for the rejection of non-ASCII input."""

    def test_non_ascii_letter_raises_value_error(self) -> None:
        """An accented letter is rejected instead of being passed through."""
        with self.assertRaises(ValueError):
            to_define("café.h")

    def test_sharp_s_raises_value_error(self) -> None:
        """``ß`` is rejected although its upper-case form is pure ASCII."""
        with self.assertRaises(ValueError):
            to_define("?ß")

    def test_various_non_ascii_inputs_raise_value_error(self) -> None:
        """Letters, symbols, digits and whitespace beyond ASCII are rejected."""
        for name in ("naïve", "日本語", "emoji_\U0001f600", "a\u00a0b", "\u0663"):
            with self.subTest(name=name), self.assertRaises(ValueError):
                to_define(name)

    def test_error_message_names_offending_characters(self) -> None:
        """The error message lists the characters that caused the failure."""
        with self.assertRaises(ValueError) as ctx:
            to_define("x-zé-öß")
        message = str(ctx.exception)
        self.assertIn("é", message)
        self.assertIn("ö", message)
        self.assertIn("ß", message)

    def test_non_ascii_is_detected_before_any_conversion(self) -> None:
        """No partial result is returned when validation fails."""
        with self.assertRaises(ValueError):
            to_define("ok.h\u00e9")


class ToDefineTypeTest(unittest.TestCase):
    """Tests for the argument type validation."""

    def test_non_string_raises_type_error(self) -> None:
        """Non-string arguments raise ``TypeError``, not ``AttributeError``."""
        for value in (None, 42, 3.5, b"bytes", ["a"], object()):
            with self.subTest(value=value), self.assertRaises(TypeError):
                to_define(value)

    def test_type_error_message_mentions_actual_type(self) -> None:
        """The ``TypeError`` message names the type that was passed in."""
        with self.assertRaises(TypeError) as ctx:
            to_define(b"config.h")
        self.assertIn("bytes", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
