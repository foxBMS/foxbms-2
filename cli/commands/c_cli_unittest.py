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

"""Command line interface definition for CLI self tests"""

import click

from ..cmd_cli_unittest import cli_unittest_impl
from ..helpers.click_helpers import (
    HELP_NAMES,
    IGNORE_UNKNOWN_OPTIONS,
    echo,
    recho,
    verbosity_option,
)

CONTEXT_SETTINGS = HELP_NAMES | IGNORE_UNKNOWN_OPTIONS


@click.group(context_settings=CONTEXT_SETTINGS, invoke_without_command=True)
@click.option(
    "-s",
    "--self-test",
    default=False,
    is_flag=True,
    help="Run foxBMS CLI self test.",
)
@click.option(
    "--coverage-report",
    default=False,
    is_flag=True,
    help="Create a coverage report (requires '--self-test').",
)
@click.argument("unittest_args", nargs=-1, type=click.UNPROCESSED)
@verbosity_option
@click.pass_context
def cli_unittest(
    ctx: click.Context,
    self_test: bool,
    coverage_report: bool,
    unittest_args: tuple[str],
    verbose: int = 0,
) -> None:
    """Run unit-tests on the CLI tool itself."""
    err = 0
    if coverage_report and not self_test:
        recho("'--coverage-report' requires '--self-test'.")
        err = 1
    if err:
        pass
    elif self_test:
        err = cli_unittest_impl.run_script_tests(coverage_report, verbose).returncode
    elif unittest_args:
        args = list(unittest_args)
        err = cli_unittest_impl.run_unittest_module(args).returncode
    else:  # no subcommand
        echo(cli_unittest.get_help(ctx))
    ctx.exit(err)
