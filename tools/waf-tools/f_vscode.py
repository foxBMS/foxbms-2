#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a waf tool to configure a VS Code workspace to foxBMS specific
needs.

For information on VS Code see https://code.visualstudio.com/.
"""

# pylint: disable=too-many-locals,too-many-statements

import os
import copy
import re
import json
import shutil
from pathlib import Path

from waflib import Context, Utils, Logs
from waflib.Node import Node
from waflib.Configure import ConfigurationContext, conf

if Utils.is_win32:
    BAUHAUS_DIR = (
        Path(os.environ.get("ProgramFiles(x86)", "C:\\Program Files(x86)")) / "Bauhaus"
    )
    FOX_WRAPPER_EXT = "bat"
else:
    BAUHAUS_DIR = Path(os.environ.get("HOME", "/")) / "bauhaus-suite"
    FOX_WRAPPER_EXT = "sh"


def dump_json_to_node(node: Node, cfg: dict):
    """Dump dictionary to node"""
    Path(node.abspath()).write_text(
        json.dumps(cfg, indent=2, sort_keys=False), encoding="utf-8"
    )


def get_axivion_modules(ax_modules_rel: Path, cafecc: str = "") -> list[str]:
    """Prepares the Axivion Modules path"""
    axivion_modules = []
    if BAUHAUS_DIR.is_dir():
        axivion_modules.append((BAUHAUS_DIR / ax_modules_rel).as_posix())
    if cafecc:
        axivion_modules.append((Path(cafecc).parent.parent / ax_modules_rel).as_posix())
    return axivion_modules


def default_includes(
    ctx: ConfigurationContext, base_dir: Node, inc_file: str
) -> list[str]:
    """Get include lines from a file"""
    inc_node = base_dir.find_node(inc_file)

    if not inc_node:
        ctx.fatal(f"Could not find '{os.path.join(base_dir.abspath(), inc_file)}'.")
    txt: str = inc_node.read(encoding="utf-8")
    return txt.splitlines()


def get_vscode_relevant_defines(compiler_builtin_defines: list[str]) -> list[str]:
    """Get all compiler builtin defines formatted for VS Code"""
    reg = re.compile(r"(#define)([ ])([a-zA-Z0-9_]{1,})([ ])([a-zA-Z0-9_\":. ]{1,})")
    vscode_defines = []
    for d in compiler_builtin_defines:
        define = d.split("/*")[0]
        _def = reg.search(define)
        if _def:
            def_name, val = _def.group(3), _def.group(5)
            if def_name in (
                "__DATE__",
                "__TIME__",
                "__EDG_VERSION__",
                "__edg_front_end__",
                "__EDG_SIZE_TYPE__",
                "__EDG_PTRDIFF_TYPE__",
            ):
                continue
            vscode_defines.append(f"{def_name}={val}")
    return vscode_defines


def get_hcg_includes(halcogen: list) -> list[str]:
    """get HALCoGen includes"""
    try:
        return [
            Path(
                os.path.join(
                    Path(halcogen[0]).parent.parent.parent,
                    "F021 Flash API",
                    "02.01.01",
                    "include",
                )
            ).as_posix()
        ]
    except IndexError:
        return []


@conf
def get_fox_py_wrapper_executable(ctx: ConfigurationContext) -> bool:
    """check which fox.py-wrapper to use (.bat, .ps1 or .sh)"""
    ctx.start_msg("Checking for 'fox.py' wrapper:")
    if Utils.is_win32:
        for i in ("bat", "ps1", "sh"):
            ctx.find_program("fox", var="FOX_WRAPPER", mandatory=False, exts=f".{i}")
            if ctx.env.FOX_WRAPPER:
                ctx.env.FOX_WRAPPER = [str(ctx.path.find_node(ctx.env.FOX_WRAPPER[0]))]
                cmd = [ctx.env.FOX_WRAPPER[0], "-h"]
                with Utils.subprocess.Popen(
                    cmd,
                    cwd=ctx.path.abspath(),
                    stderr=Utils.subprocess.PIPE,
                    stdout=Utils.subprocess.PIPE,
                ) as p:
                    p.communicate()
                if not p.returncode:
                    break  # we have successfully set 'ctx.env.FOX_WRAPPER'
        # fallback to 'find_node'
        ctx.env.append_unique("FOX_WRAPPER", str(ctx.path.find_node("fox.bat")))
    else:
        ctx.find_program("fox.sh", var="FOX_WRAPPER")
        if ctx.env.FOX_WRAPPER:
            ctx.env.FOX_WRAPPER = [str(ctx.path.find_node(ctx.env.FOX_WRAPPER[0]))]
            cmd = [ctx.env.FOX_WRAPPER[0], "-h"]
            with Utils.subprocess.Popen(cmd, cwd=ctx.path.abspath()) as p:
                p.communicate()
            if p.returncode:  # we could **NOT** find a working fox wrapper
                ctx.env.FOX_WRAPPER = ""

        # fallback to 'find_node'
        if not ctx.env.FOX_WRAPPER:
            ctx.env.append_unique("FOX_WRAPPER", str(ctx.path.find_node("fox.sh")))

    ctx.end_msg(ctx.env.get_flat("FOX_WRAPPER"))


@conf
def find_vscode(ctx: ConfigurationContext) -> bool:
    """Find VS Code"""
    is_remote_session = False
    ctx.start_msg("Checking for program 'code'")
    if Utils.is_win32:
        ctx.find_program("code", mandatory=False)
        if not ctx.env.CODE:
            code_dir = "Microsoft VS Code"
            path_list = [
                os.path.join(os.environ["LOCALAPPDATA"], "Programs", code_dir),
                os.path.join(os.environ["PROGRAMFILES"], code_dir),
            ]
            ctx.find_program(
                "code",
                path_list=path_list,
                mandatory=False,
            )
    else:
        ctx.find_program("code", mandatory=False)
        if not ctx.env.CODE:
            # we might be in a remote environment, scan for this
            code_server_dir = os.path.join(os.path.expanduser("~"), ".vscode-server")
            is_remote_session = os.path.isdir(code_server_dir)
            ctx.msg("Found 'vscode-server' (remote session)", code_server_dir)

    if not (ctx.env.CODE or is_remote_session):
        ctx.end_msg(False)
        return False

    ctx.end_msg(ctx.env.get_flat("CODE") or "remote")
    return True


@conf
def valid_configuration_files(ctx: ConfigurationContext, base_cfg_dir: Node):
    """Validate, that all VS configuration files are valid json files"""
    err = 0
    for i in base_cfg_dir.ant_glob("**/*.json"):
        try:
            i.read_json()
        except json.decoder.JSONDecodeError:
            err += 1
            Logs.error(f"'{i}' is not a valid json file.")

    if err:
        ctx.fatal("Invalid configuration files provided.")


@conf
def setup_generic(ctx: ConfigurationContext, base_cfg_dir: Node):
    """Setup the generic VS Code configuration"""
    # Setup requires:
    # - copy cspell.json verbatim
    # - copy c_cpp_properties.json and adapt paths
    # - copy settings.json and adapt paths
    # - copy tasks.json and adapt paths

    # First copy everything from the configuration directory to the actual
    # required location, so that in later steps the setup can be adapted and
    # then write the configuration to the specific configuration file.

    ctx.start_msg("Creating generic workspace")
    vscode_dir = ctx.path.make_node(".vscode")
    vscode_dir.mkdir()

    shutil.copy2(base_cfg_dir.find_node("cspell.json").abspath(), vscode_dir.abspath())
    for i in base_cfg_dir.ant_glob("generic/*.json"):
        shutil.copy2(i.abspath(), vscode_dir.abspath())

    ### c_cpp_properties.json
    c_cpp_properties_node = vscode_dir.find_node("c_cpp_properties.json")
    if not c_cpp_properties_node:
        ctx.fatal(f"Could not find 'c_cpp_properties.json' in {vscode_dir}")
    c_cpp_properties = c_cpp_properties_node.read_json()

    compiler_builtin_defines: list[str] = (
        ctx.root.find_node(ctx.env.COMPILER_BUILTIN_DEFINES_FILE[0])
        .read(encoding="utf-8")
        .splitlines()
    )

    # ctx.env.DEFINES: we need a deepcopy of these defines otherwise it alters
    # the defines passed to the compiler at build-time!
    vscode_defines = get_vscode_relevant_defines(
        compiler_builtin_defines
    ) + copy.deepcopy(ctx.env.DEFINES)

    win32_config_index = 0
    inc_path_halcogen = get_hcg_includes(ctx.env.HALCOGEN)
    c_cpp_properties["configurations"][win32_config_index]["includePath"].extend(
        [(Path(ctx.env.CC[0]).parent.parent / "include").as_posix()] + inc_path_halcogen
    )
    c_cpp_properties["configurations"][win32_config_index]["browse"]["path"].extend(
        [(Path(ctx.env.CC[0]).parent.parent / "include").as_posix()] + inc_path_halcogen
    )
    c_cpp_properties["configurations"][win32_config_index]["defines"] = vscode_defines

    include_path = default_includes(
        ctx, base_cfg_dir, "generic/g-project-include-path.txt"
    )
    inc_base = "@@ROOT@@/src"
    inc_app = f"{inc_base}/app"
    inc_state = f"{inc_app}/application/algorithm/state_estimation"
    include_path.extend(
        [
            f"{inc_state}/soc/{ctx.env.state_estimator_soc}",
            f"{inc_state}/soe/{ctx.env.state_estimator_soe}",
            f"{inc_state}/sof/{ctx.env.state_estimator_sof}",
            f"{inc_state}/soh/{ctx.env.state_estimator_soh}",
            f"{inc_app}/driver/imd/{ctx.env.imd_manufacturer}",
            f"{inc_app}/task/ftask/{ctx.env.RTOS_NAME[0]}",
            f"{inc_app}/task/os/{ctx.env.RTOS_NAME[0]}",
            f"{inc_base}/os/{ctx.env.RTOS_NAME[0]}",
            f"{inc_app}/application/bal/{ctx.env.balancing_strategy}",
            (
                f"{inc_app}/driver/ts/{ctx.env.temperature_sensor_manuf}/"
                f"{ctx.env.temperature_sensor_model}/{ctx.env.temperature_sensor_meth}"
            ),
        ]
    )
    p = []
    for i in ctx.env.INCLUDES_RTOS + ctx.env.INCLUDES_AFE:
        k = Path(ctx.root.find_node(i).path_from(ctx.path)).as_posix()
        p.append(k)
    p = [f"@@ROOT@@/{i}" for i in p]
    include_path.extend(p)
    c_cpp_properties["env"]["ProjectIncludePath"] = [
        Path(i.replace("@@ROOT@@", ctx.path.abspath())).as_posix()
        for i in sorted(include_path)
    ]

    for i in c_cpp_properties["configurations"]:
        if i["name"] == "Win32":
            # use GCC as dummy compiler
            i["compilerPath"] = Path(str(ctx.env.GCC[0])).as_posix()
    dump_json_to_node(c_cpp_properties_node, c_cpp_properties)

    ### settings.json
    settings_node = vscode_dir.find_node("settings.json")
    if not settings_node:
        ctx.fatal(f"Could not find 'settings.json' in {vscode_dir}")
    settings = settings_node.read_json()

    ### settings.json: python.*
    waf_dir = Path(Context.waf_dir).as_posix()
    waf_tools_dir = Path(
        os.path.join(ctx.path.abspath(), "tools", "waf-tools")
    ).as_posix()

    # now clean up the remaining configuration options in the template
    settings["python.analysis.extraPaths"] = [waf_dir, waf_tools_dir]
    settings["pylint.args"] = [
        f"--rcfile={Path(ctx.path.abspath()).as_posix()}/pyproject.toml"
    ]

    ### settings.json: files.*
    settings["files.exclude"] = {
        **settings["files.exclude"],
        **{
            ".vscode/**": True,
            "hal/**": True,
            "opt/**": True,
        },
    }

    dump_json_to_node(settings_node, settings)

    ### tasks.json
    tasks_node = vscode_dir.find_node("tasks.json")
    if not tasks_node:
        ctx.fatal(f"Could not find 'tasks.json' in {vscode_dir}")
    tasks = tasks_node.read_json()
    for i in tasks["tasks"]:
        i["command"] = Path(ctx.env.FOX_WRAPPER[0]).as_posix()
        i["options"]["cwd"] = Path(ctx.path.abspath()).as_posix()
        for p in i["problemMatcher"]:
            if isinstance(p, dict):
                p["fileLocation"] = ["autoDetect", Path(ctx.path.abspath()).as_posix()]
    dump_json_to_node(tasks_node, tasks)

    ctx.end_msg(vscode_dir)


@conf
def setup_src(ctx: ConfigurationContext, base_cfg_dir: Node):
    """Setup the src VS Code configuration"""
    # Setup requires:
    # - copy cspell.json verbatim
    # - copy c_cpp_properties.json and adapt paths
    # - copy settings.json and adapt paths
    # - copy tasks.json and adapt paths

    # First copy everything from the configuration directory to the actual
    # required location, so that in later steps the setup can be adapted and
    # then write the configuration to the specific configuration file.

    ctx.start_msg("Creating target workspace")
    vscode_dir = ctx.path.make_node("src/.vscode")
    vscode_dir.mkdir()

    shutil.copy2(base_cfg_dir.find_node("cspell.json").abspath(), vscode_dir.abspath())
    for i in base_cfg_dir.ant_glob("src/*.json"):
        shutil.copy2(i.abspath(), vscode_dir.abspath())

    ### c_cpp_properties.json
    c_cpp_properties_node = vscode_dir.find_node("c_cpp_properties.json")
    if not c_cpp_properties_node:
        ctx.fatal(f"Could not find 'c_cpp_properties.json' in {vscode_dir}")
    c_cpp_properties = c_cpp_properties_node.read_json()

    compiler_builtin_defines: list[str] = (
        ctx.root.find_node(ctx.env.COMPILER_BUILTIN_DEFINES_FILE[0])
        .read(encoding="utf-8")
        .splitlines()
    )

    # ctx.env.DEFINES: we need a deepcopy of these defines otherwise it alters
    # the defines passed to the compiler at build-time!
    vscode_defines = get_vscode_relevant_defines(
        compiler_builtin_defines
    ) + copy.deepcopy(ctx.env.DEFINES)

    win32_config_index = 0
    inc_path_halcogen = get_hcg_includes(ctx.env.HALCOGEN)
    c_cpp_properties["configurations"][win32_config_index]["includePath"].extend(
        [(Path(ctx.env.CC[0]).parent.parent / "include").as_posix()] + inc_path_halcogen
    )
    c_cpp_properties["configurations"][win32_config_index]["browse"]["path"].extend(
        [(Path(ctx.env.CC[0]).parent.parent / "include").as_posix()] + inc_path_halcogen
    )
    c_cpp_properties["configurations"][win32_config_index]["defines"] = vscode_defines

    include_path = default_includes(ctx, base_cfg_dir, "src/s-project-include-path.txt")

    inc_base = "@@ROOT@@/src"
    inc_app = f"{inc_base}/app"
    inc_state = f"{inc_app}/application/algorithm/state_estimation"
    include_path.extend(
        [
            f"{inc_state}/soc/{ctx.env.state_estimator_soc}",
            f"{inc_state}/soe/{ctx.env.state_estimator_soe}",
            f"{inc_state}/sof/{ctx.env.state_estimator_sof}",
            f"{inc_state}/soh/{ctx.env.state_estimator_soh}",
            f"{inc_app}/driver/imd/{ctx.env.imd_manufacturer}",
            f"{inc_app}/task/ftask/{ctx.env.RTOS_NAME[0]}",
            f"{inc_app}/task/os/{ctx.env.RTOS_NAME[0]}",
            f"{inc_base}/os/{ctx.env.RTOS_NAME[0]}",
            f"{inc_app}/application/bal/{ctx.env.balancing_strategy}",
            (
                f"{inc_app}/driver/ts/{ctx.env.temperature_sensor_manuf}/"
                f"{ctx.env.temperature_sensor_model}/{ctx.env.temperature_sensor_meth}"
            ),
        ]
    )
    p = []
    for i in ctx.env.INCLUDES_RTOS + ctx.env.INCLUDES_AFE:
        k = Path(ctx.root.find_node(i).path_from(ctx.path)).as_posix()
        p.append(k)
    p = [f"@@ROOT@@/{i}" for i in p]
    include_path.extend(p)
    c_cpp_properties["env"]["ProjectIncludePath"] = [
        Path(i.replace("@@ROOT@@", ctx.path.abspath())).as_posix()
        for i in sorted(include_path)
    ]

    for i in c_cpp_properties["configurations"]:
        if i["name"] == "Win32":
            # use GCC as dummy compiler
            i["compilerPath"] = Path(str(ctx.env.GCC[0])).as_posix()
    dump_json_to_node(c_cpp_properties_node, c_cpp_properties)

    ### settings.json
    settings_node = vscode_dir.find_node("settings.json")
    if not settings_node:
        ctx.fatal(f"Could not find 'settings.json' in {vscode_dir}")
    settings = settings_node.read_json()

    ### settings.json: python.*
    waf_dir = Path(Context.waf_dir).as_posix()
    waf_tools_dir = Path(
        os.path.join(ctx.path.abspath(), "tools", "waf-tools")
    ).as_posix()

    # now clean up the remaining configuration options in the template
    settings["python.analysis.extraPaths"] = [waf_dir, waf_tools_dir]
    settings["pylint.args"] = [
        f"--rcfile={Path(ctx.path.abspath()).as_posix()}/pyproject.toml"
    ]

    ### settings.json: files.*
    settings["files.exclude"] = {
        **settings["files.exclude"],
        **{
            ".vscode/**": True,
            "hal/**": True,
            "opt/**": True,
        },
    }

    dump_json_to_node(settings_node, settings)

    ### tasks.json
    tasks_node = vscode_dir.find_node("tasks.json")
    if not tasks_node:
        ctx.fatal(f"Could not find 'tasks.json' in {vscode_dir}")
    tasks = tasks_node.read_json()
    for i in tasks["tasks"]:
        i["command"] = Path(ctx.env.FOX_WRAPPER[0]).as_posix()
        i["options"]["cwd"] = Path(ctx.path.abspath()).as_posix()
        for p in i["problemMatcher"]:
            p["fileLocation"] = ["autoDetect", Path(ctx.path.abspath()).as_posix()]
    dump_json_to_node(tasks_node, tasks)

    ctx.end_msg(vscode_dir)


@conf
def setup_embedded_unit_tests(ctx: ConfigurationContext, base_cfg_dir: Node):
    """Setup the embedded unit test VS Code configuration"""
    # Setup requires:
    # - copy cspell.json verbatim
    # - copy c_cpp_properties.json and adapt paths
    # - copy launch.json and adapt paths
    # - copy settings.json and adapt paths
    # - copy tasks.json and adapt paths

    # First copy everything from the configuration directory to the actual
    # required location, so that in later steps the setup can be adapted and
    # then write the configuration to the specific configuration file.

    ctx.start_msg("Creating embedded unit tests workspace")
    vscode_dir = ctx.path.make_node("tests/unit/.vscode")
    vscode_dir.mkdir()

    shutil.copy2(base_cfg_dir.find_node("cspell.json").abspath(), vscode_dir.abspath())
    for i in base_cfg_dir.ant_glob("embedded-tests/*.json"):
        shutil.copy2(i.abspath(), vscode_dir.abspath())

    ### c_cpp_properties.json
    c_cpp_properties_node = vscode_dir.find_node("c_cpp_properties.json")
    if not c_cpp_properties_node:
        ctx.fatal(f"Could not find 'c_cpp_properties.json' in {vscode_dir}")
    c_cpp_properties = c_cpp_properties_node.read_json()

    # all directories in <root>/src
    include_dirs = ctx.path.ant_glob("src/**", src=False, dir=True, excl=["**/.vscode"])
    c_cpp_properties["env"]["ProjectIncludePath"] = [
        Path(i.abspath()).as_posix() for i in include_dirs
    ]

    # add some testing specific paths
    for i in default_includes(
        ctx, base_cfg_dir, "embedded-tests/e-project-include-path.txt"
    ):
        c_cpp_properties["env"]["ProjectIncludePath"].append(
            Path(i.replace("@@ROOT@@", ctx.path.abspath())).as_posix()
        )
    c_cpp_properties["env"]["ProjectIncludePath"] = sorted(
        c_cpp_properties["env"]["ProjectIncludePath"]
    )
    for i in c_cpp_properties["configurations"]:
        if i["name"] == "Win32":
            i["compilerPath"] = Path(str(ctx.env.GCC[0])).as_posix()
    dump_json_to_node(c_cpp_properties_node, c_cpp_properties)

    ### launch.json
    launch_node = vscode_dir.find_node("launch.json")
    if not launch_node:
        ctx.fatal(f"Could not find 'launch.json' in {vscode_dir}")
    launch = launch_node.read_json()
    if ctx.env.GDB:
        for i in launch["configurations"]:
            i["miDebuggerPath"] = Path(str(ctx.env.GDB[0])).as_posix()
            i["cwd"] = Path(ctx.path.abspath()).as_posix()
            i["program"] = (
                Path(ctx.path.abspath())
                / "build/unit_test/test/out/${fileBasenameNoExtension}/"
                "${fileBasenameNoExtension}.out"
            ).as_posix()

    dump_json_to_node(launch_node, launch)

    ### settings.json
    settings_node = vscode_dir.find_node("settings.json")
    if not settings_node:
        ctx.fatal(f"Could not find 'settings.json' in {vscode_dir}")
    settings = settings_node.read_json()

    ### settings.json: python.*
    waf_dir = Path(Context.waf_dir).as_posix()
    waf_tools_dir = Path(
        os.path.join(ctx.path.abspath(), "tools", "waf-tools")
    ).as_posix()

    # now clean up the remaining configuration options in the template
    settings["python.analysis.extraPaths"] = [waf_dir, waf_tools_dir]
    settings["pylint.args"] = [
        f"--rcfile={Path(ctx.path.abspath()).as_posix()}/pyproject.toml"
    ]

    ### settings.json: files.*
    settings["files.exclude"] = {
        **settings["files.exclude"],
        **{
            "**/build/**": True,
            "**/.lock-waf_*_build": True,
            ".vscode/**": True,
            "axivion/**": True,
            "gen_hcg/**": True,
        },
    }

    dump_json_to_node(settings_node, settings)

    ### tasks.json
    tasks_node = vscode_dir.find_node("tasks.json")
    if not tasks_node:
        ctx.fatal(f"Could not find 'tasks.json' in {vscode_dir}")
    tasks = tasks_node.read_json()
    for i in tasks["tasks"]:
        i["command"] = Path(ctx.env.FOX_WRAPPER[0]).as_posix()
        i["options"]["cwd"] = Path(ctx.path.abspath()).as_posix()

    dump_json_to_node(tasks_node, tasks)

    ctx.end_msg(vscode_dir)


def configure(ctx: ConfigurationContext):  # pylint: disable=too-many-branches
    """configuration step of the VS Code waf tool:

    - Find code
    - configure a project if code was found"""
    # create a VS Code workspace if code is installed on this platform
    if not ctx.find_vscode():
        return

    ctx.get_fox_py_wrapper_executable()

    # We have found 'code', check that the configuration files are valid
    base_cfg_dir = ctx.path.find_dir(os.path.join("tools", "ide", "vscode"))
    ctx.valid_configuration_files(base_cfg_dir)

    # # configure the workspaces
    ctx.setup_generic(base_cfg_dir)
    ctx.setup_src(base_cfg_dir)
    ctx.setup_embedded_unit_tests(base_cfg_dir)
