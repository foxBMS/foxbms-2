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

"""Testing file 'cli/pre_commit_scripts/sort_include_lists.py'."""

import ast
import io
import sys
import tempfile
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.pre_commit_scripts import sort_include_lists
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import sort_include_lists


class TestSortIncludeLists(unittest.TestCase):
    """Tests for wscript includes list sorting validation."""

    def _run_hook(self: "TestSortIncludeLists", content: str) -> tuple[int, str, Path]:
        """Run the hook on a temporary wscript file, return (exit_code, stderr, path)."""
        with tempfile.TemporaryDirectory() as tmp:
            file_path = Path(tmp) / "wscript"
            file_path.write_text(content, encoding="utf-8")
            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_include_lists.main([str(file_path)])
            return result, err.getvalue(), file_path

    def test_sorted_includes_no_error(self: "TestSortIncludeLists") -> None:
        """No error for sorted includes lists."""
        code = 'includes = ["a.h", "b.h", "c.h"]\n'
        result, stderr, _ = self._run_hook(code)
        self.assertEqual(result, 0)
        self.assertEqual(stderr, "")

    def test_unsorted_string_literals(self: "TestSortIncludeLists") -> None:
        """Error for unsorted includes consisting of plain strings."""
        code = 'includes = ["b.h", "a.h"]\n'
        result, stderr, path = self._run_hook(code)
        self.assertEqual(result, 1)
        self.assertIn(f"{path.as_posix()}:1: includes list is not sorted", stderr)

    def test_only_includes_lists_are_checked(self: "TestSortIncludeLists") -> None:
        """Only 'includes' is validated, not 'use' or 'source'."""
        code = 'includes = ["z", "a"]\nuse = ["y", "b"]\nsource = ["x", "c"]\n'
        result, stderr, _ = self._run_hook(code)
        self.assertEqual(result, 1)
        self.assertIn("includes list", stderr)
        self.assertNotIn("use list", stderr)
        self.assertNotIn("source list", stderr)

    def test_includes_inside_function(self: "TestSortIncludeLists") -> None:
        """Includes inside a function are also checked."""
        code = (
            "def build():\n"
            '    includes = [bld.path, bld.path.find_node("vendor"),'
            ' bld.srcnode.find_node("src/bootloader/main/include"),'
            ' bld.srcnode.find_node("src/bootloader/driver/rti")]\n'
        )
        result, stderr, _ = self._run_hook(code)
        self.assertEqual(result, 1)
        self.assertIn("includes list", stderr)

    def test_extract_path_from_bld_path_find_node(self: "TestSortIncludeLists") -> None:
        """Paths from bld.path.find_node are extracted correctly."""
        unsorted = 'includes = [bld.path.find_node("vendor"), "src"]\n'
        result, stderr, _ = self._run_hook(unsorted)
        self.assertEqual(result, 1)
        self.assertIn("includes list", stderr)

        sorted_code = 'includes = ["src", bld.path.find_node("vendor")]\n'
        result2, stderr2, _ = self._run_hook(sorted_code)
        self.assertEqual(result2, 0)
        self.assertEqual(stderr2, "")

    def test_normalization_of_paths(self: "TestSortIncludeLists") -> None:
        """Path normalization affects sorting: './b' becomes 'b'."""
        unsorted = 'includes = ["./b", "a"]\n'
        result, stderr, _ = self._run_hook(unsorted)
        self.assertEqual(result, 1)
        self.assertIn("includes list", stderr)

        sorted_code = 'includes = ["a", "./b"]\n'
        result2, stderr2, _ = self._run_hook(sorted_code)
        self.assertEqual(result2, 0)
        self.assertEqual(stderr2, "")

    def test_multiple_includes_assignments_count_errors(
        self: "TestSortIncludeLists",
    ) -> None:
        """Each unsorted includes assignment adds to the exit code."""
        code = 'includes = ["b", "a"]\nincludes = ["d", "c"]\n'
        result, stderr, _ = self._run_hook(code)
        self.assertEqual(result, 2)
        self.assertEqual(stderr.count("includes list"), 2)

    def test_error_count_capped_to_255(self: "TestSortIncludeLists") -> None:
        """More than 255 unsorted includes are capped as documented."""
        code = "\n".join('includes = ["b", "a"]' for _ in range(300))
        result, _, _ = self._run_hook(code)
        self.assertEqual(result, 255)

    def test_main_without_files(self: "TestSortIncludeLists") -> None:
        """No files should return no errors."""
        result = sort_include_lists.main([])
        self.assertEqual(result, 0)

    def test_invalid_python_syntax(self: "TestSortIncludeLists") -> None:
        """Invalid Python in a wscript file reports an error."""
        code = "def broken(:\n"
        result, stderr, path = self._run_hook(code)
        self.assertEqual(result, 1)
        self.assertIn(f"{path.as_posix()}: invalid Python syntax:", stderr)

    def test_line_number_correct(self: "TestSortIncludeLists") -> None:
        """Line number of the includes assignment is reported correctly."""
        code = 'some_variable = 1\n\nincludes = ["b", "a"]\n'
        result, stderr, path = self._run_hook(code)
        self.assertEqual(result, 1)
        self.assertIn(f"{path.as_posix()}:3:", stderr)

    def test_extract_path_handles_supported_and_fallback_expressions(
        self: "TestSortIncludeLists",
    ) -> None:
        """Extract paths from supported expressions and preserve fallbacks."""
        cases = (
            # Bare supported Waf node roots return the relative root path.
            ("bld.bldnode", "."),
            ("bld.path", "."),
            ("bld.srcnode", "."),
            # A plain string constant is used directly as the path.
            ('"local/include"', "local/include"),
            # A non-string ast.Constant cannot provide a usable path.
            ("42", "42"),
            # A call not represented by an attribute is unsupported.
            ('make_path("vendor")', 'make_path("vendor")'),
            # An unrelated attribute is not a supported bld node.
            ("other.path", "other.path"),
            # Nested attributes are retained as their source representation.
            ("other.path.child", "other.path.child"),
            # An unsupported bld attribute is treated as a fallback.
            ("bld.build_dir", "bld.build_dir"),
            # A method other than find_node does not provide an include path.
            ('bld.path.join("vendor")', 'bld.path.join("vendor")'),
            # A find_node call without an argument cannot provide a path.
            ("bld.path.find_node()", "bld.path.find_node()"),
            # A dynamic find_node argument cannot be resolved statically.
            ("bld.path.find_node(path_name)", "bld.path.find_node(path_name)"),
            # A non-string find_node argument cannot provide a path.
            ("bld.path.find_node(42)", "bld.path.find_node(42)"),
            # A static string find_node argument provides the sortable path.
            ('bld.path.find_node("vendor")', "vendor"),
        )

        for source, expected in cases:
            with self.subTest(source=source):
                expression = ast.parse(source, mode="eval").body

                self.assertEqual(
                    sort_include_lists.extract_path(expression, source),
                    expected,
                )

    def test_fix_rewrites_multiple_lists_and_returns_one(
        self: "TestSortIncludeLists",
    ) -> None:
        """--fix rewrites all lists but returns one for pre-commit."""
        source = (
            'includes = ["b.h", "a.h"]\n\ndef build():\n    includes = ["d.h", "c.h"]\n'
        )
        expected = (
            "includes = [\n"
            '    "a.h",\n'
            '    "b.h",\n'
            "]\n"
            "\n"
            "def build():\n"
            "    includes = [\n"
            '        "c.h",\n'
            '        "d.h",\n'
            "    ]\n"
        )

        with tempfile.TemporaryDirectory() as tmp:
            file_path = Path(tmp) / "wscript"
            file_path.write_text(source, encoding="utf-8")

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_include_lists.main(["--fix", str(file_path)])

            fixed_source = file_path.read_text(encoding="utf-8")

        self.assertEqual(result, 1)
        self.assertEqual(fixed_source, expected)
        self.assertIn("fixed 2 unsorted includes list(s).", err.getvalue())

    def test_fix_does_not_rewrite_invalid_python(
        self: "TestSortIncludeLists",
    ) -> None:
        """--fix reports invalid Python without modifying the file."""
        source = "def broken(:\n"

        with tempfile.TemporaryDirectory() as tmp:
            file_path = Path(tmp) / "wscript"
            file_path.write_text(source, encoding="utf-8")

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_include_lists.main(["--fix", str(file_path)])

            resulting_source = file_path.read_text(encoding="utf-8")

        self.assertEqual(result, 1)
        self.assertEqual(resulting_source, source)
        self.assertIn("invalid Python syntax:", err.getvalue())

    def test_fix_sorted_file_returns_zero_without_rewriting(
        self: "TestSortIncludeLists",
    ) -> None:
        """--fix leaves an already sorted includes list unchanged."""
        source = 'includes = ["a.h", "b.h"]\n'

        with tempfile.TemporaryDirectory() as tmp:
            file_path = Path(tmp) / "wscript"
            file_path.write_text(source, encoding="utf-8")

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_include_lists.main(["--fix", str(file_path)])

            resulting_source = file_path.read_text(encoding="utf-8")

        self.assertEqual(result, 0)
        self.assertEqual(resulting_source, source)
        self.assertEqual(err.getvalue(), "")

    def test_missing_file_is_ignored(self: "TestSortIncludeLists") -> None:
        """A path that is not a file is ignored."""
        with tempfile.TemporaryDirectory() as tmp:
            missing_file = Path(tmp) / "does-not-exist"

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_include_lists.main(["--fix", str(missing_file)])

        self.assertEqual(result, 0)
        self.assertEqual(err.getvalue(), "")

    def test_empty_and_non_list_includes_are_ignored(
        self: "TestSortIncludeLists",
    ) -> None:
        """Empty and non-list includes assignments do not produce findings."""
        code = 'includes = []\nincludes = ("b.h", "a.h")\n'

        result, stderr, _ = self._run_hook(code)

        self.assertEqual(result, 0)
        self.assertEqual(stderr, "")

    def test_fix_orders_bldnode_locals_and_srcnode(
        self: "TestSortIncludeLists",
    ) -> None:
        """--fix orders bldnode, local, and srcnode includes separately."""
        source = (
            "includes = [\n"
            '    bld.srcnode.find_node("src/z"),\n'
            '    "local/z",\n'
            '    bld.bldnode.find_node("vendor"),\n'
            '    bld.path.find_node("local/a"),\n'
            "    bld.path,\n"
            '    bld.srcnode.find_node("src/a"),\n'
            '    "local/b",\n'
            "]\n"
        )
        expected = (
            "includes = [\n"
            '    bld.bldnode.find_node("vendor"),\n'
            "    bld.path,\n"
            '    bld.path.find_node("local/a"),\n'
            '    "local/b",\n'
            '    "local/z",\n'
            '    bld.srcnode.find_node("src/a"),\n'
            '    bld.srcnode.find_node("src/z"),\n'
            "]\n"
        )

        with tempfile.TemporaryDirectory() as tmp:
            file_path = Path(tmp) / "wscript"
            file_path.write_text(source, encoding="utf-8")

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_include_lists.main(["--fix", str(file_path)])

            fixed_source = file_path.read_text(encoding="utf-8")

        self.assertEqual(result, 1)
        self.assertEqual(fixed_source, expected)
        self.assertIn("fixed 1 unsorted includes list(s).", err.getvalue())

    def test_attribute_assignment_is_not_an_includes_assignment(
        self: "TestSortIncludeLists",
    ) -> None:
        """Assignments to an attribute named includes are ignored."""
        code = 'configuration.includes = ["b.h", "a.h"]\n'

        result, stderr, _ = self._run_hook(code)

        self.assertEqual(result, 0)
        self.assertEqual(stderr, "")

    def test_bld_root_attribute_handles_non_bld_roots(
        self: "TestSortIncludeLists",
    ) -> None:
        """Only expressions rooted at bld have a Waf root attribute."""
        cases = (
            ("bld.bldnode", "bldnode"),
            ("other.srcnode", None),
            ('"local/include"', None),
        )

        for source, expected in cases:
            with self.subTest(source=source):
                expression = ast.parse(source, mode="eval").body

                # pylint: disable-next=protected-access
                result = sort_include_lists._bld_root_attribute(expression)

                self.assertEqual(result, expected)


if __name__ == "__main__":
    unittest.main()
