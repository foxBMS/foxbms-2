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

"""Validate BMS configuration files"""

import os


from bms_config_validator_base import (
    parse_config_to_system,
    AerosolSensor,
    Algorithm,
    AnalogFrontEnd,
    InsulationMonitoringDevice,
    RTOS,
    BMSSlave,
    CurrentSensor,
    Debug,
)

from waflib.Configure import ConfigurationContext, conf
from waflib.Node import Node


def read_json_from_node(ctx: ConfigurationContext, file: str | Node) -> dict:
    """Read a json file if the file exists."""
    node = file
    if isinstance(file, str):
        node = ctx.path.find_node(file)
    if not node:
        ctx.fatal(f"Could not find file {file}")
    return node.read_json()


def to_define(name: str) -> str:
    """Create a macro from a provided string"""
    return "".join([c if c.isalnum() else "_" for c in name.upper()])


@conf
def validate_bms_configuration(
    ctx: ConfigurationContext, bms_config_node: Node
) -> None:
    """Validate BMS configuration files"""
    bms_config = read_json_from_node(ctx, bms_config_node)
    try:
        system = parse_config_to_system(bms_config)
    except RuntimeError as e:
        ctx.fatal(e)

    # at this point the json configuration file has been validated and we can
    # proceed to translate the information for the build system
    ctx.set_aerosol_sensor(system.application.aerosol_sensor)
    ctx.set_algorithm(system.application.algorithm)
    ctx.set_balancing_strategy(
        system.application.balancing_strategy, system.bms_slave.analog_front_end
    )
    ctx.set_current_sensor(system.application.current_sensor)
    ctx.set_debug(system.debug)
    ctx.set_imd_sensor(system.application.insulation_monitoring_device)
    ctx.set_rtos(system.rtos)
    ctx.set_bms_slave(system.bms_slave)


@conf
def set_aerosol_sensor(
    ctx: ConfigurationContext, aerosol_sensor: AerosolSensor
) -> None:
    """Set aerosol sensor details for the build tool chain"""
    if aerosol_sensor.manufacturer == "none":
        ctx.define("FOXBMS_HAVE_AEROSOL_SENSOR", "(0u)", quote=False)
    else:
        ctx.define("FOXBMS_HAVE_AEROSOL_SENSOR", "(1u)", quote=False)

    # Set support defines
    if ctx.is_aerosol_sensor_honeywell_bas6c_x00():
        ctx.define("FOXBMS_AS_HONEYWELL", "(1u)", quote=False)
        ctx.define("FOXBMS_AS_HONEYWELL_BAS6C_X00", "(1u)", quote=False)

    ctx.env.FOXBMS_AS_MANUFACTURER = aerosol_sensor.manufacturer
    ctx.env.FOXBMS_AS_MODEL = aerosol_sensor.model


@conf
def set_algorithm(ctx: ConfigurationContext, algorithm: Algorithm) -> None:
    """Set algorithm details for the build tool chain"""
    ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC = algorithm.state_estimation.soc
    ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE = algorithm.state_estimation.soe
    ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF = algorithm.state_estimation.sof
    ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH = algorithm.state_estimation.soh
    for algo, name in zip(
        (
            ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC,
            ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE,
            ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF,
            ctx.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH,
        ),
        (
            "soc",
            "soe",
            "sof",
            "soh",
        ),
        strict=True,
    ):
        ctx.define(
            f"FOXBMS_ALGORITHM_STATE_ESTIMATOR_{algo.upper()}_{name.upper()}",
            "(1u)",
            quote=False,
        )


@conf
def set_balancing_strategy(
    ctx: ConfigurationContext, balancing_strategy: str, afe: AnalogFrontEnd
) -> None:
    """Set balancing strategy details for the build tool chain"""
    ctx.env.FOXBMS_BALANCING_STRATEGY = balancing_strategy
    ctx.define(
        f"FOXBMS_BALANCING_STRATEGY_{ctx.env.FOXBMS_BALANCING_STRATEGY.upper()}",
        "(1u)",
        quote=False,
    )

    # no balancing strategy is independent of the hardware and therefore ok
    if ctx.env.FOXBMS_BALANCING_STRATEGY == "none":
        return

    # ltc 6806 (fuel cell monitoring ic) has no balancing support
    if afe.manufacturer == "ltc" and afe.ic == "6806":
        ctx.fatal(
            f"{afe.manufacturer.upper()} {afe.ic.upper()} does not support balancing."
        )


