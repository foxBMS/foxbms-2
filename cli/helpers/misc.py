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

"""Miscellaneous helper functions and path definitions for foxBMS utilities.

This module provides helper functions for environment setup, logging
configuration, hashing, and path management, as well as constants for important
foxBMS file locations.

Attributes:
    PROJECT_ROOT: Path to the root of the project repository or package.
    ROOT_IS_PROJECT: True, when in the foxBMS repository, otherwise False.
    PROJECT_BUILD_ROOT: Path to the project's build directory.
    PATH_FILE: Path to the platform-specific paths file.
    FOXBMS_ELF_FILE: Path to the foxBMS ELF file.
    FOXBMS_BIN_FILE: Path to the foxBMS binary file.
    FOXBMS_APP_CRC_FILE: Path to the foxBMS CRC CSV file.
    FOXBMS_APP_INFO_FILE: Path to the foxBMS CRC info JSON file.
    APP_DBC_FILE: Path to the application DBC file.
    BOOTLOADER_DBC_FILE: Path to the bootloader DBC file.
"""

import hashlib
import json
import os
import sys
from datetime import UTC, datetime
from pathlib import Path

from git import Repo
from git.exc import GitError
from platformdirs import user_documents_dir

from .host_platform import get_platform


def get_project_root(path: str = ".") -> Path:
    """Find the repository or package root directory.

    Args:
        path (str): Path to retrieve the project root from.

    Returns:
        Path: Root path of the git repository or package.
    """
    root = Path(__file__).parent.parent.parent
    try:
        repo = Repo(path, search_parent_directories=True)
        root = repo.git.rev_parse("--show-toplevel")
    except GitError:
        pass
    # Directory is a foxbms-repository if the file "fox.py" exists
    if (Path(root) / "fox.py").is_file():
        return Path(root)
    return Path(__file__).parent.parent


def get_env() -> bool:
    """Check if command is run in the foxBMS repository or the fox CLI package
    and whether it is valid in this environment.

    Returns:
        ``True`` when running in a foxBMS repository checkout.

    Raises:
        SystemExit: If the command is executed in an invalid environment.
    """
    root_is_project = (PROJECT_ROOT / "fox.py").is_file()
    if str(PROJECT_ROOT) not in str(Path(__file__)):
        if root_is_project:
            sys.exit("Use 'fox.py' script when in the foxBMS repository.")
        else:
            sys.exit(
                "Use 'fox-cli' when in the fox CLI package, 'fox.py' is not available."
            )
    return root_is_project


def get_file_path() -> Path:
    """Return path to the directory containing project-files if in the package,
    otherwise return PROJECT_ROOT

    Returns:
        Base path for project data files.
    """
    if ROOT_IS_PROJECT:
        return PROJECT_ROOT
    return PROJECT_ROOT / "project_data"


PROJECT_ROOT = get_project_root()
ROOT_IS_PROJECT = get_env()
if ROOT_IS_PROJECT:
    PROJECT_BUILD_ROOT = PROJECT_ROOT / "build"
    PATH_FILE = get_file_path() / f"conf/env/paths_{get_platform()}.txt"
else:
    PROJECT_BUILD_ROOT = Path(user_documents_dir()) / "fox_cli"
    PATH_FILE = get_file_path() / f"env/paths_{get_platform()}.txt"


FOXBMS_ELF_FILE = PROJECT_BUILD_ROOT / "app_embedded/src/app/main/foxbms.elf"
FOXBMS_BIN_FILE = PROJECT_BUILD_ROOT / "app_embedded/src/app/main/foxbms.bin"
FOXBMS_APP_CRC_FILE = PROJECT_BUILD_ROOT / "app_embedded/src/app/main/foxbms.crc64.csv"
FOXBMS_APP_INFO_FILE = (
    PROJECT_BUILD_ROOT / "app_embedded/src/app/main/foxbms.crc64.json"
)
APP_DBC_FILE = PROJECT_ROOT / "tools/dbc/foxbms.dbc"
BOOTLOADER_DBC_FILE = PROJECT_ROOT / "tools/dbc/foxbms-bootloader.dbc"

EXCLUDE_FROM_FOXBMS_PATH = ["\\WindowsApps", "conda"]


def initialize_path_variable_for_foxbms() -> None:
    """Add expected foxBMS paths to the PATH environment variable.

    Only existing directories are added, and duplicates are removed.
    Paths potentially containing other undesired Python installations are
    filtered out.

    Raises:
        FileNotFoundError: If the configured path list file does not exist.
    """
    prepend_to_path = [
        i
        for i in PATH_FILE.read_text(encoding="utf-8").splitlines()
        if Path(i).is_dir()
    ]
    # Create the full path, that might have some duplicates
    full_path = prepend_to_path + os.environ.get("PATH", "").split(os.pathsep)
    # Remove duplicates, but keep list order
    new_path_list = list(dict.fromkeys(full_path))
    new_path_list_clean: list[str] = []
    for i in new_path_list:
        if any(exclude in i for exclude in EXCLUDE_FROM_FOXBMS_PATH):
            continue
        new_path_list_clean.append(i)

    os.environ["PATH"] = os.pathsep.join(new_path_list_clean)


def set_other_environment_variables_for_foxbms() -> None:
    """Set environment variables according to foxBMS specifications.

    Loads variables from conf/env/env.json and applies platform-specific
    overrides.

    Raises:
        FileNotFoundError: If the environment JSON file does not exist.
        json.JSONDecodeError: If the environment JSON file is invalid.
    """
    tmp = (PATH_FILE.parent / "env.json").read_text(encoding="utf-8")
    env_vars: dict = json.loads(tmp)
    for var, val in env_vars.items():
        try:  # noqa: SIM105 TODO evaluate whether a fix is reasonable
            val = val[get_platform()]
        except (KeyError, TypeError):
            pass
        os.environ[var] = val


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
    """Calculate the SHA256 hash of a file.

    Args:
        file_path: Path to the file to hash.
        buffer_size: Buffer size for reading the file.
        file_hash: Hash object to update (optional).

    Returns:
        SHA256 hash object after processing the file.

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
    """Return the hexadecimal SHA256 hash string of a file.

    Args:
        file_path: Path to the file to hash.
        buffer_size: Buffer size for reading the file.

    Returns:
        str: Hexadecimal SHA256 hash string.

    """
    return get_sha256_file_hash(
        file_path=file_path, buffer_size=buffer_size
    ).hexdigest()


def get_multiple_files_hash_str(files: list[Path], buffer_size: int = 65536) -> str:
    """Return the hexadecimal SHA256 hash string for multiple files.

    Args:
        files: List of file paths to hash.
        buffer_size: Buffer size for reading the files.

    Returns:
        Hexadecimal SHA256 hash string for all files.

    """
    file_hash = hashlib.sha256()
    for i in files:
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
