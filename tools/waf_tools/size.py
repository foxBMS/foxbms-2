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

# cspell:ignore SIZEFLAGS, armsize

"""Section size reporting from compiled object and linked files using the size utility.

This waf tool runs the ``armsize`` utility on compiled object files and
linked executables/libraries to produce section size reports (text, data, bss).
The output is written to ``size.log`` files alongside the original
artifacts. Size reporting is an optional post-processing step controlled by
the ``--run-size`` command-line option.

The tool hooks into task generators with the ``c``, ``cstlib``, or ``cprogram``
features and creates a :class:`size` task for each compiled object file and
the final linked output.

Usage
-----
1. The tool is loaded and configured by :func:`waf_tools.ti_arm_cgt.find_armsize`
   and :func:`waf_tools.ti_arm_cgt.options`::

    # in ti_arm_cgt.py
    @conf
    def find_armsize(ctx):
        ctx.load("size", tooldir=TOOL_DIR)

    # in ti_arm_cgt.py
    def options(opt):
        opt.load("size", tooldir=TOOL_DIR)

2. Run the build with the ``--run-size`` flag to generate size reports::

    waf build_app_ti_arm_cgt --run-size

3. Size report files are written next to their corresponding object or linked
   files with the suffix ``.size.log``.

Dependencies
------------
- TI ``armsize`` utility (``SIZE`` environment variable)
- TI ARM CGT toolchain (``CC_NAME == "armcl"``)
"""

from typing import Literal

from waflib import Context, Logs, Task, TaskGen
from waflib.Configure import ConfigurationContext
from waflib.Options import OptionsContext


class size(Task.Task):
    """Task that runs the ``SIZE`` tool on a single file and writes the report.

    Environment variables used:
        - **SIZE**: Path to the ``armsize`` executable.
        - **SIZEFLAGS**: Additional flags passed to ``armsize``.
    """

    color = "PINK"
    vars = ["SIZE", "SIZEFLAGS"]

    def run(self) -> int:
        """Execute the ``SIZE`` command and write its stdout to the target file.

        Constructs the command line from ``SIZE`` and ``SIZEFLAGS``
        environment variables plus the input path, executes it via
        ``bld.cmd_and_log``, and writes the captured stdout (containing the
        section size table) to the target ``.size.log`` file.

        Returns:
            0 on success, 1 if ``armsize`` reports an error on stderr.
        """
        cmd = (
            self.generator.env.SIZE
            + self.generator.env.SIZEFLAGS
            + [self.inputs[0].abspath()]
        )
        outfile = self.outputs[0].path_from(self.generator.path)
        env = self.env.env or None
        cwd = self.generator.bld.path.get_bld().abspath()
        out, err = self.generator.bld.cmd_and_log(
            cmd, output=Context.BOTH, quiet=Context.STDOUT, env=env, cwd=cwd
        )
        self.generator.path.make_node(outfile).write(out)
        if err:
            Logs.error(err)
            return 1
        return 0

    def keyword(self) -> Literal["Processing"]:  # noqa: D102
        return "Processing"


@TaskGen.feature("c", "cstlib", "cprogram")
@TaskGen.after_method("process_source")
def make_size_task_objects(self: TaskGen.task_gen) -> None:
    """Create size tasks for compiled object files and linked output files.

    Iterates over all compiled tasks and the link task on the task generator,
    and creates a :class:`size` task for each output file. Each generated
    task writes its output to a file with a ``.size.log`` suffix.

    Tasks are only created when the ``--run-size`` option is set and
    ``SIZE`` is available in the environment.
    """
    if not getattr(self.bld, "options", None) or not self.bld.options.run_size:
        return
    if not self.env.SIZE:
        return
    tasks = list(getattr(self, "compiled_tasks", []))
    link_task = getattr(self, "link_task", None)
    if link_task:
        tasks.append(link_task)
    for task in tasks:
        src = task.outputs
        tgt = [i.change_ext(".size.log") for i in src]
        self.create_task("size", src, tgt)


def options(opt: OptionsContext) -> None:
    """Register command-line options for the size tool."""
    opt.add_option(
        "--run-size",
        action="store_true",
        default=False,
        help="Run size tool on object and linked files.",
    )


def configure(ctx: ConfigurationContext) -> None:
    """Configure the size section reporting tool.

    The following environment variables are set:
        - **SIZE**: Path to the ``armsize`` executable.
        - **SIZEFLAGS**: Flags passed to ``armsize``.
    """
    if not ctx.env.SIZE:
        ctx.find_program("size", var="SIZE")
    if not ctx.env.SIZEFLAGS:
        ctx.env.SIZEFLAGS = []