@conf
def set_current_sensor(
    ctx: ConfigurationContext, current_sensor: CurrentSensor
) -> None:
    """Set current sensor details for the build tool chain"""
    if current_sensor.type == "can":
        ctx.env.FOXBMS_CS_TYPE = "CAN"
        ctx.define("FOXBMS_CS_TYPE_CAN", "(1u)", quote=False)
    elif current_sensor.type == "bjb-ic":
        ctx.env.FOXBMS_CS_TYPE = "BJB_IC"
        ctx.define("FOXBMS_CS_TYPE_BJB_IC", "(1u)", quote=False)

    if current_sensor.manufacturer == "isabellenhuette":
        if current_sensor.model == "ivt-s":
            ctx.define("FOXBMS_CS_ISABELLENHUETTE_IVT_S", "(1u)", quote=False)
    if current_sensor.manufacturer == "lem":
        if current_sensor.model == "cab500":
            ctx.define("FOXBMS_CS_LEM_CAB500", "(1u)", quote=False)
    ctx.env.FOXBMS_CS_MANUFACTURER = str(current_sensor.manufacturer).lower()
    ctx.env.FOXBMS_CS_MODEL = str(current_sensor.model).lower()


@conf
def set_debug(ctx: ConfigurationContext, debug: Debug) -> None:
    """Set debug details for the build tool chain"""
    for interface in debug.interfaces:
        if interface == "uart":
            ctx.env.FOXBMS_UART_SUPPORT = 1
            ctx.define("FOXBMS_UART_SUPPORT", "(1)", quote=False)


@conf
def set_imd_sensor(
    ctx: ConfigurationContext, insulation_monitoring_device: InsulationMonitoringDevice
) -> None:
    """Set IMD sensor details for the build tool chain"""
    manufacturer = insulation_monitoring_device.manufacturer
    model = insulation_monitoring_device.model
    _type = insulation_monitoring_device.type
    if manufacturer == "none":
        ctx.define("FOXBMS_HAVE_INSULATION_MONITORING", "(0u)", quote=False)
    else:
        ctx.define("FOXBMS_HAVE_INSULATION_MONITORING", "(1u)", quote=False)

    ctx.define(f"FOXBMS_IMD_TYPE_{_type.upper()}", "(1u)", quote=False)
    if manufacturer == "bender":
        if model == "ir155":
            ctx.define("FOXBMS_IMD_BENDER_IR155", "(1u)", quote=False)
        if model == "iso165c":
            ctx.define("FOXBMS_IMD_BENDER_ISO165C", "(1u)", quote=False)
    elif manufacturer == "none":
        if model == "none":
            ctx.define("FOXBMS_IMD_NONE_NONE", "(1u)", quote=False)

    imd_base_path = "src/app/driver/imd"
    incs = f"{imd_base_path}/{manufacturer}"
    incs = f"{incs}/{model}/{manufacturer}_{model}.json"
    imd_details = read_json_from_node(ctx, incs)
    imd_includes = [
        f"{imd_base_path}/{manufacturer}/{model}/{i}" for i in imd_details["include"]
    ]
    for imd_include in imd_includes:
        if not os.path.isdir(imd_include):
            ctx.fatal(f"'{imd_include}' does not exist.")

    # ell entries in imd_includes are valid -> add them as include
    ctx.env.append_unique(
        "FOXBMS_INCLUDES_IMD",
        [ctx.path.find_node(i).abspath() for i in imd_includes],
    )
    ctx.env.FOXBMS_IMD_MANUFACTURER = manufacturer
    ctx.env.FOXBMS_IMD_MODEL = model
    ctx.env.FOXBMS_IMD_TYPE = _type


