#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a waf tool to configure a VS Code workspace to foxBMS specific needs

For information on VS Code see https://code.visualstudio.com/.
"""

import os
import re
import pathlib
import json
import jsonschema

import jinja2

from waflib import Utils
from waflib import Context


def fix_jinja(txt):
    """appends empty line to file, if missing"""
    return (
        os.linesep.join([s for s in txt.splitlines() if not s.strip() == ""])
        + os.linesep
    )


def configure(conf):  # pylint: disable=too-many-statements,too-many-branches
    """configuration step of the VS Code waf tool:

    - Find code
    - configure a project if code was found"""
    # create a VS Code workspace if code is installed on this platform
    if Utils.is_win32:
        conf.find_program(
            "code",
            path_list=[
                os.path.join(
                    os.environ["LOCALAPPDATA"], "Programs", "Microsoft VS Code"
                ),
                os.path.join(os.environ["PROGRAMFILES"], "Microsoft VS Code"),
            ],
            mandatory=False,
        )
    else:
        conf.find_program("code", mandatory=False)

    if not conf.env.CODE:
        return
    conf.start_msg("Creating workspace")
    vscode_dir = conf.path.make_node(".vscode")
    vscode_dir.mkdir()
    vscode_config_dir = conf.path.find_dir(os.path.join("tools", "ide", "vscode"))
    template_loader = jinja2.FileSystemLoader(searchpath=vscode_config_dir.relpath())
    template_env = jinja2.Environment(loader=template_loader)
    if Utils.is_win32:
        git_bin_dir = os.path.join("C:", os.path.sep, "Program Files", "Git", "bin")
        conf.find_program("bash", path_list=[git_bin_dir], mandatory=False)
        if conf.options.vscshell == "bash" and conf.env.BASH:
            vscshell_exe = conf.env.BASH[0].replace("\\", "\\\\")
            waf_wrapper_script = (
                pathlib.Path(conf.path.abspath()).as_posix() + "/waf.sh"
            )
        else:
            waf_wrapper_script = "${workspaceFolder}\\waf".replace("\\", "\\\\")
            vscshell_exe = os.path.join(
                os.environ.get(
                    "windir", default=os.path.join("C:", os.path.sep, "Windows")
                ),
                "Sysnative",
                "cmd.exe",
            ).replace("\\", "\\\\")
    else:
        waf_wrapper_script = "${workspaceFolder}/waf"
        conf.find_program("bash")
        vscshell_exe = conf.env.BASH[0]
    template = template_env.get_template("tasks.json.jinja2")
    tasks = template.render(
        WAF_WRAPPER_SCRIPT=waf_wrapper_script,
    )
    vsc_tasks_file = os.path.join(vscode_dir.relpath(), "tasks.json")
    conf.path.make_node(vsc_tasks_file).write(fix_jinja(tasks))

    template = template_env.get_template("extensions.json.jinja2")
    extensions = template.render()
    vsc_extensions_file = os.path.join(vscode_dir.relpath(), "extensions.json")
    conf.path.make_node(vsc_extensions_file).write(fix_jinja(extensions))

    template = template_env.get_template("cspell.json.jinja2")
    cspell = template.render()
    vsc_cspell_file = os.path.join(vscode_dir.relpath(), "cspell.json")
    conf.path.make_node(vsc_cspell_file).write(fix_jinja(cspell))

    template = template_env.get_template("settings.json.jinja2")
    # Python and friends: Python, conda, pylint, black
    py_exe = "python"
    if conf.env.PYTHON:
        py_exe = conf.env.PYTHON[0].replace("\\", "\\\\")
    conda_exe = "conda"
    if conf.env.CONDA:
        conda_exe = conf.env.CONDA[0].replace("\\", "\\\\")
    pylint_exe = "pylint"
    if conf.env.PYLINT:
        pylint_exe = conf.env.PYLINT[0].replace("\\", "\\\\")
    pylint_cfg = ""
    if conf.env.PYLINT_CONFIG:
        pylint_cfg = conf.env.PYLINT_CONFIG[0].replace("\\", "\\\\")
    black_exe = "black"
    if conf.env.BLACK:
        black_exe = conf.env.BLACK[0].replace("\\", "\\\\")
    black_cfg = ""
    if conf.env.BLACK_CONFIG:
        black_cfg = conf.env.BLACK_CONFIG[0].replace("\\", "\\\\")
    # directory of waf and waf-tools
    waf_dir = Context.waf_dir.replace("\\", "\\\\")
    waf_tools_dir = "${workspaceFolder}\\tools\\waf-tools".replace("\\", "\\\\")
    # Clang-format
    clang_format_executable = ""
    if conf.env.CLANG_FORMAT:
        clang_format_executable = conf.env.CLANG_FORMAT[0].replace("\\", "\\\\")
    # now it is in an case save to render the template
    if not conf.env.CLANG_FORMAT[0]:
        clang_format_executable = ""
    else:
        clang_format_executable = conf.env.CLANG_FORMAT[0].replace("\\", "\\\\")
    settings = template.render(
        PROJECT_SHELL=vscshell_exe,
        PYTHONPATH=py_exe,
        WAF_DIR=waf_dir,
        WAF_TOOLS_DIR=waf_tools_dir,
        CONDA_PATH=conda_exe,
        PYLINT_PATH=pylint_exe,
        PYLINT_CONFIG=pylint_cfg,
        BLACKPATH=black_exe,
        BLACK_CONFIG=black_cfg,
        CLANG_FORMAT_EXECUTABLE=clang_format_executable,
    )

    vsc_settings_file = os.path.join(vscode_dir.relpath(), "settings.json")
    conf.path.make_node(vsc_settings_file).write(fix_jinja(settings))

    template = template_env.get_template("c_cpp_properties.json.jinja2")
    defines_read = (
        conf.root.find_node(conf.env.COMPILER_BUILTIN_DEFINES_FILE[0])
        .read()
        .splitlines()
    )
    vscode_defines = []
    reg = re.compile(r"(#define)([ ])([a-zA-Z0-9_]{1,})([ ])([a-zA-Z0-9_\":. ]{1,})")
    for d in defines_read:
        define = d.split("/*")[0]
        _def = reg.search(define)
        if _def:
            def_name, val = _def.group(3), _def.group(5)
            if def_name in ("__DATE__", "__TIME__"):
                continue
            if '"' in val:
                val = val.replace('"', '\\"')
            vscode_defines.append((def_name, val))

    bms_config = json.loads(
        conf.path.find_node(os.path.join("conf", "bms", "bms.json")).read()
    )
    bms_config_schema = json.loads(
        conf.path.find_node(
            os.path.join("conf", "bms", "schema", "bms.schema.json")
        ).read()
    )
    try:
        jsonschema.validate(instance=bms_config, schema=bms_config_schema)
    except jsonschema.exceptions.ValidationError as err:
        good_values = ", ".join([f"'{i}'" for i in err.validator_value])
        conf.fatal(
            f"Measurement IC '{err.instance}' is not supported. Use one of "
            f"these: {good_values}."
        )
    bal = bms_config["slave-unit"]["balancing-strategy"]
    soc = bms_config["application"]["algorithm"]["state-estimation"]["soc"]
    soe = bms_config["application"]["algorithm"]["state-estimation"]["soe"]
    soh = bms_config["application"]["algorithm"]["state-estimation"]["soh"]

    imd = bms_config["application"]["insulation-monitoring-device"]
    imd_manufacturer = imd["manufacturer"]
    imd_model = imd["model"]

    chip = bms_config["slave-unit"]["measurement-ic"]["chip"]
    if chip in ("6804-1", "6811-1", "6812-1"):
        chip = "6813-1"
    c_cpp_properties = template.render(
        ARMCL=conf.env.CC[0].replace("\\", "\\\\"),
        OS=bms_config["operating-system"]["name"],
        BALANCING_STRATEGY=bal,
        MEASUREMENT_IC_MANUFACTURER=bms_config["slave-unit"]["measurement-ic"][
            "manufacturer"
        ],
        MEASUREMENT_IC_CHIP=chip,
        TEMPERATURE_SENSOR_MANUFACTURER=bms_config["slave-unit"]["temperature-sensor"][
            "manufacturer"
        ],
        TEMPERATURE_SENSOR_MODEL=bms_config["slave-unit"]["temperature-sensor"][
            "model"
        ],
        TEMPERATURE_SENSOR_METHOD=bms_config["slave-unit"]["temperature-sensor"][
            "method"
        ],
        STATE_ESTIMATOR_SOC=soc,
        STATE_ESTIMATOR_SOE=soe,
        STATE_ESTIMATOR_SOH=soh,
        IMD_MANUFACTURER=imd_manufacturer,
        IMD_MODEL=imd_model,
        INCLUDES=[x.replace("\\", "\\\\") for x in conf.env.INCLUDES],
        CSTANDARD="c99",
        DEFINES=vscode_defines,
    )
    vsc_c_cpp_properties_file = os.path.join(
        vscode_dir.relpath(), "c_cpp_properties.json"
    )
    for i in conf.env.VSCODE_MK_DIRS:
        conf.path.make_node(i).mkdir()
    conf.path.make_node(vsc_c_cpp_properties_file).write(fix_jinja(c_cpp_properties))

    template = template_env.get_template("launch.json.jinja2")
    gdb_exe = "gdb"
    if conf.env.GDB:
        gdb_exe = conf.env.GDB[0].replace("\\", "\\\\")
    launch = template.render(GDB=gdb_exe)

    vsc_launch_file = os.path.join(vscode_dir.relpath(), "launch.json")
    conf.path.make_node(vsc_launch_file).write(fix_jinja(launch))

    conf.end_msg("ok")
