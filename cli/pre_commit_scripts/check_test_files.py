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

"""Test file presence and style checking for Python and C source files.

This module provides classes and methods for verifying the existence and
stylistic correctness of test files corresponding to source files in a project.
It supports both Python and C languages.

This module provides utilities to:
- Ensure that every file has a corresponding test file;
- Check that test files contain the correct docstring and main entry point;
- Enforce maximum file path lengths for different parts of the repository.

Classes:
    TestFiles: Abstract base class for test file operations.
    PythonTestFiles: Implements checks for Python test files.
    CTestFiles: Implements checks for C test files.

"""

import argparse
import ast
import sys
from abc import ABC, abstractmethod
from collections.abc import Sequence
from pathlib import Path


def _as_repo_relative(path: Path) -> Path:
    """Return a repository-relative path when possible."""
    if not path.is_absolute():
        return path
    try:
        return path.relative_to(Path.cwd())
    except ValueError:
        return path


def _starts_with(path: Path, prefix: tuple[str, ...]) -> bool:
    """Check if path parts begin with the given prefix parts."""
    return path.parts[: len(prefix)] == prefix


class TestFiles(ABC):
    """Abstract base class for test file handling logic."""

    @property
    @abstractmethod
    def test_type(self) -> str:
        """Type of test (e.g., 'Python', 'C').

        Returns:
            A string representing the type of tests handled by the subclass.

        """

    @abstractmethod
    def check_test_files(self, verbose: int = 0) -> int:
        """Run checks for all test files.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of errors detected during checks.

        """

    @abstractmethod
    def check_for_missing_test_files(self, verbose: int = 0) -> int:
        """Check that all required test files exist.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of missing test files.

        """

    @abstractmethod
    def check_style(self, verbose: int = 0) -> int:
        """Check all test files for style requirements.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of style errors detected.

        """

    def echo_feedback(self, err: int, verbose: int = 0) -> None:
        """Print feedback to the user about test file presence.

        Args:
            err: The number of errors found.
            verbose: Verbosity level.

        """
        if err:
            print(f"Expected {self.test_type} test files are missing.", file=sys.stderr)
        elif verbose:
            print(f"Found all expected {self.test_type} test files.")


class PythonTestFiles(TestFiles):
    """Handler for Python source and test files."""

    @property
    def test_type(self):
        """Type of test files handled.

        Returns:
            The string 'Python'.

        """
        return "Python"

    def __init__(self, files: set[Path]):
        """Initialize a PythonTestFiles object.

        Args:
            files: Set of file paths used to scope checks.

        """
        files = {_as_repo_relative(i) for i in files}
        self._source_files = [i for i in files if _starts_with(i, ("cli",))]
        self._test_files = [i for i in files if _starts_with(i, ("tests", "cli"))]

    def check_test_files(self, verbose: int = 0) -> int:
        """Run all checks for Python test files.

        Args:
            verbose: Verbosity level.

        Returns:
            The total number of errors found.

        """
        err = 0
        err += self.check_for_missing_test_files(verbose)
        err += self.check_style(verbose)
        return err

    def check_style(self, verbose: int = 0):
        """Check Python test files for docstring and main/unittest style.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of style errors found.

        """
        err = 0
        for i in self._test_files:
            tree = ast.parse(i.read_text(encoding="utf-8"))
            err += self._check_docstring(i, tree)
            err += self._check_main_unittest(i, tree)
        return err

    def _check_docstring(self, file: Path, tree: ast.Module) -> int:
        """Check that the test file has the required docstring.

        Args:
            file: The Path of the test file.
            tree: The AST of the test file.

        Returns:
            1 if the docstring is missing or incorrect, otherwise 0.

        """
        err = 0
        rel_parts = list(_as_repo_relative(file).parts)
        if rel_parts and rel_parts[0] == "tests":
            rel_parts = rel_parts[1:]
        if rel_parts and rel_parts[-1].startswith("test_"):
            rel_parts[-1] = rel_parts[-1][5:]
        cli_file = Path(*rel_parts).as_posix()
        if not str(ast.get_docstring(tree)).startswith(f"Testing file '{cli_file}'."):
            err += 1
            print(
                f"{file} is missing a docstring starting with \"Testing file '{cli_file}'.\"",
                file=sys.stderr,
            )
        return err

    def _check_main_unittest(self, file: Path, tree: ast.Module) -> int:
        """Check for '__main__' definition and call to unittest.main().

        Args:
            file: The Path of the test file.
            tree: The AST of the test file.

        Returns:
            1 if '__main__' or unittest.main() is missing, otherwise 0.

        """
        has_main = False
        calls_unittest_main = False
        err = 0

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
            err += 1
            print(
                f"{file} must define '__main__' and call unittest.main().",
                file=sys.stderr,
            )

        return err

    def check_for_missing_test_files(self, verbose: int = 0) -> int:
        """Check for missing test files for each Python source.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of missing test files.

        """
        err = 0
        for i in self._source_files:
            if i.name in ("__init__.py"):
                continue
            expected_test_file = Path("tests") / i.parent / f"test_{i.name}"
            if not expected_test_file.exists():
                err += 1
                print(
                    f"'{i}' expects a test file in '{expected_test_file}'.",
                    file=sys.stderr,
                )
        self.echo_feedback(err, verbose)
        return err


