#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
import re
import subprocess
import sys
from datetime import date
from pathlib import Path

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

UPDATED_RE_COMPILED = re.compile(
    "[ ]\\* (@updated)[ ][0-9]{4}-[0-9]{2}-[0-9]{2}[ ]\\(date of last update\\)$"
)


def main():
    """Automatically create the test file."""
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

    cmd = [sys.executable, os.path.join("tools", "waf"), "check_test_files"]
    logging.debug(cmd)
    with subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=ROOT
    ) as p:
        out, err = p.communicate()
        out, err = (
            out.decode("utf-8").strip().splitlines(),
            err.decode("utf-8").strip().splitlines(),
        )

    test_template = (
        Path(ROOT / os.path.join("conf", "tpl", "test_c.c"))
        .read_text(encoding="utf-8")
        .splitlines()
    )
    today_iso = date.today().strftime("%Y-%m-%d")

    for i in err:
        if not "should be in" in i:
            continue
        out_txt = test_template
        source_file = Path(i.split("Missing test file for:")[-1].split()[0]).name
        missing_file = i.split("should be in: ")[-1]
        if missing_file.endswith(")"):
            missing_file = missing_file[:-1]
        logging.debug(f"Need to create test file: '{missing_file}'.")
        missing_file = Path(missing_file)
        out_txt[42] = f" * @file    {missing_file.name}"
        out_txt[44] = f" * @date    {today_iso} (date of creation)"
        out_txt[45] = f" * @updated {today_iso} (date of last update)"
        out_txt[57] = f'TEST_FILE("{source_file}")'
        if not missing_file.parent.exists():
            missing_file.parent.mkdir()
        missing_file.write_text("\n".join(out_txt) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
