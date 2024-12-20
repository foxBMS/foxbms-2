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

"""Run uncrustify on the FreeRTOS sources in the foxBMS source tree"""

import ast
import os
from pathlib import Path

from click import secho

from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import SubprocessResult


def check_for_test_files(verbose: int = 0) -> SubprocessResult:
    """Check whether all python files have a test file"""
    result = SubprocessResult(0)

    # 1. check that all test files exist
    cli_files = (PROJECT_ROOT / "cli").rglob("**/*.py")
    test_files = [
        i.relative_to(PROJECT_ROOT)
        for i in (PROJECT_ROOT / "tests/cli").rglob("**/*.py")
    ]

    for i in cli_files:
        if i.name in ("__init__.py"):
            continue
        expected_test_file = (
            Path("tests") / i.relative_to(PROJECT_ROOT).parent / f"test_{i.name}"
        )
        if expected_test_file not in test_files:
            result += SubprocessResult(1)
            secho(
                f"'{i}' expects a test file in '{PROJECT_ROOT/expected_test_file}'.",
                fg="red",
                err=True,
            )
    if result.returncode:
        secho("Expected test files are missing.", fg="red", err=True)
    else:
        if verbose:
            secho("Found all expected test files.", fg="green")

    # 2. check that all test have the correct style
    for i in test_files:
        mod = ast.parse(i.read_text(encoding="utf-8"))
        cli_file = Path(
            str(i).replace("tests" + os.sep, "").replace(os.sep + "test_", os.sep)
        ).as_posix()
        if not str(ast.get_docstring(mod)).startswith(f"Testing file '{cli_file}'."):
            result += SubprocessResult(1)
            secho(
                f"{i} is missing a docstring starting with "
                f'"""Testing file \'{cli_file}\'."""',
                fg="red",
                err=True,
            )
    return result
