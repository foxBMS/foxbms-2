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

"""Implements the functionalities behind the 'cli-unittest' command"""

import logging
import os
import sys
from pathlib import Path

from click import secho

from ..helpers.click_helpers import recho
from ..helpers.misc import PROJECT_BUILD_ROOT, PROJECT_ROOT, terminal_link_print
from ..helpers.spr import SubprocessResult, run_process
from .cli_unittest_constants import UNIT_TEST_BUILD_DIR_CLI

UNIT_TEST_MODULE_BASE_COMMAND: list[Path | str] = [sys.executable, "-m", "unittest"]
COVERAGE_MODULE_BASE_COMMAND: list[Path | str] = [sys.executable, "-m", "coverage"]


def run_unittest_module(args: list[str]) -> SubprocessResult:
    """Run the unittest module with the provided arguments."""
    logging.debug(" ".join(args))
    cmd = UNIT_TEST_MODULE_BASE_COMMAND + args
    return run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)


def _add_verbosity_to_cmd_list(
    cmd: list[str | Path], verbosity: int = 0
) -> list[str | Path]:
    """Add verbosity flags to the unittest command list"""
    if not verbosity:
        return cmd
    return cmd + ["-" + "v" * verbosity]


def run_script_tests(
    coverage_report: bool = False,
    verbosity: int = 0,
    out_dir=UNIT_TEST_BUILD_DIR_CLI,
) -> SubprocessResult:
    """Run unit tests on Python modules and files in the repository."""
    if coverage_report:
        # just delete the files
        cov_file = PROJECT_ROOT / ".coverage"
        if cov_file.is_file():
            cov_file.unlink()
        _ = [i.unlink() for i in out_dir.rglob("*") if i.is_file()]  # type: ignore
        out_dir.mkdir(exist_ok=True, parents=True)
        cmd = COVERAGE_MODULE_BASE_COMMAND + [
            "run",
            "--parallel-mode",
            "--source=cli",
            "-m",
            "unittest",
            "discover",
            "-s",
            f"tests{os.sep}cli",
        ]
        cmd = _add_verbosity_to_cmd_list(cmd, verbosity)
        ret = run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
        cmd = COVERAGE_MODULE_BASE_COMMAND + [
            "run",
            "--parallel-mode",
            PROJECT_ROOT / "tests/cli/fallback/test_fallback.py",
        ]
        cmd = _add_verbosity_to_cmd_list(cmd, verbosity)
        ret += run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
        cmd = COVERAGE_MODULE_BASE_COMMAND + [
            "run",
            "--parallel-mode",
            PROJECT_ROOT / "tests/waf-tools/test_crc64_ti_impl.py",
        ]
        cmd = _add_verbosity_to_cmd_list(cmd, verbosity)
        ret += run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
        cmd = COVERAGE_MODULE_BASE_COMMAND + ["combine"]
        ret += run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
        cmd = COVERAGE_MODULE_BASE_COMMAND + ["report"]
        ret += run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
        cmd = COVERAGE_MODULE_BASE_COMMAND + ["html", "-d", out_dir]
        ret += run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
        cmd = COVERAGE_MODULE_BASE_COMMAND + [
            "xml",
            "-o",
            out_dir / "CoberturaCoverageCliSelfTest.xml",
        ]
        ret += run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)
    else:
        cmd = UNIT_TEST_MODULE_BASE_COMMAND + [
            "discover",
            "-s",
            f"tests{os.sep}cli",
        ]
        ret = run_process(cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None)

    report_link = PROJECT_BUILD_ROOT / "cli-selftest/index.html"
    if not ret.returncode:
        secho("The cli unit tests were successful.", fg="green")
    else:
        recho("The cli unit tests were not successful.")
    if report_link.is_file() and not ret.returncode:
        secho(f"\ncoverage report: {terminal_link_print(report_link)}")

    return ret
