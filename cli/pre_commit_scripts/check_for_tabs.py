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

"""Check whether the provided files contain tab characters.

This script scans each specified file for tab characters. If any are found,
it reports the file and line number (when possible) for each occurrence.
Intended for use in automated quality checks or pre-commit hooks to enforce
whitespace standards.
It is intended for use with pre-commit.
"""

import argparse
import sys
from collections.abc import Sequence
from pathlib import Path

SUPPORTED_ENCODINGS = ("ascii", "utf-8")


def main(argv: Sequence[str] | None = None) -> int:
    """Check the specified files for tab characters.

    Args:
        argv: Optional sequence of command-line arguments.

    Returns:
        The number of tab characters found (max 255).

    """
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)
    err = 0
    # pylint: disable-next=too-many-nested-blocks
    for i in [Path(i) for i in args.files]:
        if tabs := i.read_bytes().count(b"\t"):
            for encoding in SUPPORTED_ENCODINGS:
                try:
                    for n, line in enumerate(
                        i.read_text(encoding=encoding).splitlines()
                    ):
                        if "\t" in line:
                            msg = f"{i.absolute().as_posix()}:{n + 1} contains tabs."
                            print(msg, file=sys.stderr)
                    break
                except ValueError:
                    pass
            print(f"{i.absolute().as_posix()} contains tabs.", file=sys.stderr)
        err += tabs
    return min(err, 255)


if __name__ == "__main__":
    raise SystemExit(main())
