#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements the functionalities behind the 'install' command"""

import logging
import os
import shutil
from copy import deepcopy

from click import echo, secho

from ..helpers.click_helpers import recho
from ..helpers.host_platform import get_platform
from ..helpers.misc import PROJECT_ROOT

INSTALL_MESSAGE = f"""See {PROJECT_ROOT / "INSTALL.md"} for the installation\
instructions for the foxBMS toolchain."""

REQUIRED_SOFTWARE = {
    "doxygen": {"executable": "doxygen", "path": False},
    "drawio": {"executable": {"win32": "draw.io", "linux": "drawio"}, "path": False},
    "gcc": {"executable": "gcc", "path": False},
    "graphviz": {"executable": "dot", "path": False},
    "python": {"executable": "python", "path": False},
    "ruby": {"executable": "ruby", "path": False},
    "ti-compiler": {"executable": "armcl", "path": False},
    "ti-halcogen": {"executable": "halcogen", "path": False, "availability": ["win32"]},
}


def check_for_all_softwares() -> dict:
    """Checks whether all software is available or not."""
    tmp = deepcopy(REQUIRED_SOFTWARE)
    paths_file = PROJECT_ROOT / f"conf/env/paths_{get_platform()}.txt"
    paths = paths_file.read_text(encoding="utf-8")
    path = os.pathsep.join(paths.splitlines()) + os.pathsep + os.environ.get("PATH", "")
    for _, v in tmp.items():
        name = v["executable"]
        if isinstance(name, dict):
            name = name[get_platform()]
        if not isinstance(name, str):
            raise SystemExit(f"Invalid path file ({paths_file}).")
        available = shutil.which(name, path=path)
        if available:
            v["path"] = available
    return tmp


def all_software_available() -> int:
    """Simplified wrapper to check whether all software is available or not."""
    err = 0
    for k, v in check_for_all_softwares().items():
        logging.debug("%s: %s", k, v)
        if not v["path"]:
            availability = v.get("availability", ["linux", "win32"])
            if get_platform() not in availability:
                name = v["executable"]
                recho(
                    f"{k} ({name}) is not available on {get_platform()}.", fg="yellow"
                )
                # no need to to raise an error, as the program is simply not
                # available on this platform
                continue
            recho(f"{k}: {v}")
            err += 1
        else:
            echo(f"{k}: {v['path']}")
    if not err:
        secho("All required software is installed.", fg="green")
    return err
