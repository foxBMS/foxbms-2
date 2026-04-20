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

"""Implementations backing the ``db`` command.

This module provides thin wrappers around :class:`foxbms.db.FoxDB` for listing
and displaying cells via console output.

Functions:
    db_list(root)
        List all cells in the database and print their identifiers.
    db_show(root, cell_id)
        Print the dataclass representation of the selected cell, if present.

"""

from collections.abc import Callable
from functools import wraps
from pathlib import Path
from typing import Any

from ..db import FoxDB
from ..helpers.click_helpers import secho


def with_db[R](fun: Callable[..., R]) -> Callable[..., R]:
    """Decorate functions that require an initialzed database."""

    @wraps(fun)
    def wrapper(root: Path, *args: Any, **kwargs: Any) -> R:  # noqa: ANN401
        db = FoxDB(root)
        return fun(db, *args, **kwargs)

    return wrapper


@with_db
def db_list(db: FoxDB) -> None:
    """List available cells in the database and print them to the console.

    Args:
        db: Path to the cell database root directory or ZIP archive.

    Returns:
        None
            This function prints output and does not return a value.

    Raises:
        SystemExit
            If the database cannot be parsed or is invalid.
    """
    secho("Cell database contains the following cells:")
    for x in db.list_cells():
        secho(x)


@with_db
def db_show(db: FoxDB, cell_id: str) -> None:
    """Show details of a specific cell by its identifier.

    Print the cell dataclass or an error message.
    The identifier format is ``<manufacturer>-<name>``.

    Args:
        db: Path to the cell database root directory or ZIP archive.
        cell_id : Cell identifier in the form ``<manufacturer>-<name>``.

    Raises:
        SystemExit: If the database cannot be parsed or is invalid.
    """
    cell = db.show_cell(cell_id)
    if cell:
        secho(str(cell))
    else:
        secho("Cell not found in database.")
