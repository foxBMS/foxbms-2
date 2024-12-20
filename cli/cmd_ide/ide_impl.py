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

"""Implements the functionalities behind the 'ide' command"""

import logging
import shutil
from pathlib import Path

from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import run_process


def open_ide_generic() -> int:
    """Open VS Code in the repository root."""
    return open_ide(PROJECT_ROOT)


def open_ide_app() -> int:
    """Open VS Code in the 'src/app' directory."""
    return open_ide(PROJECT_ROOT / "src/app")


def open_ide_bootloader() -> int:
    """Open VS Code in the 'src/bootloader' directory."""
    return open_ide(PROJECT_ROOT / "src/bootloader")


def open_ide_embedded_unit_test_app() -> int:
    """Open VS Code in the 'tests/unit/app' directory."""
    return open_ide(PROJECT_ROOT / "tests/unit/app")


def open_ide_embedded_unit_test_bootloader() -> int:
    """Open VS Code in the 'tests/unit/bootloader' directory."""
    return open_ide(PROJECT_ROOT / "tests/unit/bootloader")


def open_ide(wd: Path = PROJECT_ROOT) -> int:
    """Open VS Code for the provided path."""
    vs_code = shutil.which("code")
    if not vs_code:
        logging.error("Could not find 'code' binary.")
        return -1
    if not wd.is_dir():
        logging.error("Working directory '%s' does not exist.", wd.absolute())
        return -1
    ret = run_process(cmd=[vs_code, str(wd)], cwd=wd)
    return ret.returncode
