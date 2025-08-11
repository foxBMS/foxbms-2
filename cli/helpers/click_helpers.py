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


"""Miscellaneous helper functions."""

from collections.abc import Callable
from typing import Any, Literal, TypeVar

import click

from .io import get_stderr, get_stdout
from .misc import set_logging_level

HELP_NAMES = {"help_option_names": ["-h", "--help"]}
DISABLE_DEFAULT_HELP = {"help_option_names": ["--dummy"]}
IGNORE_UNKNOWN_OPTIONS = {"ignore_unknown_options": True}


def recho(msg: Any | None, fg: Literal["red", "yellow"] = "red") -> None:
    """Print to stderr using click's 'secho'."""
    click.secho(msg, file=get_stderr(), fg=fg, err=True)  # noqa: TID251


def echo(msg: Any | None = None, nl: bool = True) -> None:
    """Print to stdout using click's 'echo'."""
    click.echo(msg, file=get_stdout(), nl=nl)  # noqa: TID251


def secho(msg: Any | None = None, nl: bool = True, **kwargs) -> None:
    """Print to stdout using click's 'secho'."""
    click.secho(msg, file=get_stdout(), nl=nl, **kwargs)  # noqa: TID251


def set_logging_level_cb(
    ctx: click.Context, param: click.Parameter | None, value: int = 0
) -> int:
    """Set the module logging level through a click option callback.

    Args:
        ctx: context the callback shall be applied too (unused)
        param: arguments of the callback (unused)
        value: arbitrary value passed to the callback (unused)

    """
    set_logging_level(verbosity=value)
    return value


# pylint: disable-next=invalid-name
VB_CALLBACK_TYPE = TypeVar("VB_CALLBACK_TYPE", bound=Callable[..., Any])


def verbosity_option(fun: VB_CALLBACK_TYPE) -> VB_CALLBACK_TYPE:
    """Add a verbosity option to click commands."""
    return click.option(
        "-v",
        "--verbose",
        default=0,
        count=True,
        help="Verbose information.",
        callback=set_logging_level_cb,
    )(fun)
