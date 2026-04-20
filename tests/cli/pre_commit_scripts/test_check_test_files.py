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

"""Testing file 'cli/pre_commit_scripts/check_test_files.py'."""

# pylint: disable=protected-access

import ast
import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.pre_commit_scripts import check_test_files
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import check_test_files


class TestEchoFeedback(unittest.TestCase):
    """Tests for feedback printing."""

    @patch("builtins.print")
    def test_error_prints_message(self, mock_print):
        """Error count should print missing-test message."""
        mock_test_files = MagicMock()
        mock_test_files.test_type = "Python"
        check_test_files.TestFiles.echo_feedback(mock_test_files, 1)
        mock_print.assert_called_once_with(
            "Expected Python test files are missing.", file=sys.stderr
        )

    @patch("builtins.print")
    def test_verbose_prints_success_message(self, mock_print):
        """Verbose mode should print success message when no errors."""
        mock_test_files = MagicMock()
        mock_test_files.test_type = "C"
        check_test_files.TestFiles.echo_feedback(mock_test_files, 0, 1)
        mock_print.assert_called_once_with("Found all expected C test files.")

    @patch("builtins.print")
    def test_no_output_without_error_and_verbose(self, mock_print):
        """No output when there are no errors and verbosity is off."""
        mock_test_files = MagicMock()
        mock_test_files.test_type = "C"
        check_test_files.TestFiles.echo_feedback(mock_test_files, 0, 0)
        mock_print.assert_not_called()


class TestTypeProperties(unittest.TestCase):
    """Direct tests for test type properties."""

    def test_python_test_type(self):
        """Python test type property returns expected value."""
        handler = check_test_files.PythonTestFiles(set())
        self.assertEqual(handler.test_type, "Python")

    def test_c_test_type(self):
        """C test type property returns expected value."""
        handler = check_test_files.CTestFiles(set())
        self.assertEqual(handler.test_type, "C")


class TestPythonTestFilesInit(unittest.TestCase):
    """Tests for candidate classification in PythonTestFiles."""

    def test_filters_source_and_test_paths(self):
        """Only cli and tests/cli Python paths should be tracked."""
        files = {
            Path("cli/commands/c_log.py"),
            Path("tests/cli/commands/test_c_log.py"),
            Path("src/app/main.c"),
            Path("README.md"),
        }
        handler = check_test_files.PythonTestFiles(files)
        self.assertEqual(handler._source_files, [Path("cli/commands/c_log.py")])
        self.assertEqual(
            handler._test_files, [Path("tests/cli/commands/test_c_log.py")]
        )


class TestPythonCheckStyle(unittest.TestCase):
    """Tests for Python style checks."""

    @patch("cli.pre_commit_scripts.check_test_files.ast.parse")
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles._check_docstring")
    @patch(
        "cli.pre_commit_scripts.check_test_files.PythonTestFiles._check_main_unittest"
    )
    @patch("pathlib.Path.read_text")
    def test_runs_docstring_and_main_checks(
        self,
        mock_read_text,
        mock_main,
        mock_doc,
        mock_parse,
    ):
        """Each test file should be parsed and validated."""
        handler = check_test_files.PythonTestFiles(
            {Path("tests/cli/commands/test_c_log.py")}
        )
        mock_read_text.return_value = '"""doc"""'
        mock_parse.return_value = ast.parse("pass")
        mock_doc.return_value = 1
        mock_main.return_value = 1

        ret = handler.check_style()

        self.assertEqual(ret, 2)
        mock_read_text.assert_called_once_with(encoding="utf-8")
        mock_doc.assert_called_once()
        mock_main.assert_called_once()


class TestPythonCheckTestFiles(unittest.TestCase):
    """Tests wrapper that combines Python checks."""

    @patch(
        "cli.pre_commit_scripts.check_test_files.PythonTestFiles.check_style",
        return_value=2,
    )
    @patch(
        "cli.pre_commit_scripts.check_test_files.PythonTestFiles.check_for_missing_test_files",
        return_value=3,
    )
    def test_sums_style_and_missing(self, mock_missing, mock_style):
        """check_test_files should sum both check results."""
        handler = check_test_files.PythonTestFiles(set())
        ret = handler.check_test_files(1)
        self.assertEqual(ret, 5)
        mock_missing.assert_called_once_with(1)
        mock_style.assert_called_once_with(1)


