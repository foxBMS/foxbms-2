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

"""Click commands for miscellaneous foxBMS repository utilities."""

from pathlib import Path

import click

from ..cmd_misc.check_repository_depth import check_repository_depth
from ..cmd_misc.list_prefixes import get_prefixes
from ..cmd_misc.run_uncrustify import lint_freertos
from ..cmd_misc.verify_checksums import verify
from ..helpers.click_helpers import HELP_NAMES, echo, recho, verbosity_option

CONTEXT_SETTINGS = HELP_NAMES | {"ignore_unknown_options": True}


@click.group(context_settings=CONTEXT_SETTINGS, invoke_without_command=True)
@click.option(
    "--list-prefixes",
    is_flag=True,
    default=False,
    help="List all @prefix markers found in C source files.",
)
@click.pass_context
def misc(ctx: click.Context, list_prefixes: bool) -> None:
    """Miscellaneous command group entry point.

    Args:
        ctx: Active Click context.
        list_prefixes: If ``True``, print all discovered prefixes.
    """
    if list_prefixes:
        for prefix in get_prefixes():
            echo(prefix)
        ctx.exit(0)

    if not ctx.invoked_subcommand:
        echo(misc.get_help(ctx))


@misc.command("check-repository-depth")
@verbosity_option
@click.pass_context
def cmd_check_repository_depth(ctx: click.Context, verbose: int = 0) -> None:
    """Find the longest path depth in the repository."""
    ctx.exit(check_repository_depth())


@misc.command("verify-checksum")
@click.argument(
    "files",
    nargs=-1,
    is_eager=True,
    type=click.Path(exists=True, dir_okay=True, path_type=Path),
)
@click.argument("known-hash", type=click.STRING)
@verbosity_option
@click.pass_context
def cmd_verify_checksum(
    ctx: click.Context, files: list[Path], known_hash: str, verbose: int = 0
) -> None:
    """Verify checksums of provided files or directories."""
    if not files:
        recho("No files provided.")
        ctx.exit(1)
    ctx.exit(verify(list(files), known_hash))


@misc.command("uncrustify-freertos")
@click.option(
    "--check/--no-check",
    default=True,
    is_flag=True,
    help="Run uncrustify in check or edit mode.",
)
@click.pass_context
def cmd_uncrustify_freertos(ctx: click.Context, check: bool) -> None:
    """Run uncrustify on FreeRTOS sources."""
    ctx.exit(lint_freertos(check))
