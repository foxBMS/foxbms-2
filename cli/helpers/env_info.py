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

"""Store Python environment information"""

from pathlib import Path

from ..foxbms_version import __version__
from .host_platform import CURRENT_PREFIX
from .project_context import get_env

ENV_DIR_PREFIX = Path(f"{CURRENT_PREFIX}/envs/")
ENV_DIR_SUFFIX_REPO_USAGE = Path("2026-07-pale-fox")
ENV_DIR_SUFFIX_INSTALL = Path(f"local/{__version__}")


def get_env_dir(is_repository_usage: bool, current_prefix: Path) -> Path:
    """Return the environment directory for repository or installation usage.

    Args:
        is_repository_usage: Whether the environment is for repository usage.
        current_prefix: The current prefix path.

    Returns:
        The environment directory path.
    """
    if is_repository_usage:
        return Path(f"{current_prefix}/envs/") / ENV_DIR_SUFFIX_REPO_USAGE
    return Path(f"{current_prefix}/envs/") / ENV_DIR_SUFFIX_INSTALL


ENV_DIR = get_env_dir(get_env(), CURRENT_PREFIX)
