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

"""Define required software tools for working with foxBMS"""

from typing import NotRequired, TypedDict

from .host_platform import HostPlatform, get_platform

ExecutableByPlatform = dict[HostPlatform, str]


class ToolDefinition(TypedDict):
    """Typed description of required software metadata."""

    executable: str | ExecutableByPlatform
    path: str | bool
    relaxed: NotRequired[bool]
    availability: NotRequired[list[HostPlatform]]


REQUIRED_SOFTWARE: dict[str, ToolDefinition] = {
    "doxygen": {"executable": "doxygen", "path": False},
    "drawio": {"executable": {"win32": "draw.io", "linux": "drawio"}, "path": False},
    "gcc": {"executable": "gcc", "path": False},
    "git": {"executable": "git", "path": False, "relaxed": True},
    "graphviz": {"executable": "dot", "path": False},
    "python": {"executable": "python", "path": False},
    "ruby": {"executable": "ruby", "path": False},
    "ti-compiler": {"executable": "armcl", "path": False},
    "ti-halcogen": {"executable": "halcogen", "path": False, "availability": ["win32"]},
    "ti-sh": {"executable": "sh", "path": False},
    "ti-unzip": {"executable": "unzip", "path": False},
}


def get_tool_executable_name(tool: ToolDefinition) -> str:
    """Return the executable name of a tool depending on the host platform.

    Args:
        tool: The tool name.

    Returns:
        The executable name of the tool.
    """
    name = tool["executable"]
    if isinstance(name, dict):
        platform = get_platform()
        name = name[platform]
    return name
