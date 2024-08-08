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

"""Check that the Axivion version specifier is the same in all configuration
files"""

import json
import logging
import sys
import shutil
from subprocess import Popen, PIPE
from pathlib import Path


def get_repo_root():
    """extract the commit date of a file as datetime from git"""
    git = shutil.which("git")
    if not git:
        sys.exit("Could not find git.")
    cmd = [git, "rev-parse", "--show-toplevel"]
    with Popen(cmd, cwd=str(Path(__file__).parent), stdout=PIPE) as p:
        std_out = p.communicate()[0]
    root = Path(std_out.decode("utf-8").strip())
    logging.debug("Repository root is: %s", root)
    return root


CONFIG_FILES_EXCL = ["axivion_config.json", "axivion_self_tests.json"]


def get_axivion_configuration_files() -> list:
    """Find all Axivion configuration files"""
    repo_root = get_repo_root()
    return [
        i
        for i in list((repo_root / "tests/axivion").glob("*.json"))
        + list((repo_root / "tests/unit/axivion").glob("*.json"))
        if i.name not in CONFIG_FILES_EXCL
    ]


def main():
    """Runs the Axivion version check script"""
    errors = []
    file_and_version_info = []
    for i in get_axivion_configuration_files():
        with open(i, encoding="utf-8") as f:
            cfg: dict = json.load(f)
        if not cfg.get("_Version", ""):
            errors.append(f"{i}: '_Version' not specified")
        if not cfg.get("_VersionNum", ""):
            errors.append(f"{i}: '_VersionNum' not specified")

        file_and_version_info.append((i, cfg["_Version"], cfg["_VersionNum"]))

        versions = []
        version_nums = []
        for j in file_and_version_info:
            if j[1] not in versions:
                versions.append(j[1])
            version_num_str = ".".join([str(k) for k in j[2]])
            if version_num_str not in version_nums:
                version_nums.append(version_num_str)

    if len(versions) > 1:
        for i in versions:
            errors.append(
                f"Found multiple versions ('_Version'): {', '.join(versions)}"
            )
    if len(version_nums) > 1:
        for i in version_nums:
            errors.append(
                f"Found multiple versions numbers ('_VersionNum'): {', '.join(version_nums)}"
            )

    for i in errors:
        print(i, file=sys.stderr)

    if any(True for i in sys.argv if "-v" in i):
        print("All version numbers match.")

    sys.exit(len(errors))


if __name__ == "__main__":
    main()
