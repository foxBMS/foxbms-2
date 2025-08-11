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

"""Command line interface definition for VS Code"""

import click

from ..cmd_ide import ide_impl
from ..helpers.click_helpers import HELP_NAMES


@click.command(context_settings=HELP_NAMES)
@click.option(
    "--generic/--no-generic",
    default=True,
    is_flag=True,
    help="Open a generic VS Code workspace in the repository root.",
)
@click.option(
    "--embedded-unit-test-app/--no-embedded-unit-test-app",
    default=False,
    is_flag=True,
    help="Open a VS Code workspace for app embedded unit test development.",
)
@click.option(
    "--embedded-unit-test-bootloader/--no-embedded-unit-test-bootloader",
    default=False,
    is_flag=True,
    help="Open a VS Code workspace for bootloader embedded unit test development.",
)
@click.option(
    "--app/--no-app",
    default=False,
    is_flag=True,
    help="Open a VS Code workspace for embedded app development.",
)
@click.option(
    "--bootloader/--no-bootloader",
    default=False,
    is_flag=True,
    help="Open a VS Code workspace for embedded bootloader development.",
)
@click.option(
    "--cli/--no-cli",
    default=False,
    is_flag=True,
    help="Open a VS Code workspace for fox CLI development.",
)
@click.pass_context
def ide(  # pylint:disable=too-many-arguments,too-many-positional-arguments
    ctx: click.Context,
    generic: bool = True,
    embedded_unit_test_app: bool = False,
    embedded_unit_test_bootloader: bool = False,
    app: bool = False,
    bootloader: bool = False,
    cli: bool = False,
) -> None:
    """Open pre-configured VS Code workspaces."""
    if generic:
        ide_impl.open_ide_generic()
    if embedded_unit_test_app:
        ide_impl.open_ide_embedded_unit_test_app()
    if embedded_unit_test_bootloader:
        ide_impl.open_ide_embedded_unit_test_bootloader()
    if app:
        ide_impl.open_ide_app()
    if bootloader:
        ide_impl.open_ide_bootloader()
    if cli:
        ide_impl.open_ide_cli()
    ctx.exit(0)
