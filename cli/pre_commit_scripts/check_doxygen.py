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

"""Script to check the file level doxygen comment"""

import argparse
import re
import sys
from pathlib import Path
from subprocess import PIPE, Popen
from typing import Sequence

RE_DATE_FIELD = re.compile(r" \* @date    \d{4}-\d{2}-\d{2} \(date of creation\)")
RE_UPDATED_FIELD = re.compile(r" \* @updated \d{4}-\d{2}-\d{2} \(date of last update\)")
RE_INGROUP_FIELD = re.compile(r" \* @ingroup [A-Z]{1,}")
RE_PREFIX_FIELD = re.compile(r" \* @prefix  [A-Z]{1,5}")
RE_BRIEF_FIELD = re.compile(r" \* @brief   [A-Z]{1,}")
RE_DETAILS_FIELD = re.compile(r" \* @details [A-Z]{1,}")

IDX_DEFAULTS = {
    "confidential": 9,
    "BSD-3-Clause": 41,
}
IDX_MAP = {
    "src/app/driver/afe/nxp/mc33775a/nxp_mc33775a-ll.c": 22,
    "tests/cli/pre_commit_scripts/test_check_license_info/invalid-license.c": 43,
    "tests/cli/pre_commit_scripts/test_check_license_info/no-license.c": 0,
}

IGNORE_ERROR = {
    "author": [
        "src/app/driver/afe/nxp/mc33775a/nxp_mc33775a-ll.c",
    ],
}


# pylint: disable-next=too-many-branches, too-many-statements
def run_check(filename: Path, version: str) -> int:
    """Runs the check"""
    fp = filename.as_posix()
    try:
        txt = filename.read_text(encoding="ascii")
    except UnicodeDecodeError:
        print(f"{fp}: Could not ASCII-decode this file.", file=sys.stderr)
        return 1
    txt_lines = txt.splitlines()
    offset = IDX_DEFAULTS["confidential"]
    if fp in IDX_MAP:
        offset = IDX_MAP[fp]
    elif " * SPDX-License-Identifier: BSD-3-Clause" in txt_lines:
        offset = IDX_DEFAULTS["BSD-3-Clause"]
    err = 0
    try:
        if not txt_lines[offset] == "/**":
            err += 1
            print(f"{fp}: Doxygen comment start marker is missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen comment start marker is missing.", file=sys.stderr)
    try:
        if not txt_lines[offset + 1] == f" * @file    {filename.name}":
            err += 1
            print(f"{fp}: Doxygen @file field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @file field is wrong/missing.", file=sys.stderr)
    try:
        if not txt_lines[offset + 2] == " * @author  foxBMS Team":
            if fp not in IGNORE_ERROR["author"]:
                err += 1
                print(f"{fp}: Doxygen @author field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @author field is wrong/missing.", file=sys.stderr)
    try:
        if not RE_DATE_FIELD.match(txt_lines[offset + 3]):
            err += 1
            print(f"{fp}: Doxygen @date field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @date field is wrong/missing.", file=sys.stderr)
    try:
        if not RE_UPDATED_FIELD.match(txt_lines[offset + 4]):
            err += 1
            print(f"{fp}: Doxygen @updated field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @updated field is wrong/missing.", file=sys.stderr)
    try:
        if not txt_lines[offset + 5] == f" * @version v{version}":
            err += 1
            print(f"{fp}: Doxygen @version field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @version field is wrong/missing.", file=sys.stderr)
    try:
        if not RE_INGROUP_FIELD.match(txt_lines[offset + 6]):
            err += 1
            print(f"{fp}: Doxygen @ingroup field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @ingroup field is wrong/missing.", file=sys.stderr)
    try:
        if not RE_PREFIX_FIELD.match(txt_lines[offset + 7]):
            err += 1
            print(f"{fp}: Doxygen @prefix field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @prefix field is wrong/missing.", file=sys.stderr)
    try:
        if not RE_BRIEF_FIELD.match(txt_lines[offset + 9]):
            err += 1
            print(f"{fp}: Doxygen @brief field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @brief field is wrong/missing.", file=sys.stderr)

    # now read until we find the @details comment:
    idx_details = offset + 10
    # enumerate catches IndexError so there is no need to try/except here
    for ln, line in enumerate(txt_lines[offset + 10 :]):
        print(ln + offset + 10, line)
        if line.startswith(" * @details "):
            idx_details = ln + offset + 10
            break
    try:
        if not RE_DETAILS_FIELD.match(txt_lines[idx_details]):
            err += 1
            print(f"{fp}: Doxygen @details field is wrong/missing.", file=sys.stderr)
    except IndexError:
        err += 1
        print(f"{fp}: Doxygen @details field is wrong/missing.", file=sys.stderr)

    # we do not check for comment closing was the compiler will do that.
    return err


def check_doxygen(files: Sequence[Path], version: str) -> int:
    """Check test files"""
    err = 0
    for i in files:
        err += run_check(i, version)
    return err


def main(argv: Sequence[str] | None = None) -> int:
    """Doxygen file comment checker"""
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)
    err = 0
    cmd = [sys.executable, "fox.py", "--show-config"]
    with Popen(cmd, stdout=PIPE) as p:
        ver_str = p.communicate()[0].decode("utf-8")
    try:
        version = ver_str.split(":")[1].strip()
    except IndexError:
        print("Could not determine foxBMS version.", file=sys.stderr)
        return 1
    files = [Path(i) for i in args.files]
    err = check_doxygen(files=files, version=version)
    return err


if __name__ == "__main__":
    raise SystemExit(main())
