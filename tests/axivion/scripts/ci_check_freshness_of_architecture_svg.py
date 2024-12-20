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

"""This script is intended for CI in order to check if the architecture is recent

It will check the change date of the architecture SVG in the documentation and
of all files that constitute input to this file and fail if the architecture
SVG is not newer (or at least equally) old as the other files.

In order to fix a failure, check the documentation on how to update the
architecture SVG.
"""

import argparse
import datetime
import logging
import os
import sys
import shutil
from pathlib import Path
from subprocess import Popen, PIPE

SCRIPT_DIR = Path(__file__).parent


def get_git_change_date(filename):
    """extract the commit date of a file as datetime from git"""
    git = shutil.which("git")
    cmd = [git, "log", "-1", "--date=short", "--pretty=%at", filename]
    with Popen(cmd, cwd=SCRIPT_DIR, stdout=PIPE) as p:
        std_out = p.communicate()[0]
    extracted_date = std_out.decode("utf-8").strip()
    timestamp = datetime.datetime.fromtimestamp(int(extracted_date))
    logging.debug("extracted timestamp from file %s: %s", filename, timestamp)
    return timestamp


def main():
    """This script checks the recentness of the architecture svg"""
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

    logging.debug("Searching for the file change dates.")

    file_architecture = os.path.abspath(
        os.path.join(
            SCRIPT_DIR,
            "..",
            "targets",
            "app",
            "architecture.gxl",
        )
    )
    file_layout = os.path.abspath(
        os.path.join(
            SCRIPT_DIR,
            "..",
            "targets",
            "app",
            "architecture_hierarchy_belongs_to_layout.gvl",
        )
    )
    file_output = os.path.abspath(
        os.path.join(
            SCRIPT_DIR,
            "..",
            "..",
            "..",
            "docs",
            "software",
            "architecture",
            "img",
            "axivion_architecture.svg",
        )
    )

    def get_date_helper(filename):
        date = get_git_change_date(filename)
        logging.debug("timestamp for %s: %s", filename, date)
        return date

    date_architecture = get_date_helper(file_architecture)
    date_layout = get_date_helper(file_layout)
    date_output = get_date_helper(file_output)

    date_oldest_educt = max(date_architecture, date_layout)
    logging.info("oldest educt is from %s.", date_oldest_educt)

    logging.info("youngest product is from %s.", date_output)

    if date_oldest_educt > date_output:
        logging.error(
            "The Axivion architecture SVG is not up to date. Refer "
            "to the documentation on how to update the architecture SVG."
        )
        sys.exit(1)

    logging.debug("Architecture SVG is up to date.")


if __name__ == "__main__":
    main()
