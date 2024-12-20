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

"""Simple subprocess wrapper"""

import logging
import shutil
from dataclasses import dataclass
from pathlib import Path
from subprocess import PIPE, Popen
from typing import Sequence

from click import secho

from .misc import PROJECT_ROOT


@dataclass
class SubprocessResult:
    """Subprocess result"""

    returncode: int = 1
    """returncode of the process"""
    out: str = ""
    """standard out of the process"""
    err: str = ""
    """standard error of the process"""

    def __add__(self, other: "SubprocessResult") -> "SubprocessResult":
        returncode = abs(self.returncode)
        if other.returncode:
            returncode += other.returncode
        out = self.out
        if other.out:
            out += "\n\n" + other.out
        err = self.err
        if other.err:
            err += "\n\n" + other.err
        return SubprocessResult(returncode, out, err)

    def __str__(self) -> str:
        return f"return code: {self.returncode}\n\nout:{self.out}\n\n{self.err}\n"


def prepare_subprocess_output(
    returncode: int, out: bytes, err: bytes
) -> SubprocessResult:
    """Decode the subprocess output"""
    if out:
        out_str = out.decode("utf-8").strip()
    else:
        out_str = ""
    if err:
        err_str = err.decode("utf-8").strip()
    else:
        err_str = ""
    return SubprocessResult(returncode=returncode, out=out_str, err=err_str)


def run_process(
    cmd: Sequence[str | Path],
    cwd: str | Path = PROJECT_ROOT,
    stdout=PIPE,
    stderr=PIPE,
    env=None,
) -> SubprocessResult:
    """Run the provided command"""
    logging.debug("Original cmd: %s", cmd)
    if len(cmd) == 0:
        secho("No program provided.", fg="red", err=True)
        return prepare_subprocess_output(
            1, out=b"", err="No program provided.".encode(encoding="utf-8")
        )
    executable = cmd[0]
    if not shutil.which(executable):
        secho(f"Program '{cmd[0]}' does not exist.", fg="red", err=True)
        return prepare_subprocess_output(
            1,
            out=b"",
            err=f"Program '{cmd[0]}' does not exist.".encode(encoding="utf-8"),
        )
    logging.debug("Original cmd: %s", cmd)
    # fix executable name (required on Windows because of PATHEXT)
    cmd_str = [str(shutil.which(executable))] + [str(i) for i in cmd[1:]]
    logging.debug("Stringified cmd: %s", " ".join(cmd_str))
    with Popen(cmd_str, cwd=cwd, stdout=stdout, stderr=stderr, env=env) as p:
        out, err = p.communicate()
    return prepare_subprocess_output(p.returncode, out, err)
