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

"""Platform detection utility.

This module provides a function to detect the current operating system
platform ('linux' or 'win32'). Exits if the platform is unsupported.
"""

import sys
from pathlib import Path
from typing import Literal

from . import PREFIX_LINUX, PREFIX_WIN32

HostPlatform = Literal["win32", "linux"]


def get_platform() -> HostPlatform:
    """Identify the current operating system platform.

    Returns:
        The name of the platform, either 'linux' or 'win32'.

    Exits:
        Exits the program with an error message if the platform is unsupported.
    """
    if sys.platform.lower() == "linux":
        return "linux"
    if sys.platform.lower() == "win32":
        return "win32"
    sys.exit("Running on an unsupported platform.")


def get_platform_prefix(platform: str = get_platform()) -> Path:
    """Return the installation prefix for a supported platform.

    Args:
        platform: The name of the platform, either 'linux' or 'win32'.

    Returns:
        The installation prefix as a Path object.

    Raises:
        NotImplementedError: If the platform is unsupported.
    """
    match platform:
        case "win32":
            return Path(PREFIX_WIN32)
        case "linux":
            return Path(PREFIX_LINUX)
        case _:
            msg = f"Unsupported platform: {platform}"
            raise NotImplementedError(msg)


CURRENT_PREFIX = get_platform_prefix()
