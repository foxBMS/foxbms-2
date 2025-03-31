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

"""Testing file 'cli/cmd_embedded_ut/embedded_ut_impl.py'."""

import importlib
import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import ANY, MagicMock, call, mock_open, patch

try:
    from cli.cmd_embedded_ut import embedded_ut_impl
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_embedded_ut import embedded_ut_impl
    from cli.helpers.spr import SubprocessResult


class TestEmbeddedUTImplModuleImport(unittest.TestCase):
    """Testing 'embedded_ut_impl.py'"""

    def setUp(self):
        importlib.reload(embedded_ut_impl)
        return super().setUp()

    def test_platform_linux(self):
        """test Linux."""
        with patch("sys.platform", new="linux"):
            importlib.reload(embedded_ut_impl)
        self.assertTrue(
            str(embedded_ut_impl.CEEDLING_PROJECT_FILE_SRC_APP).count("posix") == 1
        )

    def test_platform_win32(self):
        """test dummy'PLATFORM' value."""
        with patch("sys.platform", new="win32"):
            importlib.reload(embedded_ut_impl)
        self.assertTrue(
            str(embedded_ut_impl.CEEDLING_PROJECT_FILE_SRC_APP).count("win32") == 1
        )


class TestEmbeddedUTImplRunEmbeddedTests(unittest.TestCase):
    """Testing 'run_embedded_tests' function."""

    @classmethod
    def setUpClass(cls):
        cls.out_start = (
            "Testing project: '{}'\n"
            "Running 'ceedling' directly: all arguments are passed to "
            "ceedling verbatim.\n"
        )
        return super().setUpClass()

    def test_run_embedded_tests_invalid_project(self):
        """Invalid project"""
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = self.out_start.format("dummy")
        with redirect_stderr(err), redirect_stdout(out):
            with self.assertRaises(SystemExit) as cm:
                embedded_ut_impl.run_embedded_tests([], project="dummy")
        self.assertEqual(
            cm.exception.code,
            "Something went wrong.\nExpect argument from list "
            "('app', 'bootloader'), but got 'dummy'.",
        )
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

    @patch("cli.cmd_embedded_ut.embedded_ut_impl._make_unit_test_dir")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._copy_config_if_needed")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._run_ceedling")
    def test_run_embedded_tests_no_args(
        self,
        m_run_ceedling: MagicMock,
        m_copy_config_if_needed: MagicMock,
        m_make_unit_test_dir: MagicMock,
    ):
        """Invalid project"""
        m_run_ceedling.return_value = SubprocessResult(0)
        m_copy_config_if_needed.return_value = None
        m_make_unit_test_dir.return_value = None
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = self.out_start.format("app")
        with redirect_stderr(err), redirect_stdout(out):
            ret = embedded_ut_impl.run_embedded_tests([], project="app")
        self.assertEqual(ret, SubprocessResult(0))
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)
        m_run_ceedling.assert_called_once_with(
            ["help"],
            cwd=Path(__file__).parents[3] / "build/app_host_unit_test",
            stderr=None,
            stdout=None,
        )

    @patch("cli.cmd_embedded_ut.embedded_ut_impl._make_unit_test_dir")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._copy_config_if_needed")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.glob")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.get_multiple_files_hash_str")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._run_halcogen")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._run_ceedling")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._print_result")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_run_embedded_tests_no_pickle(
        self,
        m_print_result: MagicMock,
        m_run_ceedling: MagicMock,
        m_run_halcogen: MagicMock,
        m_get_multiple_files_hash_str: MagicMock,
        m_glob: MagicMock,
        m_is_file: MagicMock,
        m_copy_config_if_needed: MagicMock,
        m_make_unit_test_dir: MagicMock,
    ):
        """Pickle file does not exist"""
        m_run_ceedling.return_value = SubprocessResult(0)
        m_run_halcogen.return_value = None
        m_get_multiple_files_hash_str.return_value = "1234"
        m_glob.return_value = []
        m_is_file.return_value = False
        m_copy_config_if_needed.return_value = False
        m_make_unit_test_dir.return_value = None
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = self.out_start.format("app")
        with redirect_stderr(err), redirect_stdout(out):
            ret = embedded_ut_impl.run_embedded_tests(
                ["gcov:test_abc.c"], project="app"
            )
        self.assertEqual(ret, SubprocessResult(0))
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

        root = Path(__file__).parents[3]
        cwd = root / "build/app_host_unit_test"
        m_print_result.assert_called_once_with("app", cwd, SubprocessResult(0), True)
        m_run_ceedling.assert_called_once_with(
            ["gcov:test_abc.c"], cwd=cwd, stderr=None, stdout=None
        )
        m_run_halcogen.assert_called_once_with(
            cwd,
            cwd / "app.hcg",
            root / "conf/hcg/app.dil",
            cwd / "app.dil",
            cwd / "manual_runner.pickle",
        )
        m_get_multiple_files_hash_str.assert_called_once_with([])
        m_glob.assert_has_calls([call("source/*.c"), call("include/*.h")])
        m_is_file.assert_called_once()
        m_copy_config_if_needed.assert_called_once_with("app")
        m_make_unit_test_dir.assert_called_once_with("app")

    @patch("cli.cmd_embedded_ut.embedded_ut_impl._make_unit_test_dir")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._copy_config_if_needed")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("builtins.open", new_callable=mock_open)
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.pickle.load")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.glob")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.get_multiple_files_hash_str")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._run_halcogen")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._run_ceedling")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._print_result")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_run_embedded_tests_pickle_available_but_invalid(
        self,
        m_print_result: MagicMock,
        m_run_ceedling: MagicMock,
        m_run_halcogen: MagicMock,
        m_get_multiple_files_hash_str: MagicMock,
        m_glob: MagicMock,
        m_load: MagicMock,
        m_open: MagicMock,
        m_is_file: MagicMock,
        m_copy_config_if_needed: MagicMock,
        m_make_unit_test_dir: MagicMock,
    ):
        """Pickle file exist, but is invalid"""
        m_run_ceedling.return_value = SubprocessResult(0)
        m_run_halcogen.return_value = None
        m_get_multiple_files_hash_str.return_value = "1234"
        m_glob.return_value = []
        m_load.side_effect = [AttributeError]
        m_is_file.return_value = True
        m_copy_config_if_needed.return_value = False
        m_make_unit_test_dir.return_value = None
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = self.out_start.format("app")
        with redirect_stderr(err), redirect_stdout(out):
            ret = embedded_ut_impl.run_embedded_tests(
                ["gcov:test_abc.c"], project="app"
            )
        self.assertEqual(ret, SubprocessResult(0))
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

        root = Path(__file__).parents[3]
        cwd = root / "build/app_host_unit_test"
        m_print_result.assert_called_once_with("app", cwd, SubprocessResult(0), True)
        m_run_ceedling.assert_called_once_with(
            ["gcov:test_abc.c"], cwd=cwd, stderr=None, stdout=None
        )
        m_run_halcogen.assert_called_once_with(
            cwd,
            cwd / "app.hcg",
            root / "conf/hcg/app.dil",
            cwd / "app.dil",
            cwd / "manual_runner.pickle",
        )
        m_get_multiple_files_hash_str.assert_called_once_with([])
        m_glob.assert_has_calls([call("source/*.c"), call("include/*.h")])
        m_load.assert_called_once()
        m_open.assert_called_with(cwd / "manual_runner.pickle", "rb")
        m_is_file.assert_called_once()
        m_copy_config_if_needed.assert_called_once_with("app")
        m_make_unit_test_dir.assert_called_once_with("app")

    @patch("cli.cmd_embedded_ut.embedded_ut_impl._make_unit_test_dir")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._copy_config_if_needed")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("builtins.open", new_callable=mock_open)
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.pickle.load")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.glob")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.get_multiple_files_hash_str")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._run_ceedling")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._print_result")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_run_embedded_tests_pickle_available_and_valid(
        self,
        m_print_result: MagicMock,
        m_run_ceedling: MagicMock,
        m_get_multiple_files_hash_str: MagicMock,
        m_glob: MagicMock,
        m_load: MagicMock,
        m_open: MagicMock,
        m_is_file: MagicMock,
        m_copy_config_if_needed: MagicMock,
        m_make_unit_test_dir: MagicMock,
    ):
        """Pickle file exist, but is invalid"""
        m_run_ceedling.return_value = SubprocessResult(0)
        m_get_multiple_files_hash_str.return_value = "1234"
        m_glob.return_value = []
        m_load.return_value = "1234"
        m_is_file.return_value = True
        m_copy_config_if_needed.return_value = False
        m_make_unit_test_dir.return_value = None
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = self.out_start.format("bootloader")
        with redirect_stderr(err), redirect_stdout(out):
            ret = embedded_ut_impl.run_embedded_tests(
                ["gcov:test_abc.c"], project="bootloader"
            )
        self.assertEqual(ret, SubprocessResult(0))
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

        root = Path(__file__).parents[3]
        cwd = root / "build/bootloader_host_unit_test"
        m_print_result.assert_called_once_with(
            "bootloader", cwd, SubprocessResult(0), True
        )
        m_run_ceedling.assert_called_once_with(
            ["gcov:test_abc.c"], cwd=cwd, stderr=None, stdout=None
        )
        m_get_multiple_files_hash_str.assert_called_once_with([])
        m_glob.assert_has_calls([call("source/*.c"), call("include/*.h")])
        m_load.assert_called_once()
        m_open.assert_called_with(cwd / "manual_runner.pickle", "rb")
        m_is_file.assert_called_once()
        m_copy_config_if_needed.assert_called_once_with("bootloader")
        m_make_unit_test_dir.assert_called_once_with("bootloader")


