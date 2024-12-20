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

"""Command line interface definition for running scripts"""

import click

from ..cmd_script import script_impl

CONTEXT_SETTINGS = {
    "help_option_names": ["--dummy"],
    "ignore_unknown_options": True,
}


@click.command("run-script", context_settings=CONTEXT_SETTINGS)
@click.option(
    "--cwd",
    type=click.Path(exists=True, file_okay=False, dir_okay=True),
    is_eager=True,
    help="Directory where the script is run from.",
)
@click.argument("script_args", nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def run_script(
    ctx: click.Context,
    cwd: str,
    script_args: tuple[str],
) -> None:
    """Run the provided script."""
    ret = script_impl.run_python_script(
        list(script_args), cwd=cwd, stdout=None, stderr=None
    )
    ctx.exit(ret.returncode)


@click.command("run-program", context_settings=CONTEXT_SETTINGS)
@click.option(
    "--cwd",
    type=click.Path(exists=True, file_okay=False, dir_okay=True),
    is_eager=True,
    help="Directory where the script is run from.",
)
@click.argument("program_args", nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def run_program(
    ctx: click.Context,
    cwd: str,
    program_args: tuple[str],
) -> None:
    """Run the provided program."""
    ret = script_impl.run_program(list(program_args), cwd=cwd, stdout=None, stderr=None)
    ctx.exit(ret.returncode)
