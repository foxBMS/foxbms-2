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

"""Enforce repository file path length constraints.

This module defines and applies maximum allowed path lengths for files in
different directories of the repository (e.g., default, docs, src).
The limits are defined to ensure compatibility (notably with Windows
MAX_PATH restrictions) and maintain manageable file organization.
It checks all tracked files using git and reports paths that exceed their
configured maximum lengths.

Key functionalities:
- Define per-directory maximum path lengths.
- Check all git-tracked files for path length violations.
- Report any files exceeding their respective limits.

Intended for use as a repository maintenance utility.
"""

import re
from shutil import which
from typing import NotRequired, TypedDict

from ..helpers.click_helpers import recho
from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import run_process

DEFAULT_MAX = 150
DOC_MAX = 125
SRC_MAX = 100


class MaxConfigEntry(TypedDict):
    """Store the maximum allowed path length"""

    max: int
    pattern: NotRequired[re.Pattern[str]]


_MAX_CONFIG: dict[str, MaxConfigEntry] = {
    "default": {
        # no file path shall be longer than 150 characters, so that we have 110
        # characters left on Windows (MAX_PATH)
        "max": DEFAULT_MAX,
    },
    "docs": {
        "pattern": re.compile(r"^docs/"),
        "max": DOC_MAX,
    },
    "src": {
        # the 'src'-source tree shall never be deeper than 100 as we need to
        # create unit test files and these can then become very long in the
        # build directory
        "pattern": re.compile(r"^src(?!/os)"),
        "max": SRC_MAX,
    },
}


def _check_path_length(file: str, allowed: int) -> int:
    """Check if the given file path length exceeds the allowed maximum.

    Args:
        file: The file path to check.
        allowed: The maximum allowed path length.

    Returns:
        0 if path is within limit, 1 otherwise.

    """
    err = 0
    if len(file) > allowed:
        recho(f"File path '{file}' is too long ({len(file)}, allowed {allowed}).")
        err += 1
    return err


def check_repository_depth() -> int:
    """Check that file paths in the repository do not exceed maximum allowed lengths.

    Returns:
        Number of files exceeding their respective path length limits.

    """
    err = 0
    git = str(which("git"))
    cmd = [git, "ls-files"]
    all_files = sorted(run_process(cmd, cwd=PROJECT_ROOT).out.splitlines(), key=len)

    for v in _MAX_CONFIG.values():
        pattern = v.get("pattern", None)
        files = all_files if not pattern else [f for f in all_files if pattern.match(f)]
        err += _check_path_length(files[-1], v["max"])
    return min(err, 256)