class TestEmbeddedUTImplPrivateCleanupHcGSources(unittest.TestCase):
    """Testing the '_cleanup_hcg_sources' function"""

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.unlink")
    def test_cleanup_hcg_sources(self, mock_unlink: MagicMock):
        """Test when base directory does not exist."""
        mock_unlink.return_value = None
        # pylint: disable-next=protected-access
        embedded_ut_impl._cleanup_hcg_sources(Path("does/not/exist"))

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.unlink")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.read_text")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    def test_cleanup_hcg_sources_exists_no_match(
        self,
        mock_is_file: MagicMock,
        mock_read_text: MagicMock,
        mock_unlink: MagicMock,
    ):
        """Test when base does exists, but regex does not match."""
        mock_unlink.return_value = None
        mock_read_text.return_value = "foo"
        mock_is_file.return_value = True
        with self.assertRaises(SystemExit) as cm:
            # pylint: disable-next=protected-access
            embedded_ut_impl._cleanup_hcg_sources(Path("asd"))
        self.assertEqual(cm.exception.code, "Could not determine clock frequency.")

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.unlink")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.read_text")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.write_text")
    def test_cleanup_hcg_sources_exists_match(
        self,
        mock_write_text: MagicMock,
        mock_is_file: MagicMock,
        mock_read_text: MagicMock,
        mock_unlink: MagicMock,
    ):
        """Test when base does exists and regex matches."""
        mock_unlink.return_value = None
        mock_read_text.return_value = (
            "#define configCPU_CLOCK_HZ ( ( unsigned portLONG ) 100000000 )\n"
        )
        mock_is_file.return_value = True
        mock_write_text.return_value = None

        # pylint: disable-next=protected-access
        embedded_ut_impl._cleanup_hcg_sources(Path("asd"))