class TestPythonCheckDocstring(unittest.TestCase):
    """Tests for Python docstring format checks."""

    def test_valid_docstring(self):
        """Correct docstring prefix should pass."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        tree = ast.parse('"""Testing file \'cli/commands/c_log.py\'."""')
        ret = handler._check_docstring(Path("tests/cli/commands/test_c_log.py"), tree)
        self.assertEqual(ret, 0)

    def test_invalid_docstring(self):
        """Wrong docstring should fail."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        tree = ast.parse('"""wrong"""')
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler._check_docstring(
                Path("tests/cli/commands/test_c_log.py"), tree
            )
        self.assertEqual(ret, 1)
        self.assertIn("is missing a docstring", err.getvalue())

    def test_non_test_path_without_prefix_strip(self):
        """Non-tests path should be validated without tests/test_ stripping."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        tree = ast.parse('"""Testing file \'cli/commands/c_log.py\'."""')
        ret = handler._check_docstring(Path("cli/commands/c_log.py"), tree)
        self.assertEqual(ret, 0)


class TestPythonCheckMain(unittest.TestCase):
    """Tests for __main__/unittest.main() checks."""

    def test_main_with_unittest_is_valid(self):
        """Valid __main__ and unittest.main should pass."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        script = """
import unittest
if __name__ == '__main__':
    unittest.main()
"""
        ret = handler._check_main_unittest(Path("x.py"), ast.parse(script))
        self.assertEqual(ret, 0)

    def test_missing_unittest_main_is_invalid(self):
        """Missing unittest.main call should fail."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        script = """
if __name__ == '__main__':
    print('hi')
"""
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler._check_main_unittest(Path("x.py"), ast.parse(script))
        self.assertEqual(ret, 1)
        self.assertIn("must define '__main__'", err.getvalue())

    def test_invalid_main_compare_is_ignored(self):
        """If condition not checking __name__ should not count as main guard."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        script = """
import unittest
if something == '__main__':
    unittest.main()
"""
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler._check_main_unittest(Path("x.py"), ast.parse(script))
        self.assertEqual(ret, 1)
        self.assertIn("must define '__main__'", err.getvalue())

    def test_main_body_non_call_node_is_ignored(self):
        """Non-call expression statements inside main should be ignored."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        script = """
if __name__ == '__main__':
    value = 1
"""
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler._check_main_unittest(Path("x.py"), ast.parse(script))
        self.assertEqual(ret, 1)
        self.assertIn("must define '__main__'", err.getvalue())


class TestPythonMissingTests(unittest.TestCase):
    """Tests for Python source-to-test presence checks."""

    @patch("pathlib.Path.exists", return_value=True)
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles.echo_feedback")
    def test_all_expected_tests_exist(self, mock_feedback, mock_exists):
        """No errors when expected test file exists."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 0)
        mock_exists.assert_called_once_with()
        mock_feedback.assert_called_once_with(0, 0)

    @patch("pathlib.Path.exists", return_value=False)
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles.echo_feedback")
    def test_missing_test_file_is_reported(self, mock_feedback, mock_exists):
        """Missing expected test should be reported."""
        handler = check_test_files.PythonTestFiles({Path("cli/commands/c_log.py")})
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 1)
        self.assertIn("expects a test file", err.getvalue())
        mock_exists.assert_called_once_with()
        mock_feedback.assert_called_once_with(1, 0)

    @patch("pathlib.Path.exists", return_value=False)
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles.echo_feedback")
    def test_init_file_is_skipped(self, mock_feedback, mock_exists):
        """__init__.py source should not trigger existence checks."""
        handler = check_test_files.PythonTestFiles({Path("cli/__init__.py")})
        ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 0)
        mock_exists.assert_not_called()
        mock_feedback.assert_called_once_with(0, 0)


class TestCTestFilesInit(unittest.TestCase):
    """Tests for candidate classification in CTestFiles."""

    def test_filters_source_and_test_paths(self):
        """Only src app/bootloader/opt and tests/unit test_*.c should be tracked."""
        files = {
            Path("src/app/main/main.c"),
            Path("src/portable/file.c"),
            Path("tests/unit/app/main/test_main.c"),
            Path("tests/unit/app/main/helper.c"),
        }
        handler = check_test_files.CTestFiles(files)
        self.assertEqual(handler._source_files, [Path("src/app/main/main.c")])
        self.assertEqual(handler._test_files, [Path("tests/unit/app/main/test_main.c")])


class TestCSourceToTestMapping(unittest.TestCase):
    """Tests for C source-to-test path mapping."""

    def test_source_file_to_test_file(self):
        """C source path should map to tests/unit path."""
        ret = check_test_files.CTestFiles._source_file_to_test_file(
            Path("src/bootloader/main/main.c")
        )
        self.assertEqual(ret, Path("tests/unit/bootloader/main/test_main.c"))


