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

"""Miscellaneous helper functions.

This module provides helper functions for environment setup, logging
configuration, hashing, and path management, as well as constants for important
foxBMS file locations.
"""

import hashlib
from datetime import UTC, datetime
from pathlib import Path

from .project_context import PROJECT_ROOT, ROOT_IS_PROJECT


def terminal_link_print(link: Path | str) -> str:
    """Create a clickable hyperlink string for terminal output.

    Args:
        link: Hyperlink to be printed.

    Returns:
        Clickable terminal hyperlink.
    """
    return f"\033]8;;{link}\033\\{link}\033]8;;\033\\"


def get_sha256_file_hash(
    file_path: Path, buffer_size: int = 65536, file_hash: "hashlib._Hash | None" = None
) -> "hashlib._Hash":
    """Calculate the SHA-256 hash of a file.

    Args:
        file_path: Path to the file to hash.
        buffer_size: Buffer size for reading the file.
        file_hash: Hash object to update (optional).

    Returns:
        SHA-256 hash object after processing the file.
    """
    if not file_hash:
        file_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        while True:
            data = f.read(buffer_size)
            if not data:
                break
            file_hash.update(data)
    return file_hash


def get_sha256_file_hash_str(file_path: Path, buffer_size: int = 65536) -> str:
    """Return the hexadecimal SHA-256 hash string of a file.

    Args:
        file_path: Path to the file to hash.
        buffer_size: Buffer size for reading the file.

    Returns:
        str: Hexadecimal SHA-256 hash string.
    """
    return get_sha256_file_hash(
        file_path=file_path, buffer_size=buffer_size
    ).hexdigest()


def get_multiple_files_hash_str(files: list[Path], buffer_size: int = 65536) -> str:
    """Return the hexadecimal SHA-256 hash string for multiple files.

    Args:
        files: List of file paths to hash.
        buffer_size: Buffer size for reading the files.

    Returns:
        Hexadecimal SHA-256 hash string for all files.
    """
    file_hash = hashlib.sha256()
    for i in sorted(files, key=lambda p: p.as_posix()):
        file_hash = get_sha256_file_hash(
            i, buffer_size=buffer_size, file_hash=file_hash
        )
    return file_hash.hexdigest()


def file_name_from_current_time() -> Path:
    """Create a file-system-friendly ISO timestamp as a Path.

    Returns:
        Current ISO timestamp with colons replaced by underscores.
    """
    return Path(str(datetime.now(tz=UTC).isoformat()).replace(":", "_"))


def create_pre_commit_file() -> None:
    """Add or update a pre-commit file in the .git/hooks directory.

    If already present with correct content, does nothing.
    """
    if not ROOT_IS_PROJECT:
        return
    path_dir = PROJECT_ROOT / ".git/hooks"
    # check if we are in a git repo
    if not path_dir.is_dir():
        return
    text = (
        "#!/usr/bin/env bash\n"
        "#\n"
        'SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"\n'
        '"$SCRIPT_DIR/../../fox.sh" pre-commit run\n'
    )
    pre_commit = path_dir / "pre-commit"
    if pre_commit.is_file():
        pre_commit_txt = pre_commit.read_text(encoding="utf-8")
        if pre_commit_txt == text:
            return
    pre_commit.write_text(text, encoding="utf-8", newline="\n")
    pre_commit.chmod(0o755)
