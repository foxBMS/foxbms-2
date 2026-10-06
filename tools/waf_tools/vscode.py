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

# cspell:ignore noprofile,norc,rcfile,testbuild,tgen

"""VS Code workspace generation tool for the Waf build system.

This waf tool automates the generation and configuration of Visual Studio Code
workspace files (https://code.visualstudio.com/) for the foxBMS project.
It produces IDE configuration files (``c_cpp_properties.json``,
``settings.json``, ``tasks.json``, ``launch.json``) tailored to the active
build variant so that IntelliSense, build tasks, and debugging
work out of the box.

The tool supports multiple workspace variants, each targeting a different
build configuration:

- **generic** - General-purpose workspace for the foxBMS repository.
- **cli** - Command-line-focused workspace.
- **app_ti_arm_cgt** - Embedded application compiled with TI ARM CGT.
- **bootloader_ti_arm_cgt** - Bootloader compiled with TI ARM CGT.
- **app_unit_test_gcc** - Application unit tests compiled with GCC.
- **bootloader_unit_test_gcc** - Bootloader unit tests compiled with GCC.

For each variant the tool resolves compiler built-in defines, include
paths (including dynamic paths derived from the project configuration),
and ``use`` dependency chains, then merges this information into the
corresponding VS Code JSON configuration files. Template JSON files are
read from ``tools/ide/vscode/<target>/`` and patched in place.

Usage
-----
1. Load the tool during ``configure``::

    def configure(ctx):
        ctx.load("vscode", tooldir=TOOLDIR)

   The ``configure`` step locates the ``code`` executable, determines the
   platform-specific shell wrapper (``pwsh`` on Windows, ``bash`` on Linux),
   and finds ``gcc``/``gdb`` for IntelliSense and debugging.

2. Run the ``vscode`` command to generate all workspaces at once::

    waf vscode

   Or generate individual workspaces::

    waf vscode_app_ti_arm_cgt
    waf vscode_app_unit_test_gcc

3. Declare a task generator with the ``vscode`` feature in a build script::

    def build(bld):
        bld(
            features="vscode",
            target=<VS Code workspace variant>,
            vscode_dir=<Path to VS Code entry point as Node>,
            environment=<Waf build environment>,
            use_compiler_defines=<True/False>,
            use_dynamic_includes=<True/False>,
            include_dirs=<include paths>,   # e.g. ["src/app", "src/os"]
            glob_patterns=<include glob patterns>,   # e.g. ["src/app/**/"]
            glob_patterns_excl=<exclude glob patterns>,
            use_compiler_includes=<True/False>,
            pylint=<True/False>,
            python_extra_paths=<True/False>,
            files_exclude={<glob_pattern>: <True/False>}   # e.g. {".vscode/**": True}
            build_tasks=<True/False>,
            defines=<extra defines>,
        )

Dependencies
------------
- VS Code (``code`` executable, optional)
- GCC (used as dummy ``compilerPath`` for IntelliSense)
- GDB (used for debug configurations in ``launch.json``)
- Platform shell: ``pwsh`` (Windows) or ``bash`` (Linux)
- TI ARM CGT or GCC compiler configured in the respective build environment
  (for compiler built-in defines extraction)
"""

import json
import os
import re
import shutil
from pathlib import Path

from waflib import Context, Logs, Options, Utils
from waflib.Build import BuildContext
from waflib.Configure import ConfigurationContext, conf
from waflib.Errors import WafError
from waflib.Node import Node
from waflib.TaskGen import before_method, feature, task_gen

COMMON_WAF_BASE_DIR = "waf3-2.1.9-beba77c244731800bf15a003232e7040"

VSCODE_ALLOWED_VARIANTS = (
    "vscode",
    "app_ti_arm_cgt",
    "bootloader_ti_arm_cgt",
    "app_unit_test_gcc",
    "bootloader_unit_test_gcc",
)


