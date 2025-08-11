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

"""Testing file 'cli/cmd_misc/check_test_files.py'."""

import ast
import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_misc import check_test_files
    from cli.helpers.spr import SubprocessResult
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc import check_test_files
    from cli.helpers.spr import SubprocessResult


class TestCheckTestFilesPrivateGetCliFiles(unittest.TestCase):
    """Test of '_get_cli_files' function."""

    @patch("cli.cmd_misc.check_test_files.PROJECT_ROOT")
    def test__get_cli_files(self, m_project_root: MagicMock):
        """dummy test"""
        m_project_root.return_value = Path("foo")
        ret = check_test_files._get_cli_files()  # pylint: disable=protected-access
        self.assertEqual(ret, [])


class TestCheckTestFilesPrivateGetTestFiles(unittest.TestCase):
    """Test of '_get_test_files' function."""

    @patch("cli.cmd_misc.check_test_files.PROJECT_ROOT")
    def test__get_cli_files(self, m_project_root: MagicMock):
        """dummy test"""
        m_project_root.return_value = Path("foo")
        ret = check_test_files._get_test_files()  # pylint: disable=protected-access
        self.assertEqual(ret, [])


class TestCheckTestFilesPrivateCheckMainUnittest(unittest.TestCase):
    """Test of '_check_main_unittest' function."""

    def test_no_main_defined(self):
        """Test for no '__main__' defined"""
        script = "print('Hello, World!')"
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual(
            " must define '__main__' and call unittest.main().\n", err.getvalue()
        )

    def test_main_defined_no_unittest(self):
        """Test for '__main__' defined with no unittest.main() call"""
        script = """
if __name__ == '__main__':
    print('Hello, World!')
"""
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual(
            " must define '__main__' and call unittest.main().\n", err.getvalue()
        )

    def test_main_defined_with_unittest(self):
        """Test for '__main__' defined and unittest.main() called"""
        script = """
import unittest

if __name__ == '__main__':
    unittest.main()
"""
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual("", err.getvalue())

    def test_main_not_correct(self):
        """Test for '__main__' incorrect definition"""
        script = """
import unittest

if name == '__main__':
    unittest.main()
"""
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual(
            " must define '__main__' and call unittest.main().\n", err.getvalue()
        )

    def test_main_defined_with_other_call(self):
        """Test for '__main__' defined with a different function called"""
        script = """
import unittest

if __name__ == '__main__':
    some_function()
"""
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual(
            " must define '__main__' and call unittest.main().\n", err.getvalue()
        )

    def test_nested_if_statement(self):
        """Test for '__main__' defined and unittest.main() called nested in if statement"""
        script = """
import unittest

if True:
    if __name__ == '__main__':
        unittest.main()
"""
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual("", err.getvalue())

    def test_main_defined_with_unittest_in_function(self):
        """Test with '__main__' defined with unittest.main() called in different function"""
        script = """
import unittest

def run_tests():
    unittest.main()

if __name__ == '__main__':
    run_tests()
"""

        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual(
            " must define '__main__' and call unittest.main().\n", err.getvalue()
        )

    def test_main_defined_no_call(self):
        """Test with '__main__' defined without a call"""
        script = """
import unittest

if __name__ == '__main__':
    ret = True
"""
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_main_unittest("", ast.parse(script))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual(
            " must define '__main__' and call unittest.main().\n", err.getvalue()
        )


class TestCheckTestFilesPrivateCheckDocstring(unittest.TestCase):
    """Test of '_check_docstring' function."""

    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).parents[3]
        return super().setUpClass()

    def test_invalid_docstring(self):
        """Test function with invalid docstring"""
        test_file = Path("tests/cli/test_cli.py")
        docstring = '"""Testing file \'cli/foo.py\'."""'
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_docstring(test_file, ast.parse(docstring))
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertRegex(
            err.getvalue(),
            r"tests[\\\/]cli[\\\/]test_cli\.py is missing a docstring "
            r'starting with """Testing file \'cli\/cli\.py\'\."""',
        )

    def test_valid_docstring(self):
        """Test function with valid docstring"""
        test_file = Path("tests/cli/test_cli.py")
        docstring = '"""Testing file \'cli/cli.py\'."""'
        err = io.StringIO()
        with redirect_stderr(err):  # pylint: disable-next=protected-access
            ret = check_test_files._check_docstring(test_file, ast.parse(docstring))
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertEqual("", err.getvalue())


