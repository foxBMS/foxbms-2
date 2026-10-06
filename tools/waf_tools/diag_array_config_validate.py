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

"""Validate and render the foxBMS diagnosis array configuration.

This module validates the ``diag_array_cfg.json`` configuration file that
defines the diagnostic event table used by the foxBMS diagnostic engine.
It ensures consistency between the JSON configuration, the ``DIAG_ID_e``
enumeration in ``diag_cfg.h``, and the DBC file's fatal error signal choices.

The validation performs the following checks:
    - **Structure** - All required keys (``event``, ``sensitivity``, ``severity``,
      ``delay``, ``enabled``, ``callback``) are present in each entry.
    - **Value ranges** - Sensitivity, severity, delay, and enabled values are
      restricted to the supported C literal mappings.
    - **Header consistency** - Event names and their order match the ``DIAG_ID_e``
      enum entries in ``src/app/engine/config/diag_cfg.h``.
    - **DBC consistency** - All fatal error events have matching entries in the
      ``FatalErrorCode`` signal choices of the ``f_BmsFatalError`` CAN message
      defines in ``tools/dbc/foxbms.dbc``.

Additionally, the module provides :func:`render_diag_array` which converts
validated entries into C struct initializer lines for use by the code
generator (:mod:`waf_tools.config_codegen_task`).

Usage
-----
The validator is typically passed as a callback to the ``codegen_config``
feature in a waf build script::

    bld(
        features="codegen_config",
        json=bld.srcnode.find_node(f"conf/bms/diag_array_cfg.json"),
        validator=diag_array_config_validate.validate_diag_array_configuration,
        target=f"{cp}diag_array_cfg",
    )

Dependencies
------------
- :mod:`waf_tools.config_validate_utils` - JSON file reading utilities.
- ``cantools`` - DBC file parsing for CAN signal validation.
"""

import re
from typing import Literal

import cantools.database
from config_validate_utils import read_json_from_node
from pydantic import ConfigDict, ValidationError
from pydantic.dataclasses import dataclass
from waflib.Build import BuildContext
from waflib.Node import Node


class InvalidDiagConfigurationError(RuntimeError):
    """Raised for an invalid diagnostic array configuration.

    This exception is used throughout the diagnosis array validation pipeline
    to signal missing keys, type mismatches, unsupported values, or
    inconsistencies between ``diag_array_cfg.json``, ``diag_cfg.h`` and the
    DBC file.
    """


ENTRY_CONFIG = ConfigDict(strict=True)

DIAG_CFG_H_PATH = "src/app/engine/config/diag_cfg.h"
DBC_PATH = "tools/dbc/foxbms.dbc"

Sensitivity = Literal[1, 3, 5, 10, 20, 50, 100, 500]
Severity = Literal["FATAL_ERROR", "WARNING", "INFO"]
Delay = Literal[-1, 0, 100, 200, 1000, 2000]


SENSITIVITY_TO_C_LITERAL: dict[Sensitivity, str] = {
    1: "DIAG_SEN_EVENT_1",
    3: "DIAG_SEN_EVENT_3",
    5: "DIAG_SEN_EVENT_5",
    10: "DIAG_SEN_EVENT_10",
    20: "DIAG_SEN_EVENT_20",
    50: "DIAG_SEN_EVENT_50",
    100: "DIAG_SEN_EVENT_100",
    500: "DIAG_SEN_EVENT_500",
}
SEVERITY_TO_C_LITERAL: dict[Severity, str] = {
    "FATAL_ERROR": "DIAG_FATAL_ERROR",
    "WARNING": "DIAG_WARNING",
    "INFO": "DIAG_INFO",
}
DELAY_TO_C_LITERAL: dict[Delay, str] = {
    -1: "DIAG_DELAY_DISCARD",
    0: "DIAG_NO_DELAY",
    100: "DIAG_DELAY_100ms",
    200: "DIAG_DELAY_200ms",
    1000: "DIAG_DELAY_1000ms",
    2000: "DIAG_DELAY_2000ms",
}
ENABLED_TO_C_LITERAL: dict[bool, str] = {
    True: "DIAG_EVALUATION_ENABLED",
    False: "DIAG_EVALUATION_DISABLED",
}


@dataclass(config=ENTRY_CONFIG, frozen=True)
class DiagArrayEntry:
    """Single entry in the diagnostic array configuration.

    Represents one row of the generated ``diag_diagnosisIdConfiguration``
    array, i.e. one diagnostic event including its evaluation parameters.

    Attributes:
        event: Diagnosis ID, must match the corresponding ``DIAG_ID_e`` entry.
        description: Description used in the documentation.
        sensitivity: Number of occurrences before the event is reported.
        severity: Severity class (``FATAL_ERROR``, ``WARNING``, ``INFO``).
        delay: Delay before the error reaction is triggered
            (``0``: no delay, ``-1``: discard).
        enabled: Whether the evaluation of this event is enabled.
        callback: Name of the C callback function handling the event.
    """

    event: str
    description: str
    sensitivity: Sensitivity
    severity: Severity
    delay: Delay
    enabled: bool
    callback: str


