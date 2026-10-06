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

"""Validate or fix list sorting in test configuration JSON files.

Only list values of the keys ``source``, ``includes`` and ``mocks`` are
validated.
"""

import argparse
import json
import sys
from collections.abc import Sequence
from pathlib import Path

_SORTED_LIST_KEYS = {"source", "includes", "mocks"}


def _sort_key(value: object) -> str:
    """Return a deterministic sort key for a JSON value."""
    return json.dumps(value, separators=(",", ":"), sort_keys=True)


def _format_location(path: str) -> str:
    """Return a user-friendly list location from an internal JSON path."""
    if path == "$":
        return "top-level list"
    if path.startswith("$."):
        return path[2:]
    if path.startswith("$"):
        return path[1:]
    return path


def _check_sorted_lists(
    value: object,
    path: str,
    filename: Path,
    current_key: str | None = None,
    fix: bool = False,
) -> int:
    """Recursively validate sorted lists in JSON content.

    Args:
        value: Current JSON value to inspect.
        path: JSON-path-like location of the value.
        filename: Path of the file being checked.
        current_key: Optional key of the current value, if applicable.
        fix: optional check to try a fix automatically

    Returns:
        Number of unsorted list findings below the current value.

    """
    err = 0
    if isinstance(value, dict):
        for key, item in value.items():
            err += _check_sorted_lists(
                item, f"{path}.{key}", filename, current_key=key, fix=fix
            )
        return err

    if isinstance(value, list):
        if current_key in _SORTED_LIST_KEYS:
            sorted_value = sorted(value, key=_sort_key)
            if value != sorted_value:
                location = _format_location(path)
                if location == "top-level list":
                    message = f"{filename.as_posix()}: top-level list is not sorted."
                else:
                    message = (
                        f"{filename.as_posix()}: list at {location} is not sorted."
                    )
                print(message, file=sys.stderr)
                if fix:
                    value[:] = sorted_value
                err += 1
        for idx, item in enumerate(value):
            err += _check_sorted_lists(item, f"{path}[{idx}]", filename, fix=fix)
    return err


def main(argv: Sequence[str] | None = None) -> int:
    """Check all provided JSON files for unsorted test configuration lists.

    Args:
        argv: Optional sequence of command-line arguments.

    Returns:
        Number of unsorted target lists found, capped to ``255``.

    """
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Sort unsorted lists and rewrite changed files",
    )
    args = parser.parse_args(argv)

    err = 0
    for file in [Path(i) for i in args.files]:
        try:
            content = json.loads(file.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            print(
                f"{file.as_posix()}: invalid JSON: {exc.msg}.",
                file=sys.stderr,
            )
            err += 1
            continue
        file_err = _check_sorted_lists(content, "$", file, fix=args.fix)
        err += file_err

        if args.fix and file_err:
            file.write_text(
                json.dumps(content, indent=2, ensure_ascii=False) + "\n",
                encoding="utf-8",
            )

    if args.fix:
        return 1 if err else 0
    return min(err, 255)


if __name__ == "__main__":
    raise SystemExit(main())
