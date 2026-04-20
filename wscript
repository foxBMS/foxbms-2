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
# This includes configuration the toolchain for building foxBMS binaries, the
# documentation and running various checks on the source files.


import argparse
import os
import shutil
import sys
import tarfile
from binascii import hexlify

from waflib import Build, Context, Errors, Logs, Scripting, Utils
from waflib.Build import (
    BuildContext,
    CleanContext,
    ListContext,
    StepContext,
)
from waflib.ConfigSet import ConfigSet
from waflib.Configure import ConfigurationContext

Context.Context.line_just = 50

out = "build"
top = "."

APPNAME = "foxBMS"

# foxBMS version; this is included in the embedded binaries, as well as the
# documentation and fox.py
VERSION = "1.11.0"

# Single source of truth for variant metadata.
# - 'cat' controls command-context generation.
# - 'dir' is recursed in build().
# - 'doc' displayed when using '--help'.
# - 'env' is selected als build environment in build().
VARIANT_CONFIGS = {
    "app_doxygen": {
        "dir": "docs",
        "doc": "doxygen documentation for the app",
        "env": "doxygen",
    },
    "app_doxygen_unit_test": {
        "dir": "docs",
        "doc": "doxygen documentation for the app's unit tests",
        "env": "doxygen",
    },
    "app_embedded": {
        "cat": "binary",
        "dir": "src",
        "doc": "embedded binary of the app",
        "env": "",
    },
    "bootloader_doxygen": {
        "dir": "docs",
        "doc": "doxygen documentation for the bootloader",
        "env": "doxygen",
    },
    "bootloader_doxygen_unit_test": {
        "dir": "docs",
        "doc": "doxygen documentation for the bootloader's unit tests",
        "env": "doxygen",
    },
    "bootloader_embedded": {
        "cat": "binary",
        "dir": "src",
        "doc": "embedded binary of the bootloader",
        "env": "",
    },
    "docs": {
        "dir": "docs",
        "doc": "general documentation",
        "env": "docs",
    },
}

TOOLDIR = "tools/waf-tools"

BMS_CONFIG = {
    "bms": "conf/bms/bms.json",
}


for var, var_cfg in VARIANT_CONFIGS.items():
    contexts: tuple = (BuildContext, CleanContext)
    if var_cfg.get("cat") in ("binary",):
        contexts += (ListContext, StepContext)
    old_contexts = contexts
    for cont in contexts:
        name = cont.__name__.replace("Context", "").lower()

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


def options(opt):
    opt.load("sphinx", tooldir=TOOLDIR)
    opt.load("doxygen", tooldir=TOOLDIR)
    opt.load("f_ti_arm_cgt", tooldir=TOOLDIR)
    # load bootstrap-library-project-tool
    opt.load("f_bootstrap_library_project", tooldir=TOOLDIR)
    opt.load("all_commands", tooldir=TOOLDIR)

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

    opt.load("f_lauterbach", tooldir=TOOLDIR)

    # test function that shall not be communicate through the CLI
    opt.add_option(
        "--target-test",
        dest="target_test",
        default=False,
        action="store_true",
    )
    opt.parser.get_option("--target-test").help = argparse.SUPPRESS


