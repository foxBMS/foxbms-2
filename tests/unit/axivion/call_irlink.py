#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
import shutil
import subprocess
import sys
from pathlib import Path

AXIVION_BUILD_DIR = Path("tests/unit/build/axivion")


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

    irlink = shutil.which("irlink")
    if not irlink:
        sys.exit("Could not find 'irlink'.")

    tests_dir = AXIVION_BUILD_DIR / "build/app_host_unit_test/test/runners"

    tests = [str(i) for i in tests_dir.glob("test_*.exe")]
    logging.debug("Number of tests %s", len(tests))

    splitted_tests = []

    tmp = []
    for i in tests:
        if len(" ".join(tmp + [i])) < 10000:
            tmp.append(i)
        else:
            splitted_tests.append(tmp)
            tmp = []

    logging.debug("Number of splitted command lines: %s", len(splitted_tests))
    out_files = [
        f"{AXIVION_BUILD_DIR}/foxbms-unit-tests-{i}.ir"
        for i in range(len(splitted_tests))
    ]

    for inputs, output in zip(splitted_tests, out_files):
        cmd = [irlink, "--multi_binary", "-j"] + inputs + [output]
        logging.debug(cmd)
        with subprocess.Popen(cmd) as p:
            p.communicate()

    cmd = (
        [irlink, "--multi_binary", "-j"]
        + out_files
        + [f"{AXIVION_BUILD_DIR}/foxbms-unit-tests.ir"]
    )

    logging.debug(cmd)
    with subprocess.Popen(cmd) as p:
        p.communicate()


if __name__ == "__main__":
    main()
