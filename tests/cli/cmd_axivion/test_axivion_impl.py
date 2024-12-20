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

"""Testing file 'cli/cmd_axivion/axivion_impl.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import patch

try:
    from cli.cmd_axivion import axivion_impl
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.cmd_axivion import axivion_impl
    from cli.helpers.spr import SubprocessResult


class TestArchitecture(unittest.TestCase):
    """Test architecture"""

    @patch("cli.cmd_axivion.axivion_impl.shutil.which", return_value=None)
    def test_gravis_not_found(self, _):
        """Test for no gravis path found"""
        result = SubprocessResult(0)
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = axivion_impl.export_architecture(verbosity=0)
        self.assertEqual(result, SubprocessResult(1))
        self.assertIn("Could not find gravis!", buf.getvalue())

    @patch("cli.cmd_axivion.axivion_impl.shutil.which")
    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_gravis_found_default_path_no_verbosity(self, mock_run_process, mock_which):
        """test found gravis path with no verbosity"""
        mock_which.return_value = "/path/to/gravis"
        mock_run_process.return_value = SubprocessResult(0)
        # Calls the export_architecture function with the mocks.
        result = axivion_impl.export_architecture()
        # Assert shutil.which correctly called with gravis
        mock_which.assert_called_once_with("gravis")
        # Assert run_process called with correct args
        mock_run_process.assert_called_once_with(
            cmd=["/path/to/gravis", "--script", str(axivion_impl.GRAVIS_EXPORT_SCRIPT)],
            cwd=axivion_impl.PROJECT_ROOT,
            stderr=None,
            stdout=None,
        )
        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.shutil.which")
    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_gravis_found_default_path_with_verbosity(
        self, mock_run_process, mock_which
    ):
        """test found gravis path with verbosity"""
        mock_which.return_value = "/path/to/gravis"
        mock_run_process.return_value = SubprocessResult(0)

        result = axivion_impl.export_architecture(verbosity=1)
        # Check if shutil.which was called exactly once with argument 'gravis'
        mock_which.assert_called_once_with("gravis")
        mock_run_process.assert_called_once_with(
            cmd=[
                "/path/to/gravis",
                "--script",
                str(axivion_impl.GRAVIS_EXPORT_SCRIPT),
                "-v",
            ],
            cwd=axivion_impl.PROJECT_ROOT,
            stderr=None,
            stdout=None,
        )
        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_calls_the_up_to_date_script(self, mock_run_process):
        """test architecture up to date with no verbosity"""
        mock_run_process.return_value = SubprocessResult(0)

        result = axivion_impl.check_if_architecture_up_to_date()

        expected_cmd = [
            sys.executable,
            str(axivion_impl.ARCHITECTURE_UP_TO_DATE_SCRIPT),
        ]
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_calls_the_up_to_date_script_with_verbosity(self, mock_run_process):
        """test architecture up to date with verbosity"""
        mock_run_process.return_value = SubprocessResult(0)

        result = axivion_impl.check_if_architecture_up_to_date(1)

        expected_cmd = [
            sys.executable,
            str(axivion_impl.ARCHITECTURE_UP_TO_DATE_SCRIPT),
            "-v",
        ]
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_check_versions(self, mock_run_process):
        """test version script with no verbosity"""
        mock_run_process.return_value = SubprocessResult(0)

        result = axivion_impl.check_versions()

        expected_cmd = [sys.executable, str(axivion_impl.VERSION_CHECK_SCRIPT)]
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_check_versions_with_verbosity(self, mock_run_process):
        """test version script with verbosity"""
        mock_run_process.return_value = SubprocessResult(0)

        result = axivion_impl.check_versions(1)

        expected_cmd = [sys.executable, str(axivion_impl.VERSION_CHECK_SCRIPT), "-v"]
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.script_impl.run_python_script")
    def test_check_violations(self, mock_run_python_script):
        """test check violations"""
        # Prepare test arguments
        test_args = ["report", "analysis_report.json"]
        # Call the function under test
        result = axivion_impl.check_violations(test_args)
        # Expected command
        expected_cmd = [str(axivion_impl.CHECK_VIOLATIONS_SCRIPT)] + test_args
        # Assertions
        mock_run_python_script.assert_called_once_with(
            expected_cmd, cwd=axivion_impl.PROJECT_ROOT
        )
        self.assertEqual(result, 0)

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_combine_report_files(self, mock_run_process):
        """test combine reports with no verbosity"""
        # Test inputs
        reports = [Path("report1.json"), Path("report2.json")]
        result = axivion_impl.combine_report_files(reports)

        expected_cmd = [sys.executable, str(axivion_impl.COMBINE_REPORTS_SCRIPT)] + [
            str(i) for i in reports
        ]
        # Assertions
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value.returncode)

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_combine_report_files_with_verbosity(self, mock_run_process):
        """test combine reports with verbosity"""
        # Test inputs
        reports = [Path("report1.json"), Path("report2.json")]
        result = axivion_impl.combine_report_files(reports, 1)

        expected_cmd = [sys.executable, str(axivion_impl.COMBINE_REPORTS_SCRIPT)] + [
            str(i) for i in reports
        ]
        expected_cmd.append("-v")
        # Assertions
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )

        self.assertEqual(result, mock_run_process.return_value.returncode)

    @patch("cli.cmd_axivion.axivion_impl.shutil.which")
    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_calls_self_test_script(self, mock_run_process, mock_which):
        """test self test script for a found gravis path"""
        # Test inputs
        script_args = ["something", "some-other-thing"]
        mock_run_process.return_value = SubprocessResult(0)
        # Call the function under test
        result = axivion_impl.self_test(script_args)
        # Expected command
        expected_cmd = [
            sys.executable,
            str(axivion_impl.SELF_TEST_SCRIPT),
        ] + script_args
        # Assertions
        mock_which.assert_called_once_with("gravis")
        mock_run_process.assert_called_once_with(
            cmd=expected_cmd, cwd=axivion_impl.PROJECT_ROOT, stderr=None, stdout=None
        )
        self.assertEqual(result, mock_run_process.return_value)

    @patch("cli.cmd_axivion.axivion_impl.shutil.which", return_value=None)
    def test_calls_self_test_script_gravis_not_found(self, _):
        """test self test script for no gravis path found"""
        script_args = ["something", "some-other-thing"]
        result = SubprocessResult(0)
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = axivion_impl.self_test(script_args)
        self.assertEqual(result, SubprocessResult(1))
        self.assertIn("Could not find gravis!", buf.getvalue())

    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_start_local_dashserver(self, mock_run_process):
        """test run local dashserver"""
        mock_run_process.return_value = SubprocessResult(0)
        db_file = "db_file"
        # Call the function under test
        result = axivion_impl.start_local_dashserver(db_file)
        # Expected command
        expected_cmd = [
            "dashserver",
            "start",
            "--local",
            "--noauth",
            f"--install_file={db_file}",
        ]
        # Assertions
        mock_run_process.assert_called_once_with(expected_cmd)
        self.assertEqual(result, mock_run_process.return_value)


if __name__ == "__main__":
    unittest.main()
