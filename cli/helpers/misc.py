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


"""Miscellaneous helper functions."""

import hashlib
import logging
import os
import sys
from pathlib import Path
from typing import Union

from click import Context
from click.core import Parameter

from .host_platform import PLATFORM
from .win32_vars import HOMEDRIVE, LOCALAPPDATA, PROGRAMFILES, USERPROFILE

try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    HAVE_GIT = False

DISABLE_LOGGING_FOR_MODULES = ["git"]


def get_project_root(path: str = ".") -> Path:
    """helper function to find the repository root

    Args:
        path: path to retrieve the project root from

    Returns:
        root path of the git repository
    """
    if HAVE_GIT:
        try:
            repo = Repo(path, search_parent_directories=True)
            root = repo.git.rev_parse("--show-toplevel")
        except InvalidGitRepositoryError:
            root = Path(__file__).parent.parent.parent
    else:
        root = Path(__file__).parent.parent.parent
    return Path(root)


PROJECT_ROOT = get_project_root()

PATH_FILE = PROJECT_ROOT / f"conf/env/paths_{PLATFORM}.txt"

PATH_REPLACEMENTS = {
    "$DOT_DIR$": [
        os.path.join(USERPROFILE, "graphviz"),
        os.path.join(HOMEDRIVE, "graphviz"),
    ],
    "$DOXYGEN_DIR$": [
        os.path.join(USERPROFILE, "doxygen"),
        os.path.join(HOMEDRIVE, "doxygen"),
    ],
    "$GIT_DIR$": [
        os.path.join(PROGRAMFILES, "Git"),
        os.path.join(LOCALAPPDATA, "Programs", "Git"),
    ],
}


def replace_var(rep: str) -> list[str]:
    """replace the placeholder in the path with the actual values"""
    if PLATFORM == "linux":
        return []
    for k, v in PATH_REPLACEMENTS.items():
        if k in rep:
            tmp = rep.split(k + os.sep)[1]
            return [os.path.join(path, tmp) for path in v]
    return []


def init_path_var_for_foxbms() -> None:
    """Add paths that foxBMS expects to exist to the PATH environment variable.
    - If a path do not exist, it is not added to PATH
    - If a path is already on PATH, it is not added to PATH
    """
    prepare_config = []
    for i in PATH_FILE.read_text(encoding="utf-8").splitlines():
        if any(k for k, _ in PATH_REPLACEMENTS.items() if k in i):
            prepare_config.extend(replace_var(i))
        else:
            prepare_config.append(i)
    prepend_to_path = []
    for i in prepare_config:
        if Path(i).is_dir():
            prepend_to_path.append(i)
    # now we have all entries that should be added

    # crate the full path, that might have some duplicates
    full_path = prepend_to_path + os.environ.get("PATH", "").split(os.pathsep)
    # remove duplicates, but keep list order
    new_path_list = list(dict.fromkeys(full_path))
    new_path_list_clean: list[str] = []
    for i in new_path_list:
        if os.sep + "WindowsApps" in i:
            continue
        new_path_list_clean.append(i)

    os.environ["PATH"] = os.pathsep.join(new_path_list_clean)


def ignore_third_party_logging() -> None:
    """Disable logging for third party tool, except for errors"""
    for module in DISABLE_LOGGING_FOR_MODULES:
        logging.getLogger(module).setLevel(logging.CRITICAL)


def set_logging_level(
    verbosity: int = 1,
    _format: str = "%(asctime)s File:%(filename)-9s line:%(lineno)-4s %(levelname)-8s %(message)s",
    datefmt: Union[str, None] = None,
) -> None:
    """sets the module logging level

    :param verbosity: verbosity level
    :param _format: logging format style
    :param datefmt: date format style"""

    if verbosity < 1:
        verbosity = 1
    elif verbosity > 3:
        verbosity = 3

    logging_levels = {
        "1": logging.WARNING,
        "2": logging.INFO,
        "3": logging.DEBUG,
    }
    level = logging_levels[str(verbosity)]
    logging.basicConfig(
        format=_format,
        datefmt=datefmt,
        level=level,
    )
    logging.debug("Setting logging level to %s", level)


def set_logging_level_cb(ctx: Context, param: Parameter | None, value: int) -> None:
    """sets the module logging level through a click option callback"""
    set_logging_level(verbosity=value)


def eprint(msg: str, color=False, err: bool = False) -> None:
    """Enhanced printing function"""
    if color:
        msg = f"{color}{msg}\033[0m"
    if err:
        print(msg, file=sys.stderr)
    else:
        print(msg)


def get_sha256_file_hash(
    file_path: Path, buffer_size: int = 65536, file_hash=hashlib.sha256()
) -> "hashlib._Hash":
    """Calculate the SHA256 hash of a file"""
    with open(file_path, "rb") as f:
        while True:
            data = f.read(buffer_size)
            if not data:
                break
            file_hash.update(data)
    return file_hash


def get_sha256_file_hash_str(file_path: Path, buffer_size: int = 65536) -> str:
    """Returns the string representation of a SHA256 hash of a file"""
    return get_sha256_file_hash(
        file_path=file_path, buffer_size=buffer_size
    ).hexdigest()


def get_multiple_files_hash_str(files: list[Path], buffer_size: int = 65536) -> str:
    """Returns the string representation of a SHA256 hash for multiple files"""
    file_hash = hashlib.sha256()
    for i in files:
        file_hash = get_sha256_file_hash(
            i, buffer_size=buffer_size, file_hash=file_hash
        )
    return file_hash.hexdigest()
