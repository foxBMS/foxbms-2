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

"""Implements a waf tool to use TI ARM CGT (https://www.ti.com/tool/ARM-CGT).
"""

import binascii
import json
import os
import pathlib
import re
import shutil
import sys
from hashlib import md5

import waflib.Tools.asm
from waflib import Context, Logs, Task, TaskGen, Utils, Errors
from waflib.Configure import conf
from waflib.TaskGen import taskgen_method
from waflib.Tools import c_preproc
from waflib.Tools.ccroot import link_task

import f_ti_arm_cgt_cc_options  # pylint: disable=unused-import
import f_ti_arm_helper  # pylint: disable=unused-import
import f_ti_arm_tools  # pylint: disable=unused-import
import f_ti_color_arm_cgt

HAVE_GIT = False
try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    pass

TOOL_DIR = os.path.dirname(os.path.realpath(__file__))


def remove_targets(task):
    """General helper function to remove targets"""
    for target in task.outputs:
        if os.path.exists(target.abspath()):
            os.remove(target.abspath())


def options(opt):
    """Configurable options of the :py:mod:`f_ti_arm_cgt` tool.

    Furthermore the default formatter gets replaced by
    :py:class:`f_ti_arm_cgt.armclFormatter`.
    """
    opt.load("f_ti_arm_cgt_cc_options", tooldir=TOOL_DIR)
    opt.load("f_ti_color_arm_cgt", tooldir=TOOL_DIR)
    opt.load("f_hcg", tooldir=TOOL_DIR)


@TaskGen.extension(".asm")
def asm_hook(self, node):
    """creates the compiled task for assembler sources
    :py:meth:`f_ti_arm_cgt.create_compiled_task_asm`."""
    return self.create_compiled_task_asm("asm", node)


@taskgen_method
def create_compiled_task_asm(self, name, node):
    """creates the assembler task and binds it to the
    :py:class:`f_ti_arm_cgt.asm` class.

    The created tasks are appended to the list of ``compiled_tasks``.
    """
    out_bin = f"{node.name}.{self.idx}.obj"
    task = self.create_task(name, node, node.parent.find_or_declare(out_bin))
    try:
        self.compiled_tasks.append(task)
    except AttributeError:
        self.compiled_tasks = [task]
    return task


