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


# cspell:ignore BINFLAGS BINFINALFLAGS

"""Generation of raw binary files from linked ELF outputs.

This waf tool converts a linked ELF executable into a raw binary file using
the TI ``tiobj2bin`` utility (``OBJ2BIN``). The binary generation task is
automatically appended after the link step for task generators with the
``cprogram`` feature when the TI ARM CGT toolchain is active.

Usage
-----
1. The tool is loaded and configured by :func:`waf_tools.ti_arm_cgt.find_bingen`::

    # in ti_arm_cgt.py
    @conf
    def find_bingen(ctx):
        ctx.load("bingen", tooldir=TOOL_DIR)

2. The ``add_bingen_task`` feature hook runs automatically after linking
   for any task generator that uses the ``cprogram`` feature and the
   ``armcl`` compiler.

Dependencies
------------
- TI ``tiobj2bin`` utility (``OBJ2BIN`` environment variable)
- TI ARM CGT toolchain (``CC_NAME == "armcl"``)
"""

from waflib import Task, TaskGen
from waflib.Configure import ConfigurationContext


class bingen(Task.Task):
    """Task that converts a linked ELF file into a raw binary.

    Executes ``OBJ2BIN`` with the configured flags to produce a ``.bin``
    file from the linker output.

    Environment variables used:
        - **OBJ2BIN**: Path to the ``tiobj2bin`` executable.
        - **OBJ2BINFLAGS**: Flags passed before input/output arguments.
        - **OBJ2BINFINALFLAGS**: Flags appended after input/output arguments.
    """

    color = "PINK"
    run_str = "${OBJ2BIN} ${OBJ2BINFLAGS} ${SRC} ${TGT} ${OBJ2BINFINALFLAGS}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_bingen_task(self: TaskGen.task_gen) -> None:
    """Append a binary-generation task after the link step.

    Creates a :class:`bingen` task that converts the linked ELF output into
    a raw ``.bin`` file.
    """
    if not hasattr(self, "link_task"):
        return
    if not self.env.OBJ2BIN:
        return
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    src = self.link_task.outputs[0]
    tgt = self.link_task.outputs[0].change_ext(".bin")
    self.bingen = self.create_task("bingen", src=src, tgt=tgt)


def configure(ctx: ConfigurationContext) -> None:
    """Configure the binary generation tool.

    Locates the ``OBJ2BIN`` executable and initializes the flag variables.

    The following environment variables are set:
        - **OBJ2BIN**: Path to the ``tiobj2bin`` executable.
        - **OBJ2BINFLAGS**: Pre-source flags.
        - **OBJ2BINFINALFLAGS**: Post-target flags.
    """
    if not ctx.env.OBJ2BIN:
        ctx.find_program("OBJ2BIN", var="OBJ2BIN")
    if not ctx.env.OBJ2BINFLAGS:
        ctx.env.OBJ2BINFLAGS = []
    if not ctx.env.OBJ2BINFINALFLAGS:
        ctx.env.OBJ2BINFINALFLAGS = []
