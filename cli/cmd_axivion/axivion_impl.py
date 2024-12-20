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

"""Implements the functionalities behind the 'axivion' command"""

import logging
import os
import shutil
import sys
from pathlib import Path

from click import secho

from ..cmd_script import script_impl
from ..helpers.env_vars import PROGRAMFILES, PROGRAMFILESX86, USERPROFILE
from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import SubprocessResult, run_process

GRAVIS_EXPORT_SCRIPT = (
    PROJECT_ROOT / "tests/axivion/scripts/gravis_export_architecture_svg.py"
)

ARCHITECTURE_UP_TO_DATE_SCRIPT = (
    PROJECT_ROOT / "tests/axivion/scripts/ci_check_freshness_of_architecture_svg.py"
)

VERSION_CHECK_SCRIPT = PROJECT_ROOT / "tests/axivion/scripts/check_axivion_versions.py"


CHECK_VIOLATIONS_SCRIPT = PROJECT_ROOT / "tests/axivion/violations/check_violations.py"
FORBIDDEN_VIOLATIONS_CONFIG = (
    PROJECT_ROOT / "tests/axivion/violations/forbidden-violations.txt"
)

COMBINE_REPORTS_SCRIPT = PROJECT_ROOT / "tests/axivion/combine_report_files.py"

SELF_TEST_SCRIPT = PROJECT_ROOT / "tests/axivion/axivion_self_tests.py"


MAKE_RACE_PDFS_SCRIPT = Path(
    "C:/Bauhaus/7.5.2/doc/html/_downloads/87991abe35fa881bf14ce6d55f075d2d/make_race_pdfs.bat"
)
IR_FILE = Path(USERPROFILE) / ".bauhaus/localbuild/projects/foxbms-2.ir"
RFG_FILE = Path(USERPROFILE) / ".bauhaus/localbuild/projects/foxbms-2.rfg"

AXIVION_DASHBOARD_URL = os.environ.get("AXIVION_DASHBOARD_URL", "")


def export_architecture(verbosity: int = 0) -> SubprocessResult:
    """Calls the architecture export script."""
    gravis = shutil.which("gravis")
    if not gravis:
        path = (
            os.path.join(PROGRAMFILES, "Bauhaus", "bin")
            + os.pathsep
            + os.path.join(PROGRAMFILESX86, "Bauhaus", "bin")
        )
        gravis = shutil.which("gravis", path=path)
    logging.debug("Gravis: %s", gravis)
    if not gravis:
        secho("Could not find gravis!", fg="red", err=True)
        return SubprocessResult(1)
    cmd = [gravis, "--script", str(GRAVIS_EXPORT_SCRIPT)]
    if verbosity:
        cmd.append("-v")  # gravis supports only one '-v'
    return run_process(cmd=cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None)


def check_if_architecture_up_to_date(verbosity: int = 0) -> SubprocessResult:
    """Calls the architecture up-to-date check script."""
    cmd = [sys.executable, str(ARCHITECTURE_UP_TO_DATE_SCRIPT)]
    if verbosity:
        cmd.append("-" + verbosity * "v")
    return run_process(cmd=cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None)


def check_versions(verbosity: int = 0) -> SubprocessResult:
    """Calls the architecture up-to-date check script."""
    cmd = [sys.executable, str(VERSION_CHECK_SCRIPT)]
    if verbosity:
        cmd.append("-" + verbosity * "v")
    return run_process(cmd=cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None)


def check_violations(check_violations_args: list[str]) -> int:
    """Runs the violations check script on the provided analysis report file."""
    err = 0
    cmd = [str(CHECK_VIOLATIONS_SCRIPT)] + check_violations_args
    script_impl.run_python_script(cmd, cwd=PROJECT_ROOT)
    return err


def combine_report_files(
    reports: list[Path],
    verbosity: int = 0,
):
    """Combines several Axivion analysis reports to one."""
    base_cmd = [sys.executable, str(COMBINE_REPORTS_SCRIPT)]
    cmd = base_cmd + [str(i) for i in reports]
    if verbosity:
        cmd.append("-" + verbosity * "v")
    err = run_process(cmd=cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None).returncode
    return err


