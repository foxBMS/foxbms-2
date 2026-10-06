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

# cspell:ignore armnm

"""Symbol listing from compiled object and linked files using the nm utility.

This waf tool runs the ``armnm`` symbol listing utility on compiled object
files and linked executables/libraries to produce human-readable symbol tables.
The output is written to ``.nm.log`` files alongside the original artifacts.
Symbol listing is an optional post-processing step controlled by the
``--run-nm`` command-line option.

The tool hooks into task generators with the ``c``, ``cstlib``, or ``cprogram``
features and creates an :class:`nm` task for each compiled object file and
the final linked output.

Usage
-----
1. The tool is loaded and configured by :func:`waf_tools.ti_arm_cgt.find_armnm`
   and :func:`waf_tools.ti_arm_cgt.options`::

    # in ti_arm_cgt.py
    @conf
    def find_armnm(ctx):
        ctx.load("nm", tooldir=TOOL_DIR)

    # in ti_arm_cgt.py
    def options(opt):
        opt.load("nm", tooldir=TOOL_DIR)

2. Run the build with the ``--run-nm`` flag to generate symbol listings::

    waf build_app_ti_arm_cgt --run-nm

3. Symbol listing files are written next to their corresponding object or
   linked files with the suffix ``.nm.log``.

Dependencies
------------
- TI ``armnm`` utility (``NM`` environment variable)
- TI ARM CGT toolchain (``CC_NAME == "armcl"``)
"""

from typing import Literal

from waflib import Task, TaskGen
from waflib.Configure import ConfigurationContext
from waflib.Options import OptionsContext


class nm(Task.Task):
    """Task that runs the ``NM`` symbol listing tool on a single file.

    Executes ``NM`` with the configured flags on an input object or linked
    file and writes the symbol table to a ``.nm.log`` output file. The
    output file path is passed via the ``NM_TGT_F`` prefix flag.

    Environment variables used:
        - **NM**: Path to the ``armnm`` executable.
        - **NMFLAGS**: Additional flags passed to ``armnm``.
        - **NM_TGT_F**: Output file flag prefix.
    """

    color = "PINK"
    run_str = "${NM} ${NMFLAGS} ${NM_TGT_F}${TGT} ${SRC}"

    def keyword(self) -> Literal["Processing"]:  # noqa: D102
        return "Processing"


@TaskGen.feature("c", "cstlib", "cprogram")
@TaskGen.after_method("process_source")
def make_nm_task_objects(self: TaskGen.task_gen) -> None:
    """Create nm tasks for compiled object files and linked output files.

    Iterates over all compiled tasks and the link task on the task generator,
    and creates an :class:`nm` task for each output file. Each generated task
    writes its output to a file with a ``.nm.log`` suffix.

    Tasks are only created when the ``--run-nm`` option is set and ``NM``
    is available in the environment.
    """
    if not getattr(self.bld, "options", None) or not self.bld.options.run_nm:
        return
    if not self.bld.env.NM:
        return
    tasks = list(getattr(self, "compiled_tasks", []))
    link_task = getattr(self, "link_task", None)
    if link_task:
        tasks.append(link_task)
    for task in tasks:
        src = task.outputs
        tgt = [i.change_ext(".nm.log") for i in src]
        self.create_task("nm", src=src, tgt=tgt)


def options(opt: OptionsContext) -> None:
    """Register command-line options for the nm tool."""
    opt.add_option(
        "--run-nm",
        action="store_true",
        default=False,
        help="Run nm tool on object and linked files.",
    )


def configure(ctx: ConfigurationContext) -> None:
    """Configure the nm symbol listing tool.

    The following environment variables are set (if not already present):
        - **NM**: Path to the ``armnm`` executable.
        - **NMFLAGS**: Flags passed to ``armnm``.
    """
    if not ctx.env.NM:
        ctx.find_program("nm", var="NM")
    if not ctx.env.NMFLAGS:
        ctx.env.NMFLAGS = []
