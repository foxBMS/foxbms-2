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

# cspell:ignore armar,armasm,armnm,armsize,armhex,armofd
# cspell:ignore armabs,armacpia,armcg,armclist,armdem,armdis,armembed,armilk,
# cspell:ignore armlibinfo,armlnk,armopt,armpdd,armpprof,armstrip,armobjcopy
# cspell:ignore armobjdump,armreadelf
# cspell:ignore ARMAR,ARMASM,ARMNM,ARMSIZE,ARMHEX,ARMOFD,ARFLAGS,ASMDEFINES
# cspell:ignore BINFLAGS,BINFINALFLAGS,BINFMT,SONAME
# cspell:ignore preincluded

"""Integration of the TI ARM Code Generation Tools into the Waf build system.

This waf tool configures the TI ARM CGT compiler toolchain
(https://www.ti.com/tool/ARM-CGT) for cross-compiling C code targeting
ARM Cortex-R microcontrollers. It replaces the standard GCC-based ``cc``
tool and provides additional features such as hex/binary generation,
preprocessing, symbol listing, and linker-script handling.

Usage
-----
1. Load the tool during ``options`` and ``configure``::

    def options(opt):
        opt.load("ti_arm_cgt", tooldir=TOOLDIR)

    def configure(ctx):
        ctx.load("ti_arm_cgt", tooldir=TOOLDIR)

2. Build a program target using the TI ARM CGT toolchain::

    def build(bld):
        bld.objects(
            source="abc.c",
            target="abc",
        )

3. Optionally enable preprocessing or listing generation::

    waf build_app_ti_arm_cgt --preprocess-files
    waf build_app_ti_arm_cgt --generate-listings

Dependencies
------------
- TI ARM CGT (armcl, armar, armasm, armhex, armofd, tiobj2bin, ...)
- Ruby (for HALCoGen code generation in related tools)
- waf tools: ``nm``, ``size``, ``hexgen``, ``bingen`` (this toolchain)
"""

import os
import re
from pathlib import Path

from waflib import Context, Logs
from waflib.Configure import ConfigurationContext, conf
from waflib.Errors import WafError
from waflib.Node import Node
from waflib.Options import OptionsContext
from waflib.TaskGen import after, after_method, before_method, feature, task_gen
from waflib.Tools import asm, c
from waflib.Utils import to_list

TOOL_DIR = str(Path(__file__).parent)


