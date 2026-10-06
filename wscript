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

# This script defines how to configure and build the project.
# This includes configuration of the toolchain for building foxBMS binaries, the
# documentation and running various checks on the source files.

"""Top-level Waf build script for the foxBMS project.

This wscript is the entry point for the Waf build system. It defines how to
configure and build all foxBMS variants including embedded firmware binaries
(app and bootloader for TI ARM CGT), host-based unit tests (GCC), static
program analysis (SPA) builds, Doxygen API documentation, and Sphinx-based
general documentation.

The script performs three main tasks:

1. **Variant registration**:
    Dynamically creates Waf build/clean/list/step
    commands for each variant defined in ``VARIANT_CONFIGS``.

2. **Configuration**:
    Sets up multiple named build environments (one per
    toolchain/variant combination), each with its own compiler, flags, and
    tool settings. Environments are derived from a common default to share
    project-wide settings (``APPNAME``, ``VERSION``, ``BMS_CONFIG``).

3. **Build dispatch**:
    Selects the correct environment and source directory
    for the active variant and recurses into the appropriate sub-wscript.

Usage
-----
- Configure all environments once::

    waf configure

- Build a specific variant::

    waf build_app_ti_arm_cgt        # embedded firmware
    waf build_app_unit_test_gcc     # host unit tests
    waf build_docs                  # Sphinx documentation

  The complete list of variants can be found under :ref:`FOX_WAF`
  or via the CLI command ``waf -h``/``waf --help``.

- Clean a variant::

    waf clean_app_ti_arm_cgt

Dependencies
------------
- Waf (bundled)
- TI ARM CGT (for embedded builds)
- GCC (for host/unit-test builds)
- Ruby + CMock/Unity (for unit-test mock generation)
- Sphinx, Doxygen (for documentation builds)
"""

# cspell:ignore multicheck

import os
import sys

from waflib import Build, Context, Logs, Options, Scripting, Utils
from waflib.Build import (
    BuildContext,
    CleanContext,
    ListContext,
    StepContext,
)
from waflib.ConfigSet import ConfigSet
from waflib.Configure import ConfigurationContext
from waflib.Options import OptionsContext

out = "build"
top = "."

APPNAME = "foxBMS"

# foxBMS version; this is included in the embedded binaries, as well as the
# documentation and fox.py
VERSION = "1.12.0"