@dataclass(config=ENTRY_CONFIG, frozen=True)
class DiagArrayConfiguration:
    """Complete diagnostic array configuration.

    Aggregates all validated diagnosis entries in the order in which they are
    defined in the configuration, which corresponds to their ``DIAG_ID_e``
    index.

    Attributes:
        entries: Ordered list of validated diagnosis entries.
    """

    entries: list[DiagArrayEntry]


def _map_sensitivity(value: int) -> str:
    return SENSITIVITY_TO_C_LITERAL[value]


def _map_severity(value: str) -> str:
    return SEVERITY_TO_C_LITERAL[value]


def _map_delay(value: int) -> str:
    return DELAY_TO_C_LITERAL[value]


def _map_enabled(value: bool) -> str:
    return ENABLED_TO_C_LITERAL[value]


def _format_validation_error(raw: dict, index: int, error: ValidationError) -> str:
    count = error.error_count()
    value = "value" if count == 1 else "values"
    event = raw.get("event", "")
    context = f" (event: {event})" if isinstance(event, str) and event else ""
    lines = [f"{count} invalid {value} in array_entries at index {index}{context}:"]
    for err in error.errors(include_url=False):
        key = ".".join(str(i) for i in err["loc"]) if err["loc"] else "<entry>"
        if err["type"].startswith("missing"):
            lines.append(f"  {key}: {err['msg']}")
        else:
            lines.append(f"  {key}: {err['msg']} (got {err['input']!r})")
    return "\n".join(lines)


def _parse_array_entry(raw: dict, index: int) -> DiagArrayEntry:
    try:
        return DiagArrayEntry(**raw)
    except ValidationError as e:
        raise InvalidDiagConfigurationError(
            _format_validation_error(raw, index, e)
        ) from e


def _extract_array_entries(cfg: dict) -> list[DiagArrayEntry]:
    files = cfg.get("files", [])
    if not files:
        err_msg = "No 'files' entry found in configuration"
        raise InvalidDiagConfigurationError(err_msg)
    raw_entries = files[0].get("array_entries", [])
    if not raw_entries:
        err_msg = "No 'array_entries' found in configuration"
        raise InvalidDiagConfigurationError(err_msg)

    return [_parse_array_entry(raw, i) for i, raw in enumerate(raw_entries)]


def _extract_diag_ids_from_header(header_node: Node) -> list[str]:
    txt = header_node.read()
    pattern = r"typedef enum \{.*?\} DIAG_ID_e;"
    match = re.search(pattern, txt, re.DOTALL)
    if not match:
        err_msg = "Could not find 'typedef enum {...} DIAG_ID_e;' in diag_cfg.h"
        raise InvalidDiagConfigurationError(err_msg)

    enum_body = match.group(0)
    # the index in the array corresponds to the ID as the enumeration starts
    # with 0 in C
    return [
        i.split(",")[0].strip()
        for i in enum_body.splitlines()
        if i.strip().startswith("DIAG_ID") and i.split(",")[0].strip() != "DIAG_ID_MAX"
    ]


def _extract_fatal_error_ids(array_cfg: DiagArrayConfiguration) -> dict[str, int]:
    return {
        entry.event: i
        for i, entry in enumerate(array_cfg.entries)
        if entry.severity == "FATAL_ERROR"
    }


def _validate_fatal_errors_against_dbc(
    dbc_node: Node, fatal_error_ids: dict[str, int]
) -> None:
    """Validate that the DBC file and the source code are aligned with respect
    to the fatal errors.
    """
    db = cantools.database.load_file(dbc_node.abspath())

    if not isinstance(db, cantools.database.can.database.Database):
        err_msg = "DBC File is not of type 'Database'."
        raise InvalidDiagConfigurationError(err_msg)

    message = db.get_message_by_name("f_BmsFatalError")
    signal_name = "FatalErrorCode"
    signal = next((sig for sig in message.signals if sig.name == signal_name), None)

    if signal is None:
        err_msg = f"Signal '{signal_name}' not found in message 'f_BmsFatalError'."
        raise InvalidDiagConfigurationError(err_msg)

    if not signal.choices:
        err_msg = f"Signal '{signal_name}' does not support choices."
        raise InvalidDiagConfigurationError(err_msg)

    errors: list[str] = []
    for name, _id in fatal_error_ids.items():
        try:
            dbc_id = signal.choice_to_number(name)
        except KeyError:
            errors.append(f"Signal '{name}' (id={_id}) not found in DBC choices.")
            continue
        if _id != dbc_id:
            errors.append(
                f"Signal '{name}' and its id '{_id}' are not correctly documented "
                f"in the DBC file (DBC has {dbc_id})."
            )
    if errors:
        err_msg = "BMS fatal error message validation failed:\n" + "\n".join(errors)
        raise InvalidDiagConfigurationError(err_msg)


