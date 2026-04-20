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

"""Click commands for querying the battery-cell database."""

from pathlib import Path

import click

from ..cmd_db.db_impl import db_list, db_show
from ..helpers.click_helpers import HELP_NAMES, verbosity_option


@click.command("list")
@click.argument(
    "db-root",
    nargs=1,
    type=click.Path(exists=True, file_okay=True, dir_okay=True, path_type=Path),
)
@verbosity_option
@click.pass_context
def cmd_list(ctx: click.Context, db_root: Path, verbose: int = 0) -> None:
    """List available cells in a database root."""
    db_list(db_root)
    ctx.exit(0)


@click.command("show")
@click.argument(
    "db-root",
    nargs=1,
    type=click.Path(exists=True, file_okay=True, dir_okay=True, path_type=Path),
)
@click.option(
    "-c",
    "--cell-id",
    type=str,
    required=True,
    help="The cell identifier as a combination of the cell manufacturer "
    "and the cell name. Example LG Chem-INR18650-MJ1",
)
@verbosity_option
@click.pass_context
def cmd_show(ctx: click.Context, db_root: Path, cell_id: str, verbose: int = 0) -> None:
    """Show details for one cell identifier."""
    db_show(db_root, cell_id)
    ctx.exit(0)


@click.group(context_settings=HELP_NAMES)
def db() -> None:
    """Database command group entry point."""


db.add_command(cmd_list)
db.add_command(cmd_show)
