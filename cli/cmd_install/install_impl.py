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

# cspell:ignore Targetpath noexit

"""Implements the functionalities behind the 'install' command"""

import os
import shutil
from copy import deepcopy
from importlib.util import find_spec
from pathlib import Path

from ..foxbms_version import get_numeric_version
from ..helpers import PREFIX_LINUX, PREFIX_WIN32, TOOL_NAME, python_setup
from ..helpers.click_helpers import echo, recho, secho
from ..helpers.host_platform import get_platform
from ..helpers.logger import logger
from ..helpers.misc import PATH_FILE, PROJECT_ROOT

WIN32COM_AVAILABLE = False
if find_spec("win32com"):
    WIN32COM_AVAILABLE = True
    from win32com.client import Dispatch
    from win32com.universal import com_error


INSTALL_MESSAGE = (
    f"See {PROJECT_ROOT / 'INSTALL.md'} for the installation instructions for "
    "the foxBMS toolchain."
)

REQUIRED_SOFTWARE = {
    "ceedling": {"executable": "ceedling", "path": False},
    "doxygen": {"executable": "doxygen", "path": False},
    "drawio": {"executable": {"win32": "draw.io", "linux": "drawio"}, "path": False},
    "gcc": {"executable": "gcc", "path": False},
    "git": {"executable": "git", "path": False},
    "graphviz": {"executable": "dot", "path": False},
    "python": {"executable": "python", "path": False},
    "ruby": {"executable": "ruby", "path": False},
    "ti-compiler": {"executable": "armcl", "path": False},
    "ti-halcogen": {"executable": "halcogen", "path": False, "availability": ["win32"]},
}


def check_for_all_softwares() -> dict:
    """Checks whether all software is available or not."""
    tmp = deepcopy(REQUIRED_SOFTWARE)
    paths = PATH_FILE.read_text(encoding="utf-8")
    path = os.pathsep.join(paths.splitlines()) + os.pathsep + os.environ.get("PATH", "")
    for v in tmp.values():
        name = v["executable"]
        if isinstance(name, dict):
            name = name[get_platform()]
        if not isinstance(name, str):
            err = f"Invalid path file ({PATH_FILE})."
            raise SystemExit(err)
        available = shutil.which(name, path=path)
        if available:
            v["path"] = available
    return tmp


def all_software_available() -> int:
    """Simplified wrapper to check whether all software is available or not."""
    err = 0
    for k, v in check_for_all_softwares().items():
        logger.debug("%s: %s", k, v)
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


def _create_shortcut_win32(
    parent: Path,
    name_suffix: str,
    arguments: list,
    shortcut_command: str,
    target_path: str = "C:/Program Files/PowerShell/7/pwsh.exe",
) -> int:
    """Creates a Windows shortcut (.lnk) in the specified parent directory.

    Args:
        parent (Path): Directory where the shortcut will be created.
        name_suffix (str): Suffix to append to the shortcut name.
        arguments (list): List of arguments for the shortcut.
        shortcut_command (str): Command to execute after PowerShell starts.
        target_path (str, optional): Path to the executable. Defaults to pwsh.exe.
    """
    if not WIN32COM_AVAILABLE:
        return 1
    try:
        shell = Dispatch("WScript.Shell")  # pylint: disable=possibly-used-before-assignment
        name = f"{TOOL_NAME}{name_suffix} - {get_numeric_version()}"
        shortcut_path = parent / f"{name}.lnk"
        if shortcut_path.is_file():
            secho(f"Shortcut '{shortcut_path}' exists. Updating...", fg="yellow")
        shortcut = shell.CreateShortCut(str(shortcut_path))
        shortcut.WorkingDirectory = os.environ["USERPROFILE"]
        shortcut.Targetpath = target_path
        shortcut.Arguments = " ".join(arguments + ["-Command", shortcut_command])
        shortcut.WindowStyle = 3
        shortcut.save()
    except com_error as e:  # pylint: disable=possibly-used-before-assignment
        recho(f"Failed to create shortcut:\n{e}")
        return 1
    secho(f"Successfully created shortcut: {shortcut_path}", fg="green")
    return 0


def install_fox_cli_tools_on_host() -> int:
    """Install the fox CLI toolchain locally (i.e., outside the project
    directory).
    """
    err = 0

    # install the environment: "$PREFIX/envs/local/<version>"
    prefix = PREFIX_LINUX
    if get_platform() == "win32":
        prefix = PREFIX_WIN32
    env_install_dir = Path(prefix) / f"envs/local/{get_numeric_version()}"
    logger.debug("Installation directory: %s", env_install_dir)
    if env_install_dir.exists():
        err_msg = (
            f"Directory '{env_install_dir}' already exists.\n"
            "Remove the directory and re-run the command."
        )
        raise SystemExit(err_msg)

    activate_script = env_install_dir / "bin/activate"
    if get_platform() == "win32":
        activate_script = env_install_dir / "Scripts/Activate.ps1"
    logger.debug("Activate script: %s", activate_script)

    if python_setup.main(
        env_dir=str(env_install_dir), self_install=True, dry_run=False
    ):
        return 1

    # create platform specific shortcuts
    if get_platform() == "win32" and WIN32COM_AVAILABLE:
        app_data = Path(os.environ["APPDATA"])
        start_menu = app_data / "Microsoft/Windows/Start Menu/Programs"
        parent = start_menu / "foxbms-2"
        parent.mkdir(parents=True, exist_ok=True)

        # Usage for GUI:
        err += _create_shortcut_win32(
            parent=parent,
            name_suffix=" GUI",
            arguments=["-WindowStyle", "hidden", "-NoLogo", "-NoProfile"],
            shortcut_command=f"&{activate_script}; python -m fox_cli gui",
        )

        # Usage for Shell:
        err += _create_shortcut_win32(
            parent=parent,
            name_suffix="",
            arguments=["-NoLogo", "-NoProfile", "-noexit"],
            shortcut_command=f"&{activate_script}",
        )

    return err
