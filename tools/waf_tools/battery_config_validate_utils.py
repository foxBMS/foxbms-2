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

# cspell:ignore ddegc, DDEGC

"""Shared helper functions for battery configuration validators.

This module provides common utilities used by the battery cell and battery
system configuration validators. It handles the extraction of ``#define``
values from the foxBMS JSON configuration format and maps them to typed
Python dataclass instances.

Key functionality:
    - **Define extraction** - Locates define entries for a specific generated file
      (e.g. ``battery_cell_cfg.h``) within the JSON configuration structure.
    - **Dataclass mapping** - Converts flat define dictionaries into typed
      dataclass instances by matching field names to define name patterns.
    - **Type checking** - Validates the raw JSON values against the types
      declared in the target dataclass fields, raising descriptive errors on
      type mismatch.

Usage
-----
These utilities are not called directly from build scripts but are imported
by the specific validators::

    from battery_config_validate_utils import extract_defines, parse_defines_to_dataclass

    defines = extract_defines(cfg, "battery_cell_cfg.h", InvalidBatteryCellConfigurationError)
    voltage = parse_defines_to_dataclass(
        defines, CellVoltageLimits, "BC_VOLTAGE", InvalidBatteryCellConfigurationError
    )
"""

from dataclasses import fields
from typing import Annotated

from pydantic import BeforeValidator, ConfigDict, ValidationError

DEFINE_CONFIG = ConfigDict(strict=True)


def _int_from_number(value: object) -> object:
    if isinstance(value, bool):
        return value
    if isinstance(value, float) and value.is_integer():
        return int(value)
    return value


def _float_from_number(value: object) -> object:
    if isinstance(value, bool):
        return value
    if isinstance(value, int):
        return float(value)
    return value


IntDefine = Annotated[int, BeforeValidator(_int_from_number)]
FloatDefine = Annotated[float, BeforeValidator(_float_from_number)]
BoolDefine = bool
StrDefine = str


def _format_validation_error(
    cls: type, error: ValidationError, define_names: dict[str, str]
) -> str:
    count = error.error_count()
    define = "define" if count == 1 else "defines"
    lines = [f"{count} invalid {define} for '{cls.__name__}':"]
    for err in error.errors(include_url=False):
        if err["loc"]:
            field = str(err["loc"][0])
            name = define_names.get(field, field.upper())
        else:
            name = f"<{cls.__name__}>"
        lines.append(f"  {name}: {err['msg']} (got {err['input']!r})")
    return "\n".join(lines)


def extract_defines(
    cfg: dict, file_name: str, error_cls: type[RuntimeError]
) -> dict[str, object]:
    """Extract define values for a specific generated file.

    Searches the ``files`` list in the configuration dictionary for an entry
    whose ``file_name`` matches the requested target file. Returns the
    contained ``defines`` as a flat dictionary mapping define names to their
    raw values.

    The expected JSON structure is::

        {
            "files": [
                {
                    "file_name": "<file_name>",
                    "defines": [
                        {
                            "name": "<define_name>",
                            "value": <value>,
                            "value_type": "<value_type>"
                        },
                        ...
                    ],
                },
                ...
            ]
        }

    Args:
        cfg: Parsed JSON content of a foxBMS configuration file (e.g.
            ``battery_cell_cfg.json``), expected to contain a ``files`` list.
        file_name: The target file name to look up (e.g. ``battery_cell_cfg.h``).
        error_cls: The exception class to raise if the file entry is not found.

    Returns:
        A dictionary mapping define names to their raw values.

    Raises:
        RuntimeError: If no entry with the given ``file_name`` exists in the
            configuration's ``files`` list. The concrete exception type is
            determined by ``error_cls``.
        KeyError: If a ``files`` entry has no ``file_name`` or ``defines`` key,
            or a define entry has no ``name`` or ``value`` key.
    """
    for file_entry in cfg.get("files", []):
        if file_entry["file_name"] == file_name:
            return {d["name"]: d["value"] for d in file_entry["defines"]}
    err_msg = f"File '{file_name}' not found in configuration"
    raise error_cls(err_msg)


def parse_defines_to_dataclass[T](
    defines: dict[str, object],
    cls: type[T],
    prefix: str,
    error_cls: type[RuntimeError],
    infix: str = "",
) -> T:
    """Map a flat define dictionary to a typed dataclass instance.

    Iterates over the fields of the target dataclass ``cls`` and resolves
    each field's value from ``defines`` by constructing the expected define
    name from ``prefix``, an optional ``infix``, and the upper-cased field
    name. The resolved values are then validated against the declared field type
    by pydantic.

    The define name is constructed as follows:

    - Without infix: ``<PREFIX>_<FIELD_NAME>`` (e.g. ``BC_VOLTAGE_MAX_MSL_MV``).
    - With infix: ``<PREFIX>_<FIRST_FIELD_PART>_<INFIX>_<REMAINING_FIELD_PARTS>``
      (e.g. for field ``max_msl_ddegc`` with prefix ``BC_TEMPERATURE`` and
      infix ``DISCHARGE``: ``BC_TEMPERATURE_MAX_DISCHARGE_MSL_DDEGC``).

    The infix is inserted after the first underscore-separated part of the
    field name, which allows a single dataclass definition to be reused for
    multiple directional variants (e.g. charge/discharge).

    Args:
        defines: Flat dictionary of define names to raw values, as returned
            by :func:`extract_defines`.
        cls: The target dataclass type to instantiate. Its fields define which
            defines are required and what types are expected.
        prefix: The common prefix for all define names in this group
            (e.g. ``"BC_VOLTAGE"``, ``"BS"``).
        error_cls: The exception class to raise on missing defines or type
            mismatches.
        infix: Optional directional or categorical infix inserted into the
            define name (e.g. ``DISCHARGE``, ``CHARGE``).

    Returns:
        A fully populated instance of ``cls``.

    Raises:
        RuntimeError: If a required define is missing from ``defines`` or if
            one or more values do not match the declared field type. The
            concrete exception type is determined by ``error_cls``. For type
            mismatches the message is produced by ``_format_validation_error()``.
    """
    upper_defines = {k.upper(): v for k, v in defines.items()}
    kwargs: dict[str, object] = {}
    define_names: dict[str, str] = {}
    for f in fields(cls):
        parts = f.name.upper().split("_")
        if infix:
            parts = [parts[0]] + [infix.upper()] + parts[1:]
        field_key = "_".join(parts)
        define_name = f"{prefix}_{field_key}".upper() if prefix else field_key.upper()
        define_names[f.name] = define_name

        if define_name not in upper_defines:
            err_msg = f"Required define '{define_name}' is missing"
            raise error_cls(err_msg)

        kwargs[f.name] = upper_defines[define_name]

    try:
        return cls(**kwargs)
    except ValidationError as e:
        raise error_cls(_format_validation_error(cls, e, define_names)) from e
