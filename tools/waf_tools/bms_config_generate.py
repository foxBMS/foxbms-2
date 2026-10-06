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

"""Generate foxBMS configuration headers and environment flags.

This waf tool generates C header files containing ``#define`` directives
derived from the validated BMS system configuration (``bms.json``). It also
sets the corresponding waf environment variables so that build steps can
conditionally include source files and apply compiler flags based on the
active hardware and software configuration.

The tool covers the following configuration domains:
    - **Aerosol sensor** - Manufacturer/model detection and support defines.
    - **Algorithm** - State estimation method selection (SOC, SOE, SOF, SOH).
    - **Balancing strategy** - Active balancing algorithm selection.
    - **Current sensor** - Manufacturer, model, and communication type.
    - **IMD sensor** - Insulation monitoring device selection and type.
    - **Redundancy** - Redundant measurement path enable/disable.
    - **Debug interfaces** - UART support toggle.
    - **RTOS** - Real-time OS selection and addon configuration (e.g. TCP).
    - **BMS-Slave** - Analog front-end IC, driver type, and temperature sensor.
    - **MCU** - Cache enable/disable.

Additionally, the module provides ``@conf``-decorated helper functions that
allow build scripts to query the active sensor/device configuration
(e.g. :func:`is_current_sensor_lem_cab500`, :func:`is_imd_bender_iso165c`).

Usage
-----
1. Validate the configuration and set environment variables::

    bms_config_node = bld.srcnode.find_node(bld.env.BMS_CONFIG)
    system = bms_config_validate.validate_bms_configuration(bld, bms_config_node)
    bms_config_generate.set_env_variables(bld, system)

2. Declare task generators with the ``foxbms_config`` feature for each
   configuration domain::

    contents = [
            "aerosol_sensor",
            "algorithm",
            "balancing_strategy",
            "current_sensor",
            "imd_sensor",
            "redundant_v_t_measurement",
            "debug",
            "rtos",
            "bms_slave",
        ]
        for content in contents:
            bld(
                features="foxbms_config",
                bms_config_node=bms_config_node,
                system=system,
                content=content,
                target=content,
            )

   Each task generator produces a ``foxbms_config_<target>.h`` header file
   in the build directory.

Dependencies
------------
- :mod:`waf_tools.bms_config_model` - Typed dataclass model for the system configuration.
- :mod:`waf_tools.bms_config_validate` - Validation entry point for ``bms.json``.
"""

from collections.abc import Callable
from typing import Literal

from bms_config_model import System
from misc_helpers import map_python_bool_to_c_bool, to_define
from waflib import Errors, Utils
from waflib.Build import BuildContext
from waflib.Configure import ConfigurationContext, conf
from waflib.Task import Task
from waflib.TaskGen import after_method, feature, task_gen


class CreateFoxbmsConfig(Task):
    """Waf task that generates a foxBMS configuration header file.

    For each configured content domain (e.g. ``"rtos"``, ``"current_sensor"``),
    this task invokes the corresponding function from ``CONFIG_FUNCTIONS``
    to collect ``#define`` entries, then renders them into a C header file
    with an include guard.

    Attributes:
        system: The validated :class:`waf_tools.bms_config_model.System`
            instance providing all configuration values.
        content: List of configuration domain keys to include in this header
            (e.g., ``["rtos"]``, ``["current_sensor"]``).
    """

    ext_out = [".h"]
    color = "GREY"
    system: System
    content: list[str]

    def run(self) -> None:
        """Generate the configuration header from the defined content.

        Iterates over :attr:`content`, calls the matching function from
        ``CONFIG_FUNCTIONS`` for each entry, collects all resulting define
        dictionaries, and writes the rendered header to the output node.

        Raises ``WafError`` if an unknown content key is encountered.
        """
        defines: list[dict] = []
        for c in self.content:
            function = CONFIG_FUNCTIONS.get(c)
            if function is None:
                msg = f"Unknown config content: '{c}'"
                raise Errors.WafError(msg)
            defines.extend(function(self.generator.bld, self.system))
        header_content = self.generate_header(defines)
        self.outputs[0].write(header_content, encoding="utf-8")

    def generate_header(self, defines: list[dict]) -> str:
        """Render a list of define dictionaries into a complete C header string.

        Produces a header with an include guard derived from the output file
        name and one ``#define`` directive per entry.

        Args:
            defines: List of dictionaries, each containing ``"name"`` and
                ``"value"`` keys representing a single preprocessor define.

        Returns:
            The full header file content as a string.
        """
        guard = f"{to_define(self.outputs[0].name)}_"
        lines = []
        lines.append(f"#ifndef {guard}")
        lines.append(f"#define {guard}")
        lines.append("")
        lines.extend(f"#define {d['name']} {d['value']}" for d in defines)
        lines.append("")
        lines.append(f"#endif /* {guard} */")
        lines.append("")
        return "\n".join(lines)

    def keyword(self) -> Literal["Creating"]:  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        """Return the generated output path."""
        return str(self.outputs[0].path_from(self.generator.bld.path))