class TestCMissingTests(unittest.TestCase):
    """Tests for C source-to-test presence checks."""

    @patch("pathlib.Path.exists", return_value=True)
    @patch("cli.pre_commit_scripts.check_test_files.CTestFiles.echo_feedback")
    def test_all_expected_tests_exist(self, mock_feedback, mock_exists):
        """No errors when expected C test file exists."""
        handler = check_test_files.CTestFiles({Path("src/app/main/main.c")})
        ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 0)
        mock_exists.assert_called_once_with()
        mock_feedback.assert_called_once_with(0, 0)

    @patch("pathlib.Path.exists", return_value=False)
    @patch("cli.pre_commit_scripts.check_test_files.CTestFiles.echo_feedback")
    def test_missing_test_file_is_reported(self, mock_feedback, mock_exists):
        """Missing expected C test file should be reported."""
        handler = check_test_files.CTestFiles({Path("src/app/main/main.c")})
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 1)
        self.assertIn("Missing test file", err.getvalue())
        mock_exists.assert_called_once_with()
        mock_feedback.assert_called_once_with(1, 0)


class TestWscriptFiles(unittest.TestCase):
    """Tests for dedicated wscript pairing checks."""

    def test_type(self):
        """Wscript handler type is reported correctly."""
        handler = check_test_files.WscriptTestFiles(set())
        self.assertEqual(handler.test_type, "wscript")

    @patch("pathlib.Path.exists", return_value=False)
    @patch("cli.pre_commit_scripts.check_test_files.WscriptTestFiles.echo_feedback")
    def test_missing_wscript_test_file_is_reported(self, mock_feedback, mock_exists):
        """Missing tests/unit wscript for src wscript should be reported."""
        handler = check_test_files.WscriptTestFiles({Path("src/app/wscript")})
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 1)
        self.assertRegex(
            err.getvalue(),
            r"Missing test file 'tests[\\/]unit[\\/]app[\\/]wscript'",
        )
        mock_exists.assert_called_once_with()
        mock_feedback.assert_called_once_with(1, 0)

    @patch("pathlib.Path.exists", return_value=True)
    @patch("cli.pre_commit_scripts.check_test_files.WscriptTestFiles.echo_feedback")
    def test_existing_wscript_test_file(self, mock_feedback, mock_exists):
        """Existing tests/unit wscript should pass."""
        handler = check_test_files.WscriptTestFiles({Path("src/bootloader/wscript")})
        ret = handler.check_for_missing_test_files()
        self.assertEqual(ret, 0)
        mock_exists.assert_called_once_with()
        mock_feedback.assert_called_once_with(0, 0)

    def test_check_style_is_noop(self):
        """Wscript check has no style phase."""
        handler = check_test_files.WscriptTestFiles({Path("src/app/wscript")})
        self.assertEqual(handler.check_style(), 0)

    @patch(
        "cli.pre_commit_scripts.check_test_files.WscriptTestFiles.check_style",
        return_value=2,
    )
    @patch(
        "cli.pre_commit_scripts.check_test_files.WscriptTestFiles.check_for_missing_test_files",
        return_value=3,
    )
    def test_check_test_files_sums_results(self, mock_missing, mock_style):
        """Wscript check_test_files should sum missing and style return values."""
        handler = check_test_files.WscriptTestFiles(set())
        ret = handler.check_test_files(1)
        self.assertEqual(ret, 5)
        mock_missing.assert_called_once_with(1)
        mock_style.assert_called_once_with(1)


class TestCCheckTestFiles(unittest.TestCase):
    """Tests wrapper that combines C checks."""

    @patch(
        "cli.pre_commit_scripts.check_test_files.CTestFiles.check_style",
        return_value=2,
    )
    @patch(
        "cli.pre_commit_scripts.check_test_files.CTestFiles.check_for_missing_test_files",
        return_value=3,
    )
    def test_sums_style_and_missing(self, mock_missing, mock_style):
        """check_test_files should sum both check results."""
        handler = check_test_files.CTestFiles(set())
        ret = handler.check_test_files(1)
        self.assertEqual(ret, 5)
        mock_missing.assert_called_once_with(1)
        mock_style.assert_called_once_with(1)


class TestCCheckStyle(unittest.TestCase):
    """Tests for C style checks."""

    @patch("pathlib.Path.read_text")
    def test_detects_invalid_test_signatures(self, mock_read_text):
        """C tests with missing (void) should fail style check."""
        handler = check_test_files.CTestFiles({Path("tests/unit/app/main/test_main.c")})
        mock_read_text.return_value = (
            '#include "x.h"\nvoid testSetUp() {\n}\nvoid testTearDown(void) {\n}\n'
        )
        err = io.StringIO()
        with redirect_stderr(err):
            ret = handler.check_style()
        self.assertEqual(ret, 1)
        self.assertIn("Test files need to have the form", err.getvalue())


