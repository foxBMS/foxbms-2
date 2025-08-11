#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Standardized process execution and result handling utilities for Python's subprocess.

This module defines a data class for subprocess results, provides helpers to
decode outputs, and offers a function to run commands with consistent error
handling and output formatting.
"""

import logging
import shutil
import subprocess
import sys
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from .click_helpers import recho
from .host_platform import get_platform
from .misc import PROJECT_ROOT


@dataclass
class SubprocessResult:
    """Represent the result of a subprocess execution.

    Attributes:
        returncode: The return code from the process.
        out: The standard output captured from the process.
        err: The standard error output captured from the process.

    """

    returncode: int = 1
    """returncode of the process"""
    out: str = ""
    """standard out of the process"""
    err: str = ""
    """standard error of the process"""

    def __add__(self, other: "SubprocessResult") -> "SubprocessResult":
        """Combine two subprocess results.

        Args:
            other: Another subprocess result to add.

        Return:
            A new result combining return codes, outputs, and errors.

        """
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
        """Return a string representation of the subprocess result.

        Return:
            Formatted string with return code, stdout, and stderr.

        """
        return f"return code: {self.returncode}\n\nout:{self.out}\n\n{self.err}\n"


def prepare_subprocess_output(
    returncode: int, out: bytes, err: bytes
) -> SubprocessResult:
    """Decode subprocess output from bytes to string and return a SubprocessResult.

    Args:
        returncode (int): Return code from the process.
        out (bytes): Standard output in bytes.
        err (bytes): Standard error in bytes.

    Return:
        The decoded result.

    """
    out_str = out.decode("utf-8").strip() if out else ""
    err_str = err.decode("utf-8").strip() if err else ""
    return SubprocessResult(returncode=returncode, out=out_str, err=err_str)


def run_process(
    cmd: Sequence[str | Path],
    cwd: str | Path = PROJECT_ROOT,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    env=None,
) -> SubprocessResult:
    """Run a command as a subprocess and capture its output.

    Args:
        cmd: Command and arguments to execute.
        cwd: Working directory.
        stdout: Subprocess stdout redirection.
        stderr: Subprocess stderr redirection.
        env: Environment variables to use.

    Return:
        The result of the executed process.

    """
    logging.debug("Original cmd: %s", cmd)
    if len(cmd) == 0:
        recho("No program provided.")
        return prepare_subprocess_output(1, out=b"", err=b"No program provided.")
    executable = cmd[0]
    if not shutil.which(executable):
        recho(f"Program '{cmd[0]}' does not exist.")
        return prepare_subprocess_output(
            1,
            out=b"",
            err=f"Program '{cmd[0]}' does not exist.".encode(),
        )
    # fix executable name (required on Windows because of PATHEXT)
    cmd_str = [str(shutil.which(executable))] + [str(i) for i in cmd[1:]]
    logging.debug("Stringified cmd: %s", " ".join(cmd_str))
    platform_args = {"cwd": cwd, "stdout": stdout, "stderr": stderr, "env": env}
    if get_platform() == "win32" and not hasattr(sys.stdin, "isatty"):
        # on Windows, when the GUI starts a process this would create a
        # poping up window without using 'CREATE_NO_WINDOW'
        platform_args["creationflags"] = subprocess.CREATE_NO_WINDOW
    with subprocess.Popen(cmd_str, **platform_args) as p:
        out, err = p.communicate()
    return prepare_subprocess_output(p.returncode, out, err)
