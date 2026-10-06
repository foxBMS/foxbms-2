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

"""Validate battery system configuration files.

This module validates the ``battery_system_cfg.json`` configuration file used
by the foxBMS build system. It parses the JSON structure into typed dataclasses
representing the battery system's topology, timing, current thresholds,
operational flags, and hardware configuration.

The following configuration sections are validated:
    - **Topology** - Number of strings, modules, cell blocks, parallel cells,
      temperature sensors, and contactors.
    - **Timeouts** - Response timeouts for current, coulomb counting, and energy
      counting measurements in ms.
    - **Currents** - Maximum string current, rest current, no-current threshold,
      and contactor break current in mA.
    - **Flags** - Boolean operational switches (e.g. interlock feedback, CAN
      timing checks, balancing defaults, fuse placement checks).
    - **Fuse** - Trigger duration and maximum voltage drop configuration.
    - **Relaxation** - Relaxation period configuration.

The validation is performed during the waf build phase. Any raised error is
reported by :class:`waf_tools.config_codegen_task.CreateCfgFile`, which then
fails the task and thereby the build.

Usage
-----
The validator is typically passed as a callback to the ``codegen_config``
feature in a waf build script::

    bld(
        features="codegen_config",
        json=bld.srcnode.find_node("conf/bms/battery_system_cfg.json"),
        validator=battery_system_config_validate.validate_battery_system_configuration,
        target=f"{cp}battery_system_cfg",
    )

It can also be called directly::

    cfg = validate_battery_system_configuration(bld, battery_system_json_node)

Dependencies
------------
- :mod:`waf_tools.battery_config_validate_utils` - Shared extraction and parsing helpers.
- :mod:`waf_tools.config_validate_utils` - JSON file reading utilities.
"""

from battery_config_validate_utils import (
    DEFINE_CONFIG,
    BoolDefine,
    FloatDefine,
    IntDefine,
    extract_defines,
    parse_defines_to_dataclass,
)
from config_validate_utils import read_json_from_node
from pydantic.dataclasses import dataclass
from waflib.Build import BuildContext
from waflib.Node import Node


class InvalidBatterySystemConfigurationError(RuntimeError):
    """Raised for an invalid battery system configuration.

    This exception is used throughout the battery system configuration
    validation pipeline to signal missing defines, type mismatches, or
    constraint violations.
    """


@dataclass(config=DEFINE_CONFIG, frozen=True)
class SystemTopology:
    """Battery system topology configuration.

    Describes the physical layout of the battery pack including the number
    of strings, modules per string, cell blocks per module, parallel cells
    per cell block, temperature sensors per module, and contactors outside
    of strings.
    """

    nr_of_strings: IntDefine
    nr_of_modules_per_string: IntDefine
    nr_of_cell_blocks_per_module: IntDefine
    nr_of_parallel_cells_per_cell_block: IntDefine
    nr_of_temp_sensors_per_module: IntDefine
    nr_of_contactors_outside_strings: IntDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class SystemTimeouts:
    """Battery system timeout configuration.

    Defines the maximum allowed response times for current measurement,
    coulomb counting measurement, and energy counting measurement subsystems.
    """

    current_measurement_response_timeout_ms: IntDefine
    coulomb_counting_measurement_response_timeout_ms: IntDefine
    energy_counting_measurement_response_timeout_ms: IntDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class SystemCurrents:
    """Battery system current thresholds.

    Defines the maximum string current, rest current threshold, no-current
    detection threshold, and contactor break current limit.
    """

    maximum_string_current_ma: FloatDefine
    rest_current_ma: FloatDefine
    cs_threshold_no_current_ma: FloatDefine
    main_contactors_maximum_break_current_ma: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class SystemFlags:
    """Battery system boolean operational flags.

    Controls various system behaviors such as current direction convention,
    interlock feedback handling, CAN timing validation, balancing defaults,
    and fuse placement checks.
    """

    positive_discharge_current: BoolDefine
    ignore_interlock_feedback: BoolDefine
    check_can_timing: BoolDefine
    balancing_default_inactive: BoolDefine
    check_fuse_placed_in_normal_path: BoolDefine
    check_fuse_placed_in_charge_path: BoolDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class SystemFuse:
    """Battery system fuse configuration.

    Defines the maximum trigger duration for the main fuse and the maximum
    acceptable voltage drop across the fuse.
    """

    main_fuse_maximum_trigger_duration_ms: FloatDefine
    max_voltage_drop_over_fuse_mv: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class SystemRelaxation:
    """Battery system relaxation configuration.

    Defines the relaxation period used to determine when the battery pack
    has reached a rested state.
    """

    relaxation_period_10ms: FloatDefine