def valid_configuration_files(bld: BuildContext, base_cfg_dir: Node) -> None:
    """Validate that all VS Code configuration files are valid JSON.

    Iterates over all ``.json`` files in ``base_cfg_dir`` and attempts to
    parse them. If any file contains invalid JSON, an error is logged and
    the build is aborted.

    Args:
        bld: The active build context.
        base_cfg_dir: The directory node containing the JSON template files
                      (typically ``tools/ide/vscode/``).
    """
    err = 0
    for i in base_cfg_dir.ant_glob("**/*.json"):
        try:
            i.read_json()
        except json.decoder.JSONDecodeError:
            err += 1
            Logs.error(f"'{i}' is not a valid json file.")

    if err:
        bld.fatal("Invalid configuration files provided.")


def dump_json_to_node(node: Node, cfg: dict) -> None:
    """Serialize a dictionary as pretty-printed JSON and write it to a node.

    Args:
        node: The target file node to write
        cfg: The dictionary to serialize
    """
    Path(node.abspath()).write_text(
        json.dumps(cfg, indent=2, sort_keys=False), encoding="utf-8"
    )


def get_compiler_builtin_defines(tg: task_gen) -> list[str]:
    """Extract compiler built-in defines for the active toolchain.

    For TI ARM CGT (``armcl``), reads the ``.ppm`` file generated during the
    configure check step. For GCC, invokes the compiler with ``-dM -E -``
    to dump predefined macros.

    Args:
        tg: The task generator whose ``env`` determines the active compiler.

    Returns:
        A list of strings, each representing one ``#define`` line from the
        compiler's built-in defines.
    """
    if tg.env.CC_NAME == "armcl":
        file_path = "conf_check_1e952e18e8928208363b1f9d35afc947/testbuild/test.c.1.ppm"
        node = tg.bld.srcnode.find_node(
            f"build/{file_path}"
        ) or tg.bld.srcnode.find_node(f"build/.{file_path}")
        if not node:
            tg.bld.fatal("Could not find compiler builtin defines file.")
        return node.read(encoding="utf-8").splitlines()
    if tg.env.CC_NAME == "gcc":
        cmd = tg.bld.env.CC + tg.bld.env.CFLAGS + ["-dM", "-E", "-"]
        output = tg.bld.cmd_and_log(cmd, quiet=0)
        return output.splitlines()
    return []


def get_vscode_relevant_defines(compiler_builtin_defines: list[str]) -> list[str]:
    """Convert raw ``#define`` lines into VS Code IntelliSense format.

    Parses each define line with a regex, filters out date/time and
    EDG-internal defines that would cause IntelliSense issues, and returns the
    remaining defines in ``NAME=VALUE`` format.

    Args:
        compiler_builtin_defines: Raw ``#define`` lines as returned by
                                  :func:`get_compiler_builtin_defines`.

    Returns:
        A list of strings in ``NAME=VALUE`` format suitable for the ``defines``
        field in ``c_cpp_properties.json``.
    """
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
    """Derive the TI F021 Flash API include path from the HALCoGen installation.

    Args:
        halcogen: The ``HALCOGEN`` environment variable (list with the HALCoGen
                  executable path as first element).

    Returns:
        A list containing the POSIX-formatted Flash API include path.
    """
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


def get_use_includes(bld: BuildContext, tg: task_gen) -> list[str]:
    """Recursively resolve include paths from ``use`` dependencies.

    Walks the dependency chain defined by the ``use`` attribute on the task
    generator, posts each dependency, and collects all ``export_includes``
    paths.

    Args:
        bld: The active build context.
        tg: The task generator whose ``use`` list is resolved.

    Returns:
        A list of absolute include paths (strings) collected from all
        transitive ``use`` dependencies.
    """
    paths = []
    seen = set()

    def resolve(use_tg: task_gen) -> None:
        """Recursively collect exported include paths from a task generator."""
        if use_tg.name in seen:
            return
        seen.add(use_tg)
        use_tg.post()

        recursed_use = Utils.to_list(getattr(use_tg, "use", []))
        for rec_use in recursed_use:
            try:
                rec_tg = bld.get_tgen_by_name(rec_use)
                resolve(rec_tg)
            except WafError:
                pass

        export_incs = Utils.to_list(getattr(use_tg, "export_includes", []))
        paths.extend(inc.abspath() for inc in export_incs if inc)

    use_list = Utils.to_list(getattr(tg, "use", []))
    for use in use_list:
        try:
            use_tg = bld.get_tgen_by_name(use)
            resolve(use_tg)
        except WafError:
            pass
    return paths


