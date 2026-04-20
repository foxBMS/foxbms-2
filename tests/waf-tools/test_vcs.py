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


# cspell: ignore gabcdef gdeadbeef
"""Testing file 'tools/waf-tools/vcs.py'."""

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).parents[2]
sys.path.insert(0, str(ROOT / "tools/waf-tools"))

from vcs import (  # noqa: E402 pylint: disable=wrong-import-position
    MAXIMUM_COMMIT_HASH_LENGTH,
    REMOTE_MAXIMUM_STRING_LENGTH,
    VcsInformation,
)


class TestVcsInformation(unittest.TestCase):
    def test_defaults(self):
        v = VcsInformation()
        self.assertFalse(v.under_version_control)
        self.assertTrue(v.dirty)
        self.assertEqual(v.tag, "untagged")
        self.assertEqual(v.major, 0)
        self.assertEqual(v.minor, 0)
        self.assertEqual(v.patch, 0)
        self.assertEqual(v.remote, "unknown")
        self.assertEqual(v.distance, 65535)
        self.assertEqual(v.short_hash, "FFFFFFFFFFFFFF")
        self.assertEqual(v.full_hash, "FFFFFFFFFFFFFFFF")
        self.assertEqual(v.version, "no-vcs-0.0.0-untagged-65535-FFFFFFFFFFFFFF-")
        self.assertTrue(v.version.endswith("-"))

    def test_major_minor_patch_set_ints(self):
        v = VcsInformation()
        v.major = 1
        v.minor = 2
        v.patch = 3
        self.assertEqual((v.major, v.minor, v.patch), (1, 2, 3))

    def test_major_minor_patch_set_literals(self):
        v = VcsInformation()
        v.major = "x"
        v.minor = "y"
        v.patch = "z"
        self.assertEqual(v.major, ord("x"))
        self.assertEqual(v.minor, ord("y"))
        self.assertEqual(v.patch, ord("z"))

    def test_remote_truncation(self):
        v = VcsInformation()
        long_remote = "r" * (REMOTE_MAXIMUM_STRING_LENGTH + 50)
        v.remote = long_remote
        self.assertEqual(len(v.remote), REMOTE_MAXIMUM_STRING_LENGTH)
        self.assertEqual(v.remote, long_remote[:REMOTE_MAXIMUM_STRING_LENGTH])

    def test_remote_no_truncation_at_boundary(self):
        v = VcsInformation()
        exact_remote = "r" * REMOTE_MAXIMUM_STRING_LENGTH
        v.remote = exact_remote
        self.assertEqual(v.remote, exact_remote)

    def test_full_hash_sets_short_hash_truncated(self):
        v = VcsInformation()
        full = "a" * 40
        v.full_hash = full
        self.assertEqual(v.full_hash, full)
        self.assertEqual(v.short_hash, full[:MAXIMUM_COMMIT_HASH_LENGTH])
        self.assertEqual(len(v.short_hash), MAXIMUM_COMMIT_HASH_LENGTH)
        self.assertTrue(v.version.endswith(f"{v.short_hash}-"))

    def test_full_hash_sets_short_hash_short_input(self):
        v = VcsInformation()
        full = "c" * 10
        v.full_hash = full
        self.assertEqual(v.short_hash, full)  # shorter than maximum, so unchanged
        self.assertEqual(len(v.short_hash), 10)

    def test_dirty_property(self):
        v = VcsInformation()
        v.dirty = False
        self.assertFalse(v.dirty)
        v.dirty = True
        self.assertTrue(v.dirty)

    def test_distance_property_and_version_contains_it(self):
        v = VcsInformation()
        v.distance = 123
        self.assertEqual(v.distance, 123)
        self.assertIn("-123-", v.version)

    def test_under_version_control_affects_prefix(self):
        v = VcsInformation()
        v.under_version_control = True
        v.major = 1
        v.minor = 2
        v.patch = 3
        v.tag = "release"
        v.distance = 5
        v.full_hash = "0123456789abcdef0123456789abcdef0123"
        self.assertFalse(v.version.startswith("no-vcs-"))
        self.assertTrue(v.version.startswith("1.2.3-"))
        self.assertNotIn("untagged-", v.version)  # tag appended only when 'untagged'

    def test_untagged_tag_is_included_in_version(self):
        v = VcsInformation()
        v.under_version_control = True
        v.major = 1
        v.minor = 2
        v.patch = 3
        v.tag = "untagged"
        v.distance = 0
        v.full_hash = "b" * 20
        expected = f"1.2.3-untagged-0-{v.short_hash}-"
        self.assertEqual(v.version, expected)

    def test_short_hash_is_read_only(self):
        v = VcsInformation()
        with self.assertRaises(AttributeError):
            v.short_hash = "1234567890abcdef"


if __name__ == "__main__":
    unittest.main()
