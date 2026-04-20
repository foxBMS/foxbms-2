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

"""Validate license headers in source and script files.

This script verifies that files of various types (C, ASM, Python, YAML, TOML,
PowerShell, Shell) contain the correct license header for either "BSD-3-Clause"
or "confidential". It dispatches files to the appropriate checker by file
extension and reports any mismatches.
It is intended for use with pre-commit.
"""

import argparse
import sys
from collections.abc import Sequence
from pathlib import Path
from typing import Literal, Protocol, TypedDict, get_args

LicenseTypes = Literal["BSD-3-Clause", "confidential"]


class CheckFunction(Protocol):  # pylint: disable=too-few-public-methods
    """Callable signature used by license header checkers."""

    def __call__(  # noqa: D102
        self,
        *,
        files: Sequence[str],
        license_type: LicenseTypes,
    ) -> int: ...


class CheckEntry(TypedDict):
    """Per-checker configuration and accumulated file list."""

    check: CheckFunction
    files: list[str]


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


def _checker_for_file(path: Path) -> str | None:
    """Return checker key for a file path, or ``None`` when unsupported."""
    file_type = None
    if path.name == "wscript" or path.suffix == ".py":
        file_type = "py"
    elif path.suffix in {".c", ".h"}:
        file_type = "c"
    elif path.suffix == ".asm":
        file_type = "asm"
    elif path.suffix in {".yml", ".yaml"}:
        file_type = "yaml"
    elif path.suffix == ".toml":
        file_type = "toml"
    elif path.suffix == ".ps1":
        file_type = "pwsh"
    elif path.suffix == ".sh":
        file_type = "shell"
    return file_type


def _as_repo_relative(path: Path) -> Path:
    """Return a repository-relative path when possible."""
    if not path.is_absolute():
        return path
    try:
        return path.relative_to(Path.cwd())
    except ValueError:
        return path


def _source_to_test_file(path: Path) -> Path | None:
    """Map a src C file to its accompanying tests/unit test file path."""
    rel_path = _as_repo_relative(path)
    if rel_path.suffix != ".c":
        return None

    try:
        src_index = rel_path.parts.index("src")
    except ValueError:
        return None

    src_rel = Path(*rel_path.parts[src_index + 1 :])
    if not src_rel.parts:
        return None
    return Path("tests/unit") / src_rel.parent / f"test_{src_rel.name}"


def _source_to_test_wscript(path: Path) -> Path | None:
    """Map a src wscript file to its accompanying tests/unit wscript path."""
    rel_path = _as_repo_relative(path)
    if rel_path.name != "wscript":
        return None

    try:
        src_index = rel_path.parts.index("src")
    except ValueError:
        return None

    src_rel = Path(*rel_path.parts[src_index + 1 :])
    if not src_rel.parts:
        return None
    return Path("tests/unit") / src_rel


def _read_text_utf8_lines(file: Path) -> list[str] | None:
    """Read text file as UTF-8 and return split lines.

    Returns ``None`` and emits an error when decoding fails.
    """
    try:
        return file.read_text(encoding="utf-8").splitlines()
    except UnicodeDecodeError:
        msg = f"{file.as_posix()}: Cannot read file as UTF-8."
        print(msg, file=sys.stderr)
        return None
    except (FileNotFoundError, PermissionError) as e:
        msg = f"{file.as_posix()}: Cannot read file ({e})."
        print(msg, file=sys.stderr)
        return None


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

    Returns:
        ``1`` if the selected header range differs, otherwise ``0``.

    """
    err = 0
    if len(actual) < end:
        msg = f"{file_name.as_posix()}: License header is not correct."
        print(msg, file=sys.stderr)
        err += 1

    if actual[start:end] != expected:
        msg = f"{file_name.as_posix()}: License header is not correct."
        print(msg, file=sys.stderr)
        print("The following lines differ", file=sys.stderr)
        for i, (e, a) in enumerate(zip(expected, actual[start:end], strict=False)):
            if e != a:
                line = start + i + 1
                print(f"Line {line}: Expected: '{e}'", file=sys.stderr)
                print(f"Line {line}: Actual:   '{a}'", file=sys.stderr)
        err += 1
    return err


def check_asm(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check assembler source files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Returns:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "; @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
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
        file_path = Path(i)
        tmp = _read_text_utf8_lines(file_path)
        if not tmp:
            err += 1
            continue
        err += compare_header(
            file_name=file_path,
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

    Returns:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        "/**",
        " *",
        # pylint: disable-next=line-too-long
        " * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
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
        source_file = Path(i)
        tmp = _read_text_utf8_lines(source_file)
        if not tmp:
            err += 1
            continue
        err += compare_header(
            file_name=source_file,
            expected=license_text,
            actual=tmp,
            start=start,
            end=end,
        )

        test_file = _source_to_test_file(source_file)
        if test_file and test_file.exists():
            test_lines = _read_text_utf8_lines(test_file)
            if not test_lines:
                err += 1
                continue
            err += compare_header(
                file_name=test_file,
                expected=license_text,
                actual=test_lines,
                start=start,
                end=end,
            )
    return err


def check_py(files: Sequence[str], license_type: LicenseTypes = "confidential") -> int:
    """Check Python script/module files for correct license header.

    Args:
        files: List of file paths to check.
        license_type: Type of license to check for.

    Returns:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        "#",
        # pylint: disable-next=line-too-long
        "# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
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
        source_file = Path(i)
        tmp = _read_text_utf8_lines(source_file)
        if not tmp:
            err += 1
            continue
        err += compare_header(
            file_name=source_file,
            expected=license_text,
            actual=tmp,
            start=start,
            end=end,
        )

        test_file = _source_to_test_wscript(source_file)
        if test_file and test_file.exists():
            test_lines = _read_text_utf8_lines(test_file)
            if not test_lines:
                err += 1
                continue
            err += compare_header(
                file_name=test_file,
                expected=license_text,
                actual=test_lines,
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

    Returns:
        Count of files with incorrect headers.

    """
    err = 0
    prolog = [
        # pylint: disable-next=line-too-long
        "# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
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
        file_path = Path(i)
        tmp = _read_text_utf8_lines(file_path)
        if not tmp:
            err += 1
            continue
        err += compare_header(
            file_name=file_path,
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

    Returns:
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

    Returns:
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

    Returns:
        Count of files with incorrect headers.

    """
    # same header
    return check_py(files, license_type)


def main(argv: Sequence[str] | None = None) -> int:
    """Command-line interface for license header checking.

    Args:
        argv: Optional sequence of command-line arguments.

    Returns:
        Exit code (0 if all headers are correct, >0 otherwise).

    """
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Files to check")
    parser.add_argument(
        "--license-type",
        default="confidential",
        choices=get_args(LicenseTypes),
        help="License type",
    )
    args = parser.parse_args(argv)
    checks: dict[str, CheckEntry] = {
        "asm": {"check": check_asm, "files": []},
        "c": {"check": check_c, "files": []},
        "pwsh": {"check": check_pwsh, "files": []},
        "py": {"check": check_py, "files": []},
        "shell": {"check": check_shell, "files": []},
        "toml": {"check": check_toml, "files": []},
        "yaml": {"check": check_yaml, "files": []},
    }
    for file_arg in args.files:
        file_path = Path(file_arg)
        file_type = _checker_for_file(file_path)
        if not file_type:
            continue
        checks[file_type]["files"].append(file_arg)

    err = 0
    for data in checks.values():
        files = data["files"]
        if files:
            err += data["check"](files=files, license_type=args.license_type)
    return err


if __name__ == "__main__":
    raise SystemExit(main())
