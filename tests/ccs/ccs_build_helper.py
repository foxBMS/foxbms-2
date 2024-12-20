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

"""Helper script to configure the CCS variants build"""

import argparse
import logging
import os
import shutil
import sys
from pathlib import Path

HAVE_GIT = False
try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    pass


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of file in git repository

    Returns:
        root (string): root path of the git repository
    """
    root = os.path.join(os.path.dirname(path), "..", "..", "..")
    if HAVE_GIT:
        try:
            repo = Repo(path, search_parent_directories=True)
            root = repo.git.rev_parse("--show-toplevel")
        except InvalidGitRepositoryError:
            pass
    return root


ROOT = Path(get_git_root(os.path.realpath(__file__)))


def main():
    """This script does this and that"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    parser.add_argument(
        "--ccs-version",
        dest="ccs_version",
        required=True,
        choices=[
            "ccs1000",
            "ccs1011",
            "ccs1020",
            "ccs1030",
            "ccs1031",
            "ccs1040",
            "ccs1100",
            "ccs1100-c99",
            "ccs1200",
            "expect-failure",
        ],
        help="set the CCS version",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    logging.debug("Verbosity: %s", args.verbosity)
    logging.debug("CCS version: %s", args.ccs_version)
    if sys.platform.lower() == "linux":
        plat = "linux"
    elif sys.platform.lower() == "win32":
        plat = "win32"
    else:
        plat = ""
        sys.exit("Unsupported platform.")
    logging.debug("Platform: %s", plat)

    srcs = [
        ROOT / f"tests/ccs/{args.ccs_version}/{args.ccs_version}_cc-options.yaml",
        ROOT / f"tests/ccs/{args.ccs_version}/{args.ccs_version}_linker_pulls.json",
        ROOT / f"tests/ccs/{args.ccs_version}/{args.ccs_version}_remarks.txt",
        ROOT / f"tests/ccs/{args.ccs_version}/{args.ccs_version}_paths_{plat}.txt",
    ]
    dests = [
        ROOT / "conf/cc/cc-options.yaml",
        ROOT / "conf/cc/linker_pulls.json",
        ROOT / "conf/cc/remarks.txt",
        ROOT / f"conf/env/paths_{plat}.txt",
    ]
    for src, dest in zip(srcs, dests):
        shutil.copy(src, dest)


if __name__ == "__main__":
    main()