class asm(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """class to run the TI ARM CGT compiler in compiler mode to create object
    files from assembler sources.

    .. graphviz::
        :caption: Input-output relation for assembler source files
        :name: asm-to-obj

        digraph ASM_TO_OBJECT {
            compound=true;
            rankdir=LR;
            nd_armcl [label="armcl", style=filled, fillcolor=green];
            nd_asm  [label="*.asm", style=filled];
            nd_obj  [label="*.obj", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_cflags           [label="CFLAGS"];
                nd_cc_compile_only  [label="CC_COMPILE_ONLY"];
                nd_incpaths         [label="INCPATHS"];
                nd_defines          [label="DEFINES"];
            }
            nd_armcl    -> nd_cflags    [lhead=cluster_cmd];
            nd_asm      -> nd_cflags    [lhead=cluster_cmd];
            nd_cflags   -> nd_obj       [ltail=cluster_cmd];
        }
    """

    run_str = (
        "${CC} ${CFLAGS} ${CC_COMPILE_ONLY} "
        "${ASM_DIRECTORY}${TGT[0].parent.bldpath()} ${CPPPATH_ST:INCPATHS} "
        "${DEFINES_ST:DEFINES} ${SRC} ${CC_TGT_F}${TGT[0].relpath()}"
    )
    """str: string to be interpolated to create the command line to compile
    assembler files. See :numref:`asm-to-obj` for a simplified representation"""
    #: list of str: values that effect the signature calculation
    vars = ["CCDEPS"]
    #: list of str: extensions that trigger a re-build
    ext_in = [".h"]
    #: fun: function to be used as scanner method
    scan = c_preproc.scan

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when assembler source files are compiled"""
        return "Compiling"


@TaskGen.extension(".c")
def c_hook(self, node):
    """creates all related task generators for C sources, which are

    - :py:meth:`create_compiled_task_c`,
    - :py:meth:`create_compiled_task_c_pp`,
    - :py:meth:`create_compiled_task_c_ppi`,
    - :py:meth:`create_compiled_task_c_ppd` and
    - :py:meth:`create_compiled_task_c_ppm`.
    """
    return (
        self.create_compiled_task_c("c", node),
        self.create_compiled_task_c_pp("c_pp", node),
        self.create_compiled_task_c_ppi("c_ppi", node),
        self.create_compiled_task_c_ppd("c_ppd", node),
        self.create_compiled_task_c_ppm("c_ppm", node),
    )


@taskgen_method
def create_compiled_task_c(self, name, node):
    """Creates the assembler task and binds it to the
    :py:class:`f_ti_arm_cgt.c` class.

    The created tasks are appended to the list of ``compiled_tasks``.
    """
    if self.idx > 1:
        # The TI CGT tools do not allow setting an output file name for aux,
        # crl and rl files, therefore we need to warn the user here as these
        # auxiliary files might be overwritten without anybody noticing.
        Logs.warn(
            f"{node.relpath()} Consistency of .aux, .crl and .rl output files "
            "can not be guaranteed."
        )
    out_obj = f"{node.name}.{self.idx}.obj"
    out_aux = f"{node.name.rsplit('.')[0]}.aux"
    out_crl = f"{node.name.rsplit('.')[0]}.crl"
    out_rl = f"{node.name.rsplit('.')[0]}.rl"
    task = self.create_task(
        name,
        src=node,
        tgt=[
            node.parent.find_or_declare(out_obj),
            node.parent.find_or_declare(out_aux),
            node.parent.find_or_declare(out_crl),
            node.parent.find_or_declare(out_rl),
        ],
    )

    try:
        self.compiled_tasks.append(task)
    except AttributeError:
        self.compiled_tasks = [task]
    return task


@taskgen_method
def create_compiled_task_c_pp(self, name, node):
    """Creates the pp task and binds it to the
    :py:class:`f_ti_arm_cgt.c_pp` class.
    """
    out = f"{node.name}.{self.idx}.pp"
    task = self.create_task(name, node, node.parent.find_or_declare(out))
    try:
        self.c_pp_tasks.append(task)
    except AttributeError:
        self.c_pp_tasks = [task]
    return task


@taskgen_method
def create_compiled_task_c_ppi(self, name, node):
    """Creates the ppi task and binds it to the
    :py:class:`f_ti_arm_cgt.c_ppi` class.
    """
    out = f"{node.name}.{self.idx}.ppi"
    task = self.create_task(name, node, node.parent.find_or_declare(out))
    return task


@taskgen_method
def create_compiled_task_c_ppd(self, name, node):
    """Creates the ppd task and binds it to the
    :py:class:`f_ti_arm_cgt.c_ppd` class.
    """
    out = f"{node.name}.{self.idx}.ppd"
    task = self.create_task(name, node, node.parent.find_or_declare(out))
    return task


@taskgen_method
def create_compiled_task_c_ppm(self, name, node):
    """Creates the ppm task and binds it to the
    :py:class:`f_ti_arm_cgt.c_ppm` class.
    """
    out = f"{node.name}.{self.idx}.ppm"
    task = self.create_task(name, node, node.parent.find_or_declare(out))
    return task


class c(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """This class implements the TI ARM CGT compiler in compiler mode to create
    object files from c sources. Additionally an aux file (user information
    file), a crl files (cross-reference listing file) and a rl file (output
    preprocessor listing) are generated.

    .. graphviz::
        :caption: Input-output relation for assembler source files
        :name: c-to-obj

        digraph C_TO_OBJECT {
            compound=true;
            rankdir=LR;
            nd_armcl [label="armcl", style=filled, fillcolor=green];
            nd_c    [label="*.c", style=filled];
            nd_obj  [label="*.obj", style=filled];
            nd_aux  [label="*.aux", style=filled];
            nd_crl  [label="*.crl", style=filled];
            nd_rl   [label="*.rl", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_cflags               [label="CFLAGS"];
                nd_cflags_compile_only  [label="CFLAGS_COMPILE_ONLY"];
                nd_cc_compile_only      [label="CC_COMPILE_ONLY"];
                nd_incpaths             [label="INCPATHS"];
                nd_defines              [label="DEFINES"];
            }
            nd_armcl    ->  nd_cflags   [lhead=cluster_cmd];
            nd_c        ->  nd_cflags   [lhead=cluster_cmd];
            nd_cflags   ->  nd_obj      [ltail=cluster_cmd];
            nd_cflags   ->  nd_aux      [ltail=cluster_cmd];
            nd_cflags   ->  nd_crl      [ltail=cluster_cmd];
            nd_cflags   ->  nd_rl       [ltail=cluster_cmd];
        }
    """

    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${CFLAGS_COMPILE_ONLY} ${CC_COMPILE_ONLY} "
        "${OBJ_DIRECTORY}${TGT[0].parent.bldpath()} ${CPPPATH_ST:INCPATHS} "
        "${DEFINES_ST:DEFINES} ${SRC[0].abspath()} ${CC_TGT_F}${TGT[0].abspath()}"
    )
    """str: string to be interpolated to create the command line to compile C
    files. See :numref:`c-to-obj` for a simplified representation"""
    #: list of str: values that effect the signature calculation
    vars = ["CCDEPS", "CMD_FILES_HASH"]
    #: list of str: extensions that trigger a re-build
    ext_in = [".h"]
    #: fun: function to be used as scanner method
    scan = c_preproc.scan

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when source files are compiled"""
        return "Compiling"


class c_pp(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """class to run the TI ARM CGT compiler in "preproc_only" mode, to
    have a pp file that includes the preprocess information

    .. graphviz::
        :caption: Input-output relation for C source files and preprocess information
        :name: c-to-pp

        digraph C_TO_PPI {
            compound=true;
            rankdir=LR;
            nd_armcl [label="armcl", style=filled, fillcolor=green];
            nd_c    [label="*.c", style=filled];
            nd_pp   [label="*.pp", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_cflags       [label="CFLAGS"];
                nd_cflags_pp    [label="PP"];
                nd_incpaths     [label="INCPATHS"];
                nd_defines      [label="DEFINES"];
            }
            nd_armcl    ->  nd_cflags   [lhead=cluster_cmd];
            nd_c        ->  nd_cflags   [lhead=cluster_cmd];
            nd_cflags   ->  nd_ppi      [ltail=cluster_cmd];
        }
    """

    #: str: color in which the command line is displayed in the terminal
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPO} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )
    """str: string to be interpolated to create the command line to get include
    information from C files. See :numref:`c-to-pp` for a simplified
    representation"""
    #: list of str: values that effect the signature calculation
    vars = ["CCDEPS", "CMD_FILES_HASH"]
    #: list of str: extensions that trigger a re-build
    ext_in = [".h"]
    #: fun: function to be used as scanner method
    scan = c_preproc.scan

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when source files are parsed for pp information"""
        return "Preprocessing"


class c_ppi(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """class to run the TI ARM CGT compiler in "preproc_includes" mode, to
    have a ppi file that includes the include information

    .. graphviz::
        :caption: Input-output relation for C source files and include information
        :name: c-to-ppi

        digraph C_TO_PPI {
            compound=true;
            rankdir=LR;
            nd_armcl [label="armcl", style=filled, fillcolor=green];
            nd_c    [label="*.c", style=filled];
            nd_ppi  [label="*.ppi", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_cflags       [label="CFLAGS"];
                nd_cflags_ppi   [label="PPI"];
                nd_incpaths     [label="INCPATHS"];
                nd_defines      [label="DEFINES"];
            }
            nd_armcl    ->  nd_cflags   [lhead=cluster_cmd];
            nd_c        ->  nd_cflags   [lhead=cluster_cmd];
            nd_cflags   ->  nd_ppi      [ltail=cluster_cmd];
        }
    """

    #: str: color in which the command line is displayed in the terminal
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPI} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )
    """str: string to be interpolated to create the command line to get include
    information from C files. See :numref:`c-to-ppi` for a simplified
    representation"""
    #: list of str: values that effect the signature calculation
    vars = ["CCDEPS", "CMD_FILES_HASH"]
    #: list of str: extensions that trigger a re-build
    ext_in = [".h"]
    #: fun: function to be used as scanner method
    scan = c_preproc.scan

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when source files are parsed for ppi information"""
        return "Parsing"


