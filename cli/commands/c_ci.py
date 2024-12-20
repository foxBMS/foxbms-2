#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Command line interface definition for CI tools."""

from pathlib import Path
from typing import get_args

import click

from ..cmd_ci.check_ci_config import check_ci_config
from ..cmd_ci.check_coverage import check_coverage
from ..cmd_ci.create_readme import create_readme
from ..cmd_cli_unittest.cli_unittest_constants import CliUnitTestVariants
from ..cmd_embedded_ut.embedded_ut_constants import EmbeddedUnitTestVariants

CONTEXT_SETTINGS = {
    "help_option_names": ["-h", "--help"],
    "ignore_unknown_options": True,
}


@click.group(context_settings=CONTEXT_SETTINGS, hidden=True)
def ci() -> None:
    """Continuous Integration related tests

    Most of these scripts and tools will **not** work outside Fraunhofer IISB."""


@ci.command("create-readme")
@click.pass_context
def cmd_create_readme(
    ctx: click.Context,
) -> None:
    """Create the CI readme."""
    ctx.exit(create_readme())


@ci.command("check-ci-config")
@click.pass_context
def cmd_check_ci_config(
    ctx: click.Context,
) -> None:
    """Validate the CI configuration file."""
    ctx.exit(check_ci_config())


@ci.command("check-coverage")
@click.option(
    "--project",
    type=click.Choice(
        get_args(EmbeddedUnitTestVariants) + get_args(CliUnitTestVariants)
    ),
    default="app",
)
@click.pass_context
def cmd_check_coverage(
    ctx: click.Context,
    project: str,
) -> None:
    """Check the unit test coverage."""
    ctx.exit(check_coverage(project))


@ci.command("path-shall-not-exist")
@click.argument(
    "path",
    type=click.Path(exists=False, dir_okay=True, file_okay=True, path_type=Path),
)
@click.pass_context
def cmd_path_shall_not_exist(ctx: click.Context, path: Path) -> None:
    """Ensure that a directory or file does not exist."""
    if path.exists():
        click.echo(f"Path '{path}' exists.", err=True)
        ctx.exit(1)
    ctx.exit(0)