class CTestFiles(TestFiles):
    """Handler for C source and test files."""

    @property
    def test_type(self):
        """Type of test files handled.

        Returns:
            The string 'C'.

        """
        return "C"

    @staticmethod
    def _source_file_to_test_file(file: Path) -> Path:
        """Convert a C source file path to its corresponding test file path.

        Args:
            file: The Path of the source file.

        Returns:
            The Path of the expected test file.

        """
        rel_file = file.relative_to(Path("src"))
        return Path("tests/unit") / rel_file.parent / f"test_{file.name}"

    def __init__(self, files: set[Path]):
        """Initialize a CTestFiles object.

        Args:
            files: Set of file paths used to scope checks.

        """
        self.root = Path("src")
        self.prefix = Path("tests/unit")
        files = {_as_repo_relative(i) for i in files}
        self._source_files = [
            i
            for i in files
            if _starts_with(i, self.root.parts)
            and i.relative_to(self.root).parts[0] in {"app", "bootloader"}
        ]
        self._test_files = [
            i
            for i in files
            if i.name.startswith("test_") and _starts_with(i, self.prefix.parts)
        ]

    def check_test_files(self, verbose: int = 0) -> int:
        """Run all checks for C test files.

        Args:
            verbose: Verbosity level.

        Returns:
            The total number of errors found.

        """
        err = 0
        err += self.check_for_missing_test_files(verbose)
        err += self.check_style(verbose)
        return err

    def check_for_missing_test_files(self, verbose: int = 0) -> int:
        """Check for missing test files for each C source.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of missing test files.

        """
        err = 0
        source_to_test_files = [
            self._source_file_to_test_file(i) for i in self._source_files
        ]
        for i in source_to_test_files:
            if not i.exists():
                err += 1
                print(f"Missing test file '{i}'", file=sys.stderr)
        self.echo_feedback(err, verbose)
        return err

    def check_style(self, verbose: int = 0) -> int:
        """Check C test files for required function signature style.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of style errors found.

        """
        err = 0
        for test in self._test_files:
            lines = test.read_text(encoding="utf-8").splitlines()
            for i, line in enumerate(lines):
                if line.startswith("void test") and not line.endswith("(void) {"):
                    err += 1
                    print(
                        f"{test}:{i + 1}: Test files need to have "
                        f"the form 'test<TestName> (void) {{' ({line}",
                        file=sys.stderr,
                    )
        return err


class WscriptTestFiles(TestFiles):
    """Handler for wscript source/test file pairs."""

    @property
    def test_type(self):
        """Type of test files handled.

        Returns:
            The string 'wscript'.

        """
        return "wscript"

    def __init__(self, files: set[Path]):
        """Initialize a WscriptTestFiles object.

        Args:
            files: Set of file paths used to scope checks.

        """
        self.root = Path("src")
        self.prefix = Path("tests/unit")
        files = {_as_repo_relative(i) for i in files}
        self._source_files = [
            i
            for i in files
            if _starts_with(i, self.root.parts)
            and i.relative_to(self.root).parts[0] in {"app", "bootloader"}
        ]

    def check_test_files(self, verbose: int = 0) -> int:
        """Run all checks for wscript test files.

        Args:
            verbose: Verbosity level.

        Returns:
            The total number of errors found.

        """
        err = 0
        err += self.check_for_missing_test_files(verbose)
        err += self.check_style(verbose)
        return err

    def check_for_missing_test_files(self, verbose: int = 0) -> int:
        """Check for missing tests/unit wscript files for each src wscript.

        Args:
            verbose: Verbosity level.

        Returns:
            The number of missing test files.

        """
        err = 0
        for i in self._source_files:
            expected_test_file = self.prefix / i.relative_to(self.root)
            if not expected_test_file.exists():
                err += 1
                print(f"Missing test file '{expected_test_file}'", file=sys.stderr)
        self.echo_feedback(err, verbose)
        return err

    def check_style(self, verbose: int = 0) -> int:
        """No style checks for wscript pairing checks.

        Args:
            verbose: Verbosity level.

        Returns:
            Always ``0``.

        """
        return 0


def main(argv: Sequence[str] | None = None) -> int:
    """Run test-file checks for provided paths.

    If no files are provided, no checks are run.

    Args:
        argv: Optional command-line arguments.

    Returns:
        Exit code where ``0`` means all checks passed.

    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase verbosity (can be provided multiple times).",
    )
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)

    files = {Path(i) for i in args.files}
    if not files:
        return 0

    py_files = {i for i in files if i.suffix == ".py"}
    c_files = {i for i in files if i.suffix == ".c"}
    wscript_files = {i for i in files if i.name == "wscript"}

    ret = PythonTestFiles(py_files).check_test_files(args.verbose)
    ret += CTestFiles(c_files).check_test_files(args.verbose)
    ret += WscriptTestFiles(wscript_files).check_test_files(args.verbose)
    return ret


if __name__ == "__main__":
    raise SystemExit(main())
