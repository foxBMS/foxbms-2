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

"""Integration of the draw.io diagram editor into the Waf build system.

This waf tool configures the drawio-desktop application
(https://github.com/jgraph/drawio-desktop/releases) so that ``.drawio``
diagram files can be exported to SVG format as part of the Waf build process.

The tool registers a file extension handler for ``.drawio`` files. When a
task generator includes ``.drawio`` files, a :class:`drawioToSvg` task is
automatically created that invokes the draw.io CLI to produce SVG output.

.. note::
    The ``.drawio`` source files must not be empty. The drawio-desktop CLI
    cannot export empty files and will fail with ``Error: Export failed: <path>``.

Usage
-----
1. The tool is loaded in :func:`waf_tools.sphinx_build.configure` and
   configured by :func:`find_drawio`::

    # in sphinx_build.py
    def configure(ctx):
        ctx.find_drawio()

2. In a build script, declare a task generator with ``.drawio`` source files::

    def build(bld):
        bld(
            source=bld.path.ant_glob("**/*.drawio"),
        )

Dependencies
------------
- drawio-desktop (``draw.io`` on Windows, ``drawio`` on Linux)
"""

from typing import Literal

from waflib import Logs, Task, TaskGen, Utils
from waflib.Configure import ConfigurationContext, conf
from waflib.Node import Node


class drawioToSvg(Task.Task):
    """Task that converts a ``.drawio`` diagram file to SVG format.

    Executes the drawio-desktop CLI with ``--export --output`` on an input
    ``.drawio`` file and writes the resulting SVG to the build directory.

    A task semaphore limits execution to one instance at a time to avoid
    conflicts on shared profile/disk cache resources.

    Environment variables used:
        - **DRAWIO**: Path to the drawio-desktop executable.
        - **DRAWIO_EXTRA_OPTS**: Additional CLI flags for headless export.
    """

    color = "CYAN"
    run_str = "${DRAWIO} --export --output ${TGT} ${SRC} ${DRAWIO_EXTRA_OPTS}"
    # drawio-desktop CLI is not designed for parallel execution, therefore
    # multiple instances may conflict on shared profile/disk cache.
    # https://github.com/jgraph/drawio-desktop/issues/2247#issuecomment-3536967826
    semaphore = Task.TaskSemaphore(1)

    def exec_command(self, cmd: str | list[str], **kw) -> int:
        """Execute the drawio-desktop command while silencing stdout in non-verbose mode."""
        if not Logs.verbose:
            kw["stdout"] = Utils.subprocess.DEVNULL
        return super().exec_command(cmd, **kw)

    def keyword(self) -> Literal["Compiling"]:  # noqa: D102
        return "Compiling"

    def __str__(self) -> str:
        """Return the source and output paths."""
        return f"{self.inputs[0].relpath()} -> {self.outputs[0].relpath()}"


@TaskGen.extension(".drawio")
def drawio_hook(self: TaskGen.task_gen, node: Node) -> None:
    """Create a :class:`drawioToSvg` task for a ``.drawio`` source node.

    Registered as an extension handler for ``.drawio`` files. When a task
    generator includes a ``.drawio`` source, this hook automatically creates
    a conversion task that produces an SVG file with the same base name.

    Args:
        self: The task generator instance.
        node: The ``.drawio`` source node that triggered this handler.
    """
    out = node.change_ext(".svg")
    self.create_task("drawioToSvg", node, out)


@conf
def find_drawio(ctx: ConfigurationContext) -> None:
    """Find the drawio-desktop executable and set default CLI options.

    The following environment variables are set:
        - **DRAWIO**: Path to the drawio-desktop executable.
        - **DRAWIO_EXTRA_OPTS**: Default CLI flags for headless SVG export.
    """
    if Utils.is_win32:
        ctx.find_program("draw.io", var="DRAWIO")
    else:
        ctx.find_program("drawio", var="DRAWIO")

    ctx.env.DRAWIO_EXTRA_OPTS = [
        "--disable-gpu",
        "--disable-software-rasterizer",
        "--disable-features=DefaultPassthroughCommandDecoder",
    ]