class c_ppd(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """class to run the TI ARM CGT compiler in "preproc_dependency" mode, to
    have a ppd file that includes dependency information

    .. graphviz::
        :caption: Input-output relation for C source files and include information
        :name: c-to-ppd

        digraph C_TO_PPD {
            compound=true;
            rankdir=LR;
            nd_armcl [label="armcl", style=filled, fillcolor=green];
            nd_c    [label="*.c", style=filled];
            nd_ppd  [label="*.ppd", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_cflags       [label="CFLAGS"];
                nd_cflags_ppd   [label="PPD"];
                nd_incpaths     [label="INCPATHS"];
                nd_defines      [label="DEFINES"];
            }
            nd_armcl    ->  nd_cflags   [lhead=cluster_cmd];
            nd_c        ->  nd_cflags   [lhead=cluster_cmd];
            nd_cflags   ->  nd_ppd      [ltail=cluster_cmd];
        }
    """

    #: str: color in which the command line is displayed in the terminal
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPD} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )
    """str: string to be interpolated to create the command line to get
    dependency information from C files. See :numref:`c-to-ppd` for a
    simplified representation"""
    #: list of str: values that effect the signature calculation
    vars = ["CCDEPS", "CMD_FILES_HASH"]
    #: list of str: extensions that trigger a re-build
    ext_in = [".h"]
    #: fun: function to be used as scanner method
    scan = c_preproc.scan

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when source files are parsed for ppd information"""
        return "Parsing"


class c_ppm(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """class to run the TI ARM CGT compiler in "preproc_macros" mode, to have
    a ppm file that includes all compile time macros

    .. graphviz::
        :caption: Input-output relation for C source files and include information
        :name: c-to-ppm

        digraph C_TO_PPM {
            compound=true;
            rankdir=LR;
            nd_armcl [label="armcl", style=filled, fillcolor=green];
            nd_c    [label="*.c", style=filled];
            nd_ppm  [label="*.ppm", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_cflags       [label="CFLAGS"];
                nd_cflags_ppm   [label="PPM"];
                nd_incpaths     [label="INCPATHS"];
                nd_defines      [label="DEFINES"];
            }
            nd_armcl    ->  nd_cflags   [lhead=cluster_cmd];
            nd_c        ->  nd_cflags   [lhead=cluster_cmd];
            nd_cflags   ->  nd_ppm      [ltail=cluster_cmd];
        }
    """

    #: str: color in which the command line is displayed in the terminal
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPM} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )
    """str: string to be interpolated to create the command line to get
    macro information from C files. See :numref:`c-to-ppm` for a
    simplified representation"""
    #: list of str: values that effect the signature calculation
    vars = ["CCDEPS", "CMD_FILES_HASH"]
    #: list of str: extensions that trigger a re-build
    ext_in = [".h"]
    #: fun: function to be used as scanner method
    scan = c_preproc.scan

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when source files are parsed for ppm information"""
        return "Parsing"