@conf
def set_rtos(ctx: ConfigurationContext, rtos: RTOS) -> None:
    """Set RTOS details for the build tool chain."""
    ctx.env.FOXBMS_RTOS_NAME = rtos.name
    ctx.define(f"FOXBMS_RTOS_{rtos.name.upper()}", "(1u)", quote=False)

    # root directory of the RTOS
    rtos_base_path = f"src/os/{rtos.name}"
    # the kernel is in a directory with the same name as the RTOS
    kernel_base_path = f"{rtos_base_path}/{rtos.name}"

    rtos_kernel_details = read_json_from_node(
        ctx, f"{kernel_base_path}/{rtos.name}_cfg.json"
    )

    rtos_kernel_includes = [
        f"{kernel_base_path}/{i}" for i in rtos_kernel_details["include"]
    ]
    ctx.env.append_unique(
        "FOXBMS_INCLUDES_RTOS_KERNEL",
        [ctx.path.find_node(i).abspath() for i in rtos_kernel_includes],
    )

    ctx.env.append_unique("FOXBMS_RTOS_ADDONS", rtos.addons)
    for addon in rtos.addons:
        addon_base_path = f"{rtos_base_path}"
        if rtos.name == "freertos":
            addon_base_path = f"{addon_base_path}/freertos-plus"
            if "tcp" in addon:
                ctx.env.FOXBMS_TCP_SUPPORT = 1
                ctx.define("FOXBMS_TCP_SUPPORT", "(1)", quote=False)
                if ctx.env.FOXBMS_UART_SUPPORT == 1:
                    # when using the TCP port and UART is defined, then we
                    # need to set this so that we can map the printf function
                    ctx.define("ipconfigHAS_DEBUG_PRINTF", 1)
        addon_base_path = f"{addon_base_path}/{addon}"
        rtos_addon_details = read_json_from_node(
            ctx, f"{addon_base_path}/{addon}_cfg.json"
        )
        rtos_addon_includes = [
            f"{addon_base_path}/{addon}" for addon in rtos_addon_details["include"]
        ]
        ctx.env.append_unique(
            "FOXBMS_INCLUDES_RTOS_KERNEL_ADDONS",
            [ctx.path.find_node(i).abspath() for i in rtos_addon_includes],
        )
    if not ctx.env.FOXBMS_TCP_SUPPORT:
        ctx.define("EMAC_TxInterruptServiceRoutine", "phantomInterrupt", quote=False)
        ctx.define("EMAC_RxInterruptServiceRoutine", "phantomInterrupt", quote=False)


@conf
def set_bms_slave(ctx: ConfigurationContext, bms_slave: BMSSlave) -> None:
    """Set BMS-Slave details for the build tool chain"""
    # vendor/ic includes and foxBMS specific driver adaptions
    afe_include_directory = bms_slave.analog_front_end.ic
    afe_driver_type = "fsm"
    afe_ic_manufacturer_define = "FOXBMS_AFE_DRIVER_" + to_define(
        bms_slave.analog_front_end.manufacturer
    )
    afe_ic_full_define = (
        "FOXBMS_AFE_DRIVER_"
        + to_define(bms_slave.analog_front_end.manufacturer)
        + "_"
        + to_define(bms_slave.analog_front_end.ic)
    )

    if bms_slave.analog_front_end.manufacturer == "ltc":
        if bms_slave.analog_front_end.ic in ("6804-1", "6811-1", "6812-1"):
            afe_include_directory = "6813-1"
    elif bms_slave.analog_front_end.manufacturer == "nxp":
        afe_driver_type = "no-fsm"
    elif bms_slave.analog_front_end.manufacturer == "adi":
        afe_driver_type = "no-fsm"
    elif bms_slave.analog_front_end.manufacturer == "debug":
        pass
    elif bms_slave.analog_front_end.manufacturer == "maxim":
        pass
    elif bms_slave.analog_front_end.manufacturer == "st":
        afe_driver_type = "no-fsm"
    elif bms_slave.analog_front_end.manufacturer == "ti":
        pass

    afe_driver_type_define = "FOXBMS_AFE_DRIVER_TYPE_" + to_define(afe_driver_type)

    ctx.env.FOXBMS_BMS_SLAVE_AFE_MANUFACTURER = bms_slave.analog_front_end.manufacturer
    ctx.env.FOXBMS_BMS_SLAVE_AFE_IC = bms_slave.analog_front_end.ic

    ctx.define(afe_ic_manufacturer_define, "(1u)", quote=False)
    ctx.define(afe_ic_full_define, "(1u)", quote=False)
    ctx.define(afe_driver_type_define, "(1u)", quote=False)

    # get AFE includes
    afe_base_path = "src/app/driver/afe"
    incs = (
        f"{afe_base_path}/"
        f"{bms_slave.analog_front_end.manufacturer}/"
        f"{afe_include_directory}/"
        f"{bms_slave.analog_front_end.manufacturer}_{afe_include_directory}.json"
    )
    afe_details = read_json_from_node(ctx, incs)
    afe_includes = [
        f"{afe_base_path}/{bms_slave.analog_front_end.manufacturer}/{afe_include_directory}/{i}"
        for i in afe_details["include"]
    ]
    for afe_include in afe_includes:
        if not os.path.isdir(afe_include):
            if afe_include.endswith("diag"):
                continue
            ctx.fatal(f"'{afe_include}' does not exist.")
    ctx.env.append_unique(
        "FOXBMS_INCLUDES_AFE",
        [
            ctx.path.find_node(i).abspath()
            for i in afe_includes
            if ctx.path.find_node(i)
        ],
    )
    # temperature sensor on Slave unit: bms.json:slave-unit:temperature-sensor
    ctx.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER = (
        bms_slave.temperature_sensor.manufacturer
    )
    ctx.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL = (
        bms_slave.temperature_sensor.model
    )
    ctx.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_METHOD = (
        bms_slave.temperature_sensor.method
    )


