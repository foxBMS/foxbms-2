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

# pylint: disable=invalid-name

"""Template for Python scripts"""
import argparse
import json
import logging
import os
import sys
from pathlib import Path
from shutil import which
from subprocess import PIPE, Popen

import git
from git.exc import InvalidGitRepositoryError

SCRIPT_DIR = Path(__file__).parent.resolve()

try:
    repo = git.Repo(SCRIPT_DIR, search_parent_directories=True)
    REPO_ROOT = Path(repo.git.rev_parse("--show-toplevel"))
except InvalidGitRepositoryError:
    sys.exit("Test can only be run in a git repository.")


def main():
    """This script compiles the c standard test suite and runs the tests"""
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

    gcc = which("gcc")
    source = SCRIPT_DIR / "c-std-test.c"
    include_specifier = "-I"
    include = (
        Path(REPO_ROOT / os.path.join("src", "app", "main", "include"))
        .resolve()
        .as_posix()
    )
    standard_specifier = "--std="
    standards = ["c89", "c90", "iso9899:199409", "c99", "c11", "c17"]
    out_specifier = "-o"

    outs = {}
    for i in standards:
        cmd = [
            gcc,
            f"{include_specifier}{include}",
            f"{standard_specifier}{i}",
            "-DASSERT_LEVEL=0",
            str(source.absolute().as_posix()),
            out_specifier,
            f"{SCRIPT_DIR.as_posix()}/test-{i.replace(':', '-')}",
        ]
        cwd = SCRIPT_DIR
        logging.debug(" ".join(cmd))

        with Popen(cmd, cwd=cwd, stdout=PIPE, stderr=PIPE) as p:
            _, err = p.communicate()
        err = [
            i.strip()
            for i in err.decode("utf-8").splitlines()
            if i.strip().startswith("#warning")
        ]
        outs[i] = err

    with open(Path(SCRIPT_DIR / "c-std-test.json"), "r", encoding="utf-8") as f:
        expected_output = json.load(f)

    test_errors = 0
    for std, warning in outs.items():
        expected_warning = expected_output[std]
        if not warning == expected_warning:
            logging.error(f"Warning did not match for {std}.")
            logging.error(f"Expected: {warning}")
            logging.error(f"Got:      {expected_warning}")
            test_errors += 1
        else:
            logging.debug(f"Got expected warning for {std}.")
    sys.exit(test_errors)


if __name__ == "__main__":
    main()
