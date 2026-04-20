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

"""Error handling when the Python environment is missing."""

# !!! only standard library imports !!!
# no imports from the cli package, the script must be self-contained for usage
# in fox.ps1/sh

import sys
import time
from multiprocessing import Process
from pathlib import Path
from subprocess import PIPE, Popen

COMPLETED = False
MAX_SLEEP_TIME = 5 * 60  # in seconds

ROOT = Path(__file__).parents[2]


def _run_worker_process(process: Process, error_message: str) -> int:
    """Run a worker process with progress dots and timeout handling.

    Args:
        process: Process instance that executes a worker function.
        error_message: Message printed when execution exceeds timeout.

    Returns:
        ``0`` on success, ``1`` on timeout.
    """
    process.start()
    sleep_time = 0
    while process.is_alive():
        print(".", end="", flush=True)
        time.sleep(0.5)
        sleep_time += 1
        if sleep_time > MAX_SLEEP_TIME:
            process.terminate()
            print()
            print(error_message, file=sys.stderr)
            return 1
    print()
    return 0


def get_python_version(dry_run: bool = True) -> tuple[str, str]:
    """Determine the command used to invoke Python 3.12 on this platform.

    Args:
        dry_run: If ``True``, skip process execution and use mocked output.

    Returns:
        Tuple of executable name and optional version selector argument.
    """
    if sys.platform.lower().startswith("win32"):
        py = "py"
        ver = "-3.12"
        cmd = [py, ver, "--version"]
    else:
        py = "python3.12"
        ver = ""
        cmd = [py, ver, "--version"]

    cmd = list(filter(None, cmd))
    msg = "\nRunning"
    if dry_run:
        msg += " (dry run)"
    msg += f":\n -> cmd: {' '.join(cmd)}"
    print(msg)
    if dry_run:
        out = b"Python 3.12.10"
    else:
        with Popen(cmd, cwd=ROOT, stdout=PIPE) as proc:
            out = proc.communicate(timeout=5)[0]
        if proc.returncode:
            msg = (
                "Go To python.org and download the latest version of Python 3.12.\n"
                "Then re-run the command."
            )
            print(msg, file=sys.stderr)
            sys.exit(1)
    print(f" -> out: Python version: {out.decode(encoding='utf-8').strip()}\n")
    return py, ver


def install_confirmation(confirmation: bool) -> None:
    """Request user confirmation before automatic environment installation.

    Args:
        confirmation: When ``True``, skip interactive confirmation.
    """
    # make the script usable without interactive input
    if confirmation:
        return

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
            "You can also install it manually as shown in the documentation.\n"
            "Exiting..."
        )
        print(msg, file=sys.stderr)
        sys.exit(1)


def run_cmd(cmd: list[str], cwd: str = ".", dry_run: bool = True) -> None:
    """Run a command and print execution details.

    Args:
        cmd: Command and arguments to execute.
        cwd: Working directory for command execution.
        dry_run: If ``True``, print only and do not execute.
    """
    msg = "\nRunning"
    if dry_run:
        msg += " (dry run)"
    msg += f":\n -> cwd: {Path(cwd).absolute()}\n -> cmd: {' '.join(cmd)}"
    print(msg)
    if dry_run:
        return
    with Popen(cmd, cwd=cwd, stdout=PIPE, stderr=PIPE) as proc:
        proc.communicate()


def create_env(py: str, env_dir: str, ver: str = "", dry_run: bool = True) -> None:
    """Create a virtual environment using the selected Python executable.

    Args:
        py: Python executable command.
        env_dir: Destination directory for the virtual environment.
        ver: Optional version selector argument.
        dry_run: If ``True``, print only and do not execute.
    """
    cmd = [py, ver, "-m", "venv", env_dir]
    cmd = list(filter(None, cmd))
    run_cmd(cmd, dry_run=dry_run)


def create_env_process(
    py: str, env_dir: str, ver: str = "", dry_run: bool = True
) -> int:
    """Run virtual-environment creation in a child process with timeout handling.

    Args:
        py: Python executable command.
        env_dir: Destination directory for the virtual environment.
        ver: Optional version selector argument.
        dry_run: If ``True``, print only and do not execute.

    Returns:
        ``0`` on success, ``1`` on timeout.
    """
    p_create = Process(target=create_env, args=(py, env_dir, ver, dry_run))
    return _run_worker_process(
        process=p_create,
        error_message="Could not create virtual environment",
    )


