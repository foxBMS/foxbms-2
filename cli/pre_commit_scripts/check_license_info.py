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

"""Script to check the license information for a list of provided files"""

import argparse
import sys
from pathlib import Path
from typing import Literal, Sequence, get_args

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
    """compares to lists of strings"""
    err = 0
    if not len(actual) >= end:
        print(
            f"{file_name.as_posix()}: License header is not correct.", file=sys.stderr
        )
        err += 1

    if not actual[start:end] == expected:
        print(
            f"{file_name.as_posix()}: License header is not correct.", file=sys.stderr
        )
        print("The following lines differ", file=sys.stderr)
        for i, (e, a) in enumerate(zip(expected, actual)):
            if not e == a:
                print(f"Line {i+1}: Expected: '{e}'", file=sys.stderr)
                print(f"Line {i+1}: Actual:   '{a}'", file=sys.stderr)
        err += 1
    return err


def check_asm(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check assembler sources"""
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "; @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "; All rights reserved.",
        ";",
    ]
    epilog = []
    char = "; "
    txt = []
    for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential"):
        txt.append((char + i.replace("®", "&reg;")).rstrip())
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
    """Check C sources"""
    err = 0
    prolog = [
        "/**",
        " *",
        # pylint: disable-next=line-too-long
        " * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        " * All rights reserved.",
        " *",
    ]
    epilog = [" *", " */"]
    char = " * "
    txt = []
    for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential"):
        txt.append((char + i.replace("®", "&reg;")).rstrip())
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
    """Check Python scripts"""
    err = 0
    prolog = [
        "#",
        # pylint: disable-next=line-too-long
        "# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "# All rights reserved.",
        "#",
    ]
    char = "# "
    txt = []
    for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential"):
        txt.append((char + i).rstrip())
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
    """Check YAML files"""
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "# All rights reserved.",
        "#",
    ]
    char = "# "
    txt = []
    for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential"):
        txt.append((char + i).rstrip())
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
    """Check toml files"""
    # same header
    return check_yaml(files, license_type)


def check_pwsh(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check pwsh scripts"""
    # same header
    return check_py(files, license_type)


def check_batch(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check batch scripts"""
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "@REM Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
        "@REM All rights reserved.",
        "@REM",
    ]
    char = "@REM "
    txt = []
    for i in LICENSE_TYPE_TO_LICENSE_BASE_TEXT.get(license_type, "confidential"):
        txt.append((char + i.replace("®", "&reg;")).rstrip())
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


def check_shell(
    files: Sequence[str], license_type: LicenseTypes = "confidential"
) -> int:
    """Check shell scripts"""
    # same header
    return check_py(files, license_type)


def check_dot(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check dot files"""
    # same header
    return check_yaml(files, license_type)


def main(argv: Sequence[str] | None = None) -> int:
    """License header checker"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--file-type",
        default="c",
        choices=["asm", "batch", "c", "dot", "pwsh", "py", "shell", "toml", "yaml"],
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
    err = 0
    err = globals()[f"check_{args.file_type}"](
        files=args.files, license_type=args.license_type
    )
    return err


if __name__ == "__main__":
    raise SystemExit(main())
