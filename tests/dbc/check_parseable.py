#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Ensure that the CAN definition files (symbol and dbc) are parsable."""

import sys
import logging
from pathlib import Path
import git
from git.exc import InvalidGitRepositoryError
import click
import colorama
import cantools


SCRIPT_DIR = Path(__file__).parent.resolve()

try:
    repo = git.Repo(SCRIPT_DIR, search_parent_directories=True)
    REPO_ROOT = Path(repo.git.rev_parse("--show-toplevel"))
except InvalidGitRepositoryError:
    sys.exit("Test can only be run in a git repository.")


def set_logging_level(verbose: int = 0) -> None:
    """sets the module logging level

    :param verbose: verbosity level"""
    if verbose == 1:
        logging.basicConfig(level=logging.INFO)
    elif verbose > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)


@click.command(context_settings=dict(help_option_names=["-h", "--help"]))
@click.option("-v", "--verbose", default=0, count=True, help="Verbose information.")
@click.option(
    "-s",
    "--symbol",
    "symbol_file",
    default=REPO_ROOT / "tools" / "dbc" / "foxbms.sym",
    type=click.Path(exists=True, dir_okay=False),
    is_eager=True,
    help="FILE path of the symbol file",
)
@click.option(
    "-d",
    "--dbc",
    "dbc_file",
    default=REPO_ROOT / "tools" / "dbc" / "foxbms.dbc",
    type=click.Path(exists=True, dir_okay=False),
    is_eager=True,
    help="FILE path of the dbc file",
)
@click.pass_context
def main(ctx: click.Context, verbose: int, symbol_file: Path, dbc_file: Path) -> None:
    """Convert symbol file to dbc file"""
    colorama.init()
    set_logging_level(verbose)
    logging.debug("Parsing symbol file...")
    cantools.database.load_file(str(symbol_file))
    logging.debug("done")
    logging.debug("Parsing dbc file...")
    cantools.database.load_file(str(dbc_file))
    logging.debug("done")
    logging.info(f"{symbol_file} and {dbc_file} files are parsable.")
    ctx.exit(0)


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter
