#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Helper script for updating dates in a merge request"""

import argparse
import logging
import os
import sys
from datetime import date
import re
from pathlib import Path
import subprocess

UPDATED_RE_COMPILED = re.compile(
    "[ ]\\* (@updated)[ ][0-9]{4}-[0-9]{2}-[0-9]{2}[ ]\\(date of last update\\)$"
)


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of test_f_guidelines

    Returns:
        root (string): root path of the git repository
    """
    root = os.path.join(os.path.dirname(path), "..", "..")
    cmd = ["git", "rev-parse", "--show-toplevel"]
    with subprocess.Popen(cmd, stdout=subprocess.PIPE) as p:
        root = p.communicate()[0].decode("utf-8").strip()
    return root


ROOT = Path(get_git_root(os.path.realpath(__file__)))


def main():
    """This script searches for files changed in a merge request and updates the change date"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    cmd = ["git", "diff", "--name-only", "master"]
    tmp = []
    with subprocess.Popen(cmd, stdout=subprocess.PIPE) as p:
        tmp = p.communicate()[0].decode("utf-8").strip().splitlines()
    logging.debug(f"cmd: {cmd}, out:\n{tmp}")
    if not tmp:
        logging.warning("Could not find any changed files.")
        sys.exit(0)
    changed_files = [
        Path(os.path.join(ROOT, i.strip())) for i in tmp if i.endswith((".c", ".h"))
    ]

    m = False  # pylint: disable=invalid-name
    any_file = False
    for i in changed_files:
        if not i.is_file():
            continue
        txt = i.read_text(encoding="ascii")
        for line in txt.splitlines():
            m = UPDATED_RE_COMPILED.match(line)
            if m:
                txt = txt.replace(
                    m.group(),
                    f" * @updated {date.today().strftime('%Y-%m-%d')} (date of last update)",
                )
                i.write_text(txt, encoding="ascii")
                break
        if not m:
            logging.error(f"Something went wrong, check file '{i.a_path}'.")
        m = False
        any_file = True

    if changed_files and any_file:
        cmd = ["git", "add"] + [
            i.relative_to(ROOT).as_posix() for i in changed_files if i.is_file()
        ]
        with subprocess.Popen(cmd, stdout=subprocess.PIPE) as p:
            p.communicate()

    if changed_files:
        cmd = ["git", "add"] + [i.relative_to(ROOT).as_posix() for i in changed_files]
        with subprocess.Popen(cmd, stdout=subprocess.PIPE) as p:
            p.communicate()


if __name__ == "__main__":
    main()
