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
from dataclasses import FrozenInstanceError
from pathlib import Path
from typing import get_args
from unittest.mock import MagicMock, patch

from pydantic import ValidationError

ROOT = Path(__file__).parents[2]

for i in [
    ROOT / "tools/waf_tools",
    ROOT / "tools/waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Windows
    ROOT / "tools/.waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable=wrong-import-position

from diag_array_config_validate import (  # noqa: E402
    DELAY_TO_C_LITERAL,
    ENABLED_TO_C_LITERAL,
    SENSITIVITY_TO_C_LITERAL,
    SEVERITY_TO_C_LITERAL,
    Delay,
    DiagArrayConfiguration,
    DiagArrayEntry,
    InvalidDiagConfigurationError,
    Sensitivity,
    Severity,
    _extract_array_entries,
    _extract_diag_ids_from_header,
    _extract_fatal_error_ids,
    _format_validation_error,
    _map_delay,
    _map_enabled,
    _map_sensitivity,
    _map_severity,
    _parse_array_entry,
    _parse_diag_array_config,
    _validate_events_match_header,
    _validate_fatal_errors_against_dbc,
    render_diag_array,
    validate_diag_array_configuration,
)

# pylint: enable


HEADER_TXT = """/* some header */
typedef enum {
    DIAG_ID_DUMMY,
    /* some comment*/
    DIAG_ID_SOME_EVENT, /*!< some event */
    DIAG_ID_SOME_OTHER_EVENT,
    DIAG_ID_MAX, /*!< MAX indicator - do not change */
} DIAG_ID_e;
/* more code */
"""


def _raw_entry(**overrides: object) -> dict:
    entry = {
        "event": "DIAG_ID_SOME_EVENT",
        "description": "Some diagnosis entry",
        "sensitivity": 1,
        "severity": "INFO",
        "delay": 0,
        "enabled": True,
        "callback": "DIAG_ErrorSomeEvent",
    }
    entry.update(overrides)
    return entry


def _make_cfg(raw_entries: list[dict] | None = None) -> dict:
    if not raw_entries:
        raw_entries = [_raw_entry()]
    return {
        "path": "src/app/engine/config",
        "files": [{"file_name": "diag_cfg.c", "array_entries": raw_entries}],
    }


def _make_validated_config(*raw_entries: dict) -> DiagArrayConfiguration:
    return DiagArrayConfiguration(entries=[DiagArrayEntry(**i) for i in raw_entries])


class _MockNode:
    def __init__(self, text: str = "", path: str = "mock") -> None:
        self._text = text
        self._path = path

    def read(self, *_args: object, **_kwargs: object) -> str:
        return self._text

    def abspath(self) -> str:
        return self._path


class LiteralMappingTests(unittest.TestCase):
    """Tests for the 'Literal' aliases and their C literal mappings."""

    def test_literal_aliases_match_c_mappings(self) -> None:
        """Ensure that every allowed value has exactly one C literal."""
        for alias, mapping in (
            (Sensitivity, SENSITIVITY_TO_C_LITERAL),
            (Severity, SEVERITY_TO_C_LITERAL),
            (Delay, DELAY_TO_C_LITERAL),
        ):
            with self.subTest(alias=alias):
                self.assertEqual(set(get_args(alias)), set(mapping))

    def test_mapping_helpers_resolve_all_values(self) -> None:
        """Resolve every mapped value to its C literal."""
        for helper, mapping in (
            (_map_sensitivity, SENSITIVITY_TO_C_LITERAL),
            (_map_severity, SEVERITY_TO_C_LITERAL),
            (_map_delay, DELAY_TO_C_LITERAL),
            (_map_enabled, ENABLED_TO_C_LITERAL),
        ):
            for value, expected in mapping.items():
                with self.subTest(value=value):
                    self.assertEqual(helper(value), expected)

    def test_mapping_helpers_raise_on_unknown_value(self) -> None:
        """Raise 'KeyError' for values outside of the mappings."""
        for helper, value in (
            (_map_sensitivity, 2),
            (_map_severity, "fatal_error"),
            (_map_delay, 50),
        ):
            with self.subTest(helper=helper), self.assertRaises(KeyError):
                helper(value)


class DiagArrayEntryTests(unittest.TestCase):
    """Tests for the strictness enforced by 'DiagArrayConfig'."""

    def _codes(self, error: ValidationError) -> list[str]:
        """Return the pydantic error codes of a validation error."""
        return [e["type"] for e in error.errors(include_url=False)]

    def test_valid_entry_is_accepted(self) -> None:
        """Accept a fully valid entry."""
        entry = DiagArrayEntry(**_raw_entry())
        self.assertEqual(entry.event, "DIAG_ID_SOME_EVENT")
        self.assertEqual(entry.description, "Some diagnosis entry")
        self.assertEqual(entry.sensitivity, 1)
        self.assertEqual(entry.severity, "INFO")
        self.assertEqual(entry.delay, 0)
        self.assertEqual(entry.enabled, True)
        self.assertEqual(entry.callback, "DIAG_ErrorSomeEvent")

    def test_entry_is_frozen(self) -> None:
        """Reject modifications of an entry."""
        entry = DiagArrayEntry(**_raw_entry())
        with self.assertRaises(FrozenInstanceError):
            entry.event = "DIAG_ID_SOME_OTHER_EVENT"

    def test_unsupported_sensitivity_is_rejected(self) -> None:
        """Reject sensitivity values without a C literal."""
        with self.assertRaises(ValidationError) as e:
            DiagArrayEntry(**_raw_entry(sensitivity=2))
        self.assertEqual(self._codes(e.exception), ["literal_error"])

    def test_unsupported_severity_is_rejected(self) -> None:
        """Reject severity values without a C literal."""
        with self.assertRaises(ValidationError) as e:
            DiagArrayEntry(**_raw_entry(severity="fatal_error"))
        self.assertEqual(self._codes(e.exception), ["literal_error"])

    def test_unsupported_delay_is_rejected(self) -> None:
        """Reject delay values without a C literal."""
        with self.assertRaises(ValidationError) as e:
            DiagArrayEntry(**_raw_entry(delay=500))
        self.assertEqual(self._codes(e.exception), ["literal_error"])

    def test_int_is_rejected_for_enabled(self) -> None:
        """Reject '1' and '0' for the boolean 'enabled' key."""
        with self.assertRaises(ValidationError) as e:
            DiagArrayEntry(**_raw_entry(enabled=1))
        self.assertEqual(self._codes(e.exception), ["bool_type"])

    def test_non_string_is_rejected_for_event(self) -> None:
        """Reject non-strings for the string keys."""
        with self.assertRaises(ValidationError) as e:
            DiagArrayEntry(**_raw_entry(event=5))
        self.assertEqual(self._codes(e.exception), ["string_type"])

    def test_missing_key_is_rejected(self) -> None:
        """Reject entries with a missing key"""
        raw = _raw_entry()
        del raw["callback"]
        with self.assertRaises(ValidationError) as e:
            DiagArrayEntry(**raw)
        self.assertEqual(self._codes(e.exception), ["missing"])


class FormatValidationErrorTests(unittest.TestCase):
    """Tests for '_format_validation_error()'."""

    class _MockValidationError:
        def __init__(self, errors: list[dict]) -> None:
            self._errors = errors

        def error_count(self) -> int:
            return len(self._errors)

        def errors(self, *_args: object, **_kwargs: object) -> list[dict]:
            return self._errors

    def test_empty_loc_falls_back_to_entry_marker(self) -> None:
        """Use <entry> as key if the error location is empty."""
        error = self._MockValidationError(
            [{"loc": (), "msg": "invalid", "input": 42, "type": "value_error"}]
        )
        result = _format_validation_error({"event": "DIAG_ID_SOME_EVENT"}, 3, error)
        expected = (
            "1 invalid value in array_entries at index 3 "
            "(event: DIAG_ID_SOME_EVENT):\n"
            "  <entry>: invalid (got 42)"
        )
        self.assertEqual(result, expected)

    def test_non_string_event_is_not_used_as_context(self) -> None:
        """Omit the event context if the event name is not a string."""
        error = self._MockValidationError(
            [{"loc": ("event",), "msg": "invalid", "input": 42, "type": "string_type"}]
        )
        result = _format_validation_error({"event": 5}, 0, error)
        self.assertTrue(
            result.startswith("1 invalid value in array_entries at index 0:")
        )

    def test_multiple_errors_use_plural(self) -> None:
        """Use the plural noun for more than one invalid value."""
        error = self._MockValidationError(
            [
                {"loc": ("delay",), "msg": "a", "input": 1, "type": "literal_type"},
                {"loc": ("enabled",), "msg": "b", "input": 2, "type": "bool_type"},
            ]
        )
        result = _format_validation_error({}, 1, error)
        self.assertTrue(
            result.startswith("2 invalid values in array_entries at index 1:")
        )
        self.assertIn("  delay: a (got 1)", result)
        self.assertIn("  enabled: b (got 2)", result)


class ParseArrayEntryTests(unittest.TestCase):
    """Tests for '_parse_array_entry()'."""

    def test_parse_array_entries_returns_entry(self) -> None:
        """Return a validated entry for a valid raw entry."""
        self.assertEqual(
            _parse_array_entry(_raw_entry(), 0), DiagArrayEntry(**_raw_entry())
        )

    def test_parse_array_entry_reports_index_and_events(self) -> None:
        """Report the index and the event name of an invalid entry."""
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _parse_array_entry(_raw_entry(delay=42), 7)
        msg = str(e.exception)
        self.assertTrue(
            msg.startswith(
                "1 invalid value in array_entries at index 7 (event: DIAG_ID_SOME_EVENT):"
            )
        )
        self.assertIn("delay:", msg)
        self.assertIn("(got 42)", msg)

    def test_parse_array_entry_reports_all_invalid_values(self) -> None:
        """Report every invalid value of an entry."""
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _parse_array_entry(_raw_entry(delay=42, enabled=1), 7)
        msg = str(e.exception)
        self.assertTrue(msg.startswith("2 invalid values in array_entries at index 7"))
        self.assertIn("delay:", msg)
        self.assertIn("enabled:", msg)

    def test_parse_array_entry_reports_missing_key_without_input(self) -> None:
        """Omit the input value for missing keys."""
        raw = _raw_entry()
        del raw["callback"]
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _parse_array_entry(raw, 0)
        msg = str(e.exception)
        self.assertIn("callback:", msg)
        self.assertNotIn("(got", msg)


class ExtractArrayEntriesTests(unittest.TestCase):
    """Tests for '_extract_array_entries()'."""

    def test_extract_array_entries_keeps_order(self) -> None:
        """Return the entries in the order of the configuration."""
        raw_entries = [
            _raw_entry(event="DIAG_ID_SOME_EVENT"),
            _raw_entry(event="DIAG_ID_SOME_OTHER_EVENT"),
        ]
        result = _extract_array_entries(_make_cfg(raw_entries))
        self.assertEqual(
            [i.event for i in result],
            ["DIAG_ID_SOME_EVENT", "DIAG_ID_SOME_OTHER_EVENT"],
        )

    def test_extract_array_entries_raises_on_missing_files(self) -> None:
        """Raise if the configuration has no 'files' entry."""
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _extract_array_entries({})
        self.assertEqual(str(e.exception), "No 'files' entry found in configuration")

    def test_extract_array_entries_raises_on_missing_array_entries(self) -> None:
        """Raise if the first file entry has no 'array_entries'."""
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _extract_array_entries({"files": [{"file_name": "diag_cfg.c"}]})
        self.assertEqual(str(e.exception), "No 'array_entries' found in configuration")


class ParseDiagArrayConfigTests(unittest.TestCase):
    """Tests for '_parse_diag_array_config()'."""

    def test_parse_diag_array_config_returns_configuration(self) -> None:
        """Return a configuration containing all validated entries."""
        raw_entries = [
            _raw_entry(event="DIAG_ID_SOME_EVENT"),
            _raw_entry(event="DIAG_ID_SOME_OTHER_EVENT", severity="FATAL_ERROR"),
        ]
        result = _parse_diag_array_config(_make_cfg(raw_entries))
        self.assertEqual(result, _make_validated_config(*raw_entries))
        self.assertEqual(len(result.entries), 2)

    def test_parse_diag_array_config_propagates_entry_errors(self) -> None:
        """Propagate errors of invalid entries."""
        with self.assertRaises(InvalidDiagConfigurationError):
            _parse_diag_array_config(_make_cfg([_raw_entry(severity="ERROR")]))


class ExtractDiagIdsFromHeaderTests(unittest.TestCase):
    """Tests for '_extract_diag_ids_from_header()'."""

    def test_extract_diag_ids_returns_ids_in_order(self) -> None:
        """Return all 'DIAG_ID' entries except 'DIAG_ID_MAX'."""
        result = _extract_diag_ids_from_header(_MockNode(HEADER_TXT))
        expected = ["DIAG_ID_DUMMY", "DIAG_ID_SOME_EVENT", "DIAG_ID_SOME_OTHER_EVENT"]
        self.assertEqual(result, expected)

    def test_extract_diag_ids_ignores_other_enums(self) -> None:
        """Only parse the 'DIAG_ID_e' enumeration."""
        txt = "typedef enum {\n   SOME_OTHER_ID,\n} SOME_OTHER_e;\n" + HEADER_TXT
        result = _extract_diag_ids_from_header(_MockNode(txt))
        self.assertNotIn("SOME_OTHER_ID", result)

    def test_extract_diag_ids_raises_on_missing_enum(self) -> None:
        """Raise if the 'DIAG_ID_e' enumeration is not found."""
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _extract_diag_ids_from_header(_MockNode(""))
        err_msg = "Could not find 'typedef enum {...} DIAG_ID_e;' in diag_cfg.h"
        self.assertEqual(str(e.exception), err_msg)


class ValidateEventsMatchHeaderTests(unittest.TestCase):
    """Tests for '_validate_events_match_header()'."""

    def test_matching_events_pass(self) -> None:
        """Accept identically named and ordered events."""
        cfg = _make_validated_config(
            _raw_entry(event="DIAG_ID_SOME_EVENT"),
            _raw_entry(event="DIAG_ID_SOME_OTHER_EVENT"),
        )
        header_ids = ["DIAG_ID_SOME_EVENT", "DIAG_ID_SOME_OTHER_EVENT"]
        self.assertIsNone(_validate_events_match_header(cfg, header_ids))

    def test_order_mismatch_is_reported(self) -> None:
        """Report the index of the first mismatching event."""
        cfg = _make_validated_config(
            _raw_entry(event="DIAG_ID_SOME_EVENT"),
            _raw_entry(event="DIAG_ID_SOME_OTHER_EVENT"),
        )
        header_ids = ["DIAG_ID_DUMMY", "DIAG_ID_SOME_OTHER_EVENT"]
        with self.assertRaises(InvalidDiagConfigurationError) as e:
            _validate_events_match_header(cfg, header_ids)
        err_msg = (
            "Order mismatch at index 0: diag_cfg.h expects "
            "'DIAG_ID_DUMMY', but JSON has 'DIAG_ID_SOME_EVENT'"
        )
        self.assertEqual(str(e.exception), err_msg)

    def test_length_mismatch_is_rejected(self) -> None:
        """Reject configurations with a different number of events."""
        cfg = _make_validated_config(_raw_entry(event="DIAG_ID_SOME_EVENT"))
        header_ids = ["DIAG_ID_SOME_EVENT", "DIAG_ID_SOME_OTHER_EVENT"]
        with self.assertRaises(ValueError):
            _validate_events_match_header(cfg, header_ids)


class ValidateDiagArrayConfigurationTests(unittest.TestCase):
    """Tests for 'validate_diag_array_configuration()'."""

    def test_missing_header_is_rejected(self) -> None:
        """Reject configurations when the diagnostic header is unavailable."""
        build = MagicMock()
        build.srcnode.find_node.return_value = None
        with (
            patch(
                "diag_array_config_validate.read_json_from_node",
                return_value={},
            ),
            self.assertRaises(InvalidDiagConfigurationError) as error,
        ):
            validate_diag_array_configuration(build, MagicMock())
        self.assertIn("diag_cfg.h' not found", str(error.exception))

    def test_missing_dbc_is_rejected(self) -> None:
        """Reject configurations when the DBC file is unavailable."""
        build = MagicMock()
        build.srcnode.find_node.side_effect = [MagicMock(), None]
        with (
            patch(
                "diag_array_config_validate.read_json_from_node",
                return_value={},
            ),
            self.assertRaises(InvalidDiagConfigurationError) as error,
        ):
            validate_diag_array_configuration(build, MagicMock())
        self.assertIn("foxbms.dbc' not found", str(error.exception))

    def test_validates_and_returns_configuration(self) -> None:
        """Run all validation stages and return the parsed configuration."""
        build = MagicMock()
        build.srcnode.find_node.side_effect = [MagicMock(), MagicMock()]
        array_cfg = _make_validated_config()
        with (
            patch(
                "diag_array_config_validate.read_json_from_node",
                return_value={"files": []},
            ),
            patch(
                "diag_array_config_validate._parse_diag_array_config",
                return_value=array_cfg,
            ),
            patch(
                "diag_array_config_validate._extract_diag_ids_from_header",
                return_value=["DIAG_ID_SOME_EVENT"],
            ),
            patch(
                "diag_array_config_validate._validate_events_match_header"
            ) as validate_events,
            patch(
                "diag_array_config_validate._extract_fatal_error_ids",
                return_value={},
            ),
            patch(
                "diag_array_config_validate._validate_fatal_errors_against_dbc"
            ) as validate_fatal,
        ):
            result = validate_diag_array_configuration(build, MagicMock())

        self.assertIs(result, array_cfg)
        validate_events.assert_called_once()
        validate_fatal.assert_called_once()


class ExtractFatalErrorIdsTests(unittest.TestCase):
    """Tests for '_extract_fatal_error_ids()'."""

    def test_only_fatal_errors_are_extracted(self) -> None:
        """Map fatal error events to their array index."""
        cfg = _make_validated_config(
            _raw_entry(event="DIAG_ID_DUMMY", severity="INFO"),
            _raw_entry(event="DIAG_ID_SOME_EVENT", severity="FATAL_ERROR"),
            _raw_entry(event="DIAG_ID_SOME_OTHER_EVENT", severity="WARNING"),
            _raw_entry(event="DIAG_ID_ANOTHER_EVENT", severity="FATAL_ERROR"),
        )
        result = _extract_fatal_error_ids(cfg)
        self.assertEqual(result, {"DIAG_ID_SOME_EVENT": 1, "DIAG_ID_ANOTHER_EVENT": 3})

    def test_no_fatal_errors_returns_empty_mapping(self) -> None:
        """Return an empty mapping if no fatal errors are configured."""
        cfg = _make_validated_config(_raw_entry(severity="INFO"))
        self.assertEqual(_extract_fatal_error_ids(cfg), {})


class ValidateFatalErrorsAgainstDbcTests(unittest.TestCase):
    """Tests for '_validate_fatal_errors_against_dbc()'."""

    class _Database:  # pylint: disable=too-few-public-methods
        def __init__(self, signals: list[object]) -> None:
            self.signals = signals

        def get_message_by_name(
            self, _name: str
        ) -> "ValidateFatalErrorsAgainstDbcTests._Database":
            return self

    class _Signal:  # pylint: disable=too-few-public-methods
        def __init__(self, choices: dict[str, int]) -> None:
            self.name = "FatalErrorCode"
            self.choices = choices
            self.choice_to_number = MagicMock()

    def _validate(self, database: object, fatal_error_ids: dict[str, int]) -> None:
        """Run the validator with a controlled cantools database."""
        with (
            patch(
                "diag_array_config_validate.cantools.database.load_file",
                return_value=database,
            ),
            patch(
                "diag_array_config_validate.cantools.database.can.database.Database",
                self._Database,
            ),
        ):
            return _validate_fatal_errors_against_dbc(
                MagicMock(abspath=lambda: "test.dbc"), fatal_error_ids
            )

    def test_rejects_non_database_result(self) -> None:
        """Reject a cantools result with the wrong database type."""
        with self.assertRaises(InvalidDiagConfigurationError) as error:
            self._validate(object(), {})
        self.assertEqual(str(error.exception), "DBC File is not of type 'Database'.")

    def test_rejects_missing_fatal_error_message(self) -> None:
        """Reject a database without the fatal error message."""
        database = self._Database([])
        with self.assertRaises(InvalidDiagConfigurationError) as error:
            self._validate(database, {})
        self.assertIn("Signal 'FatalErrorCode' not found", str(error.exception))

    def test_rejects_signal_without_choices(self) -> None:
        """Reject a fatal error signal without documented choices."""
        database = self._Database([self._Signal({})])
        with self.assertRaises(InvalidDiagConfigurationError) as error:
            self._validate(database, {})
        self.assertIn("does not support choices", str(error.exception))

    def test_reports_missing_and_mismatched_choices(self) -> None:
        """Report missing DBC choices and mismatched numeric IDs together."""
        signal = self._Signal({"KNOWN": 2})
        signal.choice_to_number.side_effect = [KeyError(), 7]
        database = self._Database([signal])
        with self.assertRaises(InvalidDiagConfigurationError) as error:
            self._validate(database, {"MISSING": 0, "KNOWN": 1})
        message = str(error.exception)
        self.assertIn("Signal 'MISSING' (id=0) not found", message)
        self.assertIn("Signal 'KNOWN' and its id '1'", message)

    def test_accepts_matching_choices(self) -> None:
        """Accept fatal errors whose DBC IDs match the source IDs."""
        signal = self._Signal({"KNOWN": 1})
        signal.choice_to_number.return_value = 1
        database = self._Database([signal])
        self.assertIsNone(self._validate(database, {"KNOWN": 1}))


class RenderDiagArrayTests(unittest.TestCase):
    """Tests for 'render_diag_array()'."""

    def test_render_single_entry(self) -> None:
        """Render one entry as a C struct initializer line."""
        expected = (
            "    {DIAG_ID_SOME_EVENT, DIAG_SEN_EVENT_1, DIAG_INFO, "
            "DIAG_NO_DELAY, DIAG_EVALUATION_ENABLED, DIAG_ErrorSomeEvent},"
        )
        self.assertEqual(render_diag_array([_raw_entry()]), expected)

    def test_render_multiple_entries(self) -> None:
        """Render one line per entry, separated by newlines."""
        raw_entries = [
            _raw_entry(event="DIAG_ID_DUMMY"),
            _raw_entry(event="DIAG_ID_SOME_EVENT", enabled=False, delay=-1),
        ]
        result = render_diag_array(raw_entries).splitlines()
        self.assertEqual(len(result), 2)
        self.assertIn("DIAG_ID_DUMMY", result[0])
        self.assertIn("DIAG_DELAY_DISCARD", result[1])
        self.assertIn("DIAG_EVALUATION_DISABLED", result[1])


if __name__ == "__main__":
    unittest.main()