@conf
def find_armcl(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT compiler and store it in ``ctx.env.CC``."""
    ctx.find_program("armcl", var="CC")
    ctx.get_cc_version_ti_arm_cgt()
    ctx.env.CC_NAME = "armcl"


@conf
def find_armar(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT archiver and store it in ``ctx.env.AR``."""
    ctx.env.AR = ctx.find_program("armar", var="ARMAR")
    ctx.env.ARFLAGS = ["rq"]
    ctx.load("ar")


@conf
def find_armasm(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT assembler and store it in ``ctx.env.AS``."""
    ctx.env.AS = ctx.env.CC or ctx.find_program("armasm", var="ARMASM")
    ctx.env.AS_TGT_F = ctx.env.CC_TGT_F
    ctx.env.ASMDEFINES_ST = ctx.env.DEFINES_ST
    ctx.env.asmstlib_PATTERN = "%s.lib"  # cspell:ignore asmstlib
    ctx.find_armar()
    ctx.load("asm")
    ctx.env.ASM_NAME = "armasm"


@conf
def find_armnm(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT symbol listing tool and store it in ``ctx.env.NM``."""
    ctx.env.NM = ctx.find_program("armnm", var="ARMNM")
    ctx.env.NM_TGT_F = "--output="
    ctx.load("nm", tooldir=TOOL_DIR)


@conf
def find_armsize(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT size tool and store it in ``ctx.env.SIZE``."""
    ctx.env.SIZE = ctx.find_program("armsize", var="ARMSIZE")
    ctx.load("size", tooldir=TOOL_DIR)


@conf
def find_hexgen(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT hex generation tool and store it in ``ctx.env.HEX``."""
    ctx.env.HEX = ctx.find_program("armhex", var="ARMHEX")
    ctx.env.HEX_TGT_F = ["-o", ""]
    ctx.env.HEX_MAP_TGT_F = "--map="
    ctx.load("hexgen", tooldir=TOOL_DIR)


@conf
def find_bingen(ctx: ConfigurationContext) -> None:
    """Find the TI ARM CGT binary generation tool and store it in ``ctx.env.OBJ2BIN``."""
    ctx.env.OBJ2BIN = ctx.find_program("tiobj2bin", var="OBJ2BIN")
    ctx.env.ARMOFD = ctx.find_program("armofd", var="ARMOFD")
    ctx.env.ARMHEX = ctx.find_program("armhex", var="ARMHEX")
    ctx.env.MKHEX4BIN = ctx.find_program("mkhex4bin", var="MKHEX4BIN")
    ctx.env.OBJ2BINFLAGS = []
    ctx.env.OBJ2BINFINALFLAGS = ctx.env.ARMOFD + ctx.env.ARMHEX + ctx.env.MKHEX4BIN
    ctx.load("bingen", tooldir=TOOL_DIR)


@feature("cprogram")
@after("add_hexgen_task")
def add_hexgen_task_ti(self: task_gen) -> None:
    """Add a hex-file generation task after linking.

    Appends a map-file output flag to the hex generation task and registers
    the map file as an additional output. Only runs for targets that define
    a ``linker_script_hex`` attribute.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    if not hasattr(self, "link_task"):
        return
    if not hasattr(self, "linker_script_hex"):
        return

    if not isinstance(self.linker_script_hex, Node):
        self.linker_script_hex = self.path.find_node(self.linker_script_hex)

    tgt = self.link_task.outputs[0].change_ext(".hex.map")
    flag = f"{self.env.HEX_MAP_TGT_F}{tgt.abspath()}"
    self.hexgen.env.append_unique("HEXFLAGS", flag)  # cspell:ignore HEXFLAGS
    self.hexgen.outputs.append(tgt)


@after_method("propagate_uselib_vars")
@feature("asm")
def add_preprocess_flags_for_armasm(self: task_gen) -> None:
    """Adjust assembler flags for ``.asm`` compilation tasks.

    Sets the ``--asm_directory`` flag so that assembled output files
    are placed in the correct build directory.
    """
    if getattr(self.env, "ASM_NAME", "") != "armasm":
        return
    for task in self.tasks:
        if not task.inputs:
            continue
        if task.inputs[0].suffix() == ".asm":
            task.env.append_unique(
                "ASFLAGS",
                [f"--asm_directory={task.outputs[0].parent.bldpath()}"],
            )


@after_method("propagate_uselib_vars")
@before_method("add_preprocess_flags_for_ti")
@feature("c")
def make_flags(self: task_gen) -> None:
    """Create preprocessing tasks for each C source file.

    When ``--preprocess-files`` is active, generates ``.pp`` (preprocessed
    source), ``.ppm`` (predefined macros), ``.ppi`` (included files), and
    ``.ppd`` (dependency) outputs for every C source file in the target.

    HALCoGen ``.hcg`` files are resolved to their generated sources.
    Assembly (``.asm``) files are skipped as ``armcl`` does not support
    preprocessing for them.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    if not self.source:
        return

    # During the ctx.check configure step, if "preproc_macros" is specified
    # in cflags, a .ppm file is produced. This file captures the compiler's
    # predefined macros and defines, which are later parsed to set up
    # the vscode environment
    if "--preproc_macros" in getattr(self, "cflags", []):
        out = f"{self.source[0].name}.{self.idx}.ppm"
        self.create_task("c", self.source[0], self.path.find_or_declare(out))
        return

    if not getattr(self.bld, "options", None):
        return
    if not self.bld.options.preprocess_files:
        return
    # HALCoGen configuration files are passed as source, and by that we have
    # not a c file are input here, so we need to get all the file this code
    # generator generates
    if self.source[0].suffix() == ".hcg":
        sources = self.hcg_generated_sources
    else:
        sources = self.source

    for source in sources:
        if source.suffix() == ".asm":
            # we might come here from the HALCoGen code generator, which also
            # generates .asm files, but armcl does not support preprocessing
            # for .asm files
            continue
        out_prefix = ""
        # otherwise HALCoGen preprocessor output goes in the parent directory
        if f"source{os.sep}HL_" in source.abspath():
            out_prefix = "source/"
        out = f"{out_prefix}{source.name}.{self.idx}.pp"
        self.create_task("c", source, self.path.find_or_declare(out))
        out = f"{out_prefix}{source.name}.{self.idx}.ppm"
        self.create_task("c", source, self.path.find_or_declare(out))

        out = f"{out_prefix}{source.name}.{self.idx}.ppi"
        self.create_task("c", source, self.path.find_or_declare(out))

        out = f"{out_prefix}{source.name}.{self.idx}.ppd"
        self.create_task("c", source, self.path.find_or_declare(out))


@after_method("propagate_uselib_vars")
@feature("c")
def add_preprocess_flags_for_ti(self: task_gen) -> None:
    """Adjust ``task.env.CFLAGS`` depending on the task output file type.

    For preprocessing outputs (``.pp``, ``.ppm``, ``.ppi``, ``.ppd``),
    remove ``--compile_only`` and add the corresponding
    ``--preproc_*`` flag. For regular ``.c`` -> ``.o`` compilation tasks,
    optionally enable cross-reference and function-info listings when
    ``--generate-listings`` is active.

    Warn when a source file is compiled multiple times (``self.idx > 1``)
    because TI ARM CGT does not allow setting output names for auxiliary
    listing files.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    for task in self.tasks:
        try:
            task.inputs[0]
        except IndexError:
            # we come from the 'create_version_file' task, ignore this task
            continue

        suffix = task.outputs[0].suffix()

        if suffix in (".pp", ".ppm", ".ppi", ".ppd"):
            cflags: list = getattr(task.env, "CFLAGS", [])
            to_remove: set = {"--compile_only"}
            if suffix in (".ppm", ".ppi", ".ppd"):
                to_remove.add("--emit_warnings_as_errors")
            task.env.CFLAGS = [f for f in cflags if f not in to_remove]
            task.keyword = lambda: "Preprocessing"

        if suffix == ".pp":
            task.env.append_unique("CFLAGS", ["--preproc_only"])
        if suffix == ".ppm":
            task.env.append_unique("CFLAGS", ["--preproc_macros"])
        if suffix == ".ppi":
            task.env.append_unique("CFLAGS", ["--preproc_includes"])
        if suffix == ".ppd":
            task.env.append_unique("CFLAGS", ["--preproc_dependency"])
        elif task.inputs[0].suffix() == ".c" and suffix == ".o":
            if (
                getattr(self.bld, "options", None)
                and self.bld.options.generate_listings
            ):
                if self.idx > 1:
                    # The TI CGT tools do not allow setting an output file
                    # name for aux, crl and rl files, therefore we need to
                    # warn the user here as these auxiliary files might be
                    # overwritten without anybody noticing if a file is
                    # compiled multiple times in the same build.
                    file = "".join(i.relpath() for i in task.inputs)
                    msg = (
                        f"{file}: Consistency of .aux, .crl and .rl output "
                        "files can not be guaranteed."
                    )
                    Logs.warn(f"{msg}")

                task.env.append_unique(
                    "CFLAGS",
                    [
                        "--gen_cross_reference_listing",
                        "--gen_func_info_listing",
                        "--gen_preprocessor_listing",
                        f"--obj_directory={task.outputs[0].parent.bldpath()}",
                    ],
                )


@after_method("propagate_uselib_vars", "add_preprocess_flags_for_ti")
@feature("c")
def add_cmd_file_as_deps(self: task_gen) -> None:
    """Inject command files as ``--cmd_file=`` flags and register them as
    dependencies.

    Collect entries from ``self.env.CMD_FILES`` and ``self.cmd_file``,
    convert them to ``--cmd_file=<path>`` flags appended to ``CFLAGS``,
    and add them as explicit dependency nodes on each compilation task.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    for task in self.tasks:
        dep_nodes = []
        if getattr(self.env, "CMD_FILES", None):
            dep_nodes.extend(self.env.CMD_FILES)
        if getattr(self, "cmd_file", None):
            dep_nodes.extend(self.to_nodes(self.cmd_file))
        cmd_files = [f"--cmd_file={i.path_from(self.get_cwd())}" for i in dep_nodes]
        task.env.append_unique("CFLAGS", cmd_files)
        self.env.append_unique("CFLAGS", cmd_files)
        task.dep_nodes.extend(dep_nodes)


@after_method("apply_link", "add_cmd_file_as_deps")
@feature("cprogram")
def prepend_cflags_to_linkflags(self: task_gen) -> None:
    """Copy the compiler flags into the link task's flags, stripping
    ``--compile_only``, so that the linker invocation inherits all
    relevant compiler settings.
    """
    if "armcl" not in (self.env.CC_NAME, self.env.CAFECC_TARGET):
        return
    link_task = getattr(self, "link_task", None)
    if not link_task:
        return

    linkflags = getattr(link_task.env, "LINKFLAGS", [])
    cflags = list(getattr(self.env, "CFLAGS", []))
    if "--compile_only" in cflags:
        cflags.remove("--compile_only")
    link_task.env.LINKFLAGS = []
    link_task.env.append_unique("LINKFLAGS", cflags)
    link_task.env.append_unique("LINKFLAGS", linkflags)


@after("apply_link")
@feature("cprogram")
def process_linker_script(self: task_gen) -> None:
    """Add the linker script to the link command and register it as a dependency.

    Resolves ``self.linker_script`` to a waf Node, appends its absolute path
    to ``LINKFLAGS``, and adds it to the link task's dependency list
    so that changes to the script trigger a re-link.

    Raises ``WafError`` if the linker script node cannot be found.
    """
    if not getattr(self, "linker_script", None):
        return
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    if not isinstance(self.linker_script, Node):
        node = self.path.find_node(self.linker_script)
    else:
        node = self.linker_script
    if not node:
        msg = f"could not find {self.linker_script!r}"
        raise WafError(msg)
    linkflags = []
    if getattr(self, "linkflags", None):
        linkflags = self.linkflags
    linkflags = linkflags + [str(node.abspath())]
    self.link_task.env.append_unique("LINKFLAGS", linkflags)
    self.link_task.dep_nodes.append(node)


@after("process_linker_script")
@feature("cprogram")
def add_additional_targets(self: task_gen) -> None:
    """Register XML link-info and map-file as additional link task outputs.

    Creates ``.xml`` and ``.map`` output nodes alongside the linked ELF binary
    and adds the corresponding ``--xml_link_info`` and ``--map_file``
    flags to the linker invocation.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return

    cprogram_pattern = str(self.env.cprogram_PATTERN)
    xml_node = self.path.find_or_declare(cprogram_pattern % self.target + ".xml")
    map_node = self.path.find_or_declare(cprogram_pattern % self.target + ".map")

    add_ti_arm_cgt_targets = [f"--xml_link_info={xml_node}", f"--map_file={map_node}"]

    add_targets = [xml_node, map_node]
    self.link_task.env.append_unique("LINKFLAGS", add_ti_arm_cgt_targets)
    self.link_task.set_outputs(add_targets)


@after_method("propagate_uselib_vars")
@feature("cprogram")
def add_lib_markers(self: task_gen) -> None:
    """Wrap library lists in ``--start-group`` / ``--end-group`` markers.

    This resolves circular dependencies between static and shared libraries
    by instructing the TI linker to search the group repeatedly until
    no new undefined symbols are found.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return

    if self.link_task.env.STLIB:
        libs = self.link_task.env.STLIB
        flags = [self.link_task.env.STLIB_ST % x for x in libs]
        flags.insert(0, "--start-group")
        flags.append("--end-group")

        self.link_task.env.STLIB = flags
        self.link_task.env.STLIB_ST = "%s"

    if self.link_task.env.LIB:
        libs = self.link_task.env.LIB
        flags = [self.link_task.env.LIB_ST % x for x in libs]
        flags.insert(0, "--start-group")
        flags.append("--end-group")

        self.link_task.env.LIB = flags
        self.link_task.env.LIB_ST = "%s"


def options(opt: OptionsContext) -> None:
    """Register command-line options for TI ARM CGT builds.

    Load options from dependent tools (``nm``, ``size``) and add
    ``--preprocess-files`` and ``--generate-listings`` switches.
    """
    opt.load("nm", tooldir=TOOL_DIR)
    opt.load("size", tooldir=TOOL_DIR)
    opt.add_option(
        "--preprocess-files",
        action="store_true",
        default=False,
        help="activating C file preprocessing",
    )
    opt.add_option(
        "--generate-listings",
        action="store_true",
        default=False,
        help="activating generation of listings for C files",
    )


@conf
def get_cc_version_ti_arm_cgt(ctx: ConfigurationContext) -> None:
    """Detect the TI ARM CGT compiler version.

    Runs ``armcl --compiler_revision`` and ``armcl -version`` to determine
    the short and full version strings. Stores the results in
    ``ctx.env.CC_VERSION`` and ``ctx.env.CC_VERSION_FULL``.
    """
    cmd = ctx.env.CC + ["--compiler_revision"]
    std_out, std_err = ctx.cmd_and_log(cmd, output=Context.BOTH)
    if std_err:
        ctx.fatal(f"Could not successfully run '--compiler_revision' on {ctx.env.CC}.")
    ctx.env.CC_VERSION = tuple(std_out.strip().split("."))
    cmd = ctx.env.CC + ["-version"]
    std_out, std_err = ctx.cmd_and_log(cmd, output=Context.BOTH)
    if std_err:
        ctx.fatal(f"Could not successfully run '-version' on {ctx.env.CC}.")
    version_pattern = re.compile(r"(v\d{1,}\.\d{1,}\.\d{1,}\.(LTS|STS))")
    for line in std_out.splitlines():
        full_ver = version_pattern.search(line)
        if full_ver:
            ctx.env.append_unique("CC_VERSION_FULL", full_ver.group(1))
            break
    if not ctx.env.CC_VERSION or not ctx.env.CC_VERSION_FULL:
        ctx.fatal("Could not determine compiler version.")


@conf
def find_arm_tools(ctx: ConfigurationContext) -> None:
    """Locate additional tools distributed with the TI ARM CGT compiler."""
    TI_CCS_ARM_CGT_TOOLS = [  # noqa: N806
        "armabs",
        "armacpia",
        "armadv",
        "armcg",
        "armclist",
        "armdem",
        "armdis",
        "armembed",
        "armilk",
        "armlibinfo",
        "armlnk",
        "armopt",
        "armpdd",
        "armpprof",
        "armstrip",
        # arm-none-eabi-* tools distributed with cgt
        "armobjcopy",
        "armobjdump",
        "armreadelf",
        # these tools are needed to build the runtime support libraries
        "mklib",
        "sh",
        "unzip",
        "gmake",
    ]
    for i in TI_CCS_ARM_CGT_TOOLS:
        ctx.find_program(i)


@conf
def ti_arm_cgt_common_flags(ctx: ConfigurationContext) -> None:
    """Set default compiler, linker, and assembler flags for the TI ARM CGT
    toolchain on ``ctx.env``.
    """
    v = ctx.env
    v.DEST_BINFMT = "elf"
    v.DEST_CPU = "arm32"
    v.DEST_OS = "embedded"
    v.CC_SRC_F = []
    v.CC_TGT_F = ["--output_file="]
    if not v.LINK_CC:
        v.LINK_CC = v.CC
    v.CCLNK_SRC_F = []  # cspell:ignore CCLNK
    v.CCLNK_TGT_F = ["--output_file="]
    v.CPPPATH_ST = "-I%s"
    v.DEFINES_ST = "-D%s"
    v.LIBPATH_ST = "--search_path=%s"
    v.STLIBPATH_ST = "--search_path=%s"
    v.LIB_ST = "--library=%s.lib"
    v.STLIB_ST = "--library=%s.lib"
    v.SONAME_ST = "-Wl,-h,%s"
    v.cstlib_PATTERN = "%s.lib"
    v.cprogram_PATTERN = "%s.elf"
    v.LINKFLAGS = ["-qq", "--run_linker"]
    v.AS = v.CC
    v.AS_TGT_F = v.CC_TGT_F
    v.ASMDEFINES_ST = v.DEFINES_ST


@conf
def ti_arm_cgt_common_paths(ctx: ConfigurationContext) -> None:
    """Set default include and library search paths for the TI ARM CGT
    toolchain on ``ctx.env``.
    """
    cc_path = Path(ctx.env.CC[0])
    ctx.env.append_unique(
        "INCLUDES", os.path.join(cc_path.parent.parent.absolute(), "include")
    )
    ctx.env.append_unique(
        "STLIBPATH", os.path.join(cc_path.parent.parent.absolute(), "lib")
    )


def configure(ctx: ConfigurationContext) -> None:
    """Configure the TI ARM CGT embedded compiler toolchain.

    Locates ``armcl``, ``armar``, ``armasm``, and all auxiliary tools,
    determines the compiler version, sets up default flags and paths,
    and configures the post-processing tools (``nm``, ``size``, ``hexgen``,
    ``bingen``).

    The following environment variables are set:
        - **CC** / **CC_NAME**: Path and name for ``armcl``.
        - **AR** / **ARFLAGS**: Path and flags for ``armar``.
        - **AS** / **ASM_NAME**: Path and name for ``armasm``.
        - **NM** / **SIZE** / **HEX** / **OBJ2BIN**: Post-processing tools.
        - **DEST_BINFMT**: Set to ``elf``.
        - **DEST_CPU**: Set to ``arm32``.
        - **DEST_OS**: Set to ``embedded``.
    """
    ctx.find_armcl()
    ctx.find_arm_tools()
    ctx.ti_arm_cgt_common_flags()
    ctx.ti_arm_cgt_common_paths()

    ctx.find_armar()
    ctx.find_armasm()

    # postprocessing tools
    ctx.find_armnm()
    ctx.find_armsize()
    ctx.find_hexgen()
    ctx.find_bingen()

    ctx.cc_load_tools()
    ctx.cc_add_flags()
    ctx.link_add_flags()


@feature("includes", "c", "asm")
@after_method("process_source")
def add_preinclude_deps(self: task_gen) -> None:
    """Add preinclude dependencies to C and assembly tasks."""
    nodes = to_list(getattr(self, "preinclude_deps", []))
    if not nodes:
        return
    for i in nodes:
        if not isinstance(i, Node):
            self.bld.fatal(
                f"{self.name}: 'preinclude_deps' must only contain "
                f"waflib.Node.Node instances "
                f"(got '{i!r}' of type {type(i)!r})."
            )
    for tsk in self.tasks:
        if isinstance(tsk, (c.c, asm.asm)):
            tsk.dep_nodes.extend(nodes)
