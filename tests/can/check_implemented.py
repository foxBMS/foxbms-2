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

"""Python script to check if all callbacks defined in the .dbc file are
implemented in callback functions."""
import os
import sys
import logging
import argparse
import re
from pathlib import Path
import cantools

HAVE_GIT = False
try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    pass

FILE_RE = r"\(in:(.*):(.*)\)"
FILE_RE_COMPILED = re.compile(FILE_RE)


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of test_f_guidelines

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
BDC_DIR_REL = Path(os.path.join("tools", "dbc"))
SRC_DIR_REL = Path(os.path.join("src", "app"))


def evaluate_implementation(callback_file: Path, function_name: str) -> bool:
    """Checks whether a function name is found in a source file or not."""
    found = False
    impl = callback_file.read_text()
    if function_name in impl:
        logging.info(f"Found '{function_name}' in '{callback_file}'.")
        found = True
    else:
        logging.error(f"Did not find '{function_name}' in '{callback_file}'.")
    return found


def main():
    """This script produces a symbole file for cell voltages and temperatures"""
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
        "-i",
        "--input-file",
        dest="input_file",
        action="store",
        default=BDC_DIR_REL / "foxbms.dbc",
        help="DBC file to be verified.",
    )
    parser.add_argument(
        "-c",
        "--src-dir",
        dest="src_dir",
        action="store",
        default=ROOT / SRC_DIR_REL,
        help="Path to directory containing the callback implementations",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    logging.debug(args)

    if not isinstance(args.input_file, Path):
        args.input_file = Path(args.input_file)
    input_file = args.input_file.absolute()
    can_db = cantools.database.load_file(input_file)
    if not isinstance(args.src_dir, Path):
        args.src_dir = Path(args.src_dir)
    src_dir = args.src_dir.absolute()
    source_files = list(src_dir.rglob("**/*.c"))
    if not source_files:
        sys.exit("Could not find any source files.")

    errors = 0
    for i in can_db.messages:
        if not i.comment:
            errors += 1
            logging.error(
                f"Could not find any comment for message '{i.name}' ({hex(i.frame_id)})."
            )
            continue
        logging.info(
            f"Found comment for message '{i.name}' ({hex(i.frame_id)}): '{i.comment}'."
        )
        m = FILE_RE_COMPILED.search(i.comment)  # pylint: disable=invalid-name
        if not m:
            errors += 1
            logging.error(
                "Could not find comment for message "
                f"'{i.name}' ({hex(i.frame_id)}) that matches '{FILE_RE}'."
            )
            continue
        # we found a comment should point to the correct implementation file
        files = [i for i in source_files if m[1] in str(i)]
        try:
            search_file = files[0]
        except IndexError:
            errors += 1
            logging.error(
                f"Could not find implementation file '{m[1]}' for "
                f"message '{i.name}'."
            )
            continue
        if not search_file.is_file():
            errors += 1
            logging.error(
                f"Path to implementation file '{search_file}' for "
                f"message '{i.name}' is not a file."
            )
            continue
        # now we are sure, that the file that should contain the implementation
        # exists, and we can evalute it.
        if not evaluate_implementation(search_file, m[2]):
            errors += 1
    return errors


if __name__ == "__main__":
    sys.exit(main())