@feature("foxbms_config")
@after_method("process_rule")
def create_foxbms_config(self: task_gen) -> None:
    """Process the ``foxbms_config`` feature on a task generator.

    Creates a :class:`CreateFoxbmsConfig` task that reads the BMS
    configuration JSON node and produces the corresponding
    ``foxbms_config_<target>.h`` header file in the build directory.

    Required task generator attributes:
        - **bms_config_node (Node)**: The JSON configuration source node
          (``bms.json``).
        - **system** (:class:`waf_tools.bms_config_model.System`): The validated
          system configuration instance.
        - **content (str | list[str])**: One or more configuration domain
          keys (must exist in ``CONFIG_FUNCTIONS``).
        - **target (str)**: Base name for the output header file.
    """
    out = self.bld.bldnode.find_or_declare(f"foxbms_config_{self.target}.h")
    task = self.create_task("CreateFoxbmsConfig", self.bms_config_node, out)
    task.system = self.system
    task.content = Utils.to_list(self.content)


def set_env_variables(bld: BuildContext, system: System) -> None:
    """Set all build environment variables based on the system configuration.

    Delegates to the domain-specific ``set_*`` functions for each
    configuration area. After this call, the waf environment (``bld.env``)
    contains all ``FOXBMS_*`` variables required by build steps
    (e.g. source file selection, VS Code workspace generation).

    Args:
        bld: The active waf build context
        system: The validated system configuration.
    """
    # app
    set_aerosol_sensor(bld, system)
    set_algorithm(bld, system)
    set_balancing_strategy(bld, system)
    set_current_sensor(bld, system)
    set_imd_sensor(bld, system)
    set_redundant_v_t_measurement(bld, system)
    # debug
    set_debug(bld, system)
    # RTOS
    set_rtos(bld, system)
    # BMS-Slave
    set_bms_slave(bld, system)


def set_aerosol_sensor(bld: BuildContext, system: System) -> list[dict]:
    """Configure aerosol sensor defines and environment variables.

    Sets ``FOXBMS_HAVE_AEROSOL_SENSOR`` to ``(1u)`` if a sensor is
    configured, otherwise ``(0u)``. Additionally emits manufacturer- and
    model-specific defines and populates
    ``bld.env.FOXBMS_AS_MANUFACTURER`` / ``FOXBMS_AS_MODEL``.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    aerosol_sensor = system.application.aerosol_sensor
    defines: list[dict] = []
    value = "(0u)" if aerosol_sensor.manufacturer == "none" else "(1u)"
    defines.append({"name": "FOXBMS_HAVE_AEROSOL_SENSOR", "value": value})

    # Set support defines
    if bld.is_aerosol_sensor_honeywell_bas6c_x00():
        defines.append({"name": "FOXBMS_AS_HONEYWELL", "value": "(1u)"})
        defines.append({"name": "FOXBMS_AS_HONEYWELL_BAS6C_X00", "value": "(1u)"})

    bld.env.FOXBMS_AS_MANUFACTURER = aerosol_sensor.manufacturer
    bld.env.FOXBMS_AS_MODEL = aerosol_sensor.model
    return defines


def set_algorithm(bld: BuildContext, system: System) -> list[dict]:
    """Configure state estimation algorithm defines and environment variables.

    Sets ``FOXBMS_ALGORITHM_STATE_ESTIMATOR_<ALGO>_<DIRECTION>`` defines
    for each state estimation method (SOC, SOE, SOF, SOH) and populates the
    corresponding environment variables used to resolve source file paths.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    algorithm = system.application.algorithm
    defines: list[dict] = []
    bld.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC = algorithm.state_estimation.soc
    bld.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE = algorithm.state_estimation.soe
    bld.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF = algorithm.state_estimation.sof
    bld.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH = algorithm.state_estimation.soh
    for name in ("soc", "soe", "sof", "soh"):
        algo = bld.env[f"FOXBMS_ALGORITHM_STATE_ESTIMATOR_{name.upper()}"]
        defines.append(
            {
                "name": (
                    f"FOXBMS_ALGORITHM_STATE_ESTIMATOR_"
                    f"{algo.upper().replace('-', '_')}_{name.upper()}"
                ),
                "value": "(1u)",
            }
        )
    return defines


