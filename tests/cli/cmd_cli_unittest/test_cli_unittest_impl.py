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

"""Testing file 'cli/cmd_cli_unittest/cli_unittest_impl.py'."""

import io
import os
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_cli_unittest.cli_unittest_impl import (
        COVERAGE_MODULE_BASE_COMMAND,
        PROJECT_ROOT,
        UNIT_TEST_MODULE_BASE_COMMAND,
        _add_verbosity_to_cmd_list,
        run_script_tests,
        run_unittest_module,
    )
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_cli_unittest.cli_unittest_impl import (
        COVERAGE_MODULE_BASE_COMMAND,
        PROJECT_ROOT,
        UNIT_TEST_MODULE_BASE_COMMAND,
        _add_verbosity_to_cmd_list,
        run_script_tests,
        run_unittest_module,
    )
    from cli.helpers.spr import SubprocessResult


class TestUnittestImpl(unittest.TestCase):
    """Test Unittest implementation script"""

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_unittest_module_called_with_args(self, mock_run_process: MagicMock):
        """Check unittest module runs with args"""
        args = ["something", "some-other-thing"]
        mock_run_process.return_value = SubprocessResult(0)
        result = run_unittest_module(args)
        expected_cmd = UNIT_TEST_MODULE_BASE_COMMAND + args
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None
        )
        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_with_coverage(self, mock_run_process: MagicMock):
        """test commands with coverage"""
        mock_run_process.return_value = SubprocessResult(0)
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = run_script_tests(coverage_report=True)
        expected_cmd = [
            COVERAGE_MODULE_BASE_COMMAND
            + [
                "run",
                "--parallel-mode",
                "--source=cli",
                "-m",
                "unittest",
                "discover",
                "-s",
                f"tests{os.sep}cli",
            ],
            COVERAGE_MODULE_BASE_COMMAND
            + [
                "run",
                "--parallel-mode",
                PROJECT_ROOT / "tests/cli/fallback/test_fallback.py",
            ],
            COVERAGE_MODULE_BASE_COMMAND
            + [
                "run",
                "--parallel-mode",
                PROJECT_ROOT / "tests/waf-tools/test_crc64_ti_impl.py",
            ],
            COVERAGE_MODULE_BASE_COMMAND + ["combine"],
            COVERAGE_MODULE_BASE_COMMAND + ["report"],
            COVERAGE_MODULE_BASE_COMMAND
            + ["html", "-d", PROJECT_ROOT / "build/cli-selftest"],
            COVERAGE_MODULE_BASE_COMMAND
            + [
                "xml",
                "-o",
                PROJECT_ROOT
                / "build/cli-selftest"
                / "CoberturaCoverageCliSelfTest.xml",
            ],
        ]
        mock_run_process.assert_has_calls(
            [
                unittest.mock.call(
                    expected_cmd[0], cwd=PROJECT_ROOT, stdout=None, stderr=None
                ),
                unittest.mock.call(
                    expected_cmd[1], cwd=PROJECT_ROOT, stdout=None, stderr=None
                ),
                unittest.mock.call(
                    expected_cmd[2], cwd=PROJECT_ROOT, stdout=None, stderr=None
                ),
                unittest.mock.call(
                    expected_cmd[3], cwd=PROJECT_ROOT, stdout=None, stderr=None
                ),
            ],
            any_order=False,
        )
        self.assertEqual(result.returncode, 0)
        self.assertRegex(
            buf.getvalue(),
            r"The cli unit tests were successful.\n"
            r"Total testing time: .*s",
        )

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_with_coverage_several_errors(
        self, mock_run_process: MagicMock
    ):
        """test commands with coverage"""
        mock_run_process.return_value = SubprocessResult(1)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = run_script_tests(coverage_report=True)
        self.assertEqual(result.returncode, mock_run_process.call_count)
        self.assertEqual(err.getvalue(), "The cli unit tests were not successful.\n")
        self.assertRegex(out.getvalue(), r"Total testing time: .*s\n")

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_without_coverage(self, mock_run_process: MagicMock):
        """test command without coverage"""
        mock_run_process.return_value = SubprocessResult(0)
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = run_script_tests(coverage_report=False)
        expected_cmd = UNIT_TEST_MODULE_BASE_COMMAND + [
            "discover",
            "-s",
            f"tests{os.sep}cli",
        ]
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None
        )
        self.assertEqual(result, mock_run_process.return_value)
        self.assertRegex(
            buf.getvalue(),
            r"The cli unit tests were successful.\n"
            r"Total testing time: .*s",
        )

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_script_failure(self, mock_run_process: MagicMock):
        """test command without coverage"""
        mock_run_process.return_value = SubprocessResult(1)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = run_script_tests(coverage_report=False)
        expected_cmd = UNIT_TEST_MODULE_BASE_COMMAND + [
            "discover",
            "-s",
            f"tests{os.sep}cli",
        ]
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None
        )
        self.assertEqual(result, mock_run_process.return_value)
        self.assertEqual(err.getvalue(), "The cli unit tests were not successful.\n")
        self.assertRegex(out.getvalue(), r"Total testing time: .*s")

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.Path.is_file", return_value=True)
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.Path.unlink", return_value=None)
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.terminal_link_print")
    def test_run_script_tests_cov_file_exists_and_tests_succeed(
        self, mock_tlp: MagicMock, mock_run_process: MagicMock, *_
    ):
        """test commands with coverage"""
        mock_tlp.return_value = "foo"
        mock_run_process.return_value = SubprocessResult(0)
        out = io.StringIO()
        with redirect_stdout(out):
            result = run_script_tests(coverage_report=True)
        self.assertEqual(result.returncode, 0)
        self.assertRegex(
            out.getvalue(),
            r"The cli unit tests were successful.\n\n"
            r"coverage report: foo\n"
            r"Total testing time: .*s",
        )


class TestUnittestImplAddVerbosityToCmdList(unittest.TestCase):
    """Test Unittest implementation script"""

    def test__add_verbosity_to_cmd_list_verbosity_0(self):
        """Do not add verbosity flag in case of verbosity 0"""
        ret = _add_verbosity_to_cmd_list(["foo"])
        self.assertEqual(["foo"], ret)

    def test__add_verbosity_to_cmd_list_verbosity_2(self):
        """Add verbosity flag '-vv' in case of verbosity 2"""
        ret = _add_verbosity_to_cmd_list(["foo"], verbosity=2)
        self.assertEqual(["foo", "-vv"], ret)


if __name__ == "__main__":
    unittest.main()
