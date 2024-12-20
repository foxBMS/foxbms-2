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

"""Implements the command line interface to the 'cli' tool.'"""

import sys
import warnings

from .helpers.misc import ignore_third_party_logging, init_path_var_for_foxbms

try:
    import click
    import colorama

    colorama.init()
except ImportError:
    print(
        "The 'click' module is required to run this application.\n"
        f"Run '{sys.executable} -m pip install click' to install it",
        file=sys.stderr,
    )
    sys.exit(1)

from .commands.c_axivion import axivion
from .commands.c_bootloader import bootloader
from .commands.c_build import waf
from .commands.c_ci import ci
from .commands.c_cli_unittest import cli_unittest
from .commands.c_embedded_ut import ceedling
from .commands.c_etl import etl
from .commands.c_ide import ide
from .commands.c_install import install
from .commands.c_misc import misc
from .commands.c_pre_commit import pre_commit
from .commands.c_program import run_program, run_script
from .commands.c_release import release
from .foxbms_version import __version__

warnings.simplefilter(action="ignore", category=FutureWarning)

CONTEXT_SETTINGS = {"help_option_names": ["-h", "--help"]}


def get_program_config() -> dict[str, str]:
    """Returns the current foxBMS repository configuration"""
    return {
        "foxBMS 2": __version__,
    }


# Options for main
@click.group(context_settings=CONTEXT_SETTINGS, invoke_without_command=True)
@click.version_option(version=__version__)
@click.option(
    "-s",
    "--show-config",
    default=False,
    is_flag=True,
    help="Shows foxBMS configuration information",
)
@click.pass_context
def main(
    ctx: click.Context,
    show_config: bool,
) -> None:
    """'fox.py' is a tool to interact with the foxBMS 2 repository.
    It supports the following commands and options:"""
    ignore_third_party_logging()
    init_path_var_for_foxbms()
    if show_config:
        config = get_program_config()
        padding = max(len(x) for x in config)
        for key, value in config.items():
            click.echo(f"{key}:{' ' * (padding - len(key))} {value}")
    elif not ctx.invoked_subcommand:
        click.echo(main.get_help(ctx))


main.add_command(axivion)
main.add_command(bootloader)
main.add_command(ceedling)
main.add_command(ci)
main.add_command(cli_unittest)
main.add_command(etl)
main.add_command(ide)
main.add_command(install)
main.add_command(misc)
main.add_command(pre_commit)
main.add_command(release)
main.add_command(run_program)
main.add_command(run_script)
main.add_command(waf)