def get_dynamic_include_paths(bld: BuildContext, tg: task_gen) -> list[str]:
    """Assemble dynamic include paths based on the project configuration.

    Constructs include paths for configurable subsystems (SOC/SOE/SOF/SOH
    estimators, IMD driver, RTOS variant, balancing strategy, temperature
    sensor, UART/TCP support) by reading the corresponding environment
    variables from the task generator's ``env``.

    Additionally appends include paths resolved from ``use`` dependencies
    via :func:`get_use_includes`.

    Args:
        bld: The active build context.
        tg: The task generator whose ``env`` provides the configuration
            variables (e.g. ``FOXBMS_RTOS_NAME``, ``FOXBMS_IMD_MANUFACTURER``).

    Returns:
        A list of absolute include path strings derived from the current
        project configuration.
    """
    inc_base = f"{bld.srcnode.abspath()}/src"
    inc_app = f"{inc_base}/app"
    inc_state = f"{inc_app}/application/algorithm/state_estimation"

    paths = [
        f"{inc_state}/soc/{tg.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC}",
        f"{inc_state}/soe/{tg.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE}",
        f"{inc_state}/sof/{tg.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF}",
        f"{inc_state}/soh/{tg.env.FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH}",
        f"{inc_app}/driver/imd/{tg.env.FOXBMS_IMD_MANUFACTURER}",
        f"{inc_app}/task/ftask/{tg.env.FOXBMS_RTOS_NAME}",
        f"{inc_app}/task/os/{tg.env.FOXBMS_RTOS_NAME}",
        f"{inc_app}/application/bal/{tg.env.FOXBMS_BALANCING_STRATEGY}",
        (
            f"{inc_app}/driver/ts/{tg.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER}/"
            f"{tg.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL}/"
            f"{tg.env.FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_METHOD}"
        ),
    ]

    if tg.env.FOXBMS_UART_SUPPORT:
        paths.append(f"{inc_app}/driver/uart")
    if tg.env.FOXBMS_TCP_SUPPORT:
        paths.extend(
            [
                f"{inc_app}/application/ethernet",
                f"{inc_app}/driver/phy",
                f"{inc_app}/driver/emac",
            ]
        )
    paths.append(f"{inc_base}/os/{tg.env.FOXBMS_RTOS_NAME}/{tg.env.FOXBMS_RTOS_NAME}")
    paths.extend(get_use_includes(bld, tg))
    return paths


def build_c_cpp_properties(bld: BuildContext, vscode_dir: Node, tg: task_gen) -> None:
    """Patch ``c_cpp_properties.json`` with project-specific defines and includes.

    Reads the template ``c_cpp_properties.json`` from ``vscode_dir``, then:

    - Populates the ``defines`` list with compiler built-in defines (if
      ``use_compiler_defines`` is set) and any extra defines from the task
      generator.
    - Resolves include paths from ``include_dirs``, ``glob_patterns`` and
      dynamic project paths (if ``use_dynamic_includes`` is set).
    - Appends TI ARM CGT compiler and HALCoGen include paths (if
      ``use_compiler_includes`` is set).
    - Sets GCC as the dummy ``compilerPath`` for IntelliSense.

    Args:
        bld: The active build context.
        vscode_dir: The target ``.vscode`` directory node.
        tg: The task generator providing configuration attributes.
    """
    node = vscode_dir.find_node("c_cpp_properties.json")
    if not node:
        return
    properties = node.read_json()

    win32_idx = 0
    win32_cfg = properties["configurations"][win32_idx]

    defines = []
    use_compiler_defines = getattr(tg, "use_compiler_defines", False)
    if use_compiler_defines:
        compiler_builtin_defines = get_compiler_builtin_defines(tg)
        defines = get_vscode_relevant_defines(compiler_builtin_defines)
    extra_defines = getattr(tg, "defines", None)
    if extra_defines:
        defines += extra_defines
    win32_cfg["defines"] = defines

    include_paths = []
    include_dirs = Utils.to_list(getattr(tg, "include_dirs", []))
    if include_dirs:
        include_paths.extend(Path(bld.srcnode.abspath()) / i for i in include_dirs)
    glob_patterns = Utils.to_list(getattr(tg, "glob_patterns", []))
    if glob_patterns:
        for pattern in glob_patterns:
            excl = ["**/.vscode"] + Utils.to_list(getattr(tg, "glob_patterns_excl", []))
            inc_dirs = bld.srcnode.ant_glob(pattern, src=False, dir=True, excl=excl)
            include_paths.extend(inc_dir.abspath() for inc_dir in inc_dirs)
    if getattr(tg, "use_dynamic_includes", False):
        include_paths.extend(get_dynamic_include_paths(bld, tg))
    include_paths = list(set(include_paths))
    properties["env"]["ProjectIncludePath"] = sorted(
        Path(i).as_posix() for i in include_paths
    )

    if getattr(tg, "use_compiler_includes", False):
        compiler_inc = [(Path(tg.env.CC[0]).parent.parent / "include").as_posix()]
        hcg_inc = get_hcg_includes(bld.env.HALCOGEN)
        win32_cfg["includePath"].extend(compiler_inc + hcg_inc)
        win32_cfg["browse"]["path"].extend(compiler_inc + hcg_inc)

    for i in properties["configurations"]:
        if i["name"] == "Win32":
            # use GCC as dummy compiler
            i["compilerPath"] = Path(str(bld.env.GCC[0])).as_posix()

    dump_json_to_node(node, properties)


