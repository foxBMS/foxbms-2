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

"""Checks that the GitLab CI configuration adheres to some rules"""

import logging
import math
import os
import sys
from pathlib import Path
from tomllib import loads
from typing import get_args
from xml.etree import ElementTree

from click import echo, secho

from ..cmd_cli_unittest.cli_unittest_constants import CliUnitTestVariants
from ..cmd_embedded_ut.embedded_ut_constants import EmbeddedUnitTestVariants
from ..helpers.host_platform import PLATFORM
from ..helpers.misc import PROJECT_ROOT

CI_COMMIT_REF_NAME = os.getenv("CI_COMMIT_REF_NAME")
TARGET_BRANCH = os.getenv("TARGET_BRANCH", "master")

if PLATFORM == "linux":
    CERTIFICATE_FILE = "/etc/ssl/certs/ca-bundle.crt"
else:
    CERTIFICATE_FILE = ""


class Coverage:
    """Coverage information from Cobertura report."""

    def __init__(self, cobertura_report: Path) -> None:
        if not cobertura_report.is_file():
            secho(f"File '{cobertura_report}' does not exist.", fg="red", err=True)
            sys.exit(1)
        root = ElementTree.parse(cobertura_report).getroot()
        for coverage in root.iter("coverage"):
            self.line_rate = float(coverage.attrib["line-rate"])
            self.branch_rate = float(coverage.attrib["branch-rate"])

    def compare(self, branch: "Coverage") -> int:
        """Compares two coverage reports"""
        err = 0
        if self.line_rate > branch.line_rate:
            logging.error(
                "The line coverage drops from %s on '%s' to %s on '%s'.",
                self.line_rate,
                TARGET_BRANCH,
                branch.line_rate,
                CI_COMMIT_REF_NAME,
            )
            err += 1
        if self.branch_rate > branch.branch_rate:
            logging.error(
                "The branch coverage drops from %s on '%s' to %s on '%s'.",
                self.branch_rate,
                TARGET_BRANCH,
                branch.branch_rate,
                CI_COMMIT_REF_NAME,
            )
            err += 1
        return err

    def __str__(self) -> str:
        return f"line-rate: {self.line_rate}\nbranch-rate: {self.branch_rate}"


def check_coverage(
    project: EmbeddedUnitTestVariants | CliUnitTestVariants = "app",
) -> int:
    """Compares two coverage reports."""
    if project in get_args(EmbeddedUnitTestVariants):
        infix = "_host_unit_test"
        cobertura_xml = "artifacts/gcov/gcovr/GcovCoverageCobertura.xml"
    elif project in get_args(CliUnitTestVariants):
        infix = "-selftest"
        cobertura_xml = "CoberturaCoverageCliSelfTest.xml"
    else:
        sys.exit(
            "Something went wrong.\nExpect argument "
            f"{get_args(EmbeddedUnitTestVariants) + get_args(CliUnitTestVariants)} "
            f"but got '{project}'."
        )

    cobertura_archive_master = Path(
        f"master-artifacts/build/{project}{infix}/{cobertura_xml}"
    )
    coverage_master = Coverage(cobertura_archive_master)
    logging.info("'%s'-branch coverage:\n%s", TARGET_BRANCH, coverage_master)

    cobertura_archive_branch = Path(f"build/{project}{infix}/{cobertura_xml}")

    coverage_branch = Coverage(cobertura_archive_branch)
    logging.info("'%s'-branch coverage:\n%s", CI_COMMIT_REF_NAME, coverage_branch)

    comparison = coverage_master.compare(coverage_branch)

    if project == "cli":
        tmp = PROJECT_ROOT / "pyproject.toml"
        if not tmp.is_file():
            comparison += 1
        cfg = loads(tmp.read_text(encoding="utf-8"))
        try:
            fail_under = float(cfg["tool"]["coverage"]["report"]["fail_under"])
        except KeyError:
            secho(
                f"File '{tmp}' does not specify the key 'fail_under' in "
                "section '[tool.coverage.report]'.",
                fg="red",
                err=True,
            )
            return comparison + 1
        except ValueError:
            secho("Could not convert key 'fail_under' to int.", fg="red", err=True)
            return comparison + 1
        # conversion necessary as coverage.py uses 0% to 100%, while Cobertura
        # uses 0.0 to 1.0.
        fail_under /= 100
        if not math.isclose(fail_under, coverage_branch.line_rate, abs_tol=0.02):
            secho(
                "Key 'fail_under' in section '[tool.coverage.report]' in File "
                f"'{tmp}' is not up-to-date ({fail_under} vs. "
                f"{coverage_branch.line_rate} (line rate))",
                fg="red",
                err=True,
            )
            return comparison + 1
    echo(f"Coverage rate is {coverage_branch.line_rate}.")
    return comparison