def set_balancing_strategy(bld: BuildContext, system: System) -> list[dict]:
    """Configure balancing strategy defines and environment variables.

    Sets ``FOXBMS_BALANCING_STRATEGY_<NAME>`` and validates that the
    selected strategy is compatible with the configured AFE.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.

    Raises:
        SystemExit: Indirectly via ``bld.fatal()`` if the AFE does not
            support the selected balancing strategy.
    """
    balancing_strategy = system.application.balancing_strategy
    bld.env.FOXBMS_BALANCING_STRATEGY = balancing_strategy
    defines: list[dict] = []
    defines.append(
        {
            "name": f"FOXBMS_BALANCING_STRATEGY_{bld.env.FOXBMS_BALANCING_STRATEGY.upper()}",
            "value": "(1u)",
        }
    )

    # no balancing strategy is independent of the hardware and therefore ok
    if bld.env.FOXBMS_BALANCING_STRATEGY == "none":
        return defines

    # ltc 6806 (fuel cell monitoring ic) has no balancing support
    afe = system.bms_slave.analog_front_end
    if afe.manufacturer == "ltc" and afe.ic == "6806":
        bld.fatal(
            f"{afe.manufacturer.upper()} {afe.ic.upper()} does not support balancing."
        )
    return defines


def set_current_sensor(bld: BuildContext, system: System) -> list[dict]:
    """Configure current sensor defines and environment variables.

    Emits defines for the communication type (``FOXBMS_CS_TYPE_CAN`` or
    ``FOXBMS_CS_TYPE_BJB_IC``) and manufacturer/model-specific defines.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    current_sensor = system.application.current_sensor
    defines: list[dict] = []
    if current_sensor.type == "can":
        bld.env.FOXBMS_CS_TYPE = "CAN"
        defines.append({"name": "FOXBMS_CS_TYPE_CAN", "value": "(1u)"})
    elif current_sensor.type == "bjb-ic":
        bld.env.FOXBMS_CS_TYPE = "BJB_IC"
        defines.append({"name": "FOXBMS_CS_TYPE_BJB_IC", "value": "(1u)"})

    if current_sensor.manufacturer == "isabellenhuette":
        if current_sensor.model == "ivt-s":
            defines.append({"name": "FOXBMS_CS_ISABELLENHUETTE_IVT_S", "value": "(1u)"})
    if current_sensor.manufacturer == "lem":
        if current_sensor.model == "cab500":
            defines.append({"name": "FOXBMS_CS_LEM_CAB500", "value": "(1u)"})
    bld.env.FOXBMS_CS_MANUFACTURER = str(current_sensor.manufacturer).lower()
    bld.env.FOXBMS_CS_MODEL = str(current_sensor.model).lower()
    return defines


def set_imd_sensor(bld: BuildContext, system: System) -> list[dict]:
    """Configure insulation monitoring device defines and environment variables.

    Sets ``FOXBMS_HAVE_INSULATION_MONITORING`` to ``(1u)`` or ``(0u)``
    depending on whether an IMD is configured. Additionally emits type-specific
    and device-specific defines.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    insulation_monitoring_device = system.application.insulation_monitoring_device
    manufacturer = insulation_monitoring_device.manufacturer
    model = insulation_monitoring_device.model
    _type = insulation_monitoring_device.type
    defines: list[dict] = []
    value = "(0u)" if manufacturer == "none" else "(1u)"
    defines.append({"name": "FOXBMS_HAVE_INSULATION_MONITORING", "value": value})
    defines.append({"name": f"FOXBMS_IMD_TYPE_{_type.upper()}", "value": "(1u)"})

    if manufacturer == "bender":
        if model == "ir155":
            defines.append({"name": "FOXBMS_IMD_BENDER_IR155", "value": "(1u)"})
        if model == "iso165c":
            defines.append({"name": "FOXBMS_IMD_BENDER_ISO165C", "value": "(1u)"})
    elif manufacturer == "none":
        if model == "none":
            defines.append({"name": "FOXBMS_IMD_NONE_NONE", "value": "(1u)"})

    bld.env.FOXBMS_IMD_MANUFACTURER = manufacturer
    bld.env.FOXBMS_IMD_MODEL = model
    bld.env.FOXBMS_IMD_TYPE = _type
    return defines


