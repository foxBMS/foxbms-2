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

# cspell:ignore ddegc

"""Validate battery cell configuration files.

This module validates the ``battery_cell_cfg.json`` configuration file used
by the foxBMS build system. It parses the JSON structure into typed dataclasses
representing the battery cell's electrical and thermal operating limits and
ensures all required defines are present and have the declared type.

The following configuration sections are validated:
    - **Temperature limits** (charge/discharge) - MSL, RSL, MOL thresholds in
      deci-degrees Celsius.
    - **Voltage limits** - Maximum, nominal, and minimum cell voltages in mV.
    - **Current limits** (charge/discharge) - MSL, RSL, MOL thresholds in mA.
    - **Capacity** - Cell capacity (mAh) and energy (Wh).
    - **Lookup tables** - File references for SOC and SOE estimation tables.

The validation is performed during the waf build phase. Any raised error is
reported by :class:`waf_tools.config_codegen_task.CreateCfgFile`, which then
fails the task and thereby the build.

Usage
-----
The validator is typically passed as a callback to the ``codegen_config``
feature in a waf build script::

    bld(
        features="codegen_config",
        json=bld.srcnode.find_node("conf/bms/battery_cell_cfg.json"),
        validator=battery_cell_config_validate.validate_battery_cell_configuration,
        target=f"{cp}battery_cell_cfg",
    )

It can also be called directly::

    cfg = validate_battery_cell_configuration(bld, battery_cell_json_node)

Dependencies
------------
- :mod:`waf_tools.battery_config_validate_utils` - Shared extraction and parsing helpers.
- :mod:`waf_tools.config_validate_utils` - JSON file reading utilities.
"""

from battery_config_validate_utils import (
    DEFINE_CONFIG,
    FloatDefine,
    StrDefine,
    extract_defines,
    parse_defines_to_dataclass,
)
from config_validate_utils import read_json_from_node
from pydantic.dataclasses import dataclass
from waflib.Build import BuildContext
from waflib.Node import Node


class InvalidBatteryCellConfigurationError(RuntimeError):
    """Raised for an invalid battery cell configuration.

    This exception is used throughout the battery cell configuration
    validation pipeline to signal missing defines, type mismatches, or
    constraint violations.
    """


@dataclass(config=DEFINE_CONFIG, frozen=True)
class CellTemperatureLimits:
    """Operating temperature limits for a battery cell.

    Represents the maximum safety limit (MSL), recommended safety limit (RSL),
    and maximum operating limit (MOL) for both high and low temperature
    boundaries.
    """

    max_msl_ddegc: FloatDefine
    max_rsl_ddegc: FloatDefine
    max_mol_ddegc: FloatDefine
    min_msl_ddegc: FloatDefine
    min_rsl_ddegc: FloatDefine
    min_mol_ddegc: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class CellVoltageLimits:
    """Operating voltage limits for a battery cell.

    Represents the maximum safety limit (MSL), recommended safety limit (RSL),
    maximum operating limit (MOL), nominal voltage, and the corresponding
    minimum thresholds.
    """

    max_msl_mv: FloatDefine
    max_rsl_mv: FloatDefine
    max_mol_mv: FloatDefine
    nominal_mv: FloatDefine
    min_msl_mv: FloatDefine
    min_rsl_mv: FloatDefine
    min_mol_mv: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class CellCurrentLimits:
    """Operating current limits for a battery cell in one direction.

    Represents the maximum safety limit (MSL), recommended safety limit (RSL),
    and maximum operating limit (MOL) for either the charge or discharge
    direction.
    """

    max_msl_ma: FloatDefine
    max_rsl_ma: FloatDefine
    max_mol_ma: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class CellCapacity:
    """Battery cell capacity and energy rating."""

    capacity_mah: FloatDefine
    energy_wh: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class CellLookupTables:
    """File references for SOC and SOE lookup tables.

    The referenced CSV files are used by the state estimation algorithms
    to map measured values to state-of-charge and state-of-energy.
    """

    soc_lookup_table: StrDefine
    soe_lookup_table: StrDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class BatteryCellConfiguration:
    """Complete validated battery cell configuration.

    Aggregates all sub-configurations (temperature, voltage, current,
    capacity, lookup tables) into a single object returned by the validation
    pipeline.
    """

    temperature_discharge: CellTemperatureLimits
    temperature_charge: CellTemperatureLimits
    voltage: CellVoltageLimits
    current_discharge: CellCurrentLimits
    current_charge: CellCurrentLimits
    capacity: CellCapacity
    lookup_tables: CellLookupTables


