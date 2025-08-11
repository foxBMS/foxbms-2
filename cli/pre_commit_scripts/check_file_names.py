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

"""Check a list of provided files for filename uniqueness within the git repository.

This script compares the names of the specified files to all files tracked by git
and reports if any filename occurs more than once in the repository. This helps
ensure that each file has a unique name to avoid confusion or conflicts.
It is intended for use with pre-commit.
"""

import argparse
import sys
from collections.abc import Sequence
from pathlib import Path
from shutil import which
from subprocess import PIPE, Popen


def main(argv: Sequence[str] | None = None) -> int:
    """Check the provided files for unique filenames within the git repository.

    Args:
        argv: Optional sequence of command-line arguments.

    Return:
        Number of files with duplicate names found (exit code).

    """
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)
    err = 0
    git = str(which("git"))
    with Popen([git, "ls-files"], stdout=PIPE) as p:
        out = p.communicate()[0]
    tmp = out.decode("utf-8").splitlines()
    repo_files: list[Path] = []
    repo_file_names = []
    for i in tmp:
        repo_files.append(Path(i))
        repo_file_names.append(Path(i).name)
    for i in args.files:
        if repo_file_names.count(Path(i).name) > 1:
            msg = f"{Path(i).as_posix()}: File name '{Path(i).name}' exists multiple times."
            print(msg, file=sys.stderr)
            for j, name in enumerate(repo_file_names):
                if Path(i).name == Path(name).name:
                    print(f"  {repo_files[j].as_posix()}", file=sys.stderr)
            err += 1
    return err


if __name__ == "__main__":
    raise SystemExit(main())