# Single source of truth for variant metadata.
# The variant name (i.e., the build command) is the key.
# - The variant is either 'app' or 'bootloader' to distinguish between the two
#   main build targets.
#   Exception: the 'docs' variant for the general documentation build
# - The 'app' and 'bootloader' variants have each an embedded build using the
#   target compiler and a SPA build using the SPA compiler in target compiler
#   configuration.
# - The 'app' and 'bootloader' variants have each an accompanying unit test for
#   the host.
# - There are optional unit tests for the embedded target.
# - The 'app' and 'bootloader' variants and their accompanying unit test for
#   the host have each a doxygen documentation variant.
# - The unit test can be built using GCC (i.e., executable on the host) or as
#   SPA variant using the SPA compiler in GCC configuration.
# The naming convention for the variants is in the form of:
# - 'app' or 'bootloader' to distinguish between the two main build targets
# - 'unit_test' if the variant is for unit tests
# - the compiler name if the variant is for an embedded build (e.g., ti_arm_cgt)
# - 'spa' if the variant is for a SPA build followed by the name of the
#   compiler configured for the SPA build (e.g., spa_ti_arm_cgt)
# These names are joined by underscores, e.g., 'app_ti_arm_cgt' in the order of
# this list.
# The value of each key is a dict with the following keys:
# - 'cat' controls command-context generation (optional, defaults to None).
# - 'dir' is recursed in build().
# - 'doc' displayed when using '--help'.
# - 'env' is selected as build environment in build() (optional, defaults to
#   the build variant).
#
# The implementation of these rules is defined in
# tools/waf_tools/validate_variants.py.
VARIANT_CONFIGS = {
    "app_doxygen": {
        "dir": "docs",
        "doc": "doxygen documentation for the app",
        "env": "doxygen",
    },
    "app_unit_test_doxygen": {
        "dir": "docs",
        "doc": "doxygen documentation for the app's unit tests",
        "env": "doxygen",
    },
    "app_ti_arm_cgt": {
        "cat": "binary",
        "dir": "src",
        "doc": "binary of the app for the target",
        "env": "ti_arm_cgt",
    },
    "app_spa_ti_arm_cgt": {
        "cat": "binary",
        "dir": "src",
        "doc": "SPA artifact of the app in TI ARM CGT configuration",
    },
    "app_unit_test_gcc": {
        "cat": "unit_test",
        "dir": "tests",
        "doc": "unit tests for the app on the host",
        "env": "unit_test_gcc",
    },
    "app_unit_test_spa_gcc": {
        "cat": "unit_test",
        "dir": "tests",
        "doc": "SPA artifact of the unit tests of the app",
        "env": "unit_test_spa_gcc",
    },
    "bootloader_doxygen": {
        "dir": "docs",
        "doc": "doxygen documentation for the bootloader",
        "env": "doxygen",
    },
    "bootloader_unit_test_doxygen": {
        "dir": "docs",
        "doc": "doxygen documentation for the bootloader's unit tests",
        "env": "doxygen",
    },
    "bootloader_ti_arm_cgt": {
        "cat": "binary",
        "dir": "src",
        "doc": "binary of the bootloader for the target",
        "env": "ti_arm_cgt",
    },
    "bootloader_unit_test_ti_arm_cgt": {
        "cat": "binary",
        "dir": "src",
        "doc": "unit tests of the bootloader for the target",
        "env": "ti_arm_cgt",
    },
    "bootloader_spa_ti_arm_cgt": {
        "cat": "binary",
        "dir": "src",
        "doc": "SPA artifact of the bootloader in TI ARM CGT configuration",
    },
    "bootloader_unit_test_gcc": {
        "cat": "unit_test",
        "dir": "tests",
        "doc": "unit tests for the bootloader on the host",
        "env": "unit_test_gcc",
    },
    "bootloader_unit_test_spa_gcc": {
        "cat": "unit_test",
        "dir": "tests",
        "doc": "SPA artifact of the unit tests of the bootloader",
        "env": "unit_test_spa_gcc",
    },
    "docs": {
        "dir": "docs",
        "doc": "general documentation",
        "env": "docs",
    },
}


TOOLDIR = "tools/waf_tools"

BMS_CONFIG = {
    "bms": "conf/bms/bms.json",
}


for var, var_cfg in VARIANT_CONFIGS.items():
    contexts: tuple = (BuildContext, CleanContext)
    if var_cfg.get("cat") in ("binary", "unit_test"):
        contexts += (ListContext, StepContext)
    old_contexts = contexts
    for cont in contexts:
        name = cont.__name__.replace("Context", "").lower()
        # Skip creating 'clean_docs' here as it is already provided by the
        # 'sphinx_build' waf tool (see tools/waf_tools/sphinx_build.py).
        if name == "clean" and var == "docs":
            continue

        class tmp_1(cont):
            """Helper class to create the build variant commands"""

            if name == "build":
                __doc__ = f"builds the {var_cfg.get('doc', var)}"
            elif name == "install":
                __doc__ = f"installs the {var_cfg.get('doc', var)}"
            elif name == "clean":
                __doc__ = f"cleans the {var_cfg.get('doc', var)}"
            elif name == "list":
                __doc__ = f"lists the {var_cfg.get('doc', var)}"
            elif name == "step":
                __doc__ = f"steps the {var_cfg.get('doc', var)}"
            cmd = str(name) + "_" + var
            variant = var


