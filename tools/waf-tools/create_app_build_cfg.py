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

"""Create build configuration metadata for generated application artifacts."""

import csv
import re
from dataclasses import dataclass
from datetime import UTC, datetime

from c_template import render_template
from waflib.Configure import ConfigurationContext, conf


@dataclass
class BuildConfiguration:  # pylint: disable=too-many-instance-attributes
    """Build configuration values written into ``app_build_cfg.c``."""

    soc_algorithm: str
    soe_algorithm: str
    sof_algorithm: str
    soh_algorithm: str
    imd: str
    balancing_strategy: str
    rtos: str
    afe: str
    temperature_sensor: str
    temperature_sensor_method: str


AFE_NAME_MAP: dict[tuple[str, str], str] = {
    ("adi", "ades1830"): "ADI_ADES1830",
    ("debug", "can"): "DEBUG_CAN",
    ("debug", "default"): "DEBUG_DEFAULT",
    ("ltc", "6804-1"): "LTC_LTC6804_1",
    ("ltc", "6806"): "LTC_LTC6806",
    ("ltc", "6811-1"): "LTC_LTC6811_1",
    ("ltc", "6812-1"): "LTC_LTC6812_1",
    ("ltc", "6813-1"): "LTC_LTC6813_1",
    ("maxim", "max17852"): "MAXIM_MAX17852",
    ("nxp", "mc33775a"): "NXP_MC33775A",
    ("ti", "dummy"): "TI_DUMMY",
}


BUILD_CONFIGURATION_FIELDS: tuple[str, ...] = (
    "socAlgorithm",
    "soeAlgorithm",
    "sofAlgorithm",
    "sohAlgorithm",
    "imd",
    "balancingStrategy",
    "rtos",
    "afe",
    "temperatureSensor",
    "temperatureSensorMethod",
)


def _to_upper_snake_case(identifier: str) -> str:
    """Convert lower camelCase or lower-case identifiers to UPPER_SNAKE_CASE."""
    return re.sub(r"(?<!^)(?=[A-Z])", "_", identifier).upper()


def _to_snake_case(identifier: str) -> str:
    """Convert lower camelCase or lower-case identifiers to snake_case."""
    return re.sub(r"(?<!^)(?=[A-Z])", "_", identifier).lower()


def _build_configuration_entry(field_name: str, value: str) -> str:
    """Format one C struct assignment line for the build configuration block."""
    enum_prefix = _to_upper_snake_case(field_name)
    return f"    .{field_name} = {enum_prefix}_{value},"


@conf
def create_app_build_cfg_c(ctx: ConfigurationContext, txt: str) -> str:
    """Inject build configuration and update header metadata in a C source.

    Args:
        ctx : Waf ConfigurationContext Waf
        txt : Input C source template as string

    Returns:
        The generated C source code containing build configuration information
    """
    build_configuration = get_build_configuration(ctx)
    today = datetime.now(tz=UTC).date().strftime("%Y-%m-%d")

    build_configuration_block = "\n".join(
        [
            "const VER_BUILD_CONFIGURATION_s ver_foxbmsBuildConfiguration = {",
            *[
                _build_configuration_entry(
                    i,
                    getattr(build_configuration, _to_snake_case(i)),
                )
                for i in BUILD_CONFIGURATION_FIELDS
            ],
            "};",
        ]
    )

    includes = '#include "app_build_cfg.h"'
    return render_template(
        txt,
        {
            "file_name": "app_build_cfg.c",
            "author": "foxBMS Team",
            "date": f"{today} (date of creation)",
            "updated": f"{today} (date of last update)",
            "version": f"v{ctx.env.VERSION}",
            "ingroup": "GENERAL",
            "prefix": "VER",
            "brief": "Build configuration information that is generated by the toolchain.",
            "details": "Build configuration information that is generated by the toolchain.",
            "include_directives": includes,
            "extern_constant_and_variable_definitions": build_configuration_block,
        },
    )


