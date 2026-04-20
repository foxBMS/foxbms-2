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

"""Specifications for setting dynamic values of the fox CLI package"""

import re
import sys
from pathlib import Path
from typing import Any

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from hatchling.metadata.plugin.interface import MetadataHookInterface


class CustomBuildHook(BuildHookInterface):
    """Manages the 'version.py' file during the build process"""

    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        """Add 'version.py' to the package"""
        build_data["artifacts"].append("version.py")
        return super().initialize(version, build_data)

    def finalize(
        self, version: str, build_data: dict[str, Any], artifact_path: str
    ) -> None:
        """Remove 'version.py' from the repository"""
        version_file = Path(__file__).parent / "cli" / "version.py"
        if version_file.is_file():
            version_file.unlink()
        return super().finalize(version, build_data, artifact_path)


class CustomMetaDataHook(MetadataHookInterface):
    """Dynamically set the package version from the foxBMS version
    and the package dependencies from the 'requirements.txt' file
    before the build process
    """

    def update(self, metadata: dict) -> None:
        """Update 'version' and 'dependencies' values"""
        version = get_numeric_version()
        with open(
            Path(__file__).parent / "cli" / "version.py", "w", encoding="utf-8"
        ) as f:
            f.write(f'VERSION = "{version}"')
        metadata["version"] = version
        metadata["dependencies"] = get_dependencies()


def get_dependencies() -> list[str]:
    """Reads all dependencies from the 'requirements.txt' file"""
    with open(Path("requirements.txt"), encoding="utf-8") as f:
        dependencies = f.read().splitlines()

    for idx, line in enumerate(dependencies):
        if "#" in line:
            dependencies[idx] = line.split("#")[0].split()[0]
    return dependencies


def extract_version(txt: str, pattern: re.Pattern[str]) -> str:
    """Extracts the version information from a string"""
    version = ""
    for line in txt.splitlines():
        m = pattern.search(line)
        if m:
            version = m.group(1)
    if not version:
        sys.exit("Could not determine foxBMS 2 version.")
    return version


def get_version() -> str:
    """Read the foxBMS version from the build file."""
    wscript = (Path(__file__).parent / "wscript").read_text(encoding="utf-8")
    pattern = re.compile(r"VERSION = \"((x\.y\.z)|(\d{1,}\.\d{1,}\.\d{1,}))\"")
    return extract_version(wscript, pattern)


def get_numeric_version() -> str:
    """Convert the foxBMS version to a numeric form"""
    numeric_version = get_version().split(".")
    for index, part in enumerate(numeric_version):
        if not part.isnumeric():
            numeric_version[index] = str(ord(part))
    return ".".join(numeric_version)
