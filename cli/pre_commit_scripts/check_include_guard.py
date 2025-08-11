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

"""Check the include guard pattern in C header files.

This script verifies that each given C header file (.h) contains a correctly
formatted and ordered include guard using the foxBMS naming convention.
It reports missing, duplicate, or misordered guard markers and checks
for the expected formatting pattern.
It is intended for use with pre-commit.
"""

import argparse
import sys
from collections.abc import Sequence
from pathlib import Path

MARKERS = [
    "#ifndef {}",
    "#define {}",
    "#endif /* {} */",
]


def run_check(filename: Path) -> int:
    """Check a single header file for correct foxBMS include guard usage.

    Args:
        filename (Path): Path to the C header file.

    Return:
        Number of detected include guard issues.

    """
    txt = filename.read_text(encoding="ascii")
    txt_lines = txt.splitlines()
    err = 0
    name = "".join([c if c.isalnum() else "_" for c in filename.stem.upper()])
    define_guard = f"FOXBMS__{name}_H_"

    for i in MARKERS:
        marker = i.format(define_guard)
        if not txt_lines.count(marker):
            err += 1
            print(f"{filename.as_posix()}: {marker} is missing.", file=sys.stderr)
        if txt_lines.count(marker) > 1:
            err += 1
            msg = f"{filename.as_posix()}: {marker} occurs more than once."
            print(msg, file=sys.stderr)
    idx = []
    for _marker in MARKERS:
        marker = _marker.format(define_guard)
        try:
            idx.append(txt_lines.index(marker))
        except ValueError:
            idx.append(-1)
    if idx != sorted(idx):
        err += 1
        msg = f"{filename.as_posix()}: markers are not in the correct order."
        print(msg, file=sys.stderr)
    marker_0 = MARKERS[0].format(define_guard)
    marker_1 = MARKERS[1].format(define_guard)
    if f"\n\n{marker_0}\n{marker_1}\n\n" not in txt:
        err += 1
        msg = (
            f"{filename.as_posix()}: The pattern needs to be:"
            f"\n\n#ifndef {define_guard}\n#define {define_guard}\n\n"
        )
        print(msg, file=sys.stderr)
    return err


def check_include_guard(files: Sequence[Path]) -> int:
    """Check a sequence of files for correct foxBMS include guard usage.

    Args:
        files: Iterable of files to check.

    Return:
        Total number of issues found.

    """
    err = 0
    for i in files:
        if i.suffix == ".c":
            continue
        err += run_check(i)
    return err


def main(argv: Sequence[str] | None = None) -> int:
    """Command-line interface for checking include guards.

    Args:
        argv: Optional sequence of command-line arguments.

    Return:
        Number of include guard violations found (exit code).

    """
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)
    files = [Path(i) for i in args.files]
    return check_include_guard(files=files)


if __name__ == "__main__":
    raise SystemExit(main())
