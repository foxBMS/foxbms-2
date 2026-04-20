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


"""Testing file 'tools/waf-tools/create_version.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

ROOT = Path(__file__).parents[2]
FIXTURES = ROOT / "tests/waf-tools/fixtures/create_version"
FIXED_DATE = "2026-03-23"

for i in [
    ROOT / "tools/waf-tools",
    ROOT / "tools/waf3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Windows
    ROOT / "tools/waf.3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable-next=wrong-import-position
from create_version import create_version_c  # noqa:E402
from vcs import VcsInformation  # noqa:E402 pylint: disable=wrong-import-position


class CreateVersionCTests(unittest.TestCase):
    """Fixture-based tests for create_version_c."""

    @patch("create_version.datetime")
    def test_render_matches_expected_fixture(self, mock_datetime: MagicMock):
        """Render the template and compare against a committed golden file."""
        mock_datetime.now.return_value.date.return_value.strftime.return_value = (
            FIXED_DATE
        )

        version = VcsInformation()
        version.major = 7
        version.minor = 8
        version.patch = 9
        version.distance = 5
        version.full_hash = "deadbeef"
        version.remote = "origin"
        version.under_version_control = True
        version.dirty = False

        ctx = MagicMock()
        ctx.gather_and_validate_version_info.return_value = version
        ctx.env.VERSION = "7.8.9"

        template = (ROOT / "conf/tpl/c.c").read_text(encoding="utf-8")
        expected = (FIXTURES / "expected-version.c").read_text(encoding="utf-8")

        result = create_version_c(ctx, template)
        self.assertMultiLineEqual(expected, result)


if __name__ == "__main__":
    unittest.main()
