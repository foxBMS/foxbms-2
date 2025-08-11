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

"""Check the license information header in the provided files.

This script verifies that files of various types (C, ASM, Python, YAML, TOML,
PowerShell, Shell) contain the correct license header for either "BSD-3-Clause"
or "confidential". It compares the beginning of each file against the expected
header text and reports any mismatches.
It is intended for use with pre-commit.
"""

import argparse
import sys
from collections.abc import Sequence
from pathlib import Path
from typing import Literal, get_args

LicenseTypes = Literal["BSD-3-Clause", "confidential"]


LICENSE_BASE_TEXT_BSD_3_CLAUSE = [
    "SPDX-License-Identifier: BSD-3-Clause",
    "",
    "Redistribution and use in source and binary forms, with or without",
    "modification, are permitted provided that the following conditions are met:",
    "",
    "1. Redistributions of source code must retain the above copyright notice, this",
    "   list of conditions and the following disclaimer.",
    "",
    "2. Redistributions in binary form must reproduce the above copyright notice,",
    "   this list of conditions and the following disclaimer in the documentation",
    "   and/or other materials provided with the distribution.",
    "",
    "3. Neither the name of the copyright holder nor the names of its",
    "   contributors may be used to endorse or promote products derived from",
    "   this software without specific prior written permission.",
    "",
    'THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"',
    "AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
    "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE",
    "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE",
    "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
    "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR",
    "SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER",
    "CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,",
    "OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE",
    "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
    "",
    "We kindly request you to use one or more of the following phrases to refer to",
    "foxBMS in your hardware, software, documentation or advertising materials:",
    "",
    '- "This product uses parts of foxBMS®"',
    '- "This product includes parts of foxBMS®"',
    '- "This product is derived from foxBMS®"',
]

LICENSE_BASE_TEXT_CONFIDENTIAL = [
    "Confidential or no approval for publication",
]

LICENSE_TYPE_TO_LICENSE_BASE_TEXT = {
    "BSD-3-Clause": LICENSE_BASE_TEXT_BSD_3_CLAUSE,
    "confidential": LICENSE_BASE_TEXT_CONFIDENTIAL,
}


def compare_header(
    file_name: Path, expected: list[str], actual: list[str], start: int, end: int
) -> int:
    """Compare the actual header in a file with the expected license header.

    Args:
        file_name: Path to the file being checked.
        expected: The expected license header lines.
        actual: The actual lines from the file.
        start: Start index in the file lines to compare.
        end: End index in the file lines to compare.

    Return:
        1 if the header differs, 0 if it matches.

    """
    err = 0
    if not len(actual) >= end:
        msg = f"{file_name.as_posix()}: License header is not correct."
        print(msg, file=sys.stderr)
        err += 1

    if actual[start:end] != expected:
        msg = f"{file_name.as_posix()}: License header is not correct."
        print(msg, file=sys.stderr)
        print("The following lines differ", file=sys.stderr)
        for i, (e, a) in enumerate(zip(expected, actual, strict=False)):
            if e != a:
                print(f"Line {i + 1}: Expected: '{e}'", file=sys.stderr)
                print(f"Line {i + 1}: Actual:   '{a}'", file=sys.stderr)
        err += 1
    return err


def check_asm(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check assembler source files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "; @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "; All rights reserved.",
        ";",
    ]
    epilog: list[str] = []
    char = "; "
    txt = [
        (char + i.replace("®", "&reg;")).rstrip()
        for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential")
    ]
    license_text = prolog + txt + epilog
    start = 0
    end = start + len(license_text)
    for i in files:
        tmp = Path(i).read_text(encoding="utf-8").splitlines()
        err += compare_header(
            file_name=Path(i),
            expected=license_text,
            actual=tmp,
            start=start,
            end=end,
        )
    return err


def check_c(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check C source files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        "/**",
        " *",
        # pylint: disable-next=line-too-long
        " * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        " * All rights reserved.",
        " *",
    ]
    epilog = [" *", " */"]
    char = " * "
    txt = [
        (char + i.replace("®", "&reg;")).rstrip()
        for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential")
    ]
    license_text = prolog + txt + epilog
    start = 0
    end = start + len(license_text)
    for i in files:
        tmp = Path(i).read_text(encoding="utf-8").splitlines()
        err += compare_header(
            file_name=Path(i),
            expected=license_text,
            actual=tmp,
            start=start,
            end=end,
        )
    return err


def check_py(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check Python script/module files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        "#",
        # pylint: disable-next=line-too-long
        "# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "# All rights reserved.",
        "#",
    ]
    char = "# "
    txt = [
        (char + i).rstrip()
        for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential")
    ]
    license_text = prolog + txt
    start = 1
    end = start + len(license_text)
    for i in files:
        tmp = Path(i).read_text(encoding="utf-8").splitlines()
        err += compare_header(
            file_name=Path(i),
            expected=license_text,
            actual=tmp,
            start=start,
            end=end,
        )
    return err


def check_yaml(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check YAML files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "# All rights reserved.",
        "#",
    ]
    char = "# "
    txt = [
        (char + i).rstrip()
        for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential")
    ]
    license_text = prolog + txt
    start = 0
    end = start + len(license_text)
    for i in files:
        tmp = Path(i).read_text(encoding="utf-8").splitlines()
        err += compare_header(
            file_name=Path(i),
            expected=license_text,
            actual=tmp,
            start=start,
            end=end,
        )
    return err


def check_toml(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check TOML files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    # same header
    return check_yaml(files, license_type)


def check_pwsh(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check PowerShell script files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    # same header
    return check_py(files, license_type)


def check_shell(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check shell script files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Return:
        Count of files with incorrect headers.

    """
    # same header
    return check_py(files, license_type)


def main(argv: Sequence[str] | None = None) -> int:
    """Command-line interface for license header checking.

    Args:
        argv: Optional sequence of command-line arguments.

    Return:
        Exit code (0 if all headers are correct, >0 otherwise).

    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--file-type",
        default="c",
        choices=["asm", "c", "pwsh", "py", "shell", "toml", "yaml"],
        help="File type",
    )
    parser.add_argument("files", nargs="*", help="Files to check")
    parser.add_argument(
        "--license-type",
        default="confidential",
        choices=get_args(LicenseTypes),
        help="License type",
    )
    args = parser.parse_args(argv)
    return globals()[f"check_{args.file_type}"](
        files=args.files, license_type=args.license_type
    )


if __name__ == "__main__":
    raise SystemExit(main())
