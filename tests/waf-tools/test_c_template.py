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


"""Testing file 'tools/waf-tools/c_template.py'."""

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).parents[2]
FIXTURES = ROOT / "tests/waf-tools/fixtures/c_template"

for i in [
    ROOT / "tools/waf-tools",
    ROOT / "tools/waf3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Windows
    ROOT / "tools/waf.3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable-next=wrong-import-position
from c_template import render_template  # noqa:E402


class RenderCommentTemplateTests(unittest.TestCase):
    """Unit tests for string-level placeholder rendering."""

    def test_renders_comment_placeholders(self):
        """Replace placeholders delimited by /* @ and @ */."""
        template_text = "/* @name@ */ -> /* @value@ */"
        result = render_template(template_text, {"name": "TOKEN", "value": "42"})
        self.assertEqual(result, "TOKEN -> 42")

    def test_raises_key_error_for_missing_required_dollar_placeholder_value(self):
        """Raise KeyError when a required ${...} placeholder value is missing."""
        with self.assertRaises(KeyError):
            render_template("${name}", {})

    def test_raises_key_error_for_missing_required_header_guard_start(self):
        """Raise KeyError when required comment placeholder is missing."""
        with self.assertRaises(KeyError):
            render_template("/* @header_guard_start@ */\n", {})

    def test_raises_key_error_for_missing_required_inline_header_guard_start(self):
        """Raise KeyError for missing required comment placeholder used inline."""
        with self.assertRaises(KeyError):
            render_template("x/* @header_guard_start@ */y", {})

    def test_omits_missing_optional_comment_placeholder_line_without_newline(self):
        """Remove full placeholder-only lines when optional values are absent."""
        template_text = "before\n/* @include_directives@ */\nafter\n"
        result = render_template(template_text, {})
        self.assertEqual(result, "before\nafter\n")

    def test_keeps_replacement_as_is_if_it_already_ends_with_newline(self):
        """Do not append an additional newline for full-line replacements ending with newline."""
        template_text = "prefix\n/* @include_directives@ */\nsuffix\n"
        result = render_template(
            template_text,
            {"include_directives": '#include "a.h"\n'},
        )
        self.assertEqual(result, 'prefix\n#include "a.h"\nsuffix\n')

    def test_omits_missing_optional_inline_comment_placeholder(self):
        """Replace missing optional inline comment placeholders with empty text."""
        template_text = "value=/* @optional_token@ */;"
        result = render_template(template_text, {})
        self.assertEqual(result, "value=;")


if __name__ == "__main__":
    unittest.main()