class TestEmbeddedUTImplPrivateMakeUnitTestDir(unittest.TestCase):
    """Testing '_make_unit_test_dir' function."""

    def test__make_unit_test_dir_invalid_argument(self):
        """Test project dir creation for an invalid project type."""
        with self.assertRaises(SystemExit) as cm:
            embedded_ut_impl._make_unit_test_dir("dummy")  # pylint: disable=protected-access
        self.assertEqual(
            cm.exception.code,
            "Something went wrong.\nExpect argument from list "
            "('app', 'bootloader'), but got 'dummy'.",
        )

    def test__make_unit_test_dir_app(self):
        """Test project dir creation for 'app' project."""
        embedded_ut_impl._make_unit_test_dir("app")  # pylint: disable=protected-access

    def test__make_unit_test_dir_bootloader(self):
        """Test project dir creation for 'bootloader' project."""
        embedded_ut_impl._make_unit_test_dir("bootloader")  # pylint: disable=protected-access


class TestEmbeddedUTImplPrivatePrintresult(unittest.TestCase):
    """Testing '_print_result' function."""

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    def test__print_result_after_error(self, m_is_file: MagicMock):
        """Unsuccessful unit test run"""
        m_is_file.return_value = False
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            embedded_ut_impl._print_result("app", Path("foo"), SubprocessResult())

        self.assertEqual(err.getvalue(), "The app unit tests were not successful.\n")
        self.assertEqual(out.getvalue(), "")

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    def test__print_result_success_no_coverage(self, m_is_file: MagicMock):
        """Unsuccessful unit test run"""
        m_is_file.return_value = False
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = "\nThe app unit tests were successful.\n"
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            embedded_ut_impl._print_result("app", Path("foo"), SubprocessResult(0))

        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.terminal_link_print")
    def test__print_result_success_and_coverage(
        self, m_terminal_link_print: MagicMock, m_is_file: MagicMock
    ):
        """Unsuccessful unit test run"""
        m_terminal_link_print.return_value = "foo"
        m_is_file.return_value = True
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = "\nThe app unit tests were successful.\n\ncoverage report: foo\n"
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            embedded_ut_impl._print_result(
                "app", Path("foo"), SubprocessResult(0), uses_coverage=True
            )

        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)


