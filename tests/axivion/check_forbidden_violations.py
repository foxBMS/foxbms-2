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

"""Simple script that searches for the names of 'eliminated' violations in codebase"""

import logging
import argparse
import json
import sys


def main():
    """This script searches a JSON report of Axivion for the names
    of violations that we have fixed completely in order to prevent a
    regression.
    """
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
        "-f",
        "--file",
        dest="forbidden",
        action="store",
        default="forbidden_violations.txt",
        help="List of forbidden rules (one rule per line)",
    )
    parser.add_argument(
        "report",
        action="store",
        default=None,
        help="Axivion report file",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    forbidden_rules = []

    with open(args.forbidden, mode="r", encoding="utf-8") as forbidden:
        lines = forbidden.readlines()
        lines = [rule.rstrip() for rule in lines]
        for rule in lines:
            if not rule.lstrip().startswith("#"):
                forbidden_rules.append(rule)
    logging.info(f"loaded the following rules as forbidden: {forbidden_rules}")

    with open(args.report, mode="r", encoding="utf-8") as report:
        report_json = json.load(report)
    logging.info("successfully loaded a JSON report file")

    fatal_violations_found = False
    for violation in report_json:
        error_number = violation["errorNumber"]
        logging.debug(f"checking violation {error_number}")
        if error_number in forbidden_rules:
            suppressed = violation["suppressed"]
            violation_text = (
                f"found forbidden violation {error_number} in "
                f"{violation['path']}:{violation['line']}:{violation['column']}"
                f" with justification: {violation['justification']}"
                f" suppressed: {suppressed}"
            )
            if suppressed:
                logging.info(violation_text)
            else:
                logging.fatal(violation_text)
                fatal_violations_found = True

    if fatal_violations_found:
        sys.exit(1)
    else:
        logging.info("no forbidden violations found")


if __name__ == "__main__":
    main()