# pylint: disable-next=too-many-statements
def configure(conf: ConfigurationContext):
    # This basic configuration shall be loaded as initial step to every
    # environment that is created
    conf.env.APPNAME = APPNAME
    conf.env.VERSION = VERSION
    bms_config_node = conf.path.find_node(BMS_CONFIG["bms"])
    conf.env.append_unique(
        "CONFIG_BMS_JSON_HASH", hexlify(bms_config_node.h_file()).decode("utf-8")
    )
    conf.load("check_project_path", tooldir=TOOLDIR)
    # Save the default environment; all things that shall be common to all
    # environments shall be added above this line!
    default_env_node = conf.path.find_or_declare("default.env")
    env_copy = conf.env.derive()
    env_copy.store(default_env_node.abspath())
    # Basic environment creation done

    conf.load("version_validator", tooldir=TOOLDIR)
    conf.version_consistency_checker()

    # ENV: environment for sphinx documentation builds
    conf.setenv("docs")
    conf.env = ConfigSet()
    conf.env.load(default_env_node.abspath())
    conf.load("sphinx_build", tooldir=TOOLDIR)
    conf.env.detach()

    # ENV: environment for doxygen documentation builds
    conf.setenv("doxygen")
    conf.env = ConfigSet()
    conf.env.load(default_env_node.abspath())
    conf.load("doxygen", tooldir=TOOLDIR)
    conf.env.detach()

    # ENV: environment for target and SPA builds
    conf.setenv("")
    conf.env = ConfigSet()
    conf.env.load(default_env_node.abspath())
    # We have a basic setup, now we can check what features are available
    conf.find_program("python", var="PYTHON")
    conf.load("vcs_git", tooldir=TOOLDIR)
    conf.load("f_ti_arm_cgt", tooldir=TOOLDIR)
    # test code fragments, that shall work with all compilers
    snippet_main = conf.path.find_node("conf/cc/snippet_main.c").read()
    snippet_sum = conf.path.find_node("conf/cc/snippet_sum.c").read()
    msg = "Checking for code snippet (object)"
    conf.check(features="c", fragment=snippet_main, msg=msg)
    msg = "Checking for code snippet (library)"
    conf.check(features="c cstlib", fragment=snippet_sum, msg=msg)

    def full_build(bld):
        bld.env.APPNAME = "TEST_BUILD"
        c_fragment = "#include <stdint.h>\n\nint main() {\n    return 0;\n}\n"
        h_fragment = (
            "#ifndef GENERAL_H_\n#define GENERAL_H_\n#include <stdbool.h>\n"
            "#include <stdint.h>\n#endif /* GENERAL_H_ */\n"
        )
        source = bld.srcnode.make_node("test.c")
        source.parent.mkdir()
        source.write(c_fragment, encoding="utf-8")
        include = bld.srcnode.make_node("general.h")
        include.write(h_fragment, encoding="utf-8")
        linker_script = bld.path.find_node("../../src/app/main/app.cmd")
        version_header = bld.path.find_node("../../src/version/version.h")
        cflags = []
        if bld.env.RTSV_missing:
            cflags = ["--diag_remark=10366"]
        bld.tiprogram(
            includes=[include.parent, version_header.parent],
            source=[source],
            cflags=cflags,
            linker_script=linker_script,
        )

    default_env = conf.env
    test_env = conf.env.derive()
    test_env.detach()

    conf.setenv("test_env", test_env)
    rtsv_lib = "rtsv7R4_A_be_v3D16_eabi.lib"
    base_dir = f"{conf.root.find_node(conf.env.get_flat('CC')).parent.parent.abspath()}"
    rtsv_lib_path = conf.root.find_node(f"{base_dir}/lib/{rtsv_lib}")
    if not rtsv_lib_path.exists():
        Logs.warn(
            f"Runtime support library '{rtsv_lib}' missing. Need to build "
            "it first. The next step may take a while..."
        )
        conf.env.RTSV_missing = True
    else:
        conf.env.RTSV_missing = False
    conf.env.STLIB = ["c"]
    conf.env.TARGETLIB = []
    if "--undef_sym=resetEntry" in conf.env.LINKFLAGS:
        conf.env.LINKFLAGS.remove("--undef_sym=resetEntry")
    try:
        conf.check(msg="Checking for code snippet (program)", build_fun=full_build)
    except conf.errors.ConfigurationError:
        Logs.error("\n\n===============>>>>>")
        Logs.error(f"===============>>>>> Missing permissions for '{rtsv_lib_path}'. ")
        Logs.error(
            "===============>>>>> The RTS library exists, but the current "
            "users permissions are not sufficient."
        )
        Logs.error(
            "===============>>>>> Increase the current users permissions "
            "on this file and then try again."
        )
        Logs.error("===============>>>>>\n\n")
        conf.fatal("Exit.")

    conf.setenv("", default_env)

    conf.load("f_bootstrap_library_project", tooldir=TOOLDIR)

    # configure the documentation toolchain
    conf.load("f_lauterbach", tooldir=TOOLDIR)

    # Configure the build for the correct RTOS
    conf.load("bms_config_validator", tooldir=TOOLDIR)
    conf.validate_bms_configuration(bms_config_node)
    conf.load("codegen_matlab", tooldir=TOOLDIR)

    # load VS Code setup as last foxBMS specific tool to ensure that all
    # variables have a meaningful value
    conf.load("f_vscode", tooldir=TOOLDIR)
    conf.load("hash_check", tooldir=TOOLDIR)

    # configuration is done, write the config header
    config_header_name = "foxbms_config.h"
    conf.write_config_header(config_header_name)
    config_header = conf.path.find_resource(config_header_name)
    conf.env.append_unique("INCLUDES", [config_header.parent.abspath()])
    conf.env.append_unique("CFLAGS_HAL", ["--preinclude", config_header.abspath()])
    conf.env.append_unique("CFLAGS_OS", ["--preinclude", config_header.abspath()])
    conf.msg(msg="Configuration header", result=config_header.abspath())
    # NOTHING BEYOND THIS POINT MUST BE DONE IN THE CONFIGURATION STEP


