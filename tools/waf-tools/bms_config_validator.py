#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

from waflib import Logs
from waflib.Configure import ConfigurationContext, conf

CONF_OPTIONS = {
    "application": {
        "algorithm": {
            "state-estimation": {
                "soc": ["counting", "debug", "none"],
                "soe": ["counting", "debug", "none"],
                "sof": ["trapezoid"],
                "soh": ["debug", "none"],
            },
        },
        "balancing-strategy": ["voltage", "history", "none"],
        "insulation-monitoring-device": {
            "bender": ["ir155", "iso165c"],
            "none": [""],
        },
    },
    "rtos": {
        "freertos": {
            "addons": [
                "freertos-plus-tcp",
            ],
        },
        "safertos": {
            "addons": [],
        },
    },
    "slave-unit": {
        "analog-front-end": {
            "adi": ["ades1830"],
            "debug": ["default", "can"],
            "ltc": ["6804-1", "6806", "6811-1", "6812-1", "6813-1"],
            "maxim": ["max17852"],
            "nxp": ["mc33775a"],
            "ti": ["dummy"],
        },
        "temperature-sensor": {
            "epcos": {
                "b57251v5103j060": ["polynomial", "lookup-table"],
                "b57861s0103f045": ["polynomial", "lookup-table"],
            },
            "fake": {
                "none": ["polynomial", "lookup-table"],
            },
            "murata": {
                "ncxxxxh103": ["polynomial", "lookup-table"],
            },
            "semitec": {
                "103jt": ["polynomial", "lookup-table"],
            },
            "tdk": {
                "ntcgs103jf103ft8": ["lookup-table"],
                "ntcg163jx103dt1s": ["lookup-table"],
            },
            "vishay": {
                "ntcalug01a103g": ["polynomial", "lookup-table"],
                "ntcle317e4103sba": ["polynomial", "lookup-table"],
            },
        },
    },
}


def validate_rtos(rtos_config: dict | None) -> int:
    """Validate BMS RTOS configuration"""
    if not rtos_config:
        Logs.error("Key 'rtos' is missing.")
        return 1
    err = 0
    os_name = rtos_config.get("name", "")
    if not os_name:
        Logs.error("Key 'rtos:↳name' is missing.")
        return 1
    opt = [i[0] for i in list(CONF_OPTIONS["rtos"].items())]
    if os_name not in opt:
        err += 1
        err_cfg = "'rtos:↳name'"
        Logs.error(f"{err_cfg} needs to be one of {opt}.")
    addons = rtos_config.get("addons", [])
    if not addons:
        return err
    supported_addons = CONF_OPTIONS["rtos"][os_name]["addons"]
    for addon in addons:
        if addon not in supported_addons:
            err += 1
            err_cfg = f"'rtos:↳{os_name}:↳<addon-name>'"
            Logs.error(f"{err_cfg} needs to be one of {supported_addons}.")
    return err


def validate_application(bms_config: dict) -> int:
    """Validate BMS application configuration"""
    err = 0
    for algo in ("soc", "soe", "sof", "soh"):
        cfg = bms_config["application"]["algorithm"]["state-estimation"][algo]
        opt = CONF_OPTIONS["application"]["algorithm"]["state-estimation"][algo]
        if cfg not in opt:
            err += 1
            err_cfg = f"application:↳algorithm:↳state-estimation:↳{algo}"
            Logs.error(f"{err_cfg} needs to be one of {opt}.")

    cfg = bms_config["application"]["balancing-strategy"]
    opt = CONF_OPTIONS["application"]["balancing-strategy"]
    if cfg not in opt:
        err += 1
        err_cfg = "application:↳balancing-strategy"
        Logs.error(f"{err_cfg} needs to be one of {opt}.")

    cfg = bms_config["application"]["insulation-monitoring-device"]
    opt = CONF_OPTIONS["application"]["insulation-monitoring-device"]
    if cfg["manufacturer"] not in opt:
        err += 1
        err_cfg = "application:↳insulation-monitoring-device:↳manufacturer"
        Logs.error(f"{err_cfg} needs to be one of {list(opt.keys())}.")

    if cfg["model"] not in opt[cfg["manufacturer"]]:
        print(cfg["model"])
        err += 1
        err_cfg = "application:↳insulation-monitoring-device:↳model"
        Logs.error(f"{err_cfg} needs to be one of {opt[cfg['manufacturer']]}.")
    return err


def validate_bms_slave(bms_config: dict) -> int:
    """Validate BMS RTOS configuration"""
    err = 0
    # AFE
    cfg = bms_config["slave-unit"]["analog-front-end"]
    opt = CONF_OPTIONS["slave-unit"]["analog-front-end"]
    if cfg["manufacturer"] not in opt:
        err += 1
        err_cfg = "'slave-unit:↳analog-front-end:↳manufacturer'"
        Logs.error(f"{err_cfg} needs to be one of {list(opt.keys())}.")

    if cfg["ic"] not in opt[cfg["manufacturer"]]:
        err += 1
        err_cfg = "'slave-unit:↳analog-front-end:↳ic'"
        Logs.error(f"{err_cfg} needs to be one of {opt[cfg['manufacturer']]}.")

    # Temperature sensor
    cfg = bms_config["slave-unit"]["temperature-sensor"]
    opt = CONF_OPTIONS["slave-unit"]["temperature-sensor"]
    if cfg["manufacturer"] not in opt:
        err += 1
        err_cfg = "'slave-unit:↳temperature-sensor:↳manufacturer'"
        Logs.error(f"{err_cfg} needs to be one of {list(opt.keys())}.")

    if cfg["model"] not in opt[cfg["manufacturer"]]:
        err += 1
        err_cfg = "'slave-unit:↳temperature-sensor:↳model'"
        Logs.error(
            f"{err_cfg} needs to be one of {list(opt[cfg['manufacturer']].keys())}."
        )

    if cfg["method"] not in opt[cfg["manufacturer"]][cfg["model"]]:
        err += 1
        err_cfg = "'slave-unit:↳temperature-sensor:↳method'"
        Logs.error(
            f"{err_cfg} needs to be one of {opt[cfg['manufacturer']][cfg['model']]}."
        )
    return err


@conf
def validate_bms_configuration(ctx: ConfigurationContext, bms_config: dict) -> None:
    """Validate BMS configuration files"""
    err = validate_rtos(bms_config.get("rtos"))
    err += validate_application(bms_config)
    err += validate_bms_slave(bms_config)

    if err:
        ctx.fatal("Configuration error.")