def install_packages(py: str, cwd: str, dry_run: bool = True) -> None:
    """Install required Python packages into the target environment.

    Args:
        py: Python executable inside the target environment.
        cwd: Working directory for installation commands.
        dry_run: If ``True``, print only and do not execute.
    """
    requirements = ROOT / "requirements.txt"
    cmd = [py, "-m", "pip", "install", "-r", str(requirements)]
    run_cmd(cmd, cwd, dry_run)


def install_packages_process(py: str, cwd: str, dry_run: bool = True) -> int:
    """Run package installation in a child process with timeout handling.

    Args:
        py: Python executable inside the target environment.
        cwd: Working directory for installation commands.
        dry_run: If ``True``, print only and do not execute.

    Returns:
        ``0`` on success, ``1`` on timeout.
    """
    p_install = Process(target=install_packages, args=(py, cwd, dry_run))
    return _run_worker_process(
        process=p_install,
        error_message="Could not install packages into the environment",
    )


def install_fox_cli_package(py: str, cwd: str, dry_run: bool = True) -> None:
    """Install the local ``fox CLI`` package into the target environment.

    Args:
        py: Python executable inside the target environment.
        cwd: Working directory for installation commands.
        dry_run: If ``True``, print only and do not execute.
    """
    cmd = [py, "-m", "pip", "install", "."]
    run_cmd(cmd, cwd, dry_run)


def install_fox_cli_package_process(
    py: str, cwd: str, env_dir: str, dry_run: bool = True
) -> int:
    """Run local package installation in a child process with timeout handling.

    Args:
        py: Python executable inside the target environment.
        cwd: Working directory for installation commands.
        env_dir: Environment directory name used in timeout error messages.
        dry_run: If ``True``, print only and do not execute.

    Returns:
        ``0`` on success, ``1`` on timeout.
    """
    p_install = Process(target=install_fox_cli_package, args=(py, cwd, dry_run))
    return _run_worker_process(
        process=p_install,
        error_message=f"Could not install the 'fox CLI' package into the environment '{env_dir}'",
    )


def main(
    env_dir: str,
    confirm: bool = False,
    self_install: bool = False,
    dry_run: bool = True,
) -> int:
    """Run the environment installation helper workflow.

    Args:
        env_dir: Target virtual-environment directory.
        confirm: Skip interactive confirmation when ``True``.
        self_install: Also install local ``fox CLI`` package when ``True``.
        dry_run: If ``True``, print only and do not execute commands.

    Returns:
        ``0`` on success, ``1`` on failure.
    """
    print("foxBMS 2 Install Helper")
    py, ver = get_python_version(dry_run)
    if self_install:
        confirm = True
    print("--> Successfully determined Python version")

    install_confirmation(confirm)

    # create a new environment
    if create_env_process(py=py, env_dir=env_dir, ver=ver, dry_run=dry_run) != 0:
        return 1  # environment creation failed
    print("--> Successfully installed the Python environment")
    # install packages in the new environment

    env_py = str((Path(env_dir) / "bin/python").as_posix())
    if sys.platform.lower().startswith("win32"):
        env_py = str(Path(env_dir) / "Scripts/python.exe")

    if install_packages_process(py=env_py, cwd=env_dir, dry_run=dry_run) != 0:
        return 1  # package installation failed
    print("--> Successfully installed the dependencies in the Python environment")
    if not self_install:
        return 0

    # cwd is the project root
    if install_fox_cli_package_process(
        py=env_py, cwd=str(ROOT), env_dir=env_dir, dry_run=dry_run
    ):
        return 1  # could not install package
    print(f"--> Successfully installed 'fox CLI' package to '{env_dir}'")
    return 0


def _as_script() -> int:
    """Parse script arguments and run the installer workflow.

    Returns:
        Exit code from :func:`main`.
    """
    env_dir = sys.argv[1]
    confirm = False
    if "--confirm" in sys.argv:
        confirm = True
    dry_run = False
    if "--dry-run" in sys.argv:
        dry_run = True

    self_install = False
    if "--self-install" in sys.argv:
        self_install = True

    return main(
        env_dir=env_dir, confirm=confirm, self_install=self_install, dry_run=dry_run
    )


if __name__ == "__main__":
    _as_script()
