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

"""Testing file 'cli/helpers/spr.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.helpers.spr import SubprocessResult, prepare_subprocess_output, run_process
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.helpers.spr import SubprocessResult, prepare_subprocess_output, run_process


class TestSpR(unittest.TestCase):
    """Test of the main entry point"""

    def test_prepare_subprocess_output(self):
        """basic prepare_subprocess_output test"""
        ret = prepare_subprocess_output(-1, b"foxBMS", b"dummy")
        self.assertEqual(-1, ret.returncode)
        self.assertEqual("foxBMS", ret.out)
        self.assertEqual("dummy", ret.err)

        ret = prepare_subprocess_output(1, b"", b"")
        self.assertEqual(1, ret.returncode)
        self.assertEqual("", ret.out)
        self.assertEqual("", ret.err)

    def test_add_sprs(self):
        """test adding of two 'SubprocessResult's"""
        ret = prepare_subprocess_output(
            -1, b"foxBMS", b"dummy"
        ) + prepare_subprocess_output(1, b"1", b"2")
        self.assertEqual(2, ret.returncode)
        self.assertEqual("foxBMS\n\n1", ret.out)
        self.assertEqual("dummy\n\n2", ret.err)

    def test_run_process_no_program(self):
        """test running a program, without providing a program"""
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = run_process([])
        self.assertEqual(1, ret.returncode)
        self.assertEqual("", ret.out)
        self.assertEqual("No program provided.", ret.err)
        self.assertIn("No program provided.", buf.getvalue())

    def test_run_process_invalid_program(self):
        """test running a program, without providing a valid program"""
        cmd = ["does-not-exist"]
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = run_process(cmd)
        self.assertEqual(1, ret.returncode)
        self.assertEqual("", ret.out)
        self.assertEqual(f"Program '{cmd[0]}' does not exist.", ret.err)
        self.assertIn(f"Program '{cmd[0]}' does not exist.", buf.getvalue())

    def test_spr_str(self):
        """Test string-representation of the SubprocessResult class"""
        dummy = SubprocessResult(1, "abc", "def")
        self.assertEqual("return code: 1\n\nout:abc\n\ndef\n", str(dummy))


if __name__ == "__main__":
    unittest.main()