class TestCheckTestFiles(unittest.TestCase):
    """Test of 'check_test_files.py'."""

    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).parents[3]
        return super().setUpClass()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files._check_docstring")
    @patch("cli.cmd_misc.check_test_files._check_main_unittest")
    def test_check_for_test_files_test_ok_verbose(
        self,
        mock_main_unittest: MagicMock,
        mock_check_docstring: MagicMock,
        mock_gtf: MagicMock,
        mock_gcf: MagicMock,
    ):
        """Test 'check_for_test_files' function being successful with verbose set"""
        mock_gcf.return_value = [self.root / "cli/cli.py"]
        mock_gtf.return_value = [Path("tests/cli/test_cli.py")]
        mock_check_docstring.return_value = SubprocessResult(0)
        mock_main_unittest.return_value = SubprocessResult(0)
        out = io.StringIO()
        with redirect_stdout(out):
            ret = check_test_files.check_for_test_files(verbose=1)
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.out, "")
        self.assertEqual(ret.err, "")
        self.assertEqual(out.getvalue(), "Found all expected test files.\n")
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()
        mock_main_unittest.assert_called_once()
        mock_check_docstring.assert_called_once()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files._check_docstring")
    @patch("cli.cmd_misc.check_test_files._check_main_unittest")
    def test_check_for_test_files_test_ok(
        self,
        mock_main_unittest: MagicMock,
        mock_check_docstring: MagicMock,
        mock_gtf: MagicMock,
        mock_gcf: MagicMock,
    ):
        """Test 'check_for_test_files' function being successful"""
        mock_gcf.return_value = [self.root / "cli/cli.py"]
        mock_gtf.return_value = [Path("tests/cli/test_cli.py")]
        mock_check_docstring.return_value = SubprocessResult(0)
        mock_main_unittest.return_value = SubprocessResult(0)
        out = io.StringIO()
        with redirect_stdout(out):
            ret = check_test_files.check_for_test_files()
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.out, "")
        self.assertEqual(ret.err, "")
        self.assertEqual(out.getvalue(), "")
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()
        mock_main_unittest.assert_called_once()
        mock_check_docstring.assert_called_once()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files._check_docstring")
    @patch("cli.cmd_misc.check_test_files._check_main_unittest")
    def test_check_for_test_files_init(
        self,
        mock_main_unittest: MagicMock,
        mock_check_docstring: MagicMock,
        mock_gtf: MagicMock,
        mock_gcf: MagicMock,
    ):
        """Test 'check_for_test_files' function with only '__init__.py' files"""
        mock_gcf.return_value = [self.root / "__init__.py"]
        mock_gtf.return_value = [Path("tests/__init__.py")]
        mock_check_docstring.return_value = SubprocessResult(0)
        mock_main_unittest.return_value = SubprocessResult(0)
        out = io.StringIO()
        with redirect_stdout(out):
            ret = check_test_files.check_for_test_files()
        self.assertEqual(ret.returncode, 0)
        self.assertEqual(ret.out, "")
        self.assertEqual(ret.err, "")
        self.assertEqual(out.getvalue(), "")
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()
        mock_main_unittest.assert_not_called()
        mock_check_docstring.assert_not_called()

    @patch("cli.cmd_misc.check_test_files._get_cli_files")
    @patch("cli.cmd_misc.check_test_files._get_test_files")
    @patch("cli.cmd_misc.check_test_files.Path.read_text")
    def test_check_for_test_files_test_test_file_missing(
        self, mock_read_text: MagicMock, mock_gtf: MagicMock, mock_gcf: MagicMock
    ):
        """Test 'check_for_test_files' function being unsuccessful"""
        mock_gcf.return_value = [self.root / "cli/cli.py"]
        mock_gtf.return_value = []
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = check_test_files.check_for_test_files()
        self.assertEqual(ret.returncode, 1)
        self.assertEqual(ret.err, "")
        self.assertEqual(ret.out, "")
        self.assertRegex(
            err.getvalue(),
            r"\'.*cli[\\\/]cli\.py\' expects a test file in "
            r"\'.*tests[\\\/]cli[\\\/]test_cli\.py\'.\n"
            r"Expected test files are missing\.",
        )
        mock_read_text.assert_not_called()
        mock_gtf.assert_called_once()
        mock_gcf.assert_called_once()


if __name__ == "__main__":
    unittest.main()