def build_settings(bld: BuildContext, vscode_dir: Node, tg: task_gen) -> None:
    """Patch ``settings.json`` with project-specific editor settings.

    Reads the template ``settings.json`` from ``vscode_dir``, then:

    - Configures ``pylint.args`` with the project's ``pyproject.toml`` path
      (if ``pylint`` is set on the task generator).
    - Adds waf's internal library paths to ``python.analysis.extraPaths``
      (if ``python_extra_paths`` is set).
    - Merges additional file exclusion patterns into ``files.exclude``
      (if ``files_exclude`` is set).

    Args:
        bld: The active build context.
        vscode_dir: The target ``.vscode`` directory node.
        tg: The task generator providing configuration attributes.
    """
    node = vscode_dir.find_node("settings.json")
    if not node:
        return
    settings = node.read_json()

    root = Path(bld.srcnode.abspath()).as_posix()

    if getattr(tg, "pylint", False):
        settings["pylint.args"] = [f"--rcfile={root}/pyproject.toml"]

    if getattr(tg, "python_extra_paths", False):
        settings["python.analysis.extraPaths"] = [
            f"{root}/tools/{COMMON_WAF_BASE_DIR}",
            f"{root}/tools/.{COMMON_WAF_BASE_DIR}",
        ]

    files_exclude = getattr(tg, "files_exclude", None)
    if files_exclude:
        settings["files.exclude"] = {
            **settings["files.exclude"],
            **files_exclude,
        }

    dump_json_to_node(node, settings)


def build_tasks(bld: BuildContext, vscode_dir: Node) -> None:
    """Patch ``tasks.json`` with platform-specific shell and wrapper commands.

    Reads the template ``tasks.json`` from ``vscode_dir``, then for each
    defined task:

    - Sets the working directory (``options.cwd``) to the project root.
    - Sets the ``command`` to the platform shell (``pwsh`` or ``bash``).
    - Prepends shell-specific flags and the ``fox.py`` wrapper script path
      to the task arguments.
    - Updates ``problemMatcher`` file locations to use ``autoDetect`` with
      the project root.

    Args:
        bld: The active build context.
        vscode_dir: The target ``.vscode`` directory node.
    """
    node = vscode_dir.find_node("tasks.json")
    if not node:
        return
    tasks = node.read_json()

    root = Path(bld.srcnode.abspath()).as_posix()

    for task in tasks["tasks"]:
        task["options"]["cwd"] = root
        task["command"] = bld.env.VS_CODE_SHELL[0]
        if "pwsh" in bld.env.VS_CODE_SHELL[0]:
            task["args"] = [
                "-NoProfile",
                "-NoLogo",
                "-NonInteractive",
                "-File",
                bld.env.FOX_WRAPPER[0],
            ] + task["args"]
        if "bash" in bld.env.VS_CODE_SHELL[0]:
            task["args"] = [
                "--noprofile",
                "--norc",
                "-c",
                bld.env.FOX_WRAPPER[0],
            ] + task["args"]
        for p in task["problemMatcher"]:
            if isinstance(p, dict):
                p["fileLocation"] = ["autoDetect", root]

    dump_json_to_node(node, tasks)