def options(opt: OptionsContext):
    """Register all command-line options for the foxBMS build system.

    Loads option definitions from waf tools, removes unused stock waf options,
    unregisters the install/uninstall commands, and adds the
    ``--confcache`` switch.
    """
    opt.VARIANTS = VARIANT_CONFIGS
    opt.load("validate_variants", tooldir=TOOLDIR)
    opt.load("sphinx_build", tooldir=TOOLDIR)
    opt.load("doxygen", tooldir=TOOLDIR)
    opt.load("ti_arm_cgt", tooldir=TOOLDIR)
    opt.load("bootstrap_library_project", tooldir=TOOLDIR)
    opt.load("all_commands", tooldir=TOOLDIR)
    opt.load("gcov", tooldir=TOOLDIR)
    opt.load("vscode", tooldir=TOOLDIR)

    # remove options hard
    for k in (
        "--out",
        "--top",
        "--prefix",
        "--destdir",
        "--bindir",
        "--libdir",
        "--msvc_version",
        "--msvc_targets",
        "--no-msvc-lazy",
        "--force",
        "--check-c-compiler",
    ):
        option = opt.parser.get_option(k)
        if option:
            opt.parser.remove_option(k)

    Context.classes.remove(Build.InstallContext)
    Context.classes.remove(Build.UninstallContext)

    opt.add_option(
        "--confcache",
        dest="confcache",
        default=0,
        action="count",
        help="Use a configuration cache",
    )

    opt.load("lauterbach", tooldir=TOOLDIR)
    opt.load("waf_unit_test")


