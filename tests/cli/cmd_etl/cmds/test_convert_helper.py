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

"""Testing file 'cli/cmd_etl/cmds/convert_helper.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

# Module under test

try:
    from cli.cmd_etl.cmds import convert_helper
    from cli.cmd_etl.etl import OutputFormats
    from cli.cmd_etl.etl.convert import InputFormats
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.cmds import convert_helper
    from cli.cmd_etl.etl import OutputFormats
    from cli.cmd_etl.etl.convert import InputFormats


class TestConverterSetup(unittest.TestCase):
    """Tests for converter_setup."""

    def test_calls_validate_and_constructs_converter(self):
        """Ensure configuration is validated and a Converter is constructed with given config."""
        config = {
            "conversion": {
                "input_format": "GAMRY",  # leave as provided by config
                "output_format": "CSV",
                "skip_footer": 0,
            },
            "data_path": Path("/tmp/data"),  # noqa: S108
            "recursive": True,
        }

        with (
            patch.object(convert_helper, "validate_converter_config") as mock_validate,
            patch.object(convert_helper, "ConversionSettings") as mock_settings_cls,
            patch.object(convert_helper, "Converter") as mock_converter_cls,
        ):
            # Arrange mocks
            mock_settings_obj = MagicMock()
            mock_settings_cls.return_value = mock_settings_obj
            mock_converter_obj = MagicMock()
            mock_converter_cls.return_value = mock_converter_obj

            # Act
            result = convert_helper.converter_setup(config)

            # Assert
            mock_validate.assert_called_once_with(config["conversion"])
            mock_settings_cls.assert_called_once_with(**config["conversion"])
            mock_converter_cls.assert_called_once_with(
                config["data_path"], config["recursive"], mock_settings_obj
            )
            self.assertIs(result, mock_converter_obj)


class TestValidateConverterConfig(unittest.TestCase):
    """Tests for validate_converter_config."""

    def test_valid_enum_values_pass_validation(self):
        """Validation should pass when input/output formats are Enum members."""
        config = {
            "input_format": InputFormats.GAMRY,
            "output_format": OutputFormats.CSV,
            "additional": {"skip_footer": 5},  # integer
        }
        with (
            patch.object(convert_helper, "recho") as mock_recho,
            patch.object(
                convert_helper.sys,
                "exit",
                side_effect=AssertionError("Should not exit"),
            ),
        ):
            # Act / Assert: does not raise and does not log errors
            convert_helper.validate_converter_config(config)
            mock_recho.assert_not_called()

    def test_invalid_input_format_string_exits(self):
        """Exit when input_format is not a supported Enum member."""
        config = {
            "input_format": "UNKNOWN",
            "output_format": convert_helper.OutputFormats.CSV,
            "skip_footer": 0,
        }
        with (
            patch.object(convert_helper, "recho") as mock_recho,
            patch.object(
                convert_helper.sys, "exit", side_effect=SystemExit(1)
            ) as mock_exit,
        ):
            with self.assertRaises(SystemExit):
                convert_helper.validate_converter_config(config)
            self.assertTrue(mock_recho.called)
            mock_exit.assert_called_once_with(1)

    def test_invalid_output_format_string_exits(self):
        """Exit when output_format is not a supported Enum member."""
        config = {
            "input_format": convert_helper.InputFormats.GAMRY,
            "output_format": "KSV",  # not an Enum member
            "skip_footer": 0,
        }
        with (
            patch.object(convert_helper, "recho") as mock_recho,
            patch.object(
                convert_helper.sys, "exit", side_effect=SystemExit(1)
            ) as mock_exit,
        ):
            with self.assertRaises(SystemExit):
                convert_helper.validate_converter_config(config)
            self.assertTrue(mock_recho.called)
            mock_exit.assert_called_once_with(1)

    def test_missing_parameter_exits_with_key_in_message(self):
        """Exit and log an error when a required configuration key is missing."""
        config = {
            # "input_format" is missing
            "output_format": convert_helper.OutputFormats.CSV,
            "additional": {"skip_footer": 0},
        }
        with (
            patch.object(convert_helper, "recho") as mock_recho,
            patch.object(
                convert_helper.sys, "exit", side_effect=SystemExit(1)
            ) as mock_exit,
        ):
            with self.assertRaises(SystemExit):
                convert_helper.validate_converter_config(config)
            self.assertTrue(mock_recho.called)
            # Ensure the missing key name is mentioned in the log message
            self.assertIn("input_format", mock_recho.call_args[0][0])
            mock_exit.assert_called_once_with(1)

    def test_non_int_skip_footer_does_not_trigger_when_enum_input(self):
        """Skip-footer type check is not applied when input_format is an Enum member."""
        config = {
            "input_format": convert_helper.InputFormats.GAMRY,
            "output_format": convert_helper.OutputFormats.CSV,
            "additional": {"skip_footer": "not-an-int"},
        }
        with (
            patch.object(convert_helper, "recho") as mock_recho,
            self.assertRaises(SystemExit),
        ):
            convert_helper.validate_converter_config(config)
        mock_recho.assert_called_once_with("Parameter 'skip_footer' is not an integer.")


class TestRunConverter(unittest.TestCase):
    """Tests for run_converter."""

    def test_calls_convert_on_provided_object(self):
        """Ensure run_converter delegates to the convert() method of the given object."""
        converter_mock = MagicMock()
        convert_helper.run_converter(converter_mock)
        converter_mock.convert.assert_called_once()


if __name__ == "__main__":
    unittest.main()
