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

"""Implements the functionalities behind the 'waf' command"""

from pathlib import Path
from subprocess import PIPE
from sys import executable

from ..helpers.host_platform import get_platform
from ..helpers.logger import logger
from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import SubprocessResult, run_process

WAF_BIN = PROJECT_ROOT / "tools/waf"
WAF_DEFAULT_CWD = PROJECT_ROOT
WAF_BASE_CMD = [executable, str(WAF_BIN)]

if get_platform() == "win32":
    # on Windows, the GUI is launched through 'pythonw.exe' and we do not want
    # to launch the build process trough 'pythonw.exe' as this creates popping
    # up terminals.
    WAF_BASE_CMD = [executable.replace("pythonw.exe", "python.exe"), str(WAF_BIN)]


def run_waf(
    args: list[str],
    cwd: str | Path = WAF_DEFAULT_CWD,
    stdout: int | None = PIPE,
    stderr: int | None = PIPE,
) -> SubprocessResult:
    """Execute ``waf`` with the provided argument list.

    Args:
        args: Command-line arguments passed to ``waf``.
        cwd: Working directory used for process execution.
        stdout: Optional stream configuration for standard output.
        stderr: Optional stream configuration for standard error.

    Returns:
        A :class:`SubprocessResult` containing command execution details.
    """
    cmd = WAF_BASE_CMD + args
    logger.debug("%s", " ".join(cmd))
    return run_process(cmd, cwd=cwd, stdout=stdout, stderr=stderr)


def run_top_level_waf(
    args: list[str],
    stdout: int | None = PIPE,
    stderr: int | None = PIPE,
) -> SubprocessResult:
    """Execute ``waf`` from the project top-level context.

    This wrapper currently delegates directly to :func:`run_waf` and exists as
    a dedicated top-level call site for command handlers.

    Args:
        args: Command-line arguments passed to ``waf``.
        stdout: Optional stream configuration for standard output.
        stderr: Optional stream configuration for standard error.

    Returns:
        A :class:`SubprocessResult` containing command execution details.
    """
    return run_waf(args, cwd=WAF_DEFAULT_CWD, stdout=stdout, stderr=stderr)
