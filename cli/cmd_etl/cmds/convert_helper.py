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

"""Converter subcommand implementation"""

import sys

from ...helpers.click_helpers import recho
from ..etl.convert import ConversionSettings, Converter, InputFormats, OutputFormats


def converter_setup(config: dict) -> Converter:
    """Create and return a configured Converter instance.

    This function validates the provided configuration's "conversion" section,
    builds a ConversionSettings object from it, and then instantiates a Converter
    using the given data_path and recursive flags.

    Args:
        config: A dictionary containing:

            - "conversion": dict with keys:
                - "input_format": expected input format (InputFormats member).

                - "output_format": desired output format (OutputFormats member).
                - "skip_footer": optional int, number of footer lines to skip (for DTA).

            - "data_path": pathlib.Path pointing to a file or directory.
            - "recursive": bool indicating whether to traverse subdirectories.

    Returns:
        A Converter that is ready to execute the conversion.

    Exit behavior:
        If validation fails, validate_converter_config prints an error via recho and
        terminates the process with sys.exit(1).
    """
    validate_converter_config(config["conversion"])
    conversion = ConversionSettings(**config["conversion"])
    return Converter(config["data_path"], config["recursive"], conversion)


def validate_converter_config(config: dict) -> None:
    """Validate the conversion configuration dictionary.

    Validations performed:

        - input_format must be a member of InputFormats.
        - output_format must be a member of OutputFormats.

        - If input_format equals the literal string "DTA", then "skip_footer"

          must be an integer.

    On validation failure:

        - Prints a descriptive error message via recho.
        - Exits the process with sys.exit(1).

    Args:
        config: Dict with required keys:

            - "input_format": InputFormats member (or "DTA" string for the specific check).
            - "output_format": OutputFormats member.

            - "skip_footer": Required for DTA if using the string "DTA"; should be int.
            - "skip": Required for GRAPHTEC; should be int.

    Notes:
        The current implementation mixes Enum and string checks:

        - Membership checks use Enum members.
        - An additional type check for "skip_footer" is triggered only when
          input_format is the literal "DTA" string.
        - An additional type check for "skip" is triggered only when
          input_format is the literal "GRAPHTEC" string.
    """
    try:
        if config["input_format"] not in InputFormats:
            recho(
                f"Unkown input format '{config['input_format']}'! "
                "See in documentation for more information."
            )
            sys.exit(1)
        if config["output_format"] not in OutputFormats:
            recho(
                f"Unkown output format '{config['output_format']}'! See "
                "in documentation for more information."
            )
            sys.exit(1)
        if config["input_format"] == InputFormats.GAMRY:  # pragma: no cover
            if not isinstance(config["additional"]["skip_footer"], int):
                recho("Parameter 'skip_footer' is not an integer.")
                sys.exit(1)
        if config["input_format"] == InputFormats.GRAPHTEC:  # pragma: no cover
            if not isinstance(config["additional"]["skip"], int):
                recho("Parameter 'skip' is not an integer.")
                sys.exit(1)
    except KeyError as e:
        recho(f"Parameter in configuration file is missing: '{e}'")
        sys.exit(1)


def run_converter(converter_obj: Converter) -> None:
    """Execute the conversion using the provided Converter instance.

    This convenience function delegates directly to Converter.convert().

    Args:
        converter_obj: An instantiated Converter.

    Returns:
        None
    """
    converter_obj.convert()
