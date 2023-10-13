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

"""Gets the macro values from the compiler output"""

import argparse
import logging
import os
import sys

from typing import List
from pathlib import Path
import git
from git.exc import InvalidGitRepositoryError

SCRIPT_DIR = Path(__file__).parent.resolve()

REPO_ROOT = SCRIPT_DIR.parent.parent.parent
try:
    repo = git.Repo(SCRIPT_DIR, search_parent_directories=True)
    REPO_ROOT = Path(repo.git.rev_parse("--show-toplevel"))
except InvalidGitRepositoryError:
    pass


def get_ppm_files() -> List[Path]:
    """creates a generator for all relevant macro files in the build directory"""
    build_dir = REPO_ROOT / "build/bin"
    return build_dir.rglob("*.ppm")


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

    macros = []
    for i in get_ppm_files():
        txt = i.read_text(encoding="utf-8")
        for line in txt.splitlines():
            if not line.strip().startswith("#define"):
                continue
            if line.endswith("/* Predefined */"):
                continue
            if f"{os.sep}ccs{os.sep}tools" in line:  # ignore compiler builtins
                continue
            try:
                # ignore additional compiler builtins
                if line.split(" ", maxsplit=1)[1].startswith("_"):
                    continue
            except IndexError:
                pass
            if f"{os.sep}os{os.sep}freertos" in line:  # ignore FreeRTOS macros
                continue
            if f"{os.sep}hal{os.sep}" in line:  # ignore TI HAL macros
                continue
            try:
                # ignore additional compiler builtins
                if line.split(" ", maxsplit=1)[1].startswith("FOXBMS__"):
                    continue
            except IndexError:
                pass

            # remove comment
            line = line.split("/*")[0].strip()
            try:
                line = line.split(" ", maxsplit=1)[1]
            except IndexError as err:
                logging.debug(err)
                continue
            try:
                macro, value = line.split(" ", maxsplit=1)
            except ValueError:
                logging.debug(f"{macro} has no value. Ignoring...")
                continue
            if not value.startswith("("):
                value = f"({value})"
            macros.append((macro, value))
    macros = list(set((tuple(i) for i in macros)))
    logging.info(f"Adding {len(macros)} macro(s).")

    macro_cmm_file_in = REPO_ROOT / "tools/debugger/lauterbach/load_macro_values.cmm.in"
    if not macro_cmm_file_in.is_file():
        sys.exit(f"Could not find file {macro_cmm_file_in}")
    logging.info(f"Reading configuration file '{macro_cmm_file_in}'")
    txt = macro_cmm_file_in.read_text(encoding="utf-8")
    replacement = "\n".join(
        [f"sYmbol.CREATE.MACRO {macro} {value}" for macro, value in macros]
    )
    macro_cmm_file_out = REPO_ROOT / "build/load_macro_values.cmm"
    REPO_ROOT.mkdir(exist_ok=True)
    txt = txt.replace("@MACROS_AND_VALUES@", replacement)
    logging.info(f"Writing configuration file '{macro_cmm_file_out}'")
    macro_cmm_file_out.write_text(txt, encoding="utf-8")
    logging.info("Done...")


if __name__ == "__main__":
    main()
