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

"""Integration of Gcov and gcovr into the Waf build system.

This waf tool adds code-coverage support to unit test builds. When the
``--coverage`` command-line flag is passed, the tool injects GCC's
``--coverage`` compiler and linker flags into tasks that carry the ``test``
feature and schedules a post-build step that invokes *gcovr* to produce a
coverage report.

Usage
-----
1. Load the tool during ``options`` and ``configure``::

    def options(opt):
        opt.load("gcov", tooldir=TOOLDIR)

    def configure(ctx):
        ctx.load("gcov", tooldir=TOOLDIR)

2. Run the build with coverage enabled::

    waf build_(app|bootloader)_unit_test_gcc --coverage

Dependencies
------------
- GCC
- gcovr (https://gcovr.com/)
"""

from waflib import Logs
from waflib.Build import BuildContext
from waflib.Configure import ConfigurationContext
from waflib.Options import OptionsContext
from waflib.TaskGen import feature, task_gen


def options(opt: OptionsContext) -> None:
    """Register the ``--coverage`` command-line option."""
    opt.add_option(
        "--coverage",
        action="store_true",
        default=False,
        help="activating code coverage with gcov",
    )


def test_stdout_stderr(bld: BuildContext) -> None:
    """Pretty-prints stdout/stderr of unit test results to the console."""
    results = getattr(bld, "utest_results", [])  # cspell:ignore utest
    if not results:
        return
    if Logs.verbose or any(result.exit_code for result in results):
        Logs.pprint("NORMAL", "\nUnit test result details:")
    for result in results:
        default_color = "RED" if result.exit_code else "GREEN"
        if result.exit_code or Logs.verbose:
            msg = (
                f"  {result.test_path}: "
                f"{Logs.colors_lst[default_color]}{result.exit_code}"
                f"{Logs.colors_lst['NORMAL']}"
            )
            Logs.pprint("NORMAL", msg)
            if result.out:
                tmp = "\n    ".join(result.out.decode("utf-8").splitlines())
                msg = f"    {tmp}"
                Logs.pprint("NORMAL", msg)
            if result.err:
                tmp = "\n    ".join(result.err.decode("utf-8").splitlines())
                msg = f"    {tmp}"
                Logs.pprint("RED", msg)


def configure(ctx: ConfigurationContext) -> None:
    """Configure the build environment for coverage.

    Sets ``CFLAGS_COVERAGE`` and ``LINKFLAGS_COVERAGE`` to ``--coverage``
    and locates the ``gcovr`` executable.
    """
    ctx.env.CFLAGS_COVERAGE = ["--coverage"]
    ctx.env.LINKFLAGS_COVERAGE = ["--coverage"]
    ctx.find_program("gcovr", var="GCOVR")


def coverage(bld: BuildContext) -> None:
    """Executes *gcovr* with the configuration file stored in ``bld.gcovr_cfg``."""
    bld.bldnode.make_node("coverage").mkdir()
    cmd = bld.env.GCOVR + ["--config", bld.gcovr_cfg.abspath()]
    Logs.pprint("NORMAL", "Generating coverage report...\n")
    ret = bld.exec_command(cmd)
    if ret:
        msg = f"gcovr failed with exit code {ret}"
        bld.fatal(msg)


@feature("test")
def gcov(self: task_gen) -> None:
    """Register a post-build coverage step for test targets, if coverage is enabled."""
    if not self.bld.options.coverage:
        return

    if getattr(self.bld, "gcov_added", None):
        return
    self.bld.gcov_added = True
    self.bld.add_post_fun(coverage)