def set_redundant_v_t_measurement(bld: BuildContext, system: System) -> list[dict]:
    """Configure redundancy defines and environment variables.

    Sets ``FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT`` to ``(1u)`` or ``(0u)`` and
    ``bld.env.FOXBMS_REDUNDANT_V_T_MEASUREMENT`` to ``1`` or ``0``.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    redundant_v_t_measurement = system.application.redundant_v_t_measurement
    defines: list[dict] = []
    if redundant_v_t_measurement:
        defines.append(
            {"name": "FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT", "value": "(1u)"}
        )
        bld.env.FOXBMS_REDUNDANT_V_T_MEASUREMENT = 1
    else:
        defines.append(
            {"name": "FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT", "value": "(0u)"}
        )
        bld.env.FOXBMS_REDUNDANT_V_T_MEASUREMENT = 0
    return defines


def set_debug(bld: BuildContext, system: System) -> list[dict]:
    """Configure debug interface defines and environment variables.

    Iterates over the configured debug interfaces and emits the
    corresponding support defines. Sets ``bld.env.FOXBMS_UART_SUPPORT`` to
    ``1`` if UART is enabled.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    debug = system.debug
    defines: list[dict] = []
    for interface in debug.interfaces:
        if interface == "uart":
            bld.env.FOXBMS_UART_SUPPORT = 1
            defines.append({"name": "FOXBMS_UART_SUPPORT", "value": "(1)"})
    return defines