def get_build_configuration(ctx: ConfigurationContext) -> BuildConfiguration:
    """Collect all build configuration values for code generation.

    Args:
        ctx: Waf context

    Returns:
        Structured build configuration used by code-generation templates.
    """
    soc_algorithm = str(ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC).upper()
    sof_algorithm = str(ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF).upper()
    soe_algorithm = str(ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE).upper()
    soh_algorithm = str(ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH).upper()
    balancing_strategy = str(ctx.env.FOXBMS_BALANCING_STRATEGY).upper()
    rtos = str(ctx.env.FOXBMS_RTOS_NAME).upper()
    temperature_sensor_method = (
        str(ctx.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_METHOD)
        .upper()
        .replace("-", "_")
    )
    return BuildConfiguration(
        soc_algorithm=soc_algorithm,
        soe_algorithm=soe_algorithm,
        sof_algorithm=sof_algorithm,
        soh_algorithm=soh_algorithm,
        imd=get_imd(ctx),
        balancing_strategy=balancing_strategy,
        rtos=rtos,
        afe=get_afe(ctx),
        temperature_sensor=get_temperature_sensor(ctx),
        temperature_sensor_method=temperature_sensor_method,
    )


def get_imd(ctx: ConfigurationContext) -> str:
    """Get the IMD identifier from the active configuration.

    Args:
        ctx: Waf configuration context.

    Returns:
        IMD identifier
    """
    imd = str(ctx.env.FOXBMS_IMD_MANUFACTURER).upper()
    if ctx.env.FOXBMS_IMD_MODEL == "none":
        return imd
    imd += "_" + str(ctx.env.FOXBMS_IMD_MODEL).upper()
    return imd


def get_afe(ctx: ConfigurationContext) -> str:
    """Get the AFE identifier from configuration.

    Args:
        ctx: Waf configuration context.

    Returns:
        AFE token used in generated code.
    """
    afe_man = str(ctx.env.FOXBMS_BMS_SLAVE_AFE_MANUFACTURER).strip().lower()
    afe_ic = str(ctx.env.FOXBMS_BMS_SLAVE_AFE_IC).strip().lower()
    afe = AFE_NAME_MAP.get((afe_man, afe_ic))
    if afe:
        return afe

    err_msg = (
        "Could not find AFE for configured manufacturer and IC: "
        f"manufacturer='{afe_man}', ic='{afe_ic}'."
    )
    ctx.fatal(err_msg)
    return ""  # for the linter, unreachable due to ctx.fatal()


def get_temperature_sensor(ctx: ConfigurationContext) -> str:
    """Get the temperature sensor identifier from configuration.

    Args:
        ctx: Waf configuration context.

    Returns:
        Temperature sensor identifier
    """
    short_name_definition_file = "docs/software/modules/driver/ts/ts-short-names.csv"
    temperature_sensor_csv = ctx.srcnode.find_node(short_name_definition_file)
    if not temperature_sensor_csv:
        err_msg = f"Temperature sensor short name CSV file not found: {short_name_definition_file}"
        ctx.fatal(err_msg)

    temperature_sensor_manufacturer = (
        str(ctx.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER).strip().lower()
    )
    temperature_sensor_model = (
        str(ctx.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL).strip().lower()
    )

    with open(
        temperature_sensor_csv.abspath(), encoding="utf-8", newline=""
    ) as csv_file:
        reader = csv.DictReader(csv_file, delimiter=";")
        for row in reader:
            manufacturer = row.get("Manufacturer", "").strip().lower()
            model = row.get("Model", "").strip().lower()
            short_name = row.get("*Short Name*", "").strip().upper()
            if (manufacturer == temperature_sensor_manufacturer) and (
                model == temperature_sensor_model
            ):
                return short_name

    err_msg = "Could not find a matching temperature sensor short name for the configured sensor."
    ctx.fatal(err_msg)
    return ""  # for the linter, unreachable due to ctx.fatal()