class TestEmbeddedUTImplPrivateCopyConfigIfNeeded(unittest.TestCase):
    """Testing the '_copy_config_if_needed' function"""

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.copyfile")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.filecmp.cmp")
    def test__copy_config_if_needed_files_are_missing(
        self,
        m_cmp: MagicMock,
        m_is_file: MagicMock,
        _m_copyfile: MagicMock,
    ):
        """Configuration files are missing, therefore they need to be copied."""
        m_is_file.return_value = False
        m_cmp.return_value = False
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            ret = embedded_ut_impl._copy_config_if_needed("app")
        self.assertTrue(ret)
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.copyfile")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.filecmp.cmp")
    def test__copy_config_if_needed_files_changed(
        self,
        m_cmp: MagicMock,
        m_is_file: MagicMock,
        _m_copyfile: MagicMock,
    ):
        """Configuration files changed, therefore they need to be copied."""
        m_is_file.return_value = True
        m_cmp.return_value = False
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            ret = embedded_ut_impl._copy_config_if_needed("app")
        self.assertTrue(ret)
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

    @patch("builtins.open", new_callable=mock_open)
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.is_file")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.filecmp.cmp")
    def test__copy_config_if_needed_files_are_equal(
        self, m_cmp: MagicMock, m_is_file: MagicMock, _m_open: MagicMock
    ):
        """Configuration files are in sync with the target files, therefore no
        need to copy."""
        m_is_file.return_value = True
        m_cmp.return_value = True
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            ret = embedded_ut_impl._copy_config_if_needed("app")
        self.assertFalse(ret)
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)


class TestEmbeddedUTImplPrivateRunHalcogen(unittest.TestCase):
    """Testing the '_run_halcogen' function"""

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.which")
    def test__run_halcogen_no_halcogen(self, m_which: MagicMock):
        """HALCoGen is not available."""
        m_which.return_value = None
        err = io.StringIO()
        err_exp = (
            "Could not find program 'HALCogen'.\n"
            "Assuming HALCoGen sources are available...\n"
        )
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            embedded_ut_impl._run_halcogen(  # pylint: disable=protected-access
                Path("."), Path("."), Path("."), Path("."), Path(".")
            )
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.which")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.run_process")
    def test__run_halcogen_halcogen_fails(
        self, m_run_process: MagicMock, m_which: MagicMock
    ):
        """HALCoGen fails."""
        m_run_process.return_value = SubprocessResult(1)
        m_which.return_value = "halcogen"
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            with self.assertRaises(SystemExit) as cm:
                embedded_ut_impl._run_halcogen(  # pylint: disable=protected-access
                    Path("."), Path("."), Path("."), Path("."), Path(".")
                )
        self.assertEqual(cm.exception.code, "Could not run HALCoGen.")
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.Path.glob")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.which")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.run_process")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl._cleanup_hcg_sources")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.get_multiple_files_hash_str")
    @patch("builtins.open", new_callable=mock_open)
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.pickle.dump")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.copyfile")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test__run_halcogen_ok(
        self,
        m_copy_file: MagicMock,
        m_dump: MagicMock,
        m_open: MagicMock,
        m_get_multiple_files_hash_str: MagicMock,
        m_cleanup_hcg_sources: MagicMock,
        m_run_process: MagicMock,
        m_which: MagicMock,
        m_glob: MagicMock,
    ):
        """HALCoGen fails."""
        m_copy_file.return_value = None
        m_dump.return_value = None
        m_get_multiple_files_hash_str.return_value = "1234"
        m_cleanup_hcg_sources.return_value = None
        m_run_process.return_value = SubprocessResult(0)
        m_which.return_value = "halcogen"
        m_glob.return_value = []
        err = io.StringIO()
        err_exp = ""
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            embedded_ut_impl._run_halcogen(  # pylint: disable=protected-access
                Path("foo"), Path("bar"), Path("dil_in"), Path("dil_out"), Path("bla")
            )
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)
        m_which.assert_called_once_with("halcogen")
        m_run_process.assert_called_once_with(
            ["halcogen", "-i", "bar"], cwd=Path("foo"), stderr=None, stdout=None
        )
        m_cleanup_hcg_sources.assert_called_once_with(Path("foo"))
        m_get_multiple_files_hash_str.assert_called_once_with([])
        m_open.assert_called_once_with(Path("bla"), "wb")
        m_dump.assert_called_once_with("1234", ANY)
        m_copy_file.assert_called_once_with(Path("dil_in"), Path("dil_out"))