def build(bld: BuildContext):
    variant_config = VARIANT_CONFIGS.get(bld.variant)
    if not variant_config:
        bld.fatal(
            f"Build variants are:\n - {'\n - '.join(VARIANT_CONFIGS.keys())}.\n"
            f"For more details use '--help'."
        )

    env_name = variant_config.get("env", "")
    bld.env = bld.all_envs[env_name]
    bld.version_consistency_checker()
    bld.env.append_unique(
        "CMD_FILES", [bld.path.find_node("conf/cc/remarks.txt").abspath()]
    )
    if not bld.env.CONFIG_BMS_JSON_HASH[0] == hexlify(
        bld.path.find_node(BMS_CONFIG["bms"]).h_file()
    ).decode("utf-8"):
        bld.fatal(f"{BMS_CONFIG} has changed. Please run the configure command again.")

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
    def make_distcheck_cmd(self: Scripting.DistCheck, tmpdir: str = ""):  # noqa:ARG002
        dist_waf = os.path.relpath(sys.argv[0], self.path.abspath())
        cmd = [
            sys.executable,
            os.path.join(self.path.abspath(), self.get_base_name(), dist_waf),
            "configure",
            "build_all",
        ]
        return cmd

    def make_distcheck_cmd_additional(self: Scripting.DistCheck):
        """Additional commands that need to be checked in the distribution,
        which are not waf-based commands
        """
        cmd = (
            [
                sys.executable,
                os.path.join(self.path.abspath(), self.get_base_name(), "fox.py"),
                "ceedling",
                "--project",
                "app",
                "gcov:all",
            ],
            [
                sys.executable,
                os.path.join(self.path.abspath(), self.get_base_name(), "fox.py"),
                "ceedling",
                "--project",
                "bootloader",
                "gcov:all",
            ],
        )
        return cmd

    def check(self):
        with tarfile.open(self.get_arch_name()) as t:
            for x in t:
                if hasattr(tarfile, "data_filter"):
                    t.extract(x, filter="data")
                else:
                    t.extract(x)

        for cmd in (
            self.make_distcheck_cmd(""),
        ) + self.make_distcheck_cmd_additional():
            ret = Utils.subprocess.Popen(cmd, cwd=self.get_base_name()).wait()
            if ret:
                err = f"distcheck failed with code {ret}"
                raise Errors.WafError(err)

        shutil.rmtree(self.get_base_name())
