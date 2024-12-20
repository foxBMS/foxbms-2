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

"""Testing file 'cli/cli.py'."""

import importlib
import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import Mock, patch

from click.testing import CliRunner

try:
    import cli
    from cli.cli import get_program_config, main
    from cli.cmd_etl.batetl import cmd_decode, cmd_filter
    from cli.foxbms_version import __version__
    from cli.helpers.misc import PROJECT_ROOT
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    import cli
    from cli.cli import get_program_config, main
    from cli.cmd_etl.batetl import cmd_decode, cmd_filter
    from cli.foxbms_version import __version__
    from cli.helpers.misc import PROJECT_ROOT
    from cli.helpers.spr import SubprocessResult


# pylint: disable-next=unused-argument
def dummy(*args, **kwargs):
    """dummy function"""
    return 0


# pylint: disable=too-many-public-methods
class TestFoxCliMain(unittest.TestCase):
    """Test of the main entry point"""

    def test_raise_import_error(self):
        """Check behavior, when 'click' is missing"""
        buf = io.StringIO()
        with redirect_stderr(buf):
            with patch.dict("sys.modules", {"click": None}):
                with self.assertRaises(SystemExit) as cm:
                    importlib.reload(cli.cli)
        self.assertEqual(cm.exception.code, 1)
        self.assertRegex(
            buf.getvalue(),
            r"The 'click' module is required to run this application.\n"
            "Run '.*py.* -m pip install click' to install it",
        )

    def test_get_program_config(self):
        """test '--show-config' option"""
        foxbms_config = get_program_config()
        self.assertEqual({"foxBMS 2": __version__}, foxbms_config)

    def test_main_no_args(self):
        """test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main)
        self.assertEqual(0, result.exit_code)

    def test_main_show_config(self):
        """test main entry point, when no commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main, ["--show-config"])
        self.assertEqual(0, result.exit_code)

    def test_main_waf(self):
        """test main entry point, when 'waf' commands are provided"""
        runner = CliRunner()
        result = runner.invoke(main, ["waf", "--help"])
        self.assertEqual(0, result.exit_code)

        runner = CliRunner()
        result = runner.invoke(main, ["waf", "--cwd", str(PROJECT_ROOT), "--help"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_misc.run_crc_build")
    def test_misc_build_crc_code(self, mock_run_crc_build):
        """build CRC example code"""
        mock_run_crc_build.return_value = SubprocessResult(0, "", "")
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "build-crc-code"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_misc.run_doc_build")
    def test_misc_build_doc_code(self, mock_run_doc_build):
        """build documentation example code"""
        mock_run_doc_build.return_value = SubprocessResult(0, "", "")
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "build-doc-code"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_misc.lint_freertos")
    def test_misc_uncrustify_freertos(self, mock_lint_freertos):
        """build documentation example code"""
        mock_lint_freertos.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "uncrustify-freertos"])
        self.assertEqual(0, result.exit_code)

    def test_misc_convert_no_files(self):
        """build documentation example code"""
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "convert"])
        self.assertEqual(0, result.exit_code)
        self.assertIn("No files provided.", result.stdout)

    @patch("cli.commands.c_misc.convert")
    def test_misc_convert(self, mock_convert):
        """build documentation example code"""
        mock_convert.return_value = 0
        runner = CliRunner()
        result = runner.invoke(
            main, ["misc", "convert", "--to", "unix", str(Path(__file__).parent)]
        )
        self.assertEqual(0, result.exit_code)

    def test_misc_verify_checksum_no_files(self):
        """build documentation example code"""
        runner = CliRunner()
        result = runner.invoke(main, ["misc", "verify-checksum", "xyz"])
        self.assertEqual(1, result.exit_code)
        self.assertIn("No files provided.", result.stdout)

    @patch("cli.commands.c_misc.verify")
    def test_misc_verify_checksum(self, mock_verify):
        """build documentation example code"""
        mock_verify.return_value = 0
        runner = CliRunner()
        result = runner.invoke(
            main, ["misc", "verify-checksum", str(Path(__file__).parent), "xyz"]
        )
        self.assertEqual(0, result.exit_code)

    def test_install(self):
        """check installation message"""
        runner = CliRunner()
        result = runner.invoke(main, ["install"])
        self.assertEqual(0, result.exit_code)

    def test_install_check(self):
        """check installation '--check'-option"""
        runner = CliRunner()
        result = runner.invoke(main, ["install", "--check"])
        self.assertEqual(0, result.exit_code)

    def test_ci_create_readme(self):
        """check CI readme creation"""
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "create-readme"])
        self.assertEqual(0, result.exit_code)

    def test_ci_check_ci_config(self):
        """check CI configuration style"""
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "check-ci-config"])
        self.assertEqual(0, result.exit_code)

    def test_ci_check_coverage(self):
        """check CI coverage checker"""
        runner = CliRunner()
        runner.invoke(main, ["ci", "check-coverage"])
        # ignore the result (i.e., no assert), as the function will be tested
        # anyway

    def test_ci_path_shall_not_exist(self):
        """Test path checking function"""
        # path exists
        runner = CliRunner(mix_stderr=False)
        result = runner.invoke(main, ["ci", "path-shall-not-exist", "wscript"])
        self.assertIn("Path ", result.stderr)
        self.assertIn(" 'wscript' exists.", result.stderr)
        self.assertEqual(1, result.exit_code)

        # path does not exist
        runner = CliRunner()
        result = runner.invoke(main, ["ci", "path-shall-not-exist", "abc/def"])
        self.assertEqual(0, result.exit_code)

    def test_bootloader(self):
        """Test 'bootloader --help' option"""
        runner = CliRunner()
        result = runner.invoke(main, ["bootloader", "--help"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_bootloader.bootloader_impl")
    def test_bootloader_run_app(self, mock_bootloader_impl):
        """Test 'bootloader load-app' command"""
        mock_bootloader_impl.run_app.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["bootloader", "run-app"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_bootloader.bootloader_impl")
    def test_bootloader_load_app(self, mock_bootloader_impl):
        """Test 'bootloader load-app' command"""
        mock_bootloader_impl.load_app.return_value = 0
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("dummy", "w", encoding="utf-8") as f:
                f.write("dummy")
                f.flush()
                result = runner.invoke(main, ["bootloader", "load-app", str(f.name)])
                self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_bootloader.bootloader_impl")
    def test_bootloader_check(self, mock_bootloader_impl):
        """Test 'bootloader check' command"""
        mock_bootloader_impl.check_bootloader.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["bootloader", "check"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_bootloader.bootloader_impl")
    def test_bootloader_reset_bootloader(self, mock_reset_bootloader):
        """Test 'bootloader reset' command"""
        mock_reset_bootloader.reset_bootloader.return_value = 0
        runner = CliRunner()
        result = runner.invoke(main, ["bootloader", "reset"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_program.script_impl")
    def test_run_script(self, mock_script_impl):
        """Test 'run-script' command"""
        mock_script_impl.run_python_script.return_value = SubprocessResult(0, "", "")
        runner = CliRunner(mix_stderr=False)
        result = runner.invoke(main, ["run-script", "dummy-argument"])
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_program.script_impl")
    def test_run_program(self, mock_script_impl):
        """Test 'run-program' command"""
        mock_script_impl.run_program.return_value = SubprocessResult(0, "", "")
        runner = CliRunner(mix_stderr=False)
        result = runner.invoke(main, ["run-program", "dummy-argument"])
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_pre_commit.pre_commit_impl")
    def test_pre_commit(self, mock_run_program):
        """Test 'pre-commit' command"""
        mock_run_program.run_pre_commit.return_value = SubprocessResult(0, "", "")
        runner = CliRunner(mix_stderr=False)
        result = runner.invoke(main, ["pre-commit"])
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_ide.ide_impl")
    def test_ide(self, mock_ide_impl):
        """Test 'ide' command"""

        def dummy():
            pass

        mock_ide_impl.open_ide_generic = dummy
        mock_ide_impl.open_ide_embedded_unit_test_app = dummy
        mock_ide_impl.open_ide_app = dummy
        mock_ide_impl.open_ide_bootloader = dummy
        runner = CliRunner(mix_stderr=False)
        result = runner.invoke(main, ["ide"])
        self.assertEqual(0, result.exit_code)
        result = runner.invoke(main, ["ide", "--no-generic"])
        self.assertEqual(0, result.exit_code)
        result = runner.invoke(main, ["ide", "--app"])
        self.assertEqual(0, result.exit_code)
        result = runner.invoke(main, ["ide", "--bootloader"])
        self.assertEqual(0, result.exit_code)
        result = runner.invoke(main, ["ide", "--embedded-unit-test-app"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_release.release_impl")
    def test_release(self, mock_release_impl):
        """Test 'release' command"""
        mock_release_impl.dummy.return_value = SubprocessResult(0, "", "")
        runner = CliRunner()
        result = runner.invoke(main, ["release"])
        self.assertEqual(0, result.exit_code)

    @patch("cli.commands.c_embedded_ut.embedded_ut_impl")
    def test_ceedling_no_args(self, mock_embedded_ut_impl):
        """Test 'ceedling' command"""
        mock_embedded_ut_impl.run_embedded_tests.return_value = SubprocessResult(
            0, "", ""
        )
        runner = CliRunner()
        result = runner.invoke(main, ["ceedling"])
        self.assertEqual(0, result.exit_code)
        _, args, kwargs = mock_embedded_ut_impl.mock_calls[0]
        self.assertEqual(args, (["help"], "app"))
        self.assertEqual(kwargs, {"stdout": None, "stderr": None})

    @patch("cli.commands.c_embedded_ut.embedded_ut_impl")
    def test_ceedling_dummy_test(self, mock_embedded_ut_impl):
        """Test 'ceedling' command"""
        mock_embedded_ut_impl.run_embedded_tests.return_value = SubprocessResult(
            0, "", ""
        )
        runner = CliRunner()
        result = runner.invoke(main, ["ceedling", "test:test_adc"])
        self.assertEqual(0, result.exit_code)
        _, args, kwargs = mock_embedded_ut_impl.mock_calls[0]
        self.assertEqual(args, (["test:test_adc"], "app"))
        self.assertEqual(kwargs, {"stdout": None, "stderr": None})

        runner = CliRunner()
        result = runner.invoke(
            main, ["ceedling", "--project", "bootloader", "test:test_adc"]
        )
        self.assertEqual(0, result.exit_code)
        _, args, kwargs = mock_embedded_ut_impl.mock_calls[1]
        self.assertEqual(args, (["test:test_adc"], "bootloader"))
        self.assertEqual(kwargs, {"stdout": None, "stderr": None})

    @patch("cli.cmd_etl.batetl.run_decode2file")
    @patch("cli.cmd_etl.batetl.run_decode2stdout")
    @patch("cli.cmd_etl.batetl.can_decode_setup")
    def test_cmd_decode(
        self, setup_mock: Mock, run_decode2stdout_mock: Mock, run_decode2file_mock: Mock
    ) -> None:
        """Tests the decode command line interface

        :param setup_mock: A mock for the CANDecode class setup
        :param run_decode_mock: A Mock for the run_decode function, which
            executes the decoding
        """
        # Case 1: check execution with stdout as output stream
        decode_obj_mock = Mock(name="decode")
        setup_mock.return_value = decode_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("test.yml", mode="w", encoding="utf-8") as _:
                result = runner.invoke(cmd_decode, ["--config", "test.yml"])
                setup_mock.assert_called_once_with(Path("test.yml"))
                run_decode2stdout_mock.assert_called_once_with(decode_obj_mock)
                run_decode2file_mock.assert_not_called()
                self.assertEqual(result.exit_code, 0)
        setup_mock.reset_mock()
        run_decode2stdout_mock.reset_mock()
        run_decode2file_mock.reset_mock()
        # Case 2: # check execution with files as output
        with runner.isolated_filesystem():
            temp = Path("temp")
            temp.mkdir(parents=True, exist_ok=True)
            with open("test.yml", mode="w", encoding="utf-8") as _:
                result = runner.invoke(
                    cmd_decode, ["--config", "test.yml", "--output", "temp"]
                )
                setup_mock.assert_called_once_with(Path("test.yml"))
                run_decode2file_mock.assert_called_once_with(decode_obj_mock, temp)
                run_decode2stdout_mock.assert_not_called()
                self.assertEqual(result.exit_code, 0)
        # Case 3: check help message output
        result = runner.invoke(cmd_filter, [])
        self.assertTrue("Usage" in result.output)

    @patch("cli.cmd_etl.batetl.run_filter")
    @patch("cli.cmd_etl.batetl.can_filter_setup")
    def test_cmd_filter(self, setup_mock: Mock, run_filter_mock: Mock) -> None:
        """Tests the filter command line interface

        :param setup_mock: A mock for the CANFilter class setup
        :param run_filter_mock: A Mock for the run_filter function, which
            executes the filtering
        """
        # Case 1: check execution as expected
        filter_obj_mock = Mock(name="filter")
        setup_mock.return_value = filter_obj_mock
        runner = CliRunner()
        with runner.isolated_filesystem():
            with open("test.txt", mode="w", encoding="utf-8") as _:
                result = runner.invoke(cmd_filter, ["--config", "test.txt"])
                setup_mock.assert_called_once_with(Path("test.txt"))
                run_filter_mock.assert_called_once_with(filter_obj_mock)
                self.assertEqual(result.exit_code, 0)
        # Case 2: check help message output
        result = runner.invoke(cmd_filter, [])
        self.assertTrue("Usage" in result.output)


if __name__ == "__main__":
    unittest.main()
