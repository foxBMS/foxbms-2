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

"""Checks files references that are used in some documentation files."""

import argparse
import logging
import sys
import re
from pathlib import Path

import git
from git.exc import InvalidGitRepositoryError

SCRIPT_DIR = Path(__file__).parent.resolve()


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
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    try:
        repo = git.Repo(SCRIPT_DIR, search_parent_directories=True)
        root = Path(repo.git.rev_parse("--show-toplevel"))
    except InvalidGitRepositoryError:
        sys.exit("Test can only be run in a git repository.")

    ls_files = repo.git.ls_files("docs/software/modules/*.rst").splitlines()
    rst_files: list[Path] = [root / i for i in ls_files]
    reg = re.compile(
        # r"``((tests|src)\/.*)``\s+ \(`API\s+\<(.*)\>`__,\s+\`source\s\<(.*)`__\)"
        r"``((tests|src)\/.*)``\s+ \(`API\s+\<(.*)\>`__,\s+\`source\s\<(.*)\>`__\)"
    )
    err = 0
    for i in rst_files:
        if not i.is_file():
            logging.warning(f"File '{i}' does not exists.")
            continue
        txt = i.read_text(encoding="utf-8")
        found_references = []
        for line in txt.splitlines():
            m = reg.search(line)
            if not m:
                continue
            found_references.append(
                (Path(m.group(1)).name, Path(m.group(3)).name, Path(m.group(4)).name)
            )
        for ref in found_references:
            if ref[0].endswith(".c"):
                api_end = "_8c.html"
                tests_end = "_8c_source.html"
            elif ref[0].endswith(".h"):
                api_end = "_8h.html"
                tests_end = "_8h_source.html"
            expected_api_ref = ref[0].replace("_", "__")[:-2] + api_end
            expected_tests_ref = ref[0].replace("_", "__")[:-2] + tests_end
            logging.debug(ref)
            if not ref[1] == expected_api_ref:
                logging.error(
                    f"In file '{i}' for \n\t'{ref[0]}'\nexpected\n\t'"
                    f"{expected_api_ref}'\nbut found\n\t'{ref[1]}'.\n"
                )
                err += 1
            if not ref[2] == expected_tests_ref:
                logging.error(
                    f"In file '{i}' for \n\t'{ref[0]}'\nexpected\n\t'"
                    f"{expected_tests_ref}'\nbut found\n\t'{ref[2]}'.\n"
                )
                err += 1
    sys.exit(err)


if __name__ == "__main__":
    main()
