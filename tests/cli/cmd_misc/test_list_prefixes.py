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

"""Testing file 'cli/cmd_misc/list_prefixes.py'."""

import shutil
import sys
import tempfile
import unittest
from pathlib import Path

try:
    from cli.cmd_misc.list_prefixes import get_prefixes
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_misc.list_prefixes import get_prefixes


class TestListPrefixes(unittest.TestCase):
    """Test @prefix extraction for misc command."""

    def setUp(self):
        self._tmpdir = tempfile.mkdtemp()
        self.root = Path(self._tmpdir)

    def tearDown(self):
        shutil.rmtree(self._tmpdir)

    def test_collect_prefixes(self):
        """Collect prefixes from C files and return sorted result."""
        (self.root / "src" / "app").mkdir(parents=True)
        (self.root / "src" / "bootloader").mkdir(parents=True)

        (self.root / "src" / "app" / "ok.c").write_text(
            "/* @prefix ABC */\nint x = 0;\n", encoding="utf-8"
        )
        (self.root / "src" / "bootloader" / "ok.h").write_text(
            "/* @prefix DEF */\n", encoding="utf-8"
        )

        prefixes = get_prefixes(self.root)
        self.assertEqual(["ABC", "DEF"], prefixes)

    def test_ignore_files_outside_fixed_roots(self):
        """Ignore files outside src/app and src/bootloader."""
        (self.root / "src" / "app").mkdir(parents=True)
        (self.root / "src" / "other").mkdir(parents=True)
        (self.root / "tools").mkdir(parents=True)

        (self.root / "src" / "other" / "a.c").write_text(
            "/* @prefix OTHER */\n", encoding="utf-8"
        )
        (self.root / "tools" / "b.h").write_text(
            "/* @prefix TOOLS */\n", encoding="utf-8"
        )
        (self.root / "src" / "app" / "valid.c").write_text(
            "/* @prefix MAIN */\n", encoding="utf-8"
        )

        prefixes = get_prefixes(self.root)
        self.assertEqual(["MAIN"], prefixes)

    def test_missing_roots_return_empty(self):
        """Return an empty list if fixed roots do not exist."""
        self.assertEqual([], get_prefixes(self.root))

    def test_ignore_non_c_h_and_lines_without_prefix(self):
        """Ignore unsupported suffixes and files without @prefix marker."""
        (self.root / "src" / "app").mkdir(parents=True)

        (self.root / "src" / "app" / "ignore.txt").write_text(
            "@prefix TXT\n", encoding="utf-8"
        )
        (self.root / "src" / "app" / "no_marker.c").write_text(
            "/* no prefix marker */\n", encoding="utf-8"
        )
        (self.root / "src" / "app" / "multi_line.h").write_text(
            "/* first line */\n/* @prefix abc */\n", encoding="utf-8"
        )

        prefixes = get_prefixes(self.root)
        self.assertEqual(["ABC"], prefixes)


if __name__ == "__main__":
    unittest.main()