class TestEmbeddedUTImplPrivateRunCeedling(unittest.TestCase):
    """Testing the '_run_ceedling' function"""

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.which")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.run_process")
    def test__run_ceedling(self, mock_run_process: MagicMock, mock_which: MagicMock):
        """TODO"""
        mock_which.side_effect = [None]

        err = io.StringIO()
        err_exp = "Could not find program 'ruby'.\n"
        out = io.StringIO()
        out_exp = ""
        with redirect_stderr(err), redirect_stdout(out):
            # pylint: disable-next=protected-access
            result = embedded_ut_impl._run_ceedling([])
        self.assertEqual(result.returncode, 1)
        self.assertEqual(err.getvalue(), err_exp)
        self.assertEqual(out.getvalue(), out_exp)

        mock_which.side_effect = ["ruby", None]
        buf = io.StringIO()
        with redirect_stderr(buf):
            # pylint: disable-next=protected-access
            result = embedded_ut_impl._run_ceedling([])
        self.assertIn(result.err, "Could not find program 'ceedling'.")
        self.assertEqual(result.returncode, 1)

        # restore
        mock_which.side_effect = ["ruby", "ceedling"]
        mock_run_process.return_value = SubprocessResult(0)
        # pylint: disable-next=protected-access
        result = embedded_ut_impl._run_ceedling([])
        self.assertEqual(result.returncode, 0)

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.which")
    def test__run_ceedling_invalid_input(self, mock_which: MagicMock):
        """Test subprocess wrapper for Ceedling with invalid input"""
        mock_which.side_effect = [None]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = embedded_ut_impl._run_ceedling([])  # pylint:disable=protected-access
        self.assertEqual(result.returncode, 1)
        self.assertEqual("Could not find program 'ruby'.\n", buf.getvalue())

        mock_which.side_effect = ["ruby", None]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = embedded_ut_impl._run_ceedling([])  # pylint:disable=protected-access
        self.assertEqual(result.returncode, 1)
        self.assertEqual("Could not find program 'ceedling'.\n", buf.getvalue())

    @patch("cli.cmd_embedded_ut.embedded_ut_impl.shutil.which")
    @patch("cli.cmd_embedded_ut.embedded_ut_impl.run_process")
    def test__run_ceedling_valid_input(
        self, mock_run_process: MagicMock, mock_which: MagicMock
    ):
        """Test subprocess wrapper for Ceedling with valid input."""
        mock_which.side_effect = ["ruby", "ceedling"]
        mock_run_process.return_value = SubprocessResult(0)
        embedded_ut_impl._run_ceedling(args=["--help"])  # pylint:disable=protected-access
        mock_run_process.assert_called_once_with(
            ["ceedling", "--help"],
            cwd=Path(__file__).parents[3] / "build/app_host_unit_test",
            stderr=None,
            stdout=None,
        )


if __name__ == "__main__":
    unittest.main()
