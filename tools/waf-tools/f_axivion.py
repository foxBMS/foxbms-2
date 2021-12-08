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

"""Implements a waf tool to use Axivion cafeCC compiler

For information on Axivion see https://www.axivion.com/.
"""

import os
from waflib import Logs
from waflib.Tools.ccroot import link_task
from waflib import TaskGen
from waflib.TaskGen import taskgen_method
from waflib.Configure import conf

import f_ti_arm_helper  # pylint: disable=unused-import
import f_ti_arm_tools  # pylint: disable=unused-import
import f_ti_color_arm_cgt  # pylint: disable=unused-import


def configure(configure_context):
    """Searches the Axivion cafeCC compiler"""
    if os.getenv("AXIVION", None):
        configure_context.find_program("cafecc", var="AXIVION_CC")


@conf
def patch_for_axivion_build(self, bld):  # pylint: disable=unused-argument
    """Patches the Axivion build (i.e. remove tasks that do not apply)."""
    if not bld.env.AXIVION_CC:
        Logs.warn("Axivion tools not available.")
        return

    os.environ["BAUHAUS_CONFIG"] = os.path.join(bld.path.abspath(), "tests", "axivion")
    bld.env.CC = bld.env.AXIVION_CC
    bld.env.LINK_CC = bld.env.AXIVION_CC
    bld.env.append_unique("LINKFLAGS", ["-echo"])  # show real linker call
    bld.env.LINKFLAGS.extend(["-larg", "--show_plan"])
    bld.env.append_unique("CFLAGS", ["-B", self.path.abspath()])

    @TaskGen.extension(".c")
    def c_hook(self, node):
        return self.create_compiled_task_c("c", node)

    @taskgen_method
    def create_compiled_task_c(self, t_name, node):
        if self.idx > 1:
            Logs.warn(
                "Consistency of .aux, .crl and .rl output files can not be guaranteed."
            )
        out_obj = f"{node.name}.{self.idx}.obj"
        task = self.create_task(
            t_name,
            src=node,
            tgt=[
                node.parent.find_or_declare(out_obj),
            ],
        )
        try:
            self.compiled_tasks.append(task)
        except AttributeError:
            self.compiled_tasks = [task]
        return task

    class cprogram(link_task):  # pylint: disable=invalid-name,unused-variable
        """class to run the Axivion CC compiler in linker mode"""

        run_str = (
            "${LINK_CC} ${CFLAGS} ${RUN_LINKER} ${LINKFLAGS} "
            "${CCLINK_TGT_F}${TGT[0].abspath()} ${SRC} ${LINKER_SCRIPT} "
            "${LIBPATH_ST:LIBPATH} ${STLIBPATH_ST:STLIBPATH} "
            "-Wl,--start-group ${LIB_ST:LIB} ${STLIB_ST:STLIB} "
            "-Wl,--end-group ${LDFLAGS}"
        )
        ext_out = [".elf"]
        vars = ["LINKDEPS"]

        def keyword(self):
            return "Linking"

    @conf
    def tiprogram(bld, *k, **kw):  # pylint: disable=unused-variable
        if "linker_script" not in kw:
            bld.fatal("linker script missing")
        bld.env.LINKER_SCRIPT = kw["linker_script"].abspath()
        kw["features"] = "c cprogram"
        tgt_elf = bld.path.find_or_declare(f"{kw['target']}.{bld.env.DEST_BIN_FMT}")
        kw["target"] = [tgt_elf]
        bld.add_manual_dependency(tgt_elf, kw["linker_script"])
        return bld(*k, **kw)

    @TaskGen.feature("cprogram")
    def add_bingen_task(self):  # pylint: disable=unused-argument,unused-variable
        return

    @TaskGen.feature("c", "cprogram")
    def process_sizes(self):  # pylint: disable=unused-argument,unused-variable
        return

    @TaskGen.feature("c", "cprogram")
    def process_nm(self):  # pylint: disable=unused-argument,unused-variable
        return

    @TaskGen.feature("c")
    @TaskGen.after("c_pp")
    def remove_stuff_from_pp(self):  # pylint: disable=unused-argument,unused-variable
        return

    @TaskGen.feature("c", "db_check")
    @TaskGen.after_method("process_source")
    def check_data_base_init(self):  # pylint: disable=unused-argument,unused-variable
        return
