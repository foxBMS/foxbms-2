#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a waf tool to configure a Ceedling project to foxBMS specific
needs and builds and runs the tests.

For information on Ceedling see https://github.com/ThrowTheSwitch/Ceedling.
"""

from waflib import Logs, Task, TaskGen, Utils
from waflib.Configure import ConfigurationContext


class ceedling(Task.Task):  # pylint: disable=invalid-name,too-few-public-methods
    """class to run Ceedling"""

    #: str: color in which the command line is displayed in the terminal
    color = "GREEN"

    #: str: Ceedling handles the need for a re-run, so always run this task
    always_run = True

    run_str = "${CEEDLING} ${CEEDLING_OPTIONS} ${CEEDLING_TEST_OPTIONS}"
    """str: string to be interpolated to create the command line to run
    ceedling (ctx.env.CEEDLING_TEST_OPTIONS are applied)."""


# pylint: disable=invalid-name,too-few-public-methods
class ceedling_coverage(Task.Task):
    """Call ceedling with coverage options"""

    #: str: color in which the command line is displayed in the terminal
    color = "GREEN"

    #: str: Ceedling handles the need for a re-run, so always run this task
    always_run = True

    run_str = "${CEEDLING} ${CEEDLING_OPTIONS} ${CEEDLING_COVERAGE_OPTIONS}"
    """str: string to be interpolated to create the command line to run
    ceedling (ctx.env.CEEDLING_COVERAGE_OPTIONS are applied)."""


@TaskGen.feature("ceedling")
def add_ceedling_task(self):
    """Task creator for ceedling. If the coverage option is set, use the
    coverage task, otherwise the standard task without coverage."""
    if self.bld.options.coverage:
        self.create_task("ceedling_coverage")
    else:
        self.create_task("ceedling")


def configure(ctx: ConfigurationContext):
    """configuration step of the Ceedling waf tool:

    - Find required software (ruby, gcc, gcov, gcovr and ceedling)
    - configure a ceedling project"""
    ctx.find_program("ruby", mandatory=False)
    if not ctx.env.RUBY:
        Logs.warn("Ruby is missing.")
    ctx.find_program("gcc", mandatory=False)
    if not ctx.env.GCC:
        Logs.warn("GCC is missing.")
    ctx.find_program("gdb", mandatory=False)
    if not ctx.env.GDB:
        Logs.warn("GDB is missing.")
    ctx.find_program("gcov", mandatory=False)
    if not ctx.env.GCOV:
        Logs.warn("gcov is missing.")
    ctx.find_program("gcovr", mandatory=False)
    if not ctx.env.GCOVR:
        Logs.warn("gcovr is missing.")
    ctx.find_program("ceedling", mandatory=False)
    if not ctx.env.CEEDLING:
        Logs.warn("Ceedling is missing.")
    if not all(
        (
            ctx.env.RUBY,
            ctx.env.GCC,
            ctx.env.GDB,
            ctx.env.GCOV,
            ctx.env.GCOVR,
            ctx.env.CEEDLING,
        )
    ):
        return

    if Utils.is_win32:
        prefix = "C:"
    else:
        prefix = "/opt"
    expected_gem_path = f"{prefix}/foxbms/Ceedling/1.0.1"
    ctx.start_msg("Ceedling Gem directory")
    gem = ctx.root.find_dir(expected_gem_path)
    if not gem:
        ctx.end_msg(False)
        return
    ctx.env.append_unique("GEM_HOME", gem.abspath())
    ctx.end_msg(ctx.env.get_flat("GEM_HOME"))
    ctx.env.append_unique("CEEDLING_OPTIONS", ["--verbosity=normal"])
    ctx.env.append_unique("CEEDLING_TEST_OPTIONS", ["test:all"])
    ctx.env.append_unique("CEEDLING_COVERAGE_OPTIONS", ["gcov:all"])