def configure(ctx: ConfigurationContext):  # pylint: disable=too-many-statements
    """Configure all build environments for the foxBMS project.

    Creates a default environment with project-wide settings (``APPNAME``,
    ``VERSION``, ``BMS_CONFIG``) and derives specialized environments for
    each toolchain:

    - **gcc**:
        Host GCC compiler; validates object, static library, and
        program compilation.
    - **docs**:
        Sphinx documentation generation.
    - **doxygen**:
        Doxygen API documentation generation.
    - **ti_arm_cgt**:
        TI ARM CGT cross-compiler for TMS570 target binaries;
        configures silicon version, ABI, optimisation, linker flags,
        hex/size/nm tools, and Lauterbach trace support.
    - **unit_test_gcc**:
        Host GCC with CMock/Unity/gcov for unit testing.
    - **unit_test_spa_gcc**:
        SPA in GCC configuration.

    Finally queues the ``vscode`` command to regenerate IDE workspace files.
    """
    # This basic configuration shall be loaded as initial step to every
    # environment that is created
    ctx.env.APPNAME = APPNAME
    ctx.env.VERSION = VERSION
    ctx.env.BMS_CONFIG = ctx.path.find_node(BMS_CONFIG["bms"]).relpath()

    checks = [
        {"msg": "check object", "features": "c"},
        {"msg": "check staticlib", "features": "c cstlib"},
        {"msg": "check program", "features": "c cprogram"},
    ]
    checks_executable = [
        {
            "msg": "check unit test",
            "features": "c cprogram",
            "execute": True,
        },
    ]

    ctx.load("check_project_path", tooldir=TOOLDIR)
    ctx.load("config_codegen_task", tooldir=TOOLDIR)
    # Save the default environment; all things that shall be common to all
    # environments shall be added above this line!
    default_env_node = ctx.path.find_or_declare("default.env")
    env_copy = ctx.env.derive()
    env_copy.store(default_env_node.abspath())
    # Basic environment creation done

    ctx.load("vscode", tooldir=TOOLDIR)
    ctx.load("version_validator", tooldir=TOOLDIR)
    ctx.version_consistency_checker()

    # ENV: environment for arbitrary host builds
    env_name = "gcc"
    try:
        ctx.setenv(env_name)
        ctx.env = ConfigSet()
        ctx.env.load(default_env_node.abspath())
        ctx.load("gcc")
        ctx.env.append_unique("CFLAGS", ["-Wall", "-Wextra", "-Werror"])
        ctx.load("waf_unit_test")

        ctx.multicheck(*checks, *checks_executable)

        ctx.env.detach()
    except ctx.errors.ConfigurationError:
        ctx.msg(f"'{env_name}' environment", result=False)
        ctx.setenv("")
        ctx.all_envs.pop(env_name, None)

    # ENV: environment for sphinx documentation builds
    env_name = "docs"
    try:
        ctx.setenv(env_name)
        ctx.env = ConfigSet()
        ctx.env.load(default_env_node.abspath())
        ctx.load("sphinx_build", tooldir=TOOLDIR)
        ctx.load("gcc")
        ctx.env.append_unique("CFLAGS", ["-Wall", "-Wextra", "-Werror"])
        ctx.multicheck(*checks, *checks_executable)
        ctx.env.DEFINES_DOCUMENTATION = ["DOCUMENTATION"]
        ctx.find_program("ruby", var="RUBY")
        ctx.load("mock", tooldir=TOOLDIR)
        ctx.env.detach()
    except ctx.errors.ConfigurationError:
        ctx.msg(f"'{env_name}' environment", result=False)
        ctx.setenv("")
        ctx.all_envs.pop(env_name, None)

    # ENV: environment for doxygen documentation builds
    env_name = "doxygen"
    try:
        ctx.setenv(env_name)
        ctx.env = ConfigSet()
        ctx.env.load(default_env_node.abspath())
        ctx.load("doxygen", tooldir=TOOLDIR)
        ctx.env.detach()
    except ctx.errors.ConfigurationError:
        ctx.msg(f"'{env_name}' environment", result=False)
        ctx.setenv("")
        ctx.all_envs.pop(env_name, None)

    # ENV: environment for TMS570 target builds (TI ARM CGT)
    env_name = "ti_arm_cgt"
    try:
        ctx.setenv(env_name)
        ctx.env = ConfigSet()
        ctx.env.load(default_env_node.abspath())
        ctx.load("ti_arm_cgt", tooldir=TOOLDIR)
        ctx.load("app_crc", tooldir=TOOLDIR)
        ctx.load("hcg", tooldir=TOOLDIR)
        ctx.load("version_generate", tooldir=TOOLDIR)
        ctx.load("vcs_git", tooldir=TOOLDIR)
        ctx.load("bms_config_generate", tooldir=TOOLDIR)
        ctx.load("bms_config_validate", tooldir=TOOLDIR)
        ctx.load("battery_cell_config_validate", tooldir=TOOLDIR)
        ctx.load("battery_system_config_validate", tooldir=TOOLDIR)
        ctx.load("diag_array_config_validate", tooldir=TOOLDIR)
        ctx.load("app_build_config_generate", tooldir=TOOLDIR)
        ctx.env.CFLAGS = [
            "--compile_only",
            "--silicon_version=7R5",
            "--code_state=32",
            "--float_support=VFPv3D16",
            "-g",
            "--diag_wrap=off",
            "--display_error_number",
            "--enum_type=packed",
            "--abi=eabi",
            "--c11",
            "--emit_warnings_as_errors",
        ]
        ctx.env.ASFLAGS = ctx.env.CFLAGS
        ctx.env.CFLAGS_FOXBMS = [
            "-O0",
            "-DASSERT_LEVEL=0",
            "--issue_remarks",
            "--strict_ansi",
        ]
        ctx.env.CFLAGS_HAL = ["-O3"]
        ctx.env.CFLAGS_OS = ["-O3", "--strict_ansi"]
        ctx.env.append_unique(
            "LINKFLAGS",
            [
                "--emit_warnings_as_errors",
                "--be32",
                "--rom_model",
                "--undef_sym=__TI_static_base__",
                # append '--undef_sym=resetEntry' after configuration checks to
                # avoid interference with checks
                "-o4",
                "--unused_section_elimination",
                "--zero_init=on",
                "--scan_libraries",
                "--issue_remarks",
            ],
        )
        ctx.env.HEXFLAGS = [  # cspell:ignore HEXFLAGS
            "-q",
            "--emit_warnings_as_errors",
            "--memwidth=32",
            "--tektronix",
            "-image",
            "--load_image",
            "--load_image:combine_sections=true",
            "--load_image:endian=big",
            "--load_image:file_type=executable",
            "--load_image:format=elf",
            "--load_image:machine=ARM",
            "--load_image:output_symbols=true",
            "--load_image:section_addresses=false",
        ]
        ctx.env.SIZEFLAGS = [  # cspell:ignore SIZEFLAGS
            "--common",
            "--arch=arm",
            "--format=berkeley",
            "--totals",
        ]
        ctx.env.NMFLAGS = ["--all", "-f", "-l"]
        ctx.load("lauterbach", tooldir=TOOLDIR)
        # check if the runtime support library is already available
        rts_lib_stem = "rtsv7R4_A_be_v3D16_eabi"
        armcl = ctx.root.find_node(ctx.env.CC[0])
        lib = armcl.parent.parent.find_node(f"lib/{rts_lib_stem}.lib")
        if not lib:
            # Building the specific RTS only needs to be done once per machine, so
            # we can afford to do one extra 'check', that builds the RTS implicitly
            # (that's how TI ARM CGT works).
            ctx.start_msg(f"Building {rts_lib_stem.upper()}")
            Logs.warn("\nThis may take a while...")
            ctx.check(
                features="c cprogram",
                msg="",
                cflags=["--diag_suppress=10205", "--diag_suppress=10366"],
            )
            ctx.end_msg("ok")
        ctx.multicheck(*checks)
        ctx.check(
            features="c cprogram",
            stlib=rts_lib_stem,
            uselib_store=rts_lib_stem,
            msg="Check runtime support library",
        )
        if Utils.is_win32:
            # bootloader can only be built on Windows due to dependencies being
            # only available for Windows

            # we just check that the library exists;
            # we must link against the library directly in the linker script, i.e.,
            # we can omit uselib_store for the check
            ctx.check(features="c cprogram", stlib="F021_API_CortexR4_BE_L2FMC_V3D16")
            # The macro _L2FMC must be defined before the inclusion of the header
            # file 'F021.h' on devices with the L2FMC Flash controller.
            # doc:: F021 Flash API / Version 2.01.01 / Reference Guide
            #       Literature Number: SPNU501G
            #       December 2012-Revised October 2014
            ctx.check(
                features="c cprogram",
                defines=["_L2FMC=1"],
                uselib_store="_L2FMC",
                msg="Check L2FMC Flash controller",
            )
            ctx.check(features="c cprogram", header_name="F021.h", uselib="_L2FMC")

        ctx.check(**checks[0], confcache=True, cflags="--preproc_macros")

        # it works, so add the final link flag for the actual build
        ctx.env.append_value("LINKFLAGS", "--undef_sym=resetEntry")
        ctx.load("vscode", tooldir=TOOLDIR)
        ctx.load("codegen_matlab", tooldir=TOOLDIR)
        ctx.load("hash_check", tooldir=TOOLDIR)
        ctx.env.detach()
    except ctx.errors.ConfigurationError:
        ctx.msg(f"'{env_name}' environment", result=False)
        ctx.setenv("")
        ctx.all_envs.pop(env_name, None)

    # ENV: environment for host builds (GCC)
    env_name = "unit_test_gcc"
    try:
        ctx.setenv(env_name)
        ctx.env = ConfigSet()
        ctx.env.load(default_env_node.abspath())
        ctx.load("gcc")
        ctx.load("waf_unit_test")
        ctx.find_program("ruby", var="RUBY")
        ctx.load("mock", tooldir=TOOLDIR)
        ctx.load("gcov", tooldir=TOOLDIR)
        ctx.load("hcg", tooldir=TOOLDIR)
        ctx.load("validate_test_json", tooldir=TOOLDIR)

        ctx.env.append_unique(
            "CFLAGS",
            [
                "-Wall",
                "-Wextra",
                "-Wno-unknown-pragmas",
                "-Werror",
                "-std=c11",
                "-pedantic",
                "-g",
            ],
        )
        if not Utils.is_win32:  # gcc on Windows does not require linking against m
            ctx.check_cc(lib="m", uselib_store="M")
        ctx.load("vscode", tooldir=TOOLDIR)
        ctx.env.detach()
    except ctx.errors.ConfigurationError:
        ctx.msg(f"'{env_name}' environment", result=False)
        ctx.setenv("")
        ctx.all_envs.pop(env_name, None)

    if ctx.env.VS_CODE_SHELL:
        Options.commands.insert(0, "vscode")