def parse_battery_cell_config(cfg: dict) -> BatteryCellConfiguration:
    """Parse a raw JSON dictionary into a validated battery cell configuration.

    Extracts the ``#define`` entries for ``battery_cell_cfg.h`` and
    ``battery_cell_cfg.c`` from the configuration dictionary and maps them
    to the corresponding typed dataclasses. The define name expected for each
    field is derived as described in
    :func:`~waf_tools.battery_config_validate_utils.parse_defines_to_dataclass`.

    Args:
        cfg: Parsed JSON content of ``battery_cell_cfg.json``, expected to
            contain a ``files`` list with entries for ``battery_cell_cfg.h``
            and ``battery_cell_cfg.c``.

    Returns:
        A fully populated and validated :class:`BatteryCellConfiguration`
        instance.

    Raises:
        InvalidBatteryCellConfigurationError: If a required define is missing
            or a value does not match the expected type.
    """
    defines_h = extract_defines(
        cfg, "battery_cell_cfg.h", InvalidBatteryCellConfigurationError
    )
    defines_c = extract_defines(
        cfg, "battery_cell_cfg.c", InvalidBatteryCellConfigurationError
    )

    temperature_discharge = parse_defines_to_dataclass(
        defines_h,
        CellTemperatureLimits,
        "BC_TEMPERATURE",
        InvalidBatteryCellConfigurationError,
        "DISCHARGE",
    )
    temperature_charge = parse_defines_to_dataclass(
        defines_h,
        CellTemperatureLimits,
        "BC_TEMPERATURE",
        InvalidBatteryCellConfigurationError,
        "CHARGE",
    )
    voltage = parse_defines_to_dataclass(
        defines_h, CellVoltageLimits, "BC_VOLTAGE", InvalidBatteryCellConfigurationError
    )
    current_discharge = parse_defines_to_dataclass(
        defines_h,
        CellCurrentLimits,
        "BC_CURRENT",
        InvalidBatteryCellConfigurationError,
        "DISCHARGE",
    )
    current_charge = parse_defines_to_dataclass(
        defines_h,
        CellCurrentLimits,
        "BC_CURRENT",
        InvalidBatteryCellConfigurationError,
        "CHARGE",
    )
    capacity = parse_defines_to_dataclass(
        defines_h, CellCapacity, "BC", InvalidBatteryCellConfigurationError
    )
    lookup_tables = parse_defines_to_dataclass(
        defines_c, CellLookupTables, "", InvalidBatteryCellConfigurationError
    )

    return BatteryCellConfiguration(
        temperature_discharge=temperature_discharge,
        temperature_charge=temperature_charge,
        voltage=voltage,
        current_discharge=current_discharge,
        current_charge=current_charge,
        capacity=capacity,
        lookup_tables=lookup_tables,
    )


def validate_battery_cell_configuration(
    bld: BuildContext, battery_cell_json: Node
) -> BatteryCellConfiguration:
    """Validate the battery cell configuration JSON file.

    Reads the JSON configuration from the provided node and parses it into a
    typed :class:`BatteryCellConfiguration`.
    This function is intended to be passed as the ``validator`` callback to
    the ``codegen_config`` waf feature.
    The raised error is reported by
    :class:`waf_tools.config_codegen_task.CreateCfgFile`, which then fails the
    task.

    Args:
        bld: The active waf build context.
        battery_cell_json: Node pointing to the ``battery_cell_cfg.json``
            configuration file.

    Returns:
        A validated :class:`BatteryCellConfiguration` instance.

    Raises:
        InvalidBatteryCellConfigurationError: If a file entry or a required
            define is missing, or a value has the wrong type.
    """
    battery_cell_cfg = read_json_from_node(bld, battery_cell_json)
    return parse_battery_cell_config(battery_cell_cfg)
