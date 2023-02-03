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

"""Calculates the checksum of file or directory and checks it against a known
hash."""

import argparse
import hashlib
import logging
import sys
from pathlib import Path

MAX_READ = 4096


def get_checksum(files: list[Path]):
    """returns the combined checksum for a list of files and their names."""
    checksum = hashlib.sha256()
    for _file in files:
        buf = _file.name.encode("utf-8")
        checksum.update(buf)
        with open(_file, "rb") as f:
            while buf:
                buf = f.read(MAX_READ)
                checksum.update(buf)
            logging.debug(f"{_file} --> {checksum.hexdigest()}")

    return checksum.hexdigest()


def main():
    """verifies a checksum"""
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
        dest="fd_input",
        help="FILE or DIRECTORY to be checked",
    )
    parser.add_argument(
        dest="known_hash",
        help="Known hash against which should be tested.",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    files = Path(args.fd_input)

    logging.info(f"Known hash is:      {args.known_hash}")

    if files.is_file():
        files = [files]
    else:
        files = [i for i in files.rglob("*") if i.is_file()]

    calculated_hash = get_checksum(files)
    logging.info(f"Calculated hash is: {calculated_hash}")

    if not args.known_hash == calculated_hash:
        logging.error("Hashes do not match.")
        sys.exit(1)
    else:
        logging.debug("Hash matches.")


if __name__ == "__main__":
    main()
