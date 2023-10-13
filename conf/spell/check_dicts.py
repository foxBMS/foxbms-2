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

"""Template for Python scripts"""

import argparse
import logging
import sys

from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.resolve()
ROOT_DIR = Path(__file__).parent.parent.parent


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

    dict_files = (SCRIPT_DIR / "dicts").glob("*.txt")
    err = 0
    dicts = {}
    for i in dict_files:
        txt = i.read_text(encoding="utf-8")
        if not txt:
            err += 1
            logging.error(
                f"Found empty dictionary: {i.absolute()}. "
                "Empty dicts are not allowed."
            )
        words = txt.splitlines()
        if not words == sorted(words):
            logging.error(
                f"Found unsorted dictionary: {i.absolute()}. "
                "Word lists must be alphabetically sorted."
            )
            err += 1
        if not len(words) == len(set(words)):
            logging.error(
                f"Found duplicate entries in dictionary: {i.absolute()}. "
                "Words must be unique per file."
            )
            err += 1
        for word in words:
            for source, known_words in dicts.items():
                if word in known_words:
                    logging.error(
                        f"Dictionary {i.relative_to(ROOT_DIR)} contains word "
                        f"'{word}' which is already in the word list "
                        f"provided by '{source.relative_to(ROOT_DIR)}'."
                    )
        # now we are good to add the words to the global dictionary
        dicts[i] = words
    sys.exit(err)


if __name__ == "__main__":
    main()
