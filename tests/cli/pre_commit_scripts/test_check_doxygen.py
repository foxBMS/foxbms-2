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

"""Testing file 'cli/pre_commit_scripts/check_doxygen.py'."""

import io
import sys
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest.mock import patch

try:
    from cli.helpers.misc import PROJECT_ROOT
    from cli.pre_commit_scripts import check_doxygen
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.helpers.misc import PROJECT_ROOT
    from cli.pre_commit_scripts import check_doxygen


def expected_error_msg(file_path: str, label: str):
    """Creates the expected error message"""
    return f"{file_path}: Doxygen @{label} field is wrong/missing."


class TestCheckDoxygenComment(unittest.TestCase):
    """Test of the main function"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_valid_doxygen(self):
        """A valid doxygen comment shall not raise an error"""
        result = check_doxygen.main([str(self.tests_dir / "valid-doxygen.c")])
        self.assertEqual(result, 0)

    @patch("cli.pre_commit_scripts.check_doxygen.Popen")
    def test_invalid_version(self, mock_popen):
        """A invalid number"""
        process = mock_popen.return_value.__enter__.return_value
        process.returncode = 0
        process.communicate.return_value = (b"foxBMS 2: 0.0.0", b"")
        test_file = "invalid-version.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "version"), buf.getvalue())

    @patch("cli.pre_commit_scripts.check_doxygen.Popen")
    def test_version_not_determinable(self, mock_popen):
        """A invalid number"""
        process = mock_popen.return_value.__enter__.return_value
        process.returncode = 0
        process.communicate.return_value = (b"", b"")
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / "invalid-version.c")])
        self.assertIn("Could not determine foxBMS version.", buf.getvalue())
        self.assertEqual(result, 1)

    def test_invalid_encoding(self):
        """Invalid file encoding"""
        test_file = "invalid-encoding_utf-16.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(f"{test_file}: Could not ASCII-decode this file.", buf.getvalue())

    def test_no_doxygen_comments(self):
        """All doxygen comments are missing"""
        test_file = "no-doxygen-comments.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 10)
        self.assertIn(
            f"{test_file}: Doxygen comment start marker is missing.", buf.getvalue()
        )
        self.assertIn(expected_error_msg(test_file, "file"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "author"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "date"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "updated"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "version"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "ingroup"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "prefix"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "brief"), buf.getvalue())
        self.assertIn(expected_error_msg(test_file, "details"), buf.getvalue())

    def test_no_start_comment(self):
        """Doxygen block comment start label is missing"""
        test_file = "no-start-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(
            f"{test_file}: Doxygen comment start marker is missing.", buf.getvalue()
        )

    def test_no_file_comment(self):
        """@file comment is missing"""
        test_file = "no-file-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "file"), buf.getvalue())

    def test_no_author_comment(self):
        """@author comment is missing"""
        test_file = "no-author-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "author"), buf.getvalue())

    def test_ignore_author_comment(self):
        """@author shall be ignored"""
        test_file = (
            (self.tests_dir / "ignore-author-comment.c")
            .relative_to(PROJECT_ROOT)
            .as_posix()
        )
        check_doxygen.IGNORE_ERROR["author"].append(test_file)
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([test_file])
        self.assertEqual(result, 0)

    def test_no_date_comment(self):
        """@date comment is missing"""
        test_file = "no-date-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "date"), buf.getvalue())

    def test_no_updated_comment(self):
        """@updated comment is missing"""
        test_file = "no-updated-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "updated"), buf.getvalue())

    def test_no_ingroup_comment(self):
        """@ingroup comment is missing"""
        test_file = "no-ingroup-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "ingroup"), buf.getvalue())

    def test_no_prefix_comment(self):
        """@prefix comment is missing"""
        test_file = "no-prefix-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "prefix"), buf.getvalue())

    def test_no_brief_comment(self):
        """@brief comment is missing"""
        test_file = "no-brief-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "brief"), buf.getvalue())

    def test_multiline_brief_comment(self):
        """@brief shall be supported"""
        test_file = "multiline-brief.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 0)

    def test_no_details_comment(self):
        """@details comment is missing"""
        test_file = "no-details-comment.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "details"), buf.getvalue())

    def test_empty_line_between_multiline_brief_and_details(self):
        """@details comment is missing"""
        test_file = "empty-line-between-brief-and-details.c"
        buf = io.StringIO()
        with redirect_stdout(buf):
            result = check_doxygen.main([str(self.tests_dir / test_file)])
        self.assertEqual(result, 1)
        self.assertIn(expected_error_msg(test_file, "details"), buf.getvalue())


if __name__ == "__main__":
    unittest.main()
