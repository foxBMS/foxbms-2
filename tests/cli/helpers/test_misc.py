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

"""Testing file 'cli/helpers/misc.py'."""

import importlib
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

try:
    from cli.helpers import misc
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import misc

TEST_PATH_REPLACEMENTS = {
    "$DUMMY$": ["hello"],
}


class TestMisc(unittest.TestCase):
    """Test of the main entry point"""

    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).parents[3]

    def test_no_git_available(self):
        """Check behavior, when 'git' is missing"""
        with patch.dict("sys.modules", {"git": None}):
            importlib.reload(misc)
        self.assertEqual(misc.HAVE_GIT, False)

    def test_git_available(self):
        """Check behavior, when 'git' is missing"""
        importlib.reload(misc)
        self.assertEqual(misc.HAVE_GIT, True)

    def test_get_project_root_no_git_available(self):
        """Test root finding 1"""
        with patch.dict("sys.modules", {"git": None}):
            root = misc.get_project_root()
        self.assertEqual(self.root, root)

    def test_get_project_root_git(self):
        """Test root finding 3"""
        root = misc.get_project_root()
        self.assertEqual(self.root, root)

    def test_terminal_link_print(self):
        """test clickable links"""
        result = misc.terminal_link_print("www.foxbms.org")
        self.assertEqual(
            "\033]8;;www.foxbms.org\033\\www.foxbms.org\033]8;;\033\\", result
        )

    def test_get_sha256_file_hash_str(self):
        """Test file hashing helper"""
        test = Path(__file__).parent / Path(__file__).stem / "checksum_test.txt"
        result = misc.get_sha256_file_hash_str(test)
        self.assertEqual(
            result, "bc6924dfde4fb58a5c92ecd2d08c66aa3b58f64582a1af431ac4b3dd109e92de"
        )

    def test_get_multiple_files_hash_str(self):
        """Test file hashing helper"""
        test = [Path(__file__).parent / Path(__file__).stem / "checksum_test.txt"]
        result = misc.get_multiple_files_hash_str(test)
        self.assertEqual(
            result, "bc6924dfde4fb58a5c92ecd2d08c66aa3b58f64582a1af431ac4b3dd109e92de"
        )


if __name__ == "__main__":
    unittest.main()