def build_launch(bld: BuildContext, vscode_dir: Node) -> None:
    """Patch ``launch.json`` with debugger paths and program locations.

    Reads the template ``launch.json`` from ``vscode_dir``, then for each
    debug configuration:

    - Sets ``miDebuggerPath`` to the detected GDB executable.
    - Sets ``cwd`` to the current build path.
    - Constructs the ``program`` path using VS Code variables so that
      the correct unit test executable is launched.

    Args:
        bld: The active build context.
        vscode_dir: The target ``.vscode`` directory node.
    """
    node = vscode_dir.find_node("launch.json")
    if not node:
        return
    launch = node.read_json()

    for i in launch["configurations"]:
        i["miDebuggerPath"] = Path(str(bld.env.GDB[0])).as_posix()
        i["cwd"] = Path(bld.path.abspath()).as_posix()
        i["program"] = (
            Path(vscode_dir.parent.get_bld().abspath())
            / "${relativeFileDirname}/${fileBasenameNoExtension}.exe"
        ).as_posix()

    dump_json_to_node(node, launch)


@feature("vscode")
@before_method("process_source")
def process_vscode(self: task_gen) -> None:
    """Process the ``vscode`` feature on a task generator.

    This is the main hook for VS Code workspace generation. For each task
    generator with the ``vscode`` feature it:

    1. Checks that the current build variant is in ``VSCODE_ALLOWED_VARIANTS``.
    2. Validates the JSON templates in ``tools/ide/vscode/``.
    3. Creates the target ``.vscode`` directory and copies the variant's
       template files into it.
    4. Optionally switches the build environment to the one specified by the
       ``environment`` attribute.
    5. Patches ``c_cpp_properties.json`` via :func:`build_c_cpp_properties`.
    6. Patches ``settings.json`` via :func:`build_settings`.
    7. Patches ``tasks.json`` via :func:`build_tasks` (if ``build_tasks`` is set).
    8. Patches ``launch.json`` via :func:`build_launch` (if ``build_launch`` is set).

    Required task generator attributes:
        - **target**: The VS Code workspace variant name (must match a
          subdirectory in ``tools/ide/vscode/``).
        - **vscode_dir (Node)**: The output ``.vscode`` directory node.

    Optional task generator attributes:
        - **environment (str)**: Waf environment name to switch to before
          resolving compiler defines and includes.
        - **build_tasks (bool)**: Whether to patch ``tasks.json``.
        - **build_launch (bool)**: Whether to patch ``launch.json``.
    """
    if self.bld.variant not in VSCODE_ALLOWED_VARIANTS:
        return

    self.source = []
    bld = self.bld
    base_cfg_dir = bld.srcnode.find_dir("tools/ide/vscode")

    if not bld.env.CODE:
        if Logs.verbose:
            Logs.pprint("CYAN", "VS Code not detected, skipping workspace generation.")
        return

    target = getattr(self, "target", None)
    vscode_dir = getattr(self, "vscode_dir", None)

    if not target or not vscode_dir:
        bld.fatal("VS Code feature requires 'target' and 'vscode_dir' attributes.")

    valid_configuration_files(bld, base_cfg_dir)

    if Logs.verbose:
        Logs.pprint("CYAN", f"Creating {vscode_dir}")

    vscode_dir.mkdir()

    for node in base_cfg_dir.ant_glob(f"{target}/*.json"):
        shutil.copy2(node.abspath(), vscode_dir.abspath())

    environment = getattr(self, "environment", "")
    if environment:
        self.env = self.bld.all_envs[environment]

    build_c_cpp_properties(bld, vscode_dir, self)
    build_settings(bld, vscode_dir, self)
    if getattr(self, "build_tasks", False):
        build_tasks(bld, vscode_dir)
    if getattr(self, "build_launch", False):
        build_launch(bld, vscode_dir)


