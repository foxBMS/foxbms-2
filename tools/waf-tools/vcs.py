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

"""Add VCS version information to build artifacts"""

from dataclasses import dataclass

from typing import Literal

# see src/version/version.h
REMOTE_MAXIMUM_STRING_LENGTH = 128
MAXIMUM_COMMIT_HASH_LENGTH = 14


@dataclass
class VcsInformation:  # pylint: disable=too-many-instance-attributes
    """Version information"""

    _under_version_control: bool = False
    _dirty: bool = True
    _tag: str = "untagged"
    _major: int = 0
    _minor: int = 0
    _patch: int = 0
    _remote: str = "unknown"
    _distance: int = 65535
    _full_hash: str = "FFFFFFFFFFFFFFFF"
    _short_hash: str = _full_hash[:MAXIMUM_COMMIT_HASH_LENGTH]

    @property
    def version(self) -> str:  # noqa: D102
        t = "no-vcs-"
        if self.under_version_control:
            t = ""
        t += f"{self.major}.{self.minor}.{self.patch}-"
        if self.tag == "untagged":
            t += f"{self.tag}-"
        t += f"{self.distance}-"
        t += f"{self.short_hash}-"
        return t

    @property
    def under_version_control(self) -> bool:  # noqa: D102
        return self._under_version_control

    @under_version_control.setter
    def under_version_control(self, v: bool) -> None:
        self._under_version_control = v

    @property
    def dirty(self) -> bool:  # noqa: D102
        return self._dirty

    @dirty.setter
    def dirty(self, v: bool) -> None:
        self._dirty = v

    @property
    def tag(self) -> bool:  # noqa: D102
        return self._tag

    @tag.setter
    def tag(self, v: bool) -> None:
        self._tag = v

    @property
    def major(self) -> int:  # noqa: D102
        return self._major

    @major.setter
    def major(self, v: int | Literal["x"]) -> None:
        if v == "x":
            v = ord(v)
        self._major = v

    @property
    def minor(self) -> int:  # noqa: D102
        return self._minor

    @minor.setter
    def minor(self, v: int | Literal["y"]) -> None:
        if v == "y":
            v = ord(v)
        self._minor = v

    @property
    def patch(self) -> int:  # noqa: D102
        return self._patch

    @patch.setter
    def patch(self, v: int | Literal["z"]) -> None:
        if v == "z":
            v = ord(v)
        self._patch = v

    @property
    def remote(self) -> str:  # noqa: D102
        return self._remote

    @remote.setter
    def remote(self, v: str) -> None:
        self._remote = v[:REMOTE_MAXIMUM_STRING_LENGTH]

    @property
    def distance(self) -> int:  # noqa: D102
        return self._distance

    @distance.setter
    def distance(self, v: int) -> None:
        self._distance = v

    @property
    def short_hash(self) -> str:  # noqa: D102
        return self._short_hash

    @property
    def full_hash(self) -> str:  # noqa: D102
        return self._full_hash

    @full_hash.setter
    def full_hash(self, v: str) -> None:
        self._full_hash = v
        self._short_hash = v[:MAXIMUM_COMMIT_HASH_LENGTH]
