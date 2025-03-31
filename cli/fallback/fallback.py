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

"""Error handling when the Python environment is missing."""

# only standard imports!
# no imports from the cli package, the script must be self-contained!

import sys
import time
from multiprocessing import Process
from pathlib import Path
from subprocess import PIPE, Popen

COMPLETED = False
MAX_SLEEP_TIME = 5 * 60  #  in seconds

ROOT = Path(__file__).parents[2]


def get_python_version() -> tuple[str, str]:
    """Get the name of the needed Python executable."""
    if sys.platform.lower().startswith("win32"):
        py = "py"
        ver = "-3.12"
        cmd = [py, ver, "--version"]
    else:
        py = "python3.12"
        ver = ""
        cmd = [py, ver, "--version"]

    cmd = list(filter(None, cmd))
    print(f"Running: {' '.join(cmd)}")
    with Popen(cmd, cwd=ROOT, stdout=PIPE) as proc:
        out = proc.communicate(timeout=5)[0]
    if proc.returncode:
        msg = (
            "Go To python.org and download the latest version of Python 3.12.\n"
            "Then re-run the command."
        )
        print(msg, file=sys.stderr)
        sys.exit(1)
    print(f"Python version: {out.decode(encoding='utf-8')}")
    return py, ver


def install_confirmation() -> None:
    """Ask user for confirmation to install the environment."""
    print(
        "The foxBMS Python environment is missing.\n"
        "It can be installed automatically.\n"
        "Answer 'Yes' to automatically install the environment or 'No' to "
        "abort installation process.",
    )
    valid_yes = ("y", "ye", "yes")
    valid_no = ("n", "no")
    while True:
        answer = input().lower()
        if answer in valid_yes:
            yes = True
            break
        if answer in valid_no:
            yes = False
            break
        print("yes or no answer required.", file=sys.stderr)
    if not yes:
        msg = (
            "This Python environment is required.\n"
            "You can also intall it manually as shown in the documentation.\n"
            "Exiting."
        )
        print(msg, file=sys.stderr)
        sys.exit(1)


def create_env(py: str, env_dir: str, ver: str = "") -> None:
    """Create venv environment."""
    cmd = [py, ver, "-m", "venv", env_dir]
    cmd = list(filter(None, cmd))
    print(f"Running: {' '.join(cmd)}")
    with Popen(cmd, stdout=PIPE, stderr=PIPE) as proc:
        proc.communicate()


def create_env_process(py: str, env_dir: str, ver: str = "") -> int:
    """Process to create venv environment."""
    p_create = Process(target=create_env, args=(py, env_dir, ver))
    p_create.start()
    sleep_time = 0
    while p_create.is_alive():
        print(".", end="", flush=True)
        time.sleep(0.5)
        sleep_time += 1
        if sleep_time > MAX_SLEEP_TIME:
            # a few minutes should be more than enough to install the environment
            p_create.terminate()
            print("\n")
            print("Could not create virtual environment.", file=sys.stderr)
            return 1
    print("\n")
    return 0


def install_packages(env_dir: str) -> None:
    """Install packages in venv environment."""
    requirements = ROOT / "requirements.txt"
    if sys.platform.lower().startswith("win32"):
        cmd = [str(Path(env_dir) / "Scripts/python.exe")]
    else:
        cmd = [str((Path(env_dir) / "bin/python").as_posix())]
    cmd.extend(["-m", "pip", "install", "-r", str(requirements)])
    print(f"Running: {' '.join(cmd)}")
    with Popen(cmd) as proc:
        proc.communicate()


def install_packages_process(env_dir: str) -> int:
    """Process to install packages in venv environment."""
    p_install = Process(target=install_packages, args=(env_dir,))
    p_install.start()
    sleep_time = 0
    while p_install.is_alive():
        print(".", end="", flush=True)
        time.sleep(0.5)
        sleep_time += 1
        if sleep_time > MAX_SLEEP_TIME:
            # a few minutes should be more than enough to install the packages
            p_install.terminate()
            print("\n")
            print("Could not intall packages into the environment.", file=sys.stderr)
            return 1
    print("\n")
    return 0


def main() -> int:
    """Run installer helper"""
    print("foxBMS 2 Install Helper")
    py, ver = get_python_version()
    install_confirmation()
    # create a new environment
    if create_env_process(py=py, env_dir=sys.argv[1], ver=ver) != 0:
        return 1  # environment creation failed
    # install packages in the new environment
    if install_packages_process(env_dir=sys.argv[1]) != 0:
        return 1  # package installation failed
    print("Successfully installed the Python environment.")
    return 0


if __name__ == "__main__":
    main()