def self_test(script_args: list[str]) -> SubprocessResult:
    """Calls the architecture export script."""
    gravis = shutil.which("gravis")
    if not gravis:
        path = (
            os.path.join(PROGRAMFILES, "Bauhaus", "bin")
            + os.pathsep
            + os.path.join(PROGRAMFILESX86, "Bauhaus", "bin")
        )
        gravis = shutil.which("gravis", path=path)
    logging.debug("Gravis: %s", gravis)
    if not gravis:
        secho("Could not find gravis!", fg="red", err=True)
        return SubprocessResult(1)
    cmd = [sys.executable, str(SELF_TEST_SCRIPT)] + script_args
    return run_process(cmd=cmd, cwd=PROJECT_ROOT, stderr=None, stdout=None)


def run_local_analysis(
    dashboard_url: str, variant: str, branch: str, project: str = "app"
) -> SubprocessResult:
    """Runs a local Axivion analysis."""
    if not dashboard_url:
        if AXIVION_DASHBOARD_URL:
            dashboard_url = AXIVION_DASHBOARD_URL
    if not dashboard_url:
        return SubprocessResult(1, out="", err="Dashboard url is not provided.")

    if not variant:
        variant = "freertos_ltc-6813-1_vbb_cc-cc-tr-none_none-no-imd_s1-m2-cb18"
    if not branch:
        branch = "master"

    bauhaus_config = PROJECT_ROOT / f"tests/axivion/targets/{project}"
    requests_ca_bundle = Path(USERPROFILE) / ".bauhaus/auto.crt"
    project_name = "foxbms-2"
    project_shadow_repo = Path(USERPROFILE) / f".bauhaus/{project_name}"
    project_shadow_repo.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env["AXIVION"] = "TRUE"
    env["AXIVION_LOCAL_BUILD"] = "TRUE"
    env["AXIVION_BRANCH_NAME"] = branch
    env["REQUESTS_CA_BUNDLE"] = str(requests_ca_bundle)
    env["BAUHAUS_CONFIG"] = str(bauhaus_config)
    env["AXIVION_DASHBOARD_URL"] = dashboard_url
    env["projectName"] = project_name
    env["analysisProjectName"] = f"{project_name}.{variant}.{branch}"
    env["projectShadowRepo"] = str(project_shadow_repo)
    return run_analysis(env=env)


def run_analysis(env: dict = None) -> SubprocessResult:
    """Runs Axivion analysis."""
    axivion_build_dir = PROJECT_ROOT / "build/app_spa"
    axivion_build_dir.mkdir(parents=True, exist_ok=True)
    if env.get("AXIVION_LOCAL_BUILD", ""):
        env["SKIP_PREBUILD_CLEAN"] = env["AXIVION_LOCAL_BUILD"]
    if env.get("AXIVION_INCREMENTAL_BUILD", ""):
        env["SKIP_POSTBUILD_CLEAN"] = env["AXIVION_INCREMENTAL_BUILD"]
    env["AXIVION_BUILD_DIR"] = str(axivion_build_dir)
    axivion_ci = shutil.which("axivion_ci")
    if not axivion_ci:
        return SubprocessResult(
            1, out="", err="Could not find 'axivion_config' executable."
        )
    cmd = [axivion_ci]
    return run_process(cmd, env=env, stdout=None, stderr=None)


def start_local_dashserver(db_file: Path) -> SubprocessResult:
    """Starts a local Axivion dashserver."""
    cmd = ["dashserver", "start", "--local", "--noauth", f"--install_file={db_file}"]
    return run_process(cmd)


def make_race_pdfs(
    ir_file: Path = IR_FILE, rfg_file: Path = RFG_FILE, project: str = "app"
) -> SubprocessResult:
    """Creates the race."""
    if not MAKE_RACE_PDFS_SCRIPT.is_file():
        return SubprocessResult(
            1,
            out="",
            err=f"{MAKE_RACE_PDFS_SCRIPT} does not exist.",
        )
    if not ir_file.is_file():
        return SubprocessResult(
            1,
            out="",
            err=f"{ir_file} does not exist.",
        )
    env = os.environ.copy()
    env["BAUHAUS_CONFIG"] = str(PROJECT_ROOT / f"tests/axivion/targets/{project}")
    cmd = [MAKE_RACE_PDFS_SCRIPT, ir_file]
    if rfg_file:
        cmd.append(rfg_file)
    return run_process(cmd=cmd, cwd=PROJECT_ROOT, env=env)
