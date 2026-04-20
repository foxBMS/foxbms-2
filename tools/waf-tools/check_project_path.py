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

"""Validate the project path on the file system

- The project path shall not contain spaces
- The project path shall not be too long as this might cause problems on
  Windows when paths in the build directory are then getting too long
"""

from pathlib import Path
from waflib import Utils, Logs


def configure(ctx):  # noqa: D103
    ctx.start_msg("Checking project path")

    if " " in ctx.path.abspath():
        ctx.end_msg("Invalid", color="RED")
        ctx.fatal(f"Project path must not contain spaces ({ctx.path}).")

    known_max_depth = 133
    expected_max_path_depth = len(ctx.path.abspath()) + known_max_depth
    if Utils.is_win32 and expected_max_path_depth > 260:
        ctx.end_msg("Invalid", color="RED")
        ctx.fatal(
            "Build path length will exceed 260 characters.\n"
            "Clone or move the repository into a shorter path."
        )
    else:
        Logs.debug(f"Expected max path depth: {expected_max_path_depth}")

    ctx.env.append_unique("PROJECT_ROOT", Path(ctx.path.abspath()).as_posix())

    ctx.end_msg(True)
