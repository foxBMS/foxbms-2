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

"""Template rendering helpers for C/H templates used by waf tools.

This module supports comment-style placeholders with the delimiter pair:
- opener: ``/* @``
- closer: ``@ */``

Example placeholder in template files:
    /* @file_name@ */
"""

import re
from string import Template


COMMENT_TEMPLATE_PLACEHOLDER_RE = re.compile(
    r"/\*\s*@\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*@\s*\*/"
)
COMMENT_TEMPLATE_LINE_RE = re.compile(  # pylint: disable-next=line-too-long
    r"(?m)^(?P<indent>[ \t]*)/\*\s*@\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*@\s*\*/[ \t]*(?P<newline>\r?\n|$)"
)
REQUIRED_COMMENT_PLACEHOLDERS = {"header_guard_start"}


def render_template(template_text: str, values: dict[str, str]) -> str:
    """Render a template string with comment-style placeholders.

    ``${name}`` placeholders are required and use Python ``string.Template``
    substitution semantics.

    Comment-style placeholders use ``/* @name@ */``. Only
    ``header_guard_start`` is required. Missing optional comment placeholders
    render as empty text. If an optional comment placeholder appears alone on a
    line, the full line is removed, including its newline.

    Args:
        template_text: Template source string.
        values: Mapping of placeholder names to replacement values.

    Returns:
        Rendered template string.

    Raises:
        KeyError: If a required placeholder has no matching value.
    """
    rendered_text = Template(template_text).substitute(values)

    def _replace_full_line_placeholder(match: re.Match[str]) -> str:
        name = match.group("name")
        replacement = values.get(name)
        if replacement is None:
            if name in REQUIRED_COMMENT_PLACEHOLDERS:
                raise KeyError(name)
            return ""
        newline = match.group("newline")
        if newline and not replacement.endswith(("\n", "\r")):
            return replacement + newline
        return replacement

    rendered_text = COMMENT_TEMPLATE_LINE_RE.sub(
        _replace_full_line_placeholder, rendered_text
    )

    def _replace_inline_placeholder(match: re.Match[str]) -> str:
        name = match.group("name")
        replacement = values.get(name)
        if replacement is None:
            if name in REQUIRED_COMMENT_PLACEHOLDERS:
                raise KeyError(name)
            return ""
        return replacement

    return COMMENT_TEMPLATE_PLACEHOLDER_RE.sub(
        _replace_inline_placeholder, rendered_text
    )
