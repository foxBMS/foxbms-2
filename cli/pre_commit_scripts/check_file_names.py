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

"""Script to check the list of provided files for uniqueness"""

import argparse
from pathlib import Path
from subprocess import PIPE, Popen
from typing import Sequence

KNOWN = [
    "tests/axivion/addon-test/test_file_comments/interlock.c",
    "tools/vendor/ceedling/plugins/dependencies/example/boss/src/main.c",
    "tests/axivion/addon-test/test_file_comments/ltc_defs.h",
    "src/os/freertos/LICENSE.md",
    "docs/developer-manual/hardware/.dummy",
    "docs/hardware/slaves/14-maxim-max17852-vx.x.x/.dummy",
    "src/os/safertos/.dummy",
    "src/os/safertos/include/.dummy",
]


def main(argv: Sequence[str] | None = None) -> int:
    """File name uniqueness checker"""
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)
    err = 0
    with Popen(["git", "ls-files"], stdout=PIPE) as p:
        out = p.communicate()[0]
    tmp = out.decode("utf-8").splitlines()
    repo_files = []
    for i in tmp:
        file_to_add = Path(i)
        if file_to_add.as_posix() not in KNOWN:
            repo_files.append(file_to_add.name)
    for i in args.files:
        if repo_files.count(Path(i).name) > 1:
            print(f"{Path(i)}: File name '{Path(i).name}' exists multiple times.")
            for j in repo_files:
                if Path(i).name == Path(j).name:
                    print(i, j)
            err += 1
    return err


if __name__ == "__main__":
    raise SystemExit(main())
