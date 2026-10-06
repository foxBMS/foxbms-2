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

r"""Check delimiter usage around rst macro substitutions of form ``|name|``."""

import argparse
import re
import sys
from collections.abc import Sequence
from pathlib import Path

MACRO_PATTERN = re.compile(r"\|[^\s|][^|]*\|")
ALLOWED_BEFORE = {"("}
ALLOWED_AFTER = {".", ":", ",", ")", "'", "?", "!"}


def _quote_chars(chars: set[str]) -> str:
    """Return a stable, quoted slash-separated list of characters."""
    return "/".join(f"'{char}'" for char in sorted(chars))


def delimiter_description() -> str:
    """Build a user-facing description of valid delimiters.

    Returns:
        A string describing the valid delimiters.
    """
    before = _quote_chars(ALLOWED_BEFORE)
    after = _quote_chars(ALLOWED_AFTER)
    return (
        "valid delimiters "
        f"(before: whitespace/start/{before}, "
        f"after: whitespace/end/{after}/'\\ '/'-')."
    )


def check_file(file: Path) -> int:
    """Check one file for invalid macro delimiter usage.

    Args:
        file: File path to check.

    Returns:
        Number of spacing violations found.
    """
    err = 0
    for line_no, line in enumerate(
        file.read_text(encoding="utf-8").splitlines(), start=1
    ):
        for match in MACRO_PATTERN.finditer(line):
            col_no = match.start() + 1
            previous = "\n" if col_no == 1 else line[col_no - 2]
            following = "\n" if match.end() == len(line) else line[match.end()]
            before_ok = previous.isspace() or previous in ALLOWED_BEFORE
            escaped_whitespace_ok = (
                following == "\\"
                and match.end() + 1 < len(line)
                and line[match.end() + 1].isspace()
            )
            plain_hyphen_ok = following == "-"
            after_ok = (
                following.isspace()
                or following in ALLOWED_AFTER
                or escaped_whitespace_ok
                or plain_hyphen_ok
            )
            if not before_ok or not after_ok:
                err += 1
                msg = (
                    f"{file.as_posix()}:{line_no}:{col_no}: macro substitution "
                    f"must use {delimiter_description()}"
                )
                print(msg, file=sys.stderr)
    return err


def main(argv: Sequence[str] | None = None) -> int:
    """Run macro-usage checks for all provided files.

    Args:
        argv: Optional command-line arguments.

    Returns:
        Exit code: number of violations, capped at ``255``.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    args = parser.parse_args(argv)

    err = 0
    for item in args.files:
        err += check_file(Path(item))

    return min(err, 255)


if __name__ == "__main__":
    raise SystemExit(main())
