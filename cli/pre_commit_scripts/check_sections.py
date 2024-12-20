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

"""Script to check the section markers in C files"""

import argparse
import sys
from pathlib import Path
from typing import Sequence

TYPES = {
    "src.c": [
        "/*========== Includes =======================================================*/",
        "/*========== Macros and Definitions =========================================*/",
        "/*========== Static Constant and Variable Definitions =======================*/",
        "/*========== Extern Constant and Variable Definitions =======================*/",
        "/*========== Static Function Prototypes =====================================*/",
        "/*========== Static Function Implementations ================================*/",
        "/*========== Extern Function Implementations ================================*/",
        "/*========== Externalized Static Function Implementations (Unit Test) =======*/",
    ],
    "src.h": [
        "/*========== Includes =======================================================*/",
        "/*========== Macros and Definitions =========================================*/",
        "/*========== Extern Constant and Variable Declarations ======================*/",
        "/*========== Extern Function Prototypes =====================================*/",
        "/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/",
    ],
    "test.c": [
        "/*========== Includes =======================================================*/",
        "/*========== Unit Testing Framework Directives ==============================*/",
        "/*========== Definitions and Implementations for Unit Test ==================*/",
        "/*========== Setup and Teardown =============================================*/",
        "/*========== Test Cases =====================================================*/",
    ],
    "test.h": [
        "/*========== Includes =======================================================*/",
        "/*========== Unit Testing Framework Directives ==============================*/",
        "/*========== Macros and Definitions =========================================*/",
    ],
}


def run_check(filename: Path, file_type) -> int:
    """Runs the check"""
    txt = filename.read_text(encoding="ascii")
    txt_lines = txt.splitlines()
    markers = TYPES[file_type]
    err = 0
    idx = []
    for marker in markers:
        try:
            idx.append(txt_lines.index(marker))
        except ValueError:
            err += 1
            print(f"{filename.as_posix()}: {marker} is missing.", file=sys.stderr)
            idx.append(-1)
        if txt_lines.count(marker) > 1:
            err += 1
            print(
                f"{filename.as_posix()}: {marker} occurs more than once.",
                file=sys.stderr,
            )
        if f"\n\n{marker}\n" not in txt:
            err += 1
            print(
                f"{filename.as_posix()}: {marker} requires a blank line "
                "before the marker.",
                file=sys.stderr,
            )
    if idx != sorted(idx):
        err += 1
        print(
            f"{filename.as_posix()}: markers are not in the correct order.",
            file=sys.stderr,
        )

    if file_type in ["src.c", "src.h"]:
        # check the file, then this exception is obvious
        if filename == Path("src/app/application/config/battery_system_cfg.h"):
            return err
        try:
            # laster marker is for unit tests
            unit_test_define = txt_lines.index(TYPES[file_type][-1]) + 1
        except ValueError:
            err += 1
            idx.append(-1)
            return err
        try:
            if not txt_lines[unit_test_define] == "#ifdef UNITY_UNIT_TEST":
                err += 1
                print(
                    f"{filename.as_posix()}: '#ifdef UNITY_UNIT_TEST' is "
                    f"missing after {TYPES[file_type][-1]}.",
                    file=sys.stderr,
                )
        except IndexError:
            err += 1
            print(
                f"{filename.as_posix()}: '#ifdef UNITY_UNIT_TEST' is "
                f"missing after {TYPES[file_type][-1]}.",
                file=sys.stderr,
            )
    return err


def check_src(files: Sequence[Path]) -> int:
    """Check src files"""
    err = 0
    for i in files:
        if i.suffix == ".c":
            err += run_check(i, "src.c")
        elif i.suffix == ".h":
            err += run_check(i, "src.h")
        else:
            print(
                f"{i.as_posix()}: Unkown file extension '{i.suffix}'.", file=sys.stderr
            )
            err += 1
    return err


def check_test(files: Sequence[Path]) -> int:
    """Check test files"""
    err = 0
    for i in files:
        if i.suffix == ".c":
            err += run_check(i, "test.c")
        elif i.suffix == ".h":
            err += run_check(i, "test.h")
        else:
            print(
                f"{i.as_posix()}: Unkown file extension '{i.suffix}'.", file=sys.stderr
            )
            err += 1
    return err


def main(argv: Sequence[str] | None = None) -> int:
    """Encoding header checker"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--file-type",
        default="src",
        choices=["src", "test"],
        help="File type",
    )
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)
    err = 0
    files = [Path(i) for i in args.files]
    err = globals()[f"check_{args.file_type}"](files=files)
    return err


if __name__ == "__main__":
    raise SystemExit(main())