class VSCode(BuildContext):
    """Build context for the ``waf vscode`` command."""

    cmd = "vscode"
    fun = "vscode"


class VSCodeGeneric(BuildContext):
    """Build context for ``waf vscode_generic``."""

    cmd = "vscode_generic"
    fun = "vscode_generic"


class VSCodeCli(BuildContext):
    """Build context for ``waf vscode_cli``."""

    cmd = "vscode_cli"
    fun = "vscode_cli"


class VSCodeAppTiArmCgt(BuildContext):
    """Build context for ``waf vscode_app_ti_arm_cgt``."""

    cmd = "vscode_app_ti_arm_cgt"
    fun = "vscode_app_ti_arm_cgt"


class VSCodeBootloaderTiArmCgt(BuildContext):
    """Build context for ``waf vscode_bootloader_ti_arm_cgt``."""

    cmd = "vscode_bootloader_ti_arm_cgt"
    fun = "vscode_bootloader_ti_arm_cgt"


class VSCodeAppUnitTestGcc(BuildContext):
    """Build context for ``waf vscode_app_unit_test_gcc``."""

    cmd = "vscode_app_unit_test_gcc"
    fun = "vscode_app_unit_test_gcc"


class VSCodeBootloaderUnitTestGcc(BuildContext):
    """Build context for ``waf vscode_bootloader_unit_test_gcc``."""

    cmd = "vscode_bootloader_unit_test_gcc"
    fun = "vscode_bootloader_unit_test_gcc"


def build_vscode(bld: BuildContext) -> None:
    """Entry point that chains all VS Code workspace generation commands."""
    # Inserts the individual variant commands into the waf command queue
    # so they are executed sequentially.
    Options.commands.insert(0, "vscode_generic")
    Options.commands.insert(1, "vscode_cli")
    Options.commands.insert(2, "vscode_app_ti_arm_cgt")
    Options.commands.insert(3, "vscode_bootloader_ti_arm_cgt")
    Options.commands.insert(4, "vscode_app_unit_test_gcc")
    Options.commands.insert(5, "vscode_bootloader_unit_test_gcc")
    bld.targets = ""


def _setup_env(
    bld: BuildContext, environment: str, variant: str, recurse_path: str, target: str
) -> None:
    """Prepare the build context for a single VS Code workspace variant.

    Switches the build context to the specified environment and variant,
    recurses into the appropriate ``wscript`` directory to collect task
    generators, and restricts the build to a single named target.

    Args:
        bld: The active build context to configure.
        environment: The waf environment name to activate.
        variant: The build variant to set on the context.
        recurse_path: The path to recurse into.
        target: The target name to restrict the build to.
    """
    bld.fun = "build"
    bld.env = bld.all_envs[environment]
    bld.variant = variant
    bld.recurse(recurse_path)
    bld.targets = target


def build_vscode_generic(bld: BuildContext) -> None:
    """Generate the generic VS Code workspace configuration."""
    _setup_env(bld, "ti_arm_cgt", "vscode", ".", "generic")


def build_vscode_cli(bld: BuildContext) -> None:
    """Generate the CLI VS Code workspace configuration."""
    _setup_env(bld, "ti_arm_cgt", "vscode", ".", "cli")


def build_vscode_app_ti_arm_cgt(bld: BuildContext) -> None:
    """Generate the embedded app VS Code workspace configuration."""
    _setup_env(bld, "ti_arm_cgt", "app_ti_arm_cgt", "src", "app")


def build_vscode_bootloader_ti_arm_cgt(bld: BuildContext) -> None:
    """Generate the embedded bootloader VS Code workspace configuration."""
    _setup_env(bld, "ti_arm_cgt", "bootloader_ti_arm_cgt", "src", "bootloader")


def build_vscode_app_unit_test_gcc(bld: BuildContext) -> None:
    """Generate the app unit test VS Code workspace configuration."""
    _setup_env(
        bld,
        "unit_test_gcc",
        "app_unit_test_gcc",
        "tests/unit/app",
        "embedded-test-app",
    )


