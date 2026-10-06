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

"""Implements a waf tool to use MATLAB® code generators (https://www.mathworks.com/)"""

import os
import threading
from pathlib import Path
from typing import Literal

from waflib import Task
from waflib.Configure import ConfigurationContext
from waflib.Node import Node
from waflib.TaskGen import extension, task_gen

g_lock = threading.Lock()


@extension(".m")
def process_m_files(self: task_gen, node: Node) -> None:
    """Process matlab source files."""
    if not self.env.MATLAB:
        self.bld.fatal("MATLAB® program not available - cannot process .m files")
    self.create_task("compile_matlab", node)


class compile_matlab(Task.Task):
    """Compile MATLAB® source files to c/h files."""

    color = "PINK"
    quiet = True
    before = ["cstlib"]
    ext_out = [".c", ".h"]

    def process_generated_sources(self, _: Node) -> None:
        """Build the dependencies for the generated sources."""
        self.outputs = self.generator.path.ant_glob(["*.c", "*.h"])
        self.generator.bld.raw_deps[self.uid()] = [self.signature()] + self.outputs
        with g_lock:
            self.add_c_tasks(self.outputs)

    run_str = (
        "${MATLAB} ${MATLAB_FLAGS} ${MATLAB_SRC_F:SRC}; ${MATLAB_POST_FLAGS};",
        process_generated_sources,
    )

    def add_c_tasks(self, outputs: list[Node]) -> None:
        """Pass created c files to the c compiler and add them as dependencies
        to the link task.
        """
        self.more_tasks = []  # pylint: disable=attribute-defined-outside-init
        for node in outputs:
            if node.name.endswith(".h"):
                continue
            tsk = self.generator.create_compiled_task("c", node)
            self.more_tasks.append(tsk)

            tsk.env.append_value("INCPATHS", [node.parent.abspath()])

            if getattr(self.generator, "link_task", None):
                run_after = self.generator.link_task.run_after | {tsk}
                self.generator.link_task.run_after = run_after
                self.generator.link_task.inputs.append(tsk.outputs[0])
        self.generator.link_task.inputs.sort(key=lambda x: x.abspath())

    def runnable_status(self) -> Literal[-1, -2, -3, -4]:
        """Check if the task needs to be executed."""
        ret = super().runnable_status()
        if ret == Task.SKIP_ME:
            lst = self.generator.bld.raw_deps[self.uid()]
            if lst[0] != self.signature():
                return Task.RUN_ME

            nodes = lst[1:]
            for x in nodes:
                try:
                    Path(x.abspath()).stat()
                except (FileNotFoundError, PermissionError, OSError):
                    return Task.RUN_ME

            nodes = lst[1:]
            self.set_outputs(nodes)
            with g_lock:
                self.add_c_tasks(nodes)

        return ret


def configure(ctx: ConfigurationContext) -> None:
    """Configuration step of the MATLAB® code generators.

    #. search for the MATLAB® program
    #. add the MATLAB® include directory to the include path
    #. register the MATLAB® compiler flags and source file format
    """
    ctx.start_msg("Checking for program 'matlab'")
    try:
        ctx.find_program("matlab", mandatory=True)
    except ctx.errors.ConfigurationError:
        ctx.end_msg(
            "not found - MATLAB® features will not be available", color="YELLOW"
        )
        return
    ctx.end_msg(ctx.env.get_flat("MATLAB") or "not found")

    ctx.env.MATLAB_FLAGS = ["-batch", "-noFigureWindows"]
    ctx.env.MATLAB_SRC_F = "run('%s')"
    ctx.env.MATLAB_POST_FLAGS = ["exit"]

    matlab_root = Path(ctx.env.get_flat("MATLAB")).parents[1]
    matlab_includes = os.path.join(matlab_root, "extern", "include")
    ctx.env.append_unique("INCLUDES", matlab_includes)
