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

"""Checks test files of all python files"""

import ast
import os
from pathlib import Path

from ..helpers.click_helpers import recho, secho
from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import SubprocessResult


def _get_cli_files() -> list[Path]:
    return list((PROJECT_ROOT / "cli").rglob("**/*.py"))


def _get_test_files() -> list[Path]:
    return [
        i.relative_to(PROJECT_ROOT)
        for i in (PROJECT_ROOT / "tests/cli").rglob("**/*.py")
    ]


def _check_main_unittest(file: Path, tree: ast.Module) -> SubprocessResult:
    """Checks existence of 'main' definition and 'unittest.main()' call."""
    has_main = False
    calls_unittest_main = False
    result = SubprocessResult(0)

    for node in ast.walk(tree):
        # Check for __main__ definition
        if not (isinstance(node, ast.If) and isinstance(node.test, ast.Compare)):
            continue

        if not (
            isinstance(node.test.left, ast.Name)
            and node.test.left.id == "__name__"
            and isinstance(node.test.comparators[0], ast.Constant)
            and node.test.comparators[0].value == "__main__"
        ):
            continue

        # we have at least '__main__' defined
        has_main = True

        # Check for unittest.main() call inside __main__
        for inner_node in node.body:
            if not (
                isinstance(inner_node, ast.Expr)
                and isinstance(inner_node.value, ast.Call)
            ):
                continue

            if (
                isinstance(inner_node.value.func, ast.Attribute)
                and inner_node.value.func.attr == "main"
                and isinstance(inner_node.value.func.value, ast.Name)
                and inner_node.value.func.value.id == "unittest"
            ):
                calls_unittest_main = True

    if not (has_main and calls_unittest_main):
        result += SubprocessResult(1)
        recho(f"{file} must define '__main__' and call unittest.main().")
    return result


def _check_docstring(file: Path, tree: ast.Module) -> SubprocessResult:
    """Checks correctness of the docstring"""
    result = SubprocessResult(0)
    cli_file = Path(
        str(file).replace("tests" + os.sep, "").replace(os.sep + "test_", os.sep)
    ).as_posix()
    if not str(ast.get_docstring(tree)).startswith(f"Testing file '{cli_file}'."):
        result += SubprocessResult(1)
        recho(
            f"{file} is missing a docstring starting with "
            f'"""Testing file \'{cli_file}\'."""'
        )
    return result


def check_for_test_files(verbose: int = 0) -> SubprocessResult:
    """Check whether all python files have a correct test file"""
    result = SubprocessResult(0)

    # 1. check that all test files exist
    cli_files = _get_cli_files()
    test_files = _get_test_files()

    for i in cli_files:
        if i.name in ("__init__.py"):
            continue
        expected_test_file = (
            Path("tests") / i.relative_to(PROJECT_ROOT).parent / f"test_{i.name}"
        )
        if expected_test_file not in test_files:
            result += SubprocessResult(1)
            recho(
                f"'{i}' expects a test file in '{PROJECT_ROOT / expected_test_file}'."
            )
    if result.returncode:
        recho("Expected test files are missing.")
    else:
        if verbose:
            secho("Found all expected test files.", fg="green")

    # 2. check that all test have the correct style
    for i in test_files:
        if i.name in ("__init__.py"):
            continue
        tree = ast.parse(i.read_text(encoding="utf-8"))
        result += _check_docstring(i, tree)
        result += _check_main_unittest(i, tree)

    return result