class cprogram(link_task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """class to run the TI ARM CGT compiler in linker mode to create the target"""

    run_str = (
        "${LINK_CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${RUN_LINKER} "
        "${LINKFLAGS} ${MAP_FILE}${TGT[2].abspath()} "
        "${XML_LINK_INFO}${TGT[1].abspath()} ${CCLINK_TGT_F}${TGT[0].abspath()} "
        "${SRC} ${LINKER_SCRIPT} ${LIBPATH_ST:LIBPATH} ${STLIBPATH_ST:STLIBPATH} "
        "${LIB_ST:LIB} ${STLIB_ST:STLIB} ${TARGETLIB_ST:TARGETLIB} ${LDFLAGS}"
    )
    #: list of str: values that effect the signature calculation
    vars = ["LINKDEPS", "CMD_FILES_HASH"]
    #: list of str: extensions that trigger a re-build
    ext_out = [".elf"]

    # set inst_to to a dummy value so that waf knows that it can execute
    # an installation task
    inst_to = True

    def exec_command(self, cmd, **kw):  # pylint: disable=arguments-differ
        kw["shell"] = isinstance(cmd, str)
        kw["cwd"] = self.get_cwd()
        kw["output"] = Context.BOTH
        kw["quiet"] = Context.BOTH
        try:
            std = self.generator.bld.cmd_and_log(cmd, **kw)
        except Errors.WafError as err:
            Logs.error(err.msg)
            if hasattr(err, "stdout"):
                Logs.error(err.stdout)
            if hasattr(err, "stderr"):
                Logs.error(err.stderr)
            ret = 1
            if hasattr(err, "returncode"):
                ret = err.returncode
            return ret
        if std[0]:
            self.generator.bld.to_log(
                f_ti_color_arm_cgt.armclFormatter.colorize(std[0])
            )
        if std[1]:
            self.generator.bld.to_log(
                f_ti_color_arm_cgt.armclFormatter.colorize(std[1])
            )

        if not hasattr(self.generator, "linker_pulls"):
            Logs.warn("No pull file specified. Check linker output!")
            return 0
        hits, errors = cprogram.parse_output(
            self.generator.linker_pulls.read(),
            self.generator.bld.path.get_bld().abspath(),
            std[0],
        )
        if Logs.verbose:
            Logs.info(os.linesep.join(hits))
        if errors:
            Logs.error(
                "Removing binary as the following errors occurred after linkage:\n"
                + os.linesep.join(errors)
            )
            # remove output since the binary was not linked as desired
            for i in self.outputs:
                i.delete()
        return len(errors)

    @staticmethod
    def parse_output(pull_config, obj_path, std):
        """parses the output of the linker"""
        if pull_config == "{}":
            return [], []
        if not std:
            return [], []
        if not "#10252" in std:
            return [], []
        # now we know that at least some warning has been printed to stdout that
        # includes the linker remark #10252 and that a pull file has been
        # specified.
        linker_pulls = json.loads(pull_config)
        correctly_found = {}
        for fun, src in linker_pulls.items():
            correctly_found[fun] = (os.path.normpath(src), False, False)

        sym = r"Symbol \"(@FUN@)\""
        link_regex = r"Symbol \"@FUN@\" \(pulled from \"@SRC@\"\)"
        link_src_reg = r"\(pulled from \"(\S*)\"\)"
        errors = []
        hits = []
        for line in std.splitlines():
            if "#10252" in line:
                for fun, (src, found_sym, found_src) in correctly_found.items():
                    sym_reg = sym.replace("@FUN@", fun)
                    src_txt = src.replace("\\", "\\\\").replace(".", r"\.")
                    link_reg = link_regex.replace("@FUN@", fun).replace(
                        "@SRC@", src_txt
                    )
                    # pylint: disable=unnecessary-dict-index-lookup
                    if re.search(sym_reg, line) and not correctly_found[fun] == (
                        src,
                        True,
                        True,
                    ):
                        if re.search(link_reg, line):
                            correctly_found[fun] = (src, True, True)
                            break
                        wrong_src = re.search(link_src_reg, line).group(1)
                        path_tuple = (src, wrong_src)
                        correctly_found[fun] = (path_tuple, True, False)
                        break
        for fun, (src, found_sym, found_src) in correctly_found.items():
            if not found_sym and not found_src:
                full_src_path = os.path.join(obj_path, src)
                errors.append(f"Did not find the symbol '{fun}'.")
            elif not found_src and found_sym:
                full_src_path = os.path.join(obj_path, src[0])
                full_wrong_path = os.path.join(obj_path, src[1])
                errors.append(
                    f"Did not find '{fun}' where it was expected ('{full_src_path}')."
                )
                errors.append(f"Instead it was found in '{full_wrong_path}'.")
            else:
                full_src_path = os.path.join(obj_path, src)
                hits.append(f"Found '{fun}' as expected in '{full_src_path}'.")

        return hits, errors

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when black is linking the target"""
        return "Linking"


class stlink_task(link_task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """static link task"""

    run_str = [
        remove_targets,
        "${AR} ${ARFLAGS} ${AR_TGT_F} ${TGT[0].abspath()} ${AR_SRC_F}${SRC}",
    ]

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when linking"""
        return "Linking"


class cstlib(stlink_task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """c static library"""

    pass  # pylint: disable-msg=unnecessary-pass


class copy_elf(Task.Task):  # pylint: disable-msg=invalid-name
    """This class implements the copying of the elf file to another location
    in the build tree"""

    #: str: color in which the command line is displayed in the terminal
    color = "CYAN"

    #: list of str: tasks after that hexgen task can be run
    after = ["link_task"]

    def run(self):
        """copy the generated elf file to build directory of the variant"""
        shutil.copy2(self.inputs[0].abspath(), self.outputs[0].abspath())

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when copying the elf file"""
        return "Copy"

    def __str__(self):
        """additional information appended to the keyword"""
        return f"{self.inputs[0]} -> {self.outputs[0]}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_copy_elf_task(self):
    """creates a task to copy the elf file into the output root
    (task :py:class:`f_ti_arm_cgt.copy_elf`)"""

    if self.bld.variant_dir == self.link_task.outputs[0].parent.abspath():
        return
    if not hasattr(self, "link_task"):
        return
    if self.bld.variant_dir:
        out_dir = self.bld.variant_dir
    else:
        out_dir = self.bld.path.get_bld()
    self.copy_elf_task = self.create_task(
        "copy_elf",
        src=self.link_task.outputs[0],
        tgt=[
            self.bld.path.find_or_declare(
                os.path.join(out_dir, self.link_task.outputs[0].name)
            )
        ],
    )


@conf
def tiprogram(bld, *k, **kw):
    """wrapper for bld.program for simpler target configuration.
    The linker script is added as env input/output dependency for the
    target.
    Based on the target name all related targets are created:

    - binary: <target>.<format>
    - linker information: <target>.<format>.xml
    - map information: <target>.<format>.map
    """

    if "target" not in kw:
        kw["target"] = "out"
    if "linker_script" not in kw:
        bld.fatal("linker script missing")
    bld.env.LINKER_SCRIPT = kw["linker_script"].abspath()

    kw["features"] = "c cprogram"

    tgt_elf = bld.path.find_or_declare(f"{kw['target']}.{bld.env.DEST_BIN_FMT}")
    tgt_xml = bld.path.find_or_declare(tgt_elf.name + ".xml")
    tgt_map = bld.path.find_or_declare(tgt_elf.name + ".map")
    kw["target"] = [tgt_elf, tgt_xml, tgt_map]

    bld.add_manual_dependency(tgt_elf, kw["linker_script"])

    if "linker_pulls" in kw:
        scan_opt = "--scan_libraries"
        if not scan_opt in bld.env.LINKFLAGS and not scan_opt in kw["linkflags"]:
            bld.fatal(
                "'linker_pulls' was specified without linker flag '--scan_libraries'."
            )
        bld.add_manual_dependency(tgt_elf, kw["linker_pulls"])

    # if a hex file should be generated, we need to append the config
    if "linker_script_hex" in kw:
        kw["features"] += " hexgen"
        bld.env.LINKER_SCRIPT_HEX = kw["linker_script_hex"].abspath()
        bld.add_manual_dependency(tgt_elf, kw["linker_script_hex"])
        # get the file hash assuming Unix-style line endings
        elf_file_hash = binascii.hexlify(
            md5(
                kw["linker_script"].read().replace("\r\n", "\n").encode("utf-8")
            ).digest()
        )
        txt = kw["linker_script_hex"].read().strip().splitlines()[0]
        txt = re.search(r"\/\*(.*)\*\/", txt)
        try:
            txt = txt.group(1)
        except IndexError:
            bld.fatal("hashing error")
        known_hash = bytes(txt.strip(), encoding="utf-8")
        if not elf_file_hash == known_hash:
            bld.fatal(
                f"The hash of '{kw['linker_script'].abspath()}' has changed from "
                f"'{known_hash.decode('utf-8')}' to '{elf_file_hash.decode('utf-8')}'.\n"
                f"Reflect the changes in the elf file linker script "
                f"('{kw['linker_script'].name}') in the hex file linker script "
                f"('{kw['linker_script_hex'].name}') and then update the file hash "
                f"generated based on the content of the elf linker script in the "
                f"hex file linker script ('{kw['linker_script_hex'].abspath()}')."
            )

    return bld(*k, **kw)


@TaskGen.feature("c", "asm", "includes")
@TaskGen.after_method("propagate_uselib_vars", "process_source")
def apply_incpaths(self):
    """Adds the include paths"""
    lst = self.to_incnodes(
        self.to_list(getattr(self, "includes", [])) + self.env.INCLUDES
    )
    self.includes_nodes = lst
    cwd = self.get_cwd()
    self.env.INCPATHS = []
    for i in lst:
        if os.sep + "ti" + os.sep in i.abspath():
            self.env.INCPATHS.append(i)
        else:
            self.env.INCPATHS.append(i.path_from(cwd))


@TaskGen.feature("c")
@TaskGen.before_method("apply_incpaths")
def check_duplicate_and_not_existing_includes(self):
    """Check if include directories are included more than once and if they
    really exist on the file system. If include directories do not exist, or
    they are included twice, raise an error."""
    includes = self.to_incnodes(
        self.to_list(getattr(self, "includes", [])) + self.env.INCLUDES
    )
    not_existing = list(
        filter(
            None,
            [None if os.path.isdir(i.abspath()) else i.abspath() for i in includes],
        )
    )
    abs_incs = [[x.abspath(), x.relpath()] for x in includes]
    seen = {}
    duplicates = []
    for x_abs, x_rel in abs_incs:
        if x_abs not in seen:
            seen[x_abs] = 1
        else:
            if seen[x_abs] == 1:
                duplicates.append((x_abs, x_rel))
            seen[x_abs] += 1
    err = ""
    if duplicates or not_existing:
        err = (
            f"There are include errors when building '{self.target}' from "
            f"sources {self.source} in build file from '{self.path}{os.sep}'.\n"
        )
    if not_existing:
        err += f"Not existing include directories are: {not_existing}\n"
    if duplicates:
        duplicates = [
            item
            for t in duplicates
            for item in t
            if not os.sep + "build" + os.sep in item
        ]
        duplicates = list(set(duplicates))
        err += f"Duplicate include directories are: {duplicates}\n"
    if err:
        self.bld.fatal(err)


class hexgen(Task.Task):  # pylint: disable-msg=invalid-name
    """Task create hex file from elf files"""

    #: str: color in which the command line is displayed in the terminal
    color = "YELLOW"

    #: list of str: tasks after that hexgen task can be run
    after = ["link_task"]

    run_str = (
        "${ARMHEX} -q ${HEXGENFLAGS} --map=${TGT[1].abspath()} ${LINKER_SCRIPT_HEX} "
        "${SRC[0].abspath()} -o ${TGT[0].abspath()}"
    )
    """str: string to be interpolated to create the command line to create a
    hex file from an elf file."""

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when generating the hex file"""
        return "Compiling"

    def __str__(self):
        """additional information appended to the keyword"""
        return f"{self.inputs[0]} -> {self.outputs[0]}"


@TaskGen.feature("hexgen")
@TaskGen.after("apply_link")
def add_hexgen_task(self):
    """creates a tasks to create a hex file from the linker output
    (task :py:class:`f_ti_arm_cgt.hexgen`)"""
    if not hasattr(self, "link_task"):
        return
    self.hexgen = self.create_task(
        "hexgen",
        src=self.link_task.outputs[0],
        tgt=[
            self.link_task.outputs[0].change_ext(".hex"),
            self.link_task.outputs[0].change_ext(".hex.map"),
        ],
    )


class bingen(Task.Task):  # pylint: disable-msg=invalid-name
    """Task create bin file from elf files"""

    #: str: color in which the command line is displayed in the terminal
    color = "CYAN"

    #: list of str: tasks after that hexgen task can be run
    after = ["link_task"]

    run_str = (
        "${TIOBJ2BIN} ${SRC[0].abspath()} ${TGT[0].abspath()} ${ARMOFD} "
        "${ARMHEX} ${MKHEX4BIN}"
    )
    """str: string to be interpolated to create the command line to create a
    bin file from an elf file."""

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when generating the bin file"""
        return "Compiling"

    def __str__(self):
        """additional information appended to the keyword"""
        return f"{self.inputs[0]} -> {self.outputs[0]}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_bingen_task(self):
    """creates a task to create a bin file from the linker output
    (task :py:class:`f_ti_arm_cgt.bingen`)
    """
    if not hasattr(self, "link_task"):
        return
    self.bingen = self.create_task(
        "bingen",
        src=self.link_task.outputs[0],
        tgt=[self.link_task.outputs[0].change_ext(".bin")],
    )


@taskgen_method
def accept_node_to_link(self, node):  # pylint: disable=unused-argument
    """filters which output files are not meant to be linked"""
    return not node.name.endswith((".aux", "crl", "rl"))


@TaskGen.feature("c", "cprogram")
@TaskGen.after("apply_link")
def process_sizes(self):
    """creates size tasks for generated object and object-like files"""
    if getattr(self, "link_task", None) is None:
        return
    for node in self.link_task.inputs:
        out = node.change_ext(".size.log")
        self.create_task("size", node, out)
    for node in self.link_task.outputs:
        if node.suffix() in (".a", "." + self.bld.env.DEST_BIN_FMT):
            out = node.change_ext(".size.log")
            self.create_task("size", node, out)


class size(Task.Task):  # pylint: disable-msg=invalid-name
    """Task to run size on all input files"""

    vars = ["ARMSIZE", "ARMSIZE_OPTS"]

    #: str: color in which the command line is displayed in the terminal
    color = "BLUE"

    def run(self):
        """implements the actual behavior of size and pipes the output into
        a file."""
        cmd = (
            Utils.subst_vars("${ARMSIZE} ${ARMSIZE_OPTS}", self.env)
            + " "
            + self.inputs[0].abspath()
        )
        cmd = cmd.split()
        outfile = self.outputs[0].path_from(self.generator.path)
        env = self.env.env or None
        cwd = self.generator.bld.path.get_bld().abspath()
        out, err = self.generator.bld.cmd_and_log(
            cmd,
            output=waflib.Context.BOTH,
            quiet=waflib.Context.STDOUT,
            env=env,
            cwd=cwd,
        )
        self.generator.path.make_node(outfile).write(out)
        if err:
            Logs.error(err)

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when size is run on object files"""
        return "Processing size"


@TaskGen.feature("c", "cprogram")
@TaskGen.after("apply_link")
def process_nm(self):
    """creates nm tasks for generated object files"""
    if getattr(self, "link_task", None) is None:
        return
    for node in self.link_task.inputs:
        out = node.change_ext(".nm.log")
        self.create_task("nm", node, out)
    for node in self.link_task.outputs:
        if node.suffix() in (".a", "." + self.bld.env.DEST_BIN_FMT):
            out = node.change_ext(".nm.log")
            self.create_task("nm", node, out)


class nm(Task.Task):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """Task to run armnm on all input files"""

    #: str: color in which the command line is displayed in the terminal
    color = "PINK"

    run_str = "${ARMNM} ${NMFLAGS} --output=${TGT} ${SRC}"
    """str: string to be interpolated to create the command line to create a
    nm file from an ``*.obj``, ``*.a`` or ``*.elf`` file."""

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when armnm is run on object files"""
        return "Processing nm"


@TaskGen.feature("c")
@TaskGen.after("c_pp")
def remove_stuff_from_pp(self):
    """creates pp tasks for generated object files"""
    for node in self.c_pp_tasks:
        outs = [node.outputs[0].change_ext(".ppr"), node.outputs[0].change_ext(".pprs")]
        self.create_task("clean_pp_file", node.outputs[0], outs)


class clean_pp_file(
    Task.Task
):  # pylint: disable-msg=invalid-name,too-few-public-methods
    """Task to remove some information from the preprocessed files"""

    #: str: color in which the command line is displayed in the terminal
    color = "PINK"

    #: tuple: strings that need to be removed from the preprocessed file
    remove_str = ("#define", "#pragma", "# pragma", "_Pragma")

    #: list: regular expressions that should be removed with a certain string
    replace_str = [(r"__attribute__\(.*\)", "")]

    def run(self):
        """Removes empty lines and strips some intrinsics that should not be
        included in the postprocessed file"""
        # read file, split text in list by lines and remove all empty entries
        txt = list(filter(str.rstrip, self.inputs[0].read().splitlines()))
        # join all lines without right side whitespace and write it to file
        txt = os.linesep.join(line.rstrip() for line in txt) + os.linesep
        self.outputs[0].write(txt, encoding="utf-8")
        txt = os.linesep.join(
            line.rstrip()
            for line in txt.split(os.linesep)
            if (not line.startswith(clean_pp_file.remove_str))
        )
        for rep in clean_pp_file.replace_str:
            txt = re.sub(rep[0], rep[1], txt)
        self.outputs[1].write(txt, encoding="utf-8")

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when post-processing the pre-processed files"""
        return "Postprocessing"


class create_version_source(Task.Task):  # pylint: disable=invalid-name
    """creates the version information file"""

    #: int: priority of the task
    weight = 1

    #: str: color in which the command line is displayed in the terminal
    color = "BLUE"

    #: list of str: specifies task, that this task needs to run before
    before = ["c"]

    #: list of str: extensions that trigger a re-build
    ext_out = [".h"]

    def get_remote(self):
        """returns the git remote"""
        # pylint: disable=no-member
        remote = "No remote"
        if self.repo:
            remote = self.repo.git.remote("get-url", "--push", "origin")
        return remote

    def get_version_from_git(self):
        """returns a version string that is extracted directly
        from the underlying git repository
        """
        # pylint: disable=no-member
        describe_output = f"no-vcs-{self.env.VERSION}-dirty"
        if self.repo:
            describe_output = self.repo.git.describe(
                "--dirty", "--tags", "--long", "--always", "--match", "*.*"
            )

        if describe_output.endswith("-dirty"):
            dirty = True
            describe_output = describe_output[:-6]
        else:
            dirty = False

        tag = "unreleased"
        major = 0
        minor = 0
        patch = 0
        distance = 0
        commit = "no-vcs"
        if describe_output.startswith("v"):
            # remove v from start
            describe_output = describe_output[1:]
            # we are on a branch with a tagged version
            tag, distance, commit = describe_output.rsplit("-", 2)
            # try to split of any additions like -rc or -alpha as we don't care
            # for them in the tag
            tag = tag.rsplit("-")[0]
            # try to extract major minor patch
            major, minor, patch = tag.split(".")
            # remove the g from commit ID
            commit = commit[1:]
            version = f"{tag}-{distance}-{commit}"
        elif describe_output.startswith("no-vcs"):
            version = describe_output
            tag = self.env.VERSION
        else:
            # no recognizable version has been tagged
            # expecting the output to be commit-id
            version = f"{tag}-{describe_output}"

        if dirty:
            version = f"{version}-dirty"

        # assemble information into a dict for later useage
        version_output = {
            "version": version,
            "dirty": dirty,
            "tag": tag,
            "major": int(major),
            "minor": int(minor),
            "patch": int(patch),
            "distance": int(distance),
            "commit": commit,
        }
        return version_output

    def get_repo_dirty_from_git(self):
        """returns a boolean marking if the project's working
        directory is dirty (which means it contains unstaged changes)
        """
        # pylint: disable=no-member
        dirty = True
        if self.repo:
            dirty = self.repo.is_dirty()
        return dirty

    def run(self):
        """Created the version information files"""
        waf_version = self.env.VERSION

        is_git_repo = "false"
        if self.repo:  # pylint: disable=no-member
            is_git_repo = "true"
        is_dirty = "false"
        if self.get_repo_dirty_from_git():
            is_dirty = "true"
        git_remote = self.get_remote()

        version_output = self.get_version_from_git()

        if version_output.get("tag") not in ("unreleased", waf_version):
            self.generator.bld.fatal(
                f"Extracted version from git repo ({version_output['tag']}) does not match "
                f"version defined in waf ({waf_version})."
            )

        # note: these values have to be in line with the corresponding defines
        # in version_cfg.h
        commit_hash_maximum_string_length = 9
        git_remote_maximum_string_length = 128
        maximum_version_size = 255
        maximum_distance_size = 65535
        distance_int = min(maximum_distance_size, version_output["distance"])
        commit_hash = version_output.get("commit", "noHash")[
            :commit_hash_maximum_string_length
        ]
        self.outputs[0].write(
            os.linesep.join(
                [
                    '#include "version_cfg.h"',
                    "const VERSION_s foxbmsVersionInfo VERSION_INFORMATION = {",
                    f"    .underVersionControl = {is_git_repo},",
                    f"    .isDirty = {is_dirty},",
                    f"    .major = {min(maximum_version_size, version_output['major'])},",
                    f"    .minor = {min(maximum_version_size, version_output['minor'])},",
                    f"    .patch = {min(maximum_version_size, version_output['patch'])},",
                    f"    .distanceFromLastRelease = {distance_int},",
                    f'    .commitHash = "{commit_hash}",',
                    f'    .gitRemote = "{git_remote[:git_remote_maximum_string_length]}",',
                    "};",
                ]
            )
            + os.linesep,
            encoding="utf-8",
        )

    def sig_explicit_deps(self):
        """Defines how to get signature of this task (and thus when to rerun it)"""
        version_output = self.get_version_from_git()
        self.m.update(version_output["version"].encode("utf-8"))


@TaskGen.feature("cprogram")
@TaskGen.after_method("process_rule")
def create_version_file(self):
    """Task generator for version information file"""
    no_version = getattr(self, "no_version", False)
    if HAVE_GIT:
        try:
            repo = Repo(self.bld.top_dir)
        except InvalidGitRepositoryError:
            if not no_version:
                Logs.warn(
                    "Not a git repository. Proceeding without version information."
                )
            repo = None
        except:  # pylint: disable=bare-except
            Logs.error(f"An unexpected error occurred:\n{sys.exc_info()[0]}")
            Logs.warn("Proceeding without version information.")
            repo = None
    else:
        Logs.warn("Git not available. Proceeding without version information.")
        repo = None
    tsk = self.create_task(
        "create_version_source",
        tgt=[
            self.path.find_or_declare("version_cfg.c"),
        ],
        repo=repo,
    )

    try:
        self.includes.append(self.path.get_bld())
    except AttributeError:
        self.includes = [self.path.get_bld()]
    try:
        self.source.append(tsk.outputs[0])
    except AttributeError:
        self.source = [self.source, tsk.outputs[0]]


@TaskGen.feature("c")
@TaskGen.before_method("apply_incpaths")
def hash_cmd_files(self):
    """calculate hashes for command files, before running the c-related tasks,
    as c-related tasks might rely on these files"""
    if not isinstance(getattr(self.env, "CMD_FILES", []), list):
        self.bld.fatal("'ctx.env.CMD_FILES' must be a list.")
    if not isinstance(getattr(self, "cmd_files", []), list):
        self.bld.fatal("keyword argument 'cmd_files' must be a list.")
    # now we sure to have at least lists or empty
    self.env.CMD_FILES = getattr(self, "cmd_files", []) + getattr(
        self.env, "CMD_FILES", []
    )
    if not self.env.CMD_FILES:
        return
    if not all(os.path.isabs(i) for i in self.env.CMD_FILES):
        self.bld.fatal(
            "Keyword argument 'cmd_files' only accepts absolute paths (use "
            "'ctx.path.find_node('xyz.txt').abspath()' in the list."
        )
    self.env.append_unique(
        "CMD_FILES_HASH", [Utils.h_file(i) for i in self.env.CMD_FILES]
    )


class search_swi(Task.Task):  # pylint: disable=invalid-name
    """Searches for swi aliases based on a regular expression"""

    color = "BLUE"

    after = ["hcg_compiler", "create_version_source"]

    swi_regex = re.compile(
        r"#\s{0,}pragma\s{0,}SWI_ALIAS\s{0,}\(\s{0,}([a-zA-Z0-9_]*)\s{0,},\s{0,}(\d)\s{0,}\)[;]?"
    )

    def run(self):
        """does the search check"""
        txt = self.inputs[0].read()
        swi_functions = [
            {"c-name": x.group(1), "entry": x.group(2)}
            for x in search_swi.swi_regex.finditer(txt)
        ]
        if not swi_functions:
            swi_functions = []
        info = json.dumps(
            {"file:": self.inputs[0].relpath(), "functions": swi_functions},
            indent=4,
        )
        self.outputs[0].write(info + os.linesep)

    def keyword(self):
        """displayed keyword when this check is run"""
        return "Searching for swi aliases"


@TaskGen.feature("swi-check")
@TaskGen.after_method("process_source")
def get_swi_aliases(self):
    """Find all swi aliases"""
    self.swi_tasks = []
    for i in self.files:
        self.swi_tasks.append(
            self.create_task("search_swi", i, i.change_ext(f"{i.suffix()}.swi.json"))
        )


class print_swi(Task.Task):  # pylint: disable=invalid-name
    """gathers all swi information in one file"""

    after = ["search_swi"]

    asm_regex = re.compile(r"(\.word)\s{1,}([a-zA-Z0-9_]*)(.*)")

    def run(self):
        """combines all swi information in one file"""
        # get jumptable information from asm file
        txt = self.inputs[0].read().splitlines()
        found = False
        table_entry = 0
        asm_table = []
        for i, val in enumerate(txt):
            if val.strip() == "jumpTable":
                found = True
                continue
            if found:
                if val.strip() == ".endasmfunc":
                    break
                match = print_swi.asm_regex.match(val.strip())
                if match:
                    asm_table.append((table_entry, match.group(2)))
                    table_entry += 1
        all_swi_functions = []
        for i in self.inputs[1:]:
            info = json.loads(i.read())
            if info.get("functions", None):
                all_swi_functions.append(info)
        for i in all_swi_functions:
            for j in i["functions"]:
                j["asm-function"] = asm_table[int(j["entry"])][1]
        all_info = json.dumps(
            all_swi_functions,
            indent=4,
        )
        self.outputs[0].write(all_info + os.linesep)


@TaskGen.feature("swi-check")
@TaskGen.after_method("process_source")
def print_swi_aliases(self):
    """Find all swi aliases"""
    src = [self.jump_table_file] + [x.outputs[0] for x in self.swi_tasks]
    self.create_task("print_swi", src=src, tgt=self.path.find_or_declare("swi.json"))


@conf
def find_armcl(conf):  # pylint: disable-msg=redefined-outer-name
    """configures the compiler, determines the compiler version, and sets the
    default include paths."""
    cc = conf.find_program(["armcl"], var="CC")  # pylint: disable-msg=invalid-name
    conf.env.CC_NAME = "cgt"
    cc_path = pathlib.Path(cc[0])
    conf.env.append_unique(
        "INCLUDES", os.path.join(cc_path.parent.parent.absolute(), "include")
    )
    conf.env.append_unique(
        "STLIBPATH", os.path.join(cc_path.parent.parent.absolute(), "lib")
    )
    conf.find_program(["armcl"], var="LINK_CC")
    cmd = Utils.subst_vars("${CC} --compiler_revision", conf.env).split(" ")
    std_out, std_err = conf.cmd_and_log(cmd, output=Context.BOTH)
    if std_err:
        conf.fatal(f"Could not successfully run '--compiler_revision' on {cc}")
    conf.env.CC_VERSION = std_out.strip()
    cmd = Utils.subst_vars("${CC} -version", conf.env).split(" ")
    std_out, std_err = conf.cmd_and_log(cmd, output=Context.BOTH)
    if std_err:
        conf.fatal(f"Could not successfully run '-version' on {cc}")
    version_pattern = re.compile(r"(v\d{1,}\.\d{1,}\.\d{1,}\.(LTS|STS))")
    for line in std_out.splitlines():
        full_ver = version_pattern.search(line)
        if full_ver:
            conf.env.CC_VERSION_FULL = full_ver.group(1)
            break
    if not conf.env.CC_VERSION or not conf.env.CC_VERSION_FULL:
        conf.fatal("Could not determine compiler version")


@conf
def find_armar(conf):  # pylint: disable-msg=redefined-outer-name
    """configures the archive tool"""
    conf.find_program(["armar"], var="AR")


@conf
def cgt_flags(conf):  # pylint: disable-msg=redefined-outer-name
    """sets flags and related configuration options of the compiler."""
    env = conf.env
    env.DEST_BIN_FMT = "elf"
    env.AR_TGT_F = ["rq"]
    env.CC_COMPILE_ONLY = ["--compile_only"]
    env.CC_TGT_F = ["--output_file="]
    env.CCLINK_TGT_F = ["--output_file="]
    env.RUN_LINKER = ["-qq", "--run_linker"]
    env.DEFINES_ST = "-D%s"
    env.CMD_FILES_ST = "--cmd_file=%s"
    env.LIB_ST = "--library=lib%s.a"
    env.TARGETLIB_ST = "--library=%s.lib"
    env.LIBPATH_ST = "--search_path=%s"
    env.STLIB_ST = "--library=lib%s.a"
    env.STLIBPATH_ST = "--search_path=%s"
    env.CPPPATH_ST = "--include_path=%s"
    env.cprogram_PATTERN = "%s"
    env.cstlib_PATTERN = "lib%s.a"
    env.MAP_FILE = "--map_file="
    env.XML_LINK_INFO = "--xml_link_info="
    env.OBJ_DIRECTORY = "--obj_directory="
    env.ASM_DIRECTORY = "--asm_directory="
    env.PPO = "--preproc_only"
    env.PPA = "--preproc_with_compile"
    env.PPM = "--preproc_macros"
    env.PPI = "--preproc_includes"
    env.PPD = "--preproc_dependency"
    conf.env.ARMSIZE_OPTS = [
        "--common",
        "--arch=arm",
        "--format=berkeley",
        "--totals",
    ]


def configure(conf):  # pylint: disable-msg=redefined-outer-name
    """configuration step of the TI ARM CGT compiler tool"""
    conf.load_special_tools("c_*.py")
    conf.start_msg("Checking for TI ARM CGT compiler and tools")
    conf.load_cc_options()
    conf.find_armcl()
    conf.find_armar()
    conf.find_arm_tools()
    conf.cgt_flags()
    conf.link_add_flags()
    conf.env.COMPILER_BUILTIN_DEFINES_FILE = [
        conf.root.find_node(conf.get_defines()).abspath()
    ]
    conf.env.DEST_OS = ["EMBEDDED"]
    conf.env.COMPILER_CC = "ti_arm_cgt"
    conf.end_msg(conf.env.get_flat("CC"))
    conf.load("f_hcg", tooldir=TOOL_DIR)
