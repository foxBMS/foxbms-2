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

"""Testing file 'cli/pre_commit_scripts/check_rst_macros.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from typing import ClassVar

try:
    from cli.pre_commit_scripts import check_rst_macros
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import check_rst_macros


class TestRstMacros(unittest.TestCase):
    """Testing rst macros pre-commit script."""

    tests_dir: ClassVar[Path]

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_ok(self) -> None:
        """Valid macro prefixes should return zero."""
        ret = check_rst_macros.main([str(self.tests_dir / "valid.rst")])
        self.assertEqual(0, ret)

    def test_ok_backslash_whitespace_after_macro(self) -> None:
        """A backslash followed by whitespace after a macro should be valid."""
        test_file = self.tests_dir / "valid.rst"
        ret = check_rst_macros.main([str(test_file)])
        self.assertEqual(0, ret)

    def test_ok_plain_hyphen_after_macro(self) -> None:
        """A plain hyphen after a macro should be valid."""
        test_file = self.tests_dir / "valid.rst"
        ret = check_rst_macros.main([str(test_file)])
        self.assertEqual(0, ret)

    def test_not_ok_backslash_hyphen_after_macro(self) -> None:
        """A backslash-hyphen after a macro should be rejected."""
        test = "invalid.rst"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_rst_macros.main([str(self.tests_dir / test)])

        err_msg = (
            f"macro substitution must use {check_rst_macros.delimiter_description()}\n"
        )
        self.assertEqual(result, 7)
        expected = (
            f"{(self.tests_dir / test).as_posix()}:1:2: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:2:3: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:3:3: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:4:4: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:5:4: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:6:3: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:7:3: {err_msg}"
        )
        self.assertEqual(expected, err.getvalue())

    def test_not_ok(self) -> None:
        """Invalid macro prefixes should be reported with position."""
        test = "invalid.rst"
        err = io.StringIO()
        with redirect_stderr(err):
            result = check_rst_macros.main([str(self.tests_dir / test)])

        err_msg = (
            f"macro substitution must use {check_rst_macros.delimiter_description()}\n"
        )
        self.assertEqual(result, 7)
        expected = (
            f"{(self.tests_dir / test).as_posix()}:1:2: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:2:3: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:3:3: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:4:4: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:5:4: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:6:3: {err_msg}"
            f"{(self.tests_dir / test).as_posix()}:7:3: {err_msg}"
        )
        self.assertEqual(expected, err.getvalue())


if __name__ == "__main__":
    unittest.main()