def build(bld: BuildContext):
    """Dispatch the build to the correct variant sub-wscript.

    Generates VS Code workspace configurations, selects the named environment
    for the active variant from ``VARIANT_CONFIGS``, runs a
    version consistency check, appends the common remarks command file, and
    recurses into the variant's directory (``src/``, ``tests/``, or ``docs/``).
    """
    if bld.env.VS_CODE_SHELL:
        # Generate VS Code workspaces that do not have their own build variant.
        # These are regenerated on every build to stay in sync with the current
        # compiler configuration
        bld(
            features="vscode",
            target="generic",
            vscode_dir=bld.path.make_node(".vscode"),
            use_compiler_defines=True,
            include_dirs=[
                "build/app_ti_arm_cgt",
                "build/app_ti_arm_cgt/src/app/application/config",
                "build/app_ti_arm_cgt/src/app/hal/include",
                "src/version",
            ],
            glob_patterns=[
                "src/app/**",
                "src/os/**",
            ],
            use_compiler_includes=True,
            pylint=True,
            files_exclude={".vscode/**": True, "opt/**": True},
            build_tasks=True,
            environment="ti_arm_cgt",
        )

        bld(
            features="vscode",
            target="cli",
            vscode_dir=bld.path.make_node("cli/.vscode"),
            pylint=True,
        )
    if bld.variant == "vscode":
        return

    variant_config = VARIANT_CONFIGS.get(bld.variant)
    if bld.cmd.startswith("clean"):
        return  # clean works without further processing
    if not variant_config:
        bld.fatal("Variant required.\nFor details use '--help'.")

    env_name = variant_config.get("env", bld.variant)
    if not env_name:
        bld.fatal(f"Build variant {bld.variant} has no configured build environment.")
    try:
        bld.env = bld.all_envs[env_name]
    except KeyError:
        msg = (
            f"Build variant '{bld.variant}' requires environment '{env_name}'"
            ", which is not available.\n"
            "Creating this environment failed during configuration. "
            "Check the configuration output for details.\n"
            "Most likely, the required part of the toolchain is not "
            "(correctly) installed."
        )
        bld.fatal(msg)

    bld.version_consistency_checker()
    bld.env.append_unique("CMD_FILES", [bld.path.find_node("conf/cc/remarks.txt")])

    bld.recurse(variant_config["dir"])


Scripting.Dist.base_name = APPNAME.lower()
Scripting.Dist.algo = "tar.gz"
Scripting.Dist.excl = DIST_EXCLUDE = (
    f"{out}/** "
    f"{APPNAME.lower()}/** "
    ".vs* "
    "**/.git "
    "**/.gitignore "
    ".gitlab/** "
    "**/.gitattributes "
    "**/*.tar.bz2 "
    "**/*.tar.gz "
    "**/*.pyc "
    "**/*.pyo "
    "tests/hil/** "
    "tools/waf*.*.**-* "
    ".lock-* "
)


class DistCheckFoxBMS(Scripting.DistCheck):
    def make_distcheck_cmd(self: Scripting.DistCheck, tmpdir: str = ""):  # noqa: ARG002
        dist_waf = os.path.relpath(sys.argv[0], self.path.abspath())
        cmd = [
            sys.executable,
            os.path.join(self.path.abspath(), self.get_base_name(), dist_waf),
            "configure",
            VARIANT_CONFIGS.keys(),
        ]
        return cmd