def _validate_events_match_header(
    array_cfg: DiagArrayConfiguration, header_ids: list[str]
) -> None:
    """Validate that the event names in the JSON configuration match the
    DIAG_ID enum entries from diag_cfg.h in both order and naming.
    """
    json_events = [e.event for e in array_cfg.entries]
    for i, (json_event, header_event) in enumerate(
        zip(json_events, header_ids, strict=True)
    ):
        if json_event != header_event:
            err_msg = (
                f"Order mismatch at index {i}: diag_cfg.h expects "
                f"'{header_event}', but JSON has '{json_event}'"
            )
            raise InvalidDiagConfigurationError(err_msg)


def _parse_diag_array_config(cfg: dict) -> DiagArrayConfiguration:
    """Parse 'diag_array_cfg.json' to a validated DiagArrayConfiguration."""
    entries = _extract_array_entries(cfg)
    return DiagArrayConfiguration(entries=entries)


def validate_diag_array_configuration(
    bld: BuildContext, diag_array_json: Node
) -> DiagArrayConfiguration:
    """Validate the diagnostic array configuration file.

    Reads the JSON configuration, parses it into a typed
    :class:`DiagArrayConfiguration`, and cross-checks the configured events
    against the ``DIAG_ID_e`` enumeration in ``diag_cfg.h`` as well as the
    ``FatalErrorCode`` signal choices in ``foxbms.dbc``.

    This function shall be called in the build phase and is intended to be
    passed as the ``validator`` callback to the ``codegen_config`` waf
    feature. The raised error is reported by
    :class:`waf_tools.config_codegen_task.CreateCfgFile`, which then fails
    the task.

    Args:
        bld: The active waf build context.
        diag_array_json: Node pointing to the ``diag_array_cfg.json``
            configuration file.

    Returns:
        DiagArrayConfiguration: Validated diagnostic array configuration.

    Raises:
        InvalidDiagConfigurationError: If ``diag_cfg.h`` or the DBC file
            cannot be found, the configuration is invalid, or it is
            inconsistent with ``diag_cfg.h`` or the DBC file.
        ValueError: If the number of configured events differs the number of
            ``DIAG_ID_e`` enumerators.
    """
    diag_cfg = read_json_from_node(bld, diag_array_json)

    diag_cfg_h = bld.srcnode.find_node(DIAG_CFG_H_PATH)
    if not diag_cfg_h:
        err_msg = f"File '{DIAG_CFG_H_PATH}' not found"
        raise InvalidDiagConfigurationError(err_msg)
    dbc_node = bld.srcnode.find_node(DBC_PATH)
    if not dbc_node:
        err_msg = f"File '{DBC_PATH}' not found"
        raise InvalidDiagConfigurationError(err_msg)

    array_cfg = _parse_diag_array_config(diag_cfg)
    header_ids = _extract_diag_ids_from_header(diag_cfg_h)
    _validate_events_match_header(array_cfg, header_ids)

    fatal_error_ids = _extract_fatal_error_ids(array_cfg)
    _validate_fatal_errors_against_dbc(dbc_node, fatal_error_ids)
    return array_cfg


def render_diag_array(entries: list[dict]) -> str:
    """Render validated diagnostic array entries into C initializer lines.

    Each raw entry is validated before rendering, so that the generated code
    can never contain unmapped values. This function shall be called by the
    Waf code generator.

    Args:
        entries: Raw ``array_entries`` as read from ``diag_array_cfg.json``.

    Returns:
        str: C initializer lines for the diagnostic array.

    Raises:
        InvalidDiagConfigurationError: If an entry is invalid.
    """
    lines = []
    for i, raw in enumerate(entries):
        entry = _parse_array_entry(raw, i)
        sensitivity = _map_sensitivity(entry.sensitivity)
        severity = _map_severity(entry.severity)
        delay = _map_delay(entry.delay)
        enabled = _map_enabled(entry.enabled)
        line = (
            f"    {{{entry.event}, {sensitivity}, {severity}, {delay}, "
            f"{enabled}, {entry.callback}}},"
        )
        lines.append(line)
    return "\n".join(lines)