@dataclass(config=DEFINE_CONFIG, frozen=True)
class BatterySystemConfiguration:
    """Complete validated battery system configuration.

    Aggregates all sub-configurations (topology, timeouts, currents, flags,
    fuse, relaxation) into a single object returned by the validation pipeline.
    """

    topology: SystemTopology
    timeouts: SystemTimeouts
    currents: SystemCurrents
    flags: SystemFlags
    fuse: SystemFuse
    relaxation: SystemRelaxation


def parse_battery_system_config(cfg: dict) -> BatterySystemConfiguration:
    """Parse a raw JSON dictionary into a validated battery system configuration.

    Extracts the ``#define`` entries for ``battery_system_cfg.h`` from the
    configuration dictionary and maps them to the corresponding typed
    dataclasses. The define name expected for each
    field is derived as described in
    :func:`~waf_tools.battery_config_validate_utils.parse_defines_to_dataclass`.

    Args:
        cfg: Parsed JSON content of ``battery_system_cfg.json``, expected to
            contain a ``files`` list with an entry for ``battery_system_cfg.h``.

    Returns:
        A fully populated and validated :class:`BatterySystemConfiguration`
        instance.

    Raises:
        InvalidBatterySystemConfigurationError: If a required define is missing
            or a value does not match the expected type.
    """
    defines = extract_defines(
        cfg, "battery_system_cfg.h", InvalidBatterySystemConfigurationError
    )

    topology = parse_defines_to_dataclass(
        defines, SystemTopology, "BS", InvalidBatterySystemConfigurationError
    )
    timeouts = parse_defines_to_dataclass(
        defines, SystemTimeouts, "BS", InvalidBatterySystemConfigurationError
    )
    currents = parse_defines_to_dataclass(
        defines, SystemCurrents, "BS", InvalidBatterySystemConfigurationError
    )
    flags = parse_defines_to_dataclass(
        defines, SystemFlags, "BS", InvalidBatterySystemConfigurationError
    )
    fuse = parse_defines_to_dataclass(
        defines, SystemFuse, "BS", InvalidBatterySystemConfigurationError
    )
    relaxation = parse_defines_to_dataclass(
        defines, SystemRelaxation, "BS", InvalidBatterySystemConfigurationError
    )

    return BatterySystemConfiguration(
        topology=topology,
        timeouts=timeouts,
        currents=currents,
        flags=flags,
        fuse=fuse,
        relaxation=relaxation,
    )


def validate_battery_system_configuration(
    bld: BuildContext, battery_system_json: Node
) -> BatterySystemConfiguration:
    """Validate the battery system configuration JSON file.

    Reads the JSON configuration from the provided node and parses it into a
    typed :class:`BatterySystemConfiguration`.
    This function is intended to be passed as the ``validator`` callback to
    the ``codegen_config`` waf feature.
    The raised error is reported by
    :class:`waf_tools.config_codegen_task.CreateCfgFile`, which then fails the
    task.

    Args:
        bld: The active waf build context.
        battery_system_json: Node pointing to the ``battery_system_cfg.json``
            configuration file.

    Returns:
        A validated :class:`BatterySystemConfiguration` instance.

    Raises:
        InvalidBatterySystemConfigurationError: If a file entry or a required
            define is missing, or a value has the wrong type.
    """
    battery_system_cfg = read_json_from_node(bld, battery_system_json)
    return parse_battery_system_config(battery_system_cfg)
