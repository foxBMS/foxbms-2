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


"""Run a 'waf configure' but expect it to fail, as the specified version is not
found"""

import sys
from pathlib import Path
from subprocess import PIPE, Popen

ROOT = Path(__file__).parent.parent.parent.parent

EXPECTED_ERRORS = [
    "Strict CCS version checking was set, and compiler version does not match.",
    "(searched for v1.1.1.LTS",
]


def main():
    """Run the 'expect-failure' test case"""
    ext = "ps1" if sys.platform.lower() == "win32" else "sh"
    fox_wrapper = ROOT / f"fox.{ext}"

    if not fox_wrapper.is_file():
        sys.exit(f"Could not find the 'fox.{ext}'-wrapper.")

    cmd = []
    if sys.platform.lower() == "win32":
        cmd.extend(
            [
                "pwsh.exe",
                "-NoProfile",
                "-NoLogo",
                "-NonInteractive",
                "-File",
            ]
        )
    cmd.extend([fox_wrapper, "waf", "configure"])
    with Popen(cmd, stdout=PIPE, stderr=PIPE) as p:
        _, stderr_bytes = p.communicate()

    stderr = stderr_bytes.decode("utf-8")
    err = 0
    if p.returncode == 0:
        err += 1
        print("The test should fail, but succeed.", file=sys.stderr)

    for i in EXPECTED_ERRORS:
        if i not in stderr:
            err += 1
            print(
                f"Expected '{i}' in stderr, but it was not found on stderr",
                file=sys.stderr,
            )
    return err


if __name__ == "__main__":
    main()