# shortcut functions used in the build scripts to determine e.g., what current
# sensor is configured


# Aerosol sensor test functions
@conf
def is_aerosol_sensor_honeywell_bas6c_x00(ctx: ConfigurationContext) -> bool:
    """Determine whether the aerosol sensor is Honeywell BAS6C-X00"""
    if (
        ctx.env.FOXBMS_AS_MANUFACTURER == "honeywell"
        and ctx.env.FOXBMS_AS_MODEL == "bas6c-x00"
    ):
        return True
    return False


# Current sensor test functions
@conf
def is_current_sensor_lem_cab500(ctx: ConfigurationContext) -> bool:
    """Determine whether the current sensor is LEM CAB500"""
    if ctx.env.FOXBMS_CS_MANUFACTURER == "lem" and ctx.env.FOXBMS_CS_MODEL == "cab500":
        return True
    return False


@conf
def is_current_sensor_isabellenhuette_ivt_s(ctx: ConfigurationContext) -> bool:
    """Determine whether the current sensor is Isabellenhuette IVT-S"""
    if (
        ctx.env.FOXBMS_CS_MANUFACTURER == "isabellenhuette"
        and ctx.env.FOXBMS_CS_MODEL == "ivt-s"
    ):
        return True
    return False


# AFE test functions
@conf
def is_bms_slave_debug_can(ctx: ConfigurationContext) -> bool:
    """Determine whether the BMS-Slave is Debug CAN"""
    if (
        ctx.env.FOXBMS_BMS_SLAVE_AFE_MANUFACTURER == "debug"
        and ctx.env.FOXBMS_BMS_SLAVE_AFE_IC == "can"
    ):
        return True
    return False


# IMD test functions
@conf
def is_imd_none(ctx: ConfigurationContext) -> bool:
    """Determine whether no IMD device shall be used"""
    if ctx.env.FOXBMS_IMD_MANUFACTURER == "none" and ctx.env.FOXBMS_IMD_MODEL == "none":
        return True
    return False


@conf
def is_imd_bender_iso165c(ctx: ConfigurationContext) -> bool:
    """Determine whether the IMD is Bender iso165c"""
    if (
        ctx.env.FOXBMS_IMD_MANUFACTURER == "bender"
        and ctx.env.FOXBMS_IMD_MODEL == "iso165c"
    ):
        return True
    return False


@conf
def is_imd_bender_ir155(ctx: ConfigurationContext) -> bool:
    """Determine whether the IMD is Bender IR155"""
    if (
        ctx.env.FOXBMS_IMD_MANUFACTURER == "bender"
        and ctx.env.FOXBMS_IMD_MODEL == "ir155"
    ):
        return True
    return False