class TestMain(unittest.TestCase):
    """Tests for main argument handling and dispatch."""

    @patch("cli.pre_commit_scripts.check_test_files.WscriptTestFiles")
    @patch("cli.pre_commit_scripts.check_test_files.CTestFiles")
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles")
    def test_dispatches_by_suffix(self, mock_py_cls, mock_c_cls, mock_wscript_cls):
        """Main should route .py and .c files to dedicated handlers."""
        mock_py = MagicMock()
        mock_c = MagicMock()
        mock_wscript = MagicMock()
        mock_py.check_test_files.return_value = 2
        mock_c.check_test_files.return_value = 3
        mock_wscript.check_test_files.return_value = 4
        mock_py_cls.return_value = mock_py
        mock_c_cls.return_value = mock_c
        mock_wscript_cls.return_value = mock_wscript

        ret = check_test_files.main(
            ["-v", "cli/a.py", "src/app/main/main.c", "src/app/wscript", "README.md"]
        )

        self.assertEqual(ret, 9)
        mock_py_cls.assert_called_once_with({Path("cli/a.py")})
        mock_c_cls.assert_called_once_with({Path("src/app/main/main.c")})
        mock_wscript_cls.assert_called_once_with({Path("src/app/wscript")})
        mock_py.check_test_files.assert_called_once_with(1)
        mock_c.check_test_files.assert_called_once_with(1)
        mock_wscript.check_test_files.assert_called_once_with(1)

    @patch("cli.pre_commit_scripts.check_test_files.WscriptTestFiles")
    @patch("cli.pre_commit_scripts.check_test_files.CTestFiles")
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles")
    def test_returns_zero_when_no_files(
        self, mock_py_cls, mock_c_cls, mock_wscript_cls
    ):
        """No filenames should return early with success."""
        ret = check_test_files.main([])
        self.assertEqual(ret, 0)
        mock_py_cls.assert_not_called()
        mock_c_cls.assert_not_called()
        mock_wscript_cls.assert_not_called()

    @patch("cli.pre_commit_scripts.check_test_files.WscriptTestFiles")
    @patch("cli.pre_commit_scripts.check_test_files.CTestFiles")
    @patch("cli.pre_commit_scripts.check_test_files.PythonTestFiles")
    def test_forwards_wscript_to_dedicated_checks(
        self, mock_py_cls, mock_c_cls, mock_wscript_cls
    ):
        """Main should forward wscript files to WscriptTestFiles."""
        mock_py = MagicMock()
        mock_c = MagicMock()
        mock_wscript = MagicMock()
        mock_py.check_test_files.return_value = 0
        mock_c.check_test_files.return_value = 0
        mock_wscript.check_test_files.return_value = 0
        mock_py_cls.return_value = mock_py
        mock_c_cls.return_value = mock_c
        mock_wscript_cls.return_value = mock_wscript

        ret = check_test_files.main(["src/app/wscript"])

        self.assertEqual(ret, 0)
        mock_py_cls.assert_called_once_with(set())
        mock_c_cls.assert_called_once_with(set())
        mock_wscript_cls.assert_called_once_with({Path("src/app/wscript")})


class TestAsRepoRelative(unittest.TestCase):
    """Tests for repository-relative path normalization."""

    def test_absolute_path_under_repo_is_made_relative(self):
        """Absolute path under cwd should become relative."""
        ret = check_test_files._as_repo_relative((Path.cwd() / "cli/a.py").resolve())
        self.assertEqual(ret, Path("cli/a.py"))

    @patch("pathlib.Path.relative_to", side_effect=ValueError)
    def test_absolute_path_outside_repo_keeps_absolute(self, _mock_relative_to):
        """Path not relative to cwd should be returned unchanged."""
        absolute = (Path.cwd() / "cli/a.py").resolve()
        ret = check_test_files._as_repo_relative(absolute)
        self.assertEqual(ret, absolute)


class TestStartsWith(unittest.TestCase):
    """Tests for prefix matching helper."""

    def test_starts_with_matches_prefix(self):
        """Prefix helper should match by path parts."""
        self.assertTrue(
            check_test_files._starts_with(Path("tests/cli/x.py"), ("tests", "cli"))
        )
        self.assertFalse(
            check_test_files._starts_with(Path("cli/x.py"), ("tests", "cli"))
        )


if __name__ == "__main__":
    unittest.main()
