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

"""Testing file 'cli/cmd_cli_unittest/cli_unittest_impl.py'."""

import io
import os
import subprocess
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch, sentinel

try:
    from cli.cmd_cli_unittest.cli_unittest_impl import (
        COVERAGE_MODULE_BASE_COMMAND,
        PROJECT_ROOT,
        UNIT_TEST_MODULE_BASE_COMMAND,
        _add_verbosity_to_cmd_list,
        _ensure_unpacked_waf,
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
        _ensure_unpacked_waf,
        run_script_tests,
        run_unittest_module,
    )
    from cli.helpers.spr import SubprocessResult


@patch("cli.cmd_cli_unittest.cli_unittest_impl.importlib.util.find_spec")
@patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
class TestEnsureUnpackedWaf(unittest.TestCase):
    """Test of the '_ensure_unpacked_waf' function"""

    def expected_cmd(self) -> list[str]:
        """Command that is expected to be run"""
        return [sys.executable, str(PROJECT_ROOT / "tools/waf"), "-h"]

    def test_waflib_available(
        self, run_mock: MagicMock, find_spec_mock: MagicMock
    ) -> None:
        """Do not call subprocess call as waflib is available"""
        find_spec_mock.return_value = sentinel.spec
        _ensure_unpacked_waf()
        find_spec_mock.assert_called_once_with("waflib")
        run_mock.assert_not_called()

    def test_waflib_missing(
        self, run_mock: MagicMock, find_spec_mock: MagicMock
    ) -> None:
        """Unpacked waf is called as waflib is not available"""
        find_spec_mock.return_value = None
        _ensure_unpacked_waf()
        run_mock.assert_called_once_with(
            self.expected_cmd(),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    def test_output_suppressed_and_check_disabled(
        self, run_mock: MagicMock, find_spec_mock: MagicMock
    ) -> None:
        """Discard output and tolerate non-zero exit code"""
        find_spec_mock.return_value = None
        _ensure_unpacked_waf()
        kwargs = run_mock.call_args.kwargs
        self.assertIs(kwargs["stdout"], subprocess.DEVNULL)
        self.assertIs(kwargs["stderr"], subprocess.DEVNULL)

    def test_nonzero_exit_does_not_raise(
        self, run_mock: MagicMock, find_spec_mock: MagicMock
    ) -> None:
        """Failing to run waf must not raise"""
        find_spec_mock.return_value = None
        run_mock.return_value = subprocess.CompletedProcess(args=[], returncode=1)
        self.assertIsNone(_ensure_unpacked_waf())  # type: ignore[func-returns-value]


class TestUnittestImpl(unittest.TestCase):
    """Test Unittest implementation script"""

    @patch("cli.cmd_cli_unittest.cli_unittest_impl._ensure_unpacked_waf")
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_unittest_module_called_with_args(
        self, mock_run_process: MagicMock, mock_ensure_unpacked_waf: MagicMock
    ) -> None:
        """Check unittest module runs with args"""
        args = ["something", "some-other-thing"]
        mock_run_process.return_value = SubprocessResult(0)
        mock_ensure_unpacked_waf.return_value = None
        result = run_unittest_module(args)
        expected_cmd = UNIT_TEST_MODULE_BASE_COMMAND + args
        mock_run_process.assert_called_once_with(
            expected_cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None
        )
        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_with_coverage(self, mock_run_process: MagicMock) -> None:
        """Test commands with coverage"""
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
        ]
        files = sorted(
            [
                PROJECT_ROOT / "tests/waf_tools/test_app_build_config_generate.py",
                PROJECT_ROOT / "tests/waf_tools/test_battery_config_validate_utils.py",
                PROJECT_ROOT / "tests/waf_tools/test_c_codegen_template.py",
                PROJECT_ROOT / "tests/waf_tools/test_config_validate_utils.py",
                PROJECT_ROOT / "tests/waf_tools/test_crc64_ti_impl.py",
                PROJECT_ROOT / "tests/waf_tools/test_diag_array_config_validate.py",
                PROJECT_ROOT / "tests/waf_tools/test_misc_helpers.py",
                PROJECT_ROOT / "tests/waf_tools/test_validate_test_json.py",
                PROJECT_ROOT / "tests/waf_tools/test_vcs.py",
                PROJECT_ROOT / "tests/waf_tools/test_vcs_git.py",
                PROJECT_ROOT / "tests/waf_tools/test_version_generate.py",
            ]
        )
        files.extend(
            [
                PROJECT_ROOT / "tests/pkg/test_hatch_build.py",
                PROJECT_ROOT / "tests/can/test_check_ids.py",
            ]
        )

        expected_cmd.extend(
            COVERAGE_MODULE_BASE_COMMAND + ["run", "--parallel-mode", i] for i in files
        )
        expected_cmd.extend(
            [
                COVERAGE_MODULE_BASE_COMMAND + ["combine"],
                COVERAGE_MODULE_BASE_COMMAND + ["report"],
                COVERAGE_MODULE_BASE_COMMAND
                + ["html", "-d", PROJECT_ROOT / "build/cli-selftest"],
                COVERAGE_MODULE_BASE_COMMAND
                + [
                    "xml",
                    "-o",
                    PROJECT_ROOT
                    / "build/cli-selftest/CoberturaCoverageCliSelfTest.xml",
                ],
            ]
        )
        mock_run_process.assert_has_calls(
            [
                unittest.mock.call(i, cwd=PROJECT_ROOT, stdout=None, stderr=None)
                for i in expected_cmd
            ],
            any_order=False,
        )
        self.assertEqual(result.returncode, 0)
        self.assertRegex(
            buf.getvalue(),
            r"The cli unit tests were successful.\n"
            r"Total testing time: .*s",
        )

    @patch("cli.cmd_cli_unittest.cli_unittest_impl._ensure_unpacked_waf")
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_with_coverage_several_errors(
        self, mock_run_process: MagicMock, mock_ensure_unpacked_waf: MagicMock
    ) -> None:
        """Test commands with coverage"""
        mock_ensure_unpacked_waf.return_value = None
        mock_run_process.return_value = SubprocessResult(1)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = run_script_tests(coverage_report=True)
        self.assertEqual(result.returncode, mock_run_process.call_count)
        self.assertEqual(err.getvalue(), "The cli unit tests were not successful.\n")
        self.assertRegex(out.getvalue(), r"Total testing time: .*s\n")

    @patch("cli.cmd_cli_unittest.cli_unittest_impl._ensure_unpacked_waf")
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    def test_run_script_tests_without_coverage(
        self, mock_run_process: MagicMock, mock_ensure_unpacked_waf: MagicMock
    ) -> None:
        """Test command without coverage"""
        mock_ensure_unpacked_waf.return_value = None
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
    def test_run_script_tests_script_failure(self, mock_run_process: MagicMock) -> None:
        """Test command without coverage"""
        failure = SubprocessResult(1)
        mock_run_process.side_effect = [None, failure]

        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            result = run_script_tests(coverage_report=False)

        expected_cmd = UNIT_TEST_MODULE_BASE_COMMAND + [
            "discover",
            "-s",
            f"tests{os.sep}cli",
        ]
        self.assertEqual(
            mock_run_process.call_args_list[1],
            unittest.mock.call(
                expected_cmd, cwd=PROJECT_ROOT, stdout=None, stderr=None
            ),
        )
        self.assertEqual(result, failure)
        self.assertEqual(err.getvalue(), "The cli unit tests were not successful.\n")
        self.assertRegex(out.getvalue(), r"Total testing time: .*s")

    @patch("cli.cmd_cli_unittest.cli_unittest_impl.Path.is_file", return_value=True)
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.Path.unlink", return_value=None)
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.run_process")
    @patch("cli.cmd_cli_unittest.cli_unittest_impl.terminal_link_print")
    def test_run_script_tests_cov_file_exists_and_tests_succeed(
        self, mock_tlp: MagicMock, mock_run_process: MagicMock, *_: MagicMock
    ) -> None:
        """Test commands with coverage"""
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

    def test__add_verbosity_to_cmd_list_verbosity_0(self) -> None:
        """Do not add verbosity flag in case of verbosity 0"""
        ret = _add_verbosity_to_cmd_list(["foo"])
        self.assertEqual(["foo"], ret)

    def test__add_verbosity_to_cmd_list_verbosity_2(self) -> None:
        """Add verbosity flag '-vv' in case of verbosity 2"""
        ret = _add_verbosity_to_cmd_list(["foo"], verbosity=2)
        self.assertEqual(["foo", "-vv"], ret)


if __name__ == "__main__":
    unittest.main()