def set_rtos(bld: BuildContext, system: System) -> list[dict]:
    """Configure RTOS defines and environment variables.

    Sets ``FOXBMS_RTOS_<NAME>`` and processes ``RTOS`` addons (e.g.
    ``freertos-plus-tcp``). When TCP support is enabled, emits
    ``FOXBMS_TCP_SUPPORT`` and conditionally ``ipconfigHAS_DEBUG_PRINTF`` if
    UART is also active.

    When TCP is not used, phantom interrupt defines are emitted for the EMAC
    `InterruptServiceRoutines`.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    rtos = system.rtos
    defines: list[dict] = []
    bld.env.FOXBMS_RTOS_NAME = rtos.name
    defines.append({"name": f"FOXBMS_RTOS_{rtos.name.upper()}", "value": "(1u)"})

    # root directory of the RTOS
    rtos_base_path = f"src/os/{rtos.name}"

    bld.env.append_unique("FOXBMS_RTOS_ADDONS", rtos.addons)
    for addon in rtos.addons:
        addon_base_path = f"{rtos_base_path}"
        if rtos.name == "freertos":
            addon_base_path = f"{addon_base_path}/freertos-plus"
            if "tcp" in addon:
                bld.env.FOXBMS_TCP_SUPPORT = 1
                defines.append({"name": "FOXBMS_TCP_SUPPORT", "value": "(1)"})
                if bld.env.FOXBMS_UART_SUPPORT == 1:
                    # when using the TCP port and UART is defined, then we
                    # need to set this so that we can map the printf function
                    defines.append({"name": "ipconfigHAS_DEBUG_PRINTF", "value": 1})
        addon_base_path = f"{addon_base_path}/{addon}"
    if not bld.env.FOXBMS_TCP_SUPPORT:
        defines.append(
            {"name": "EMAC_TxInterruptServiceRoutine", "value": "phantomInterrupt"}
        )
        defines.append(
            {"name": "EMAC_RxInterruptServiceRoutine", "value": "phantomInterrupt"}
        )
    return defines


def set_bms_slave(bld: BuildContext, system: System) -> list[dict]:
    """Configure BMS-Slave defines and environment variables.

    Emits defines for the analog front-end manufacturer, IC, and driver
    type. Populates environment variables for the AFE
    (``FOXBMS_BMS_SLAVE_AFE_MANUFACTURER``, ``FOXBMS_BMS_SLAVE_AFE_IC``)
    and the temperature sensor (manufacturer, model, method).

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of define dictionaries for header generation.
    """
    bms_slave = system.bms_slave
    # vendor/ic includes and foxBMS specific driver adaptions
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

    if bms_slave.analog_front_end.manufacturer in {"nxp", "adi", "st"}:
        afe_driver_type = "no-fsm"

    afe_driver_type_define = "FOXBMS_AFE_DRIVER_TYPE_" + to_define(afe_driver_type)

    bld.env.FOXBMS_BMS_SLAVE_AFE_MANUFACTURER = bms_slave.analog_front_end.manufacturer
    bld.env.FOXBMS_BMS_SLAVE_AFE_IC = bms_slave.analog_front_end.ic

    defines: list[dict] = []
    defines.append({"name": afe_ic_manufacturer_define, "value": "(1u)"})
    defines.append({"name": afe_ic_full_define, "value": "(1u)"})
    defines.append({"name": afe_driver_type_define, "value": "(1u)"})

    # temperature sensor on Slave unit: bms.json:slave-unit:temperature-sensor
    bld.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER = (
        bms_slave.temperature_sensor.manufacturer
    )
    bld.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL = (
        bms_slave.temperature_sensor.model
    )
    bld.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_METHOD = (
        bms_slave.temperature_sensor.method
    )
    return defines


def set_mcu(bld: BuildContext, system: System) -> list[dict]:
    """Configure MCU defines and environment variables.

    Sets ``FOXBMS_MCU_USE_CACHE`` to ``(true)`` or ``(false)`` depending on whether
    the cache is enabled in the configuration.

    Args:
        bld: The active waf build context.
        system: The validated system configuration.

    Returns:
        A list of preprocessor defines for the MCU configuration.
    """
    bld.env.FOXBMS_MCU_USE_CACHE = system.mcu.use_cache
    defines: list[dict] = []
    defines.append(
        {
            "name": "FOXBMS_MCU_USE_CACHE",
            "value": f"({map_python_bool_to_c_bool(bld.env.FOXBMS_MCU_USE_CACHE)})",
        }
    )
    return defines


CONFIG_FUNCTIONS: dict[str, Callable[[BuildContext, System], list[dict]]] = {
    "aerosol_sensor": set_aerosol_sensor,
    "algorithm": set_algorithm,
    "balancing_strategy": set_balancing_strategy,
    "current_sensor": set_current_sensor,
    "imd_sensor": set_imd_sensor,
    "redundant_v_t_measurement": set_redundant_v_t_measurement,
    "debug": set_debug,
    "rtos": set_rtos,
    "bms_slave": set_bms_slave,
    "mcu": set_mcu,
}


# shortcut functions used in the build scripts to determine e.g., what current
# sensor is configured


# Aerosol sensor test functions
@conf
def is_aerosol_sensor_honeywell_bas6c_x00(ctx: ConfigurationContext) -> bool:
    """Determine whether the aerosol sensor is Honeywell BAS6C-X00."""
    return (
        ctx.env.FOXBMS_AS_MANUFACTURER == "honeywell"
        and ctx.env.FOXBMS_AS_MODEL == "bas6c-x00"
    )


# Current sensor test functions
@conf
def is_current_sensor_lem_cab500(ctx: ConfigurationContext) -> bool:
    """Determine whether the current sensor is LEM CAB500."""
    return (
        ctx.env.FOXBMS_CS_MANUFACTURER == "lem" and ctx.env.FOXBMS_CS_MODEL == "cab500"
    )


@conf
def is_current_sensor_isabellenhuette_ivt_s(ctx: ConfigurationContext) -> bool:
    """Determine whether the current sensor is Isabellenhuette IVT-S."""
    return (
        ctx.env.FOXBMS_CS_MANUFACTURER == "isabellenhuette"
        and ctx.env.FOXBMS_CS_MODEL == "ivt-s"
    )


# AFE test functions
@conf
def is_bms_slave_debug_can(ctx: ConfigurationContext) -> bool:
    """Determine whether the BMS-Slave is Debug CAN."""
    return (
        ctx.env.FOXBMS_BMS_SLAVE_AFE_MANUFACTURER == "debug"
        and ctx.env.FOXBMS_BMS_SLAVE_AFE_IC == "can"
    )


# IMD test functions
@conf
def is_imd_none(ctx: ConfigurationContext) -> bool:
    """Determine whether no IMD device shall be used."""
    return (
        ctx.env.FOXBMS_IMD_MANUFACTURER == "none" and ctx.env.FOXBMS_IMD_MODEL == "none"
    )


@conf
def is_imd_bender_iso165c(ctx: ConfigurationContext) -> bool:
    """Determine whether the IMD is Bender iso165c."""
    return (
        ctx.env.FOXBMS_IMD_MANUFACTURER == "bender"
        and ctx.env.FOXBMS_IMD_MODEL == "iso165c"
    )


@conf
def is_imd_bender_ir155(ctx: ConfigurationContext) -> bool:
    """Determine whether the IMD is Bender IR155."""
    return (
        ctx.env.FOXBMS_IMD_MANUFACTURER == "bender"
        and ctx.env.FOXBMS_IMD_MODEL == "ir155"
    )