def build_vscode_bootloader_unit_test_gcc(bld: BuildContext) -> None:
    """Generate the bootloader unit test VS Code workspace configuration."""
    _setup_env(
        bld,
        "unit_test_gcc",
        "bootloader_unit_test_gcc",
        "tests/unit/bootloader",
        "embedded-test-bootloader",
    )


# inject command into top-level wscript. This requires that g_module is available
if Context.g_module:
    Context.g_module.__dict__["vscode"] = build_vscode
    Context.g_module.__dict__["vscode_generic"] = build_vscode_generic
    Context.g_module.__dict__["vscode_cli"] = build_vscode_cli
    Context.g_module.__dict__["vscode_app_ti_arm_cgt"] = build_vscode_app_ti_arm_cgt
    Context.g_module.__dict__["vscode_bootloader_ti_arm_cgt"] = (
        build_vscode_bootloader_ti_arm_cgt
    )
    Context.g_module.__dict__["vscode_app_unit_test_gcc"] = (
        build_vscode_app_unit_test_gcc
    )
    Context.g_module.__dict__["vscode_bootloader_unit_test_gcc"] = (
        build_vscode_bootloader_unit_test_gcc
    )


@conf
def get_fox_py_wrapper_executable(ctx: ConfigurationContext) -> None:
    """Locate the platform-specific ``fox.py`` wrapper script.

    Sets ``VS_CODE_SHELL`` to ``pwsh`` (Windows) or ``bash`` (Linux) and
    ``FOX_WRAPPER`` to the corresponding wrapper script (``fox.ps1`` or
    ``fox.sh``).
    """
    ctx.start_msg("Checking for 'fox.py' wrapper:")
    if Utils.is_win32:
        ctx.find_program("pwsh", var="VS_CODE_SHELL", mandatory=False)
        ctx.env.FOX_WRAPPER = [str(ctx.path.find_node("fox.ps1"))]
    else:
        ctx.find_program("bash", var="VS_CODE_SHELL", mandatory=False)
        ctx.env.FOX_WRAPPER = [str(ctx.path.find_node("fox.sh"))]

    ctx.end_msg(ctx.env.get_flat("FOX_WRAPPER"))


@conf
def find_vscode(ctx: ConfigurationContext) -> bool:
    """Locate the VS Code ``code`` executable.

    Searches the system PATH and, on Windows, common installation
    directories. On Linux, additionally checks for a VS Code remote server
    session.

    Returns:
        ``True`` if VS Code (or a remote session) was found, ``False`` otherwise.
    """
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
            ctx.find_program("code", path_list=path_list, mandatory=False)
    else:
        ctx.find_program("code", mandatory=False)
        if not ctx.env.CODE:
            # we might be in a remote environment, scan for this
            code_server_dir = Path("~/.vscode-server").expanduser()
            ctx.msg("Found 'vscode-server' (remote session)", code_server_dir.is_dir())

    if not (ctx.env.CODE or is_remote_session):
        ctx.end_msg(False)
        return False

    ctx.end_msg(ctx.env.get_flat("CODE") or "remote")
    return True


def configure(ctx: ConfigurationContext) -> None:
    """Configure the waf build system for VS Code workspace generation.

    Performs the following steps:

    1. Locates the VS Code ``code`` executable via :func:`find_vscode`.
       If not found, the tool is silently disabled.
    2. Detects the platform-specific shell and ``fox.py`` wrapper via
       :func:`get_fox_py_wrapper_executable`. If neither ``pwsh`` nor
       ``bash`` is available, a warning is emitted and task generation
       is skipped.
    3. Locates ``gcc`` (used as a dummy ``compilerPath`` for IntelliSense
       in ``c_cpp_properties.json``).
    4. Locates ``gdb`` (used for debug configurations in ``launch.json``).

    Args:
        ctx: The active configuration context.
    """
    # create a VS Code workspace if code is installed on this platform
    if not ctx.find_vscode():
        return
    ctx.get_fox_py_wrapper_executable()
    if not ctx.env.VS_CODE_SHELL:
        Logs.warn(
            "Could not find 'pwsh' or 'bash'.\n"
            "VS Code build tasks will not be available."
        )
        return
    ctx.find_program("gcc")
    ctx.find_program("gdb")
