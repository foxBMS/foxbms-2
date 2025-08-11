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

"""Testing file 'cli/pre_commit_scripts/check_cpp_style_comment.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.pre_commit_scripts import check_cpp_style_comment
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import check_cpp_style_comment


class TestCppComments(unittest.TestCase):
    """Testing "forbid C++-style comments" script"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_ok(self):
        """line starts with comment"""
        ret = check_cpp_style_comment.main(
            [str(self.tests_dir / "no-cpp-style-comment.c")]
        )
        self.assertEqual(0, ret)

    def test_ok_link(self):
        """line starts with comment"""
        ret = check_cpp_style_comment.main(
            [str(self.tests_dir / "no-cpp-style-comment-link.c")]
        )
        self.assertEqual(0, ret)

    def test_not_ok_0(self):
        """line starts with comment"""
        test = "cpp-style-comment_0.c"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_cpp_style_comment.main([str(self.tests_dir / test)])

        self.assertEqual(result, 1)
        self.assertEqual(
            f"{(self.tests_dir / test).as_posix()}:1: C++-style comments are not allowed.\n",
            err.getvalue(),
        )

    def test_not_ok_1(self):
        """line starts with comment"""
        test = "cpp-style-comment_1.c"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_cpp_style_comment.main([str(self.tests_dir / test)])

        self.assertEqual(result, 1)
        self.assertEqual(
            f"{(self.tests_dir / test).as_posix()}:3: C++-style comments are not allowed.\n",
            err.getvalue(),
        )

    def test_not_ok_2(self):
        """line starts with comment"""
        test = "cpp-style-comment_2.c"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_cpp_style_comment.main([str(self.tests_dir / test)])

        self.assertEqual(result, 1)
        self.assertEqual(
            f"{(self.tests_dir / test).as_posix()}:3: C++-style comments are not allowed.\n",
            err.getvalue(),
        )

    def test_not_ok_3(self):
        """line starts with comment"""
        test = "cpp-style-comment_3.c"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_cpp_style_comment.main([str(self.tests_dir / test)])

        self.assertEqual(result, 1)
        self.assertEqual(
            f"{(self.tests_dir / test).as_posix()}:2: C++-style comments are not allowed.\n",
            err.getvalue(),
        )


if __name__ == "__main__":
    unittest.main()
