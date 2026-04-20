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

"""High-level release command helpers used by ``fox release`` commands."""

from pathlib import Path

from ..helpers.misc import PROJECT_ROOT
from . import update_version_core

# Re-exported for compatibility with existing tests and callers.
MAGIC_DATE = update_version_core.MAGIC_DATE
RELEASE_ENTRY_TEMPLATE = update_version_core.RELEASE_ENTRY_TEMPLATE
date_get_today = update_version_core.date_get_today
get_previous_release = update_version_core.get_previous_release
update_c_h_files = update_version_core.update_c_h_files
update_wscript = update_version_core.update_wscript
update_citation = update_version_core.update_citation
update_changelog = update_version_core.update_changelog
update_commit_fragments = update_version_core.update_commit_fragments
update_release_csv = update_version_core.update_release_csv


def update_version(
    from_version: str, to_version: str, root: Path = PROJECT_ROOT
) -> int:
    """Run high-level update-version flow.

    Args:
        from_version: Previous version value.
        to_version: New version value.
        root: Repository root path.

    Returns:
        ``0`` on success.

    Raises:
        ValueError: On invalid command arguments or update preconditions.
    """
    if not to_version:
        err_msg = "Don't know to version to bump."
        raise ValueError(err_msg)
    if from_version == to_version:
        err_msg = "--from and --to cannot be the same value"
        raise ValueError(err_msg)

    update_version_core.apply_update_version(root, from_version, to_version)
    return 0
