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

"""Initialize PATH for the foxBMS environment.

Attributes:
    PATH_FILE: Path to the platform-specific paths file.
"""

# Keep PATH deterministic across developer machines and CI by applying this
# pipeline in order: read current PATH, remove conflicting required-tool
# entries, prepend existing foxBMS tool paths from PATH_FILE, remove duplicates
# while preserving order, then filter/normalize entries before writing PATH
# back.
# The order is intentional so foxBMS-managed paths take precedence over
# global installations.

import os
from pathlib import Path
from shutil import which

from .env_info import ENV_DIR
from .host_platform import get_platform
from .logger import logger
from .project_context import ROOT_IS_PROJECT, get_file_path
from .required_software import REQUIRED_SOFTWARE, get_tool_executable_name

if ROOT_IS_PROJECT:
    PATH_FILE = get_file_path() / f"conf/env/paths_{get_platform()}.txt"
else:
    PATH_FILE = get_file_path() / f"env/paths_{get_platform()}.txt"

# we need to ignore some executables that are found on the system PATH, as
# they could interfere with the foxBMS tools
_EXCLUDE_FROM_FOXBMS_PATH = [
    "\\WindowsApps",  # Microsoft Store installed Python
    "conda",  # conda installed Python
    # Git: it is sufficient that ``Git\\cmd`` is on the PATH, the other Git
    # paths can cause issues with the TI CCS installation, as they include a
    # 'sh' executable
    "\\Git\\bin",  # sh
    "\\Git\\usr\\bin",  # sh
]

_NEVER_REMOVE_FROM_PATH = [
    "C:\\Windows",
    "/usr/local/bin",
    "/usr/bin",
    "/usr/local/sbin",
    "/usr/sbin",
]

# On Windows the paths can be separated by either backslashes or forward
# slashes, so we need to add both versions to the lists.
EXCLUDE_FROM_FOXBMS_PATH = _EXCLUDE_FROM_FOXBMS_PATH + [
    i.replace("\\", "/") for i in _EXCLUDE_FROM_FOXBMS_PATH
]
NEVER_REMOVE_FROM_PATH = _NEVER_REMOVE_FROM_PATH + [
    i.replace("\\", "/") for i in _NEVER_REMOVE_FROM_PATH
]


def _remove_conflicting_required_tool_entries(path_entries: list[str]) -> list[str]:
    """Remove PATH entries that provide non-relaxed required tools.

    Args:
        path_entries: Existing PATH entries.

    Returns:
        Filtered PATH entries.
    """
    cleaned_path = []
    for path_entry in path_entries:
        # At start we assume, that it is a path entry that should be kept
        # if that turns out later in the loop to be false, we set it true.
        ignore_path_entry = False
        if any(never_remove in path_entry for never_remove in NEVER_REMOVE_FROM_PATH):
            # if the path entry includes one of the paths that should never be
            # removed, we keep this entry in the path variable, even if it
            # includes a binary of a required software tool.
            cleaned_path.append(path_entry)
            logger.debug(
                "Keeping PATH entry '%s' because it is a never-remove path", path_entry
            )
            continue
        for tool in REQUIRED_SOFTWARE.values():
            name = get_tool_executable_name(tool)
            if m := which(name, path=path_entry):
                if name == "python" and str(ENV_DIR) in str(Path(m)):
                    # Python executable is added by the shell wrappers, so it
                    # needs to be on PATH.
                    logger.debug(
                        "Keeping PATH entry '%s' because it includes the "
                        "Python executable from the foxBMS environment",
                        path_entry,
                    )
                    continue
                if tool.get("relaxed", False):
                    # Relaxed tools are allowed to be on PATH, as we do not so
                    # much care about how they end up being available, as long
                    # as they are available.
                    logger.debug(
                        "Keeping PATH entry '%s' because it includes a relaxed "
                        "required tool '%s'",
                        path_entry,
                        name,
                    )
                    continue
                ignore_path_entry = True
                logger.debug(
                    "PATH entry '%s' includes required tool '%s', which is "
                    "not relaxed, so this PATH entry will be ignored",
                    path_entry,
                    name,
                )
                break
        if ignore_path_entry:
            logger.debug(
                "Ignoring PATH entry '%s' because it includes a conflicting "
                "required tool",
                path_entry,
            )
            continue
        # we found a path entry that does not include one of by foxBMS required
        # software tools, so we keep this entry in the path variable
        cleaned_path.append(path_entry)
    logger.debug(
        "PATH entries after removing conflicting required tool entries: %s",
        cleaned_path,
    )
    return cleaned_path


def _remove_duplicates(path_entries: list[str]) -> list[str]:
    """Remove duplicate entries from a list while preserving order.

    Args:
        path_entries: PATH entries that may contain duplicates.

    Returns:
        PATH entries without duplicates.
    """
    path_entries = list(dict.fromkeys(path_entries))
    logger.debug("PATH entries after removing duplicates: %s", path_entries)
    return path_entries


def _filter_and_normalize_path_entries(path_entries: list[str]) -> list[str]:
    """Filter and normalize PATH entries for the foxBMS environment.

    Entries matching excluded tool paths and empty entries are removed.
    Remaining entries have trailing path separators stripped.

    Args:
        path_entries: PATH entries to process.

    Returns:
        Cleaned and normalized PATH entries.
    """
    new_path: list[str] = []
    for i in path_entries:
        if any(exclude in i for exclude in EXCLUDE_FROM_FOXBMS_PATH):
            logger.debug(
                "Excluding PATH entry '%s' because it matches an excluded tool path",
                i,
            )
            continue
        if not i:  # skip empty path entries
            continue
        logger.debug("Keeping PATH entry '%s'", i)
        new_path.append(i.removesuffix(os.sep))
    logger.debug("PATH entries after filtering and normalization: %s", new_path)
    return new_path


def _get_environment_path_entries() -> list[str]:
    """Return entries from the PATH environment variable.

    Returns:
        PATH entries from the current environment.
    """
    path_entries = os.environ.get("PATH", "").split(os.pathsep)
    logger.debug("Current PATH entries: %s", path_entries)
    return path_entries


def _prepend_existing_foxbms_path_entries(path_entries: list[str]) -> list[str]:
    """Prepend existing foxBMS path entries from PATH_FILE.

    Args:
        path_entries: Existing PATH entries.

    Returns:
        PATH entries with foxBMS entries prepended.
    """
    foxbms_path_entries = [
        i
        for i in PATH_FILE.read_text(encoding="utf-8").splitlines()
        if Path(i).is_dir()
    ]
    logger.debug("foxBMS PATH entries: %s", foxbms_path_entries)
    return foxbms_path_entries + path_entries


def initialize_path_variable_for_foxbms() -> None:
    """Add expected foxBMS paths to the PATH environment variable.

    Only existing directories are added, and duplicates are removed.
    Paths potentially containing other undesired Python installations are
    filtered out.
    """
    logger.debug("Initializing PATH variable for foxBMS environment")
    logger.debug(
        "Keeping PATH entries that include any of the following never-remove paths: %s",
        NEVER_REMOVE_FROM_PATH,
    )
    logger.debug(
        "Ignoring PATH entries that include any of the following excluded tool paths: %s",
        EXCLUDE_FROM_FOXBMS_PATH,
    )
    path = _get_environment_path_entries()
    path = _remove_conflicting_required_tool_entries(path)
    path = _prepend_existing_foxbms_path_entries(path)
    path = _remove_duplicates(path)
    path = _filter_and_normalize_path_entries(path)
    os.environ["PATH"] = os.pathsep.join(path)
    logger.debug("Final PATH entries: %s", path)
