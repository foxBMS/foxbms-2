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

"""Shared helpers to detect foxBMS project/package execution context.

Attributes:
    PROJECT_ROOT: Path to the root of the project repository or package.
    ROOT_IS_PROJECT: True, when in the foxBMS repository, otherwise False.
    PROJECT_BUILD_ROOT: Path to the project's build directory.
    FOXBMS_ELF_FILE: Path to the foxBMS ELF file.
    FOXBMS_BIN_FILE: Path to the foxBMS binary file.
    FOXBMS_APP_CRC_FILE: Path to the foxBMS CRC CSV file.
    FOXBMS_APP_INFO_FILE: Path to the foxBMS CRC info JSON file.
    APP_DBC_FILE: Path to the application DBC file.
    BOOTLOADER_DBC_FILE: Path to the bootloader DBC file.
"""

from pathlib import Path

from git import Repo
from git.exc import GitError
from platformdirs import user_documents_dir


def get_project_root(path: str = ".") -> Path:
    """Find the repository or package root directory.

    Args:
        path: Path to retrieve the project root from.

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

    Args:
        project_root: Optional project root to avoid recomputing it.

    Returns:
        ``True`` when running in a foxBMS repository checkout.

    Raises:
        SystemExit: If the command is executed in an invalid environment.
    """
    root_is_project = (PROJECT_ROOT / "fox.py").is_file()
    if str(PROJECT_ROOT) not in str(Path(__file__)):
        if root_is_project:
            msg = "Use 'fox.py' script when in the foxBMS repository."
            raise SystemExit(msg)
        msg = "Use 'fox-cli' when in the fox CLI package, 'fox.py' is not available."
        raise SystemExit(msg)

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
else:
    PROJECT_BUILD_ROOT = Path(user_documents_dir()) / "fox_cli"


FOXBMS_ELF_FILE = PROJECT_BUILD_ROOT / "app_ti_arm_cgt/src/app/main/foxbms.elf"
FOXBMS_BIN_FILE = PROJECT_BUILD_ROOT / "app_ti_arm_cgt/src/app/main/foxbms.bin"
FOXBMS_APP_CRC_FILE = (
    PROJECT_BUILD_ROOT / "app_ti_arm_cgt/src/app/main/foxbms.crc64.csv"
)
FOXBMS_APP_INFO_FILE = (
    PROJECT_BUILD_ROOT / "app_ti_arm_cgt/src/app/main/foxbms.crc64.json"
)
APP_DBC_FILE = PROJECT_ROOT / "tools/dbc/foxbms.dbc"
BOOTLOADER_DBC_FILE = PROJECT_ROOT / "tools/dbc/foxbms-bootloader.dbc"

EMBEDDED_UNIT_TEST_COVERAGE_INDEX_FILE = (
    PROJECT_BUILD_ROOT / "app_unit_test_gcc/coverage/index.html"
)

CLI_UNIT_TEST_INDEX_FILE = PROJECT_BUILD_ROOT / "cli-selftest/index.html"
