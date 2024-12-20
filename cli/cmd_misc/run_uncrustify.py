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

"""Run uncrustify on the FreeRTOS sources in the foxBMS source tree"""

import logging
import os
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path
from shutil import which

from click import secho

from ..helpers.env_vars import HOMEDRIVE
from ..helpers.host_platform import PLATFORM
from ..helpers.misc import PROJECT_ROOT
from ..helpers.spr import SubprocessResult, run_process

ROOT = Path(__file__).parent.parent.parent


FREERTOS_UNCRUSTIFY_CONFIG = str(ROOT / "src/os/freertos/.github/uncrustify.cfg")
FREERTOS_FILES = [
    str(i)
    for i in list((ROOT / "src/os/freertos").rglob("*.c"))
    + list((ROOT / "src/os/freertos").rglob("*.h"))
]


def run_uncrustify_process(
    uncrustify: str, args: list[str], _file: str
) -> SubprocessResult:
    """Runs uncrustify with the provided arguments on the specified file as a
    subprocess"""
    cmd = [uncrustify] + args + [_file]
    cwd = ROOT
    return run_process(cmd, cwd=cwd, stderr=None, stdout=None)


def lint_freertos(check=True) -> int:
    """Run uncrustify on the foxBMS FreeRTOS source tree"""

    uncrustify_install_path = os.environ.get("PATH", "")
    if PLATFORM == "win32":
        uncrustify_install_path = (
            os.path.join(HOMEDRIVE, "uncrustify", "uncrustify-0.69")
            + os.pathsep
            + uncrustify_install_path
        )
    uncrustify = which("uncrustify", path=uncrustify_install_path)

    if not uncrustify:
        secho("Could not find uncrustify.", fg="red", err=True)
        return 1

    uncrustify_args = [
        "-c",
        str(PROJECT_ROOT / "src/os/freertos/.github/uncrustify.cfg"),
    ]
    if check:
        uncrustify_args.append("--check")

    err = 0
    with ProcessPoolExecutor() as pool:
        futures = []
        for i in FREERTOS_FILES:
            if not Path(i).is_file():
                err += 1
                secho(f"'{Path(i).absolute()}' is not a file.", fg="red", err=True)
                continue
            logging.debug("Start worker for file '%s'", i)
            futures.append(
                pool.submit(
                    run_uncrustify_process,
                    uncrustify,
                    uncrustify_args,
                    i,
                )
            )

        for f in futures:
            exit_code = f.result().returncode
            if exit_code:
                logging.error("exitcode: %s", exit_code)
                logging.error("stdout: %s", f.result().out)
                logging.error("stderr: %s", f.result().err)
            else:
                logging.debug("exitcode: %s", exit_code)
                logging.debug("stdout: %s", f.result().out)
                logging.debug("stderr: %s", f.result().err)

            err += exit_code

    return err
