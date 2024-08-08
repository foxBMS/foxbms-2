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

"""Command line interface definition for Axivion"""

from pathlib import Path

import click

from ..cmd_axivion import axivion_impl

CONTEXT_SETTINGS = {"help_option_names": ["-h", "--help"]}


@click.group(context_settings=CONTEXT_SETTINGS, invoke_without_command=True)
@click.option("-v", "--verbose", default=0, count=True, help="Verbose information")
@click.option(
    "--check-versions",
    default=False,
    is_flag=True,
    help="Check that all Axivion configuration files have the same version",
)
@click.pass_context
def axivion(
    ctx: click.Context,
    verbose: bool,
    check_versions: bool,
) -> None:
    """Checks related to Axivion. This does not run the Axivion SPA build.
    To run the Axivion SPA build, use the 'waf' command."""
    if check_versions:
        ret = axivion_impl.check_versions(verbose)
        ctx.exit(ret.returncode)
    elif not ctx.invoked_subcommand:
        click.echo(axivion.get_help(ctx))


@axivion.command("self-test", context_settings={"ignore_unknown_options": True})
@click.argument("script_args", nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def cmd_self_test(ctx: click.Context, script_args: tuple[str]) -> None:
    """Axivion self-and configuration tests"""
    ret = axivion_impl.self_test(list(script_args))
    ctx.exit(ret.returncode)


@axivion.command("export-architecture")
@click.option("-v", "--verbose", default=0, count=True, help="Verbose information")
@click.pass_context
def cmd_export_architecture(
    ctx: click.Context,
    verbose: int,
) -> None:
    """Exports the architecture file"""
    ret = axivion_impl.export_architecture(verbose)
    ctx.exit(ret.returncode)


@axivion.command("check-architecture-up-to-date")
@click.option("-v", "--verbose", default=0, count=True, help="Verbose information")
@click.pass_context
def cmd_check_architecture_uptodate(
    ctx: click.Context,
    verbose: int,
) -> None:
    """Checks whether the architecture file is up-to-date"""
    ret = axivion_impl.check_if_architecture_up_to_date(verbose)
    ctx.exit(ret.returncode)


@axivion.command(
    "check-violations",
    context_settings={
        "help_option_names": ["--dummy"],
        "ignore_unknown_options": True,
    },
)
@click.argument("check_violations_args", nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def cmd_check_violations(ctx: click.Context, check_violations_args: tuple[str]) -> None:
    """Checks for Axivion violations (uses the 'check_violations.py' script)"""
    ret = axivion_impl.check_violations(list(check_violations_args))
    ctx.exit(ret)


@axivion.command("combine-reports")
@click.option("-v", "--verbose", default=0, count=True, help="Verbose information")
@click.argument(
    "reports",
    nargs=-1,
    is_eager=True,
    type=click.Path(exists=True, dir_okay=False, path_type=Path),
)
@click.pass_context
def cmd_combine_reports(
    ctx: click.Context,
    verbose: int,
    reports: list[Path],
) -> None:
    """Combines several reports into one"""
    if not reports:
        click.echo("No reports provided.", err=True)
        ctx.exit(1)
    ret = axivion_impl.combine_report_files(reports, verbose)
    ctx.exit(ret)


@axivion.command("local-analysis")
@click.option("-u", "--dashboard-url", help="Axivion Dashboard URL")
@click.option("-v", "--variant", help="Build variant")
@click.option("-b", "--branch", help="git branch")
@click.pass_context
def cmd_local_analysis(
    ctx: click.Context,
    dashboard_url: str,
    variant: str,
    branch: str,
) -> None:
    """Runs a local analysis"""
    ret = axivion_impl.run_local_analysis(dashboard_url, variant, branch)
    ctx.exit(ret.returncode)


@axivion.command("local-dashserver")
@click.option(
    "-d",
    "--db-file",
    is_eager=True,
    type=click.Path(exists=True, dir_okay=False, path_type=Path),
    help="Database file to be installed",
)
@click.pass_context
def cmd_local_dashserver(ctx: click.Context, db_file: Path) -> None:
    """Starts a local dashserver"""
    ret = axivion_impl.start_local_dashserver(db_file)
    ctx.exit(ret.returncode)


@axivion.command("make-race-pdfs")
@click.pass_context
def cmd_make_race_pdfs(ctx: click.Context) -> None:
    """Creates the race pdfs"""
    ret = axivion_impl.make_race_pdfs()
    ctx.exit(ret.returncode)
