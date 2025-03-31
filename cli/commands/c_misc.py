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

"""Command line interface definition for miscellaneous fox tools"""

from pathlib import Path

import click

from ..cmd_misc.check_test_files import check_for_test_files
from ..cmd_misc.crc_example import run_crc_build
from ..cmd_misc.doc_example import run_doc_build
from ..cmd_misc.run_uncrustify import lint_freertos
from ..cmd_misc.verify_checksums import verify
from ..helpers.click_helpers import HELP_NAMES, recho, verbosity_option

CONTEXT_SETTINGS = HELP_NAMES | {"ignore_unknown_options": True}


@click.group(context_settings=CONTEXT_SETTINGS)
def misc() -> None:
    """Miscellaneous tools or scripts that did not fit in any other category."""


@misc.command("check-for-test-files")
@verbosity_option
@click.pass_context
def cmd_check_for_test_files(ctx: click.Context, verbose: int = 0) -> None:
    """Check whether all 'cli' files have dedicated test file."""
    ctx.exit(check_for_test_files(verbose).returncode)


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
    """Verify checksum of a list of given paths."""
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
    """Run uncrustify on the FreeRTOS sources in the source tree."""
    ctx.exit(lint_freertos(check))


@misc.command("build-crc-code")
@click.pass_context
def cmd_build_crc_code(ctx: click.Context) -> None:
    """Build the CRC example code."""
    ctx.exit(run_crc_build().returncode)


@misc.command("build-doc-code")
@click.pass_context
def cmd_build_doc_code(ctx: click.Context) -> None:
    """Build the documentation example code."""
    ctx.exit(run_doc_build().returncode)
