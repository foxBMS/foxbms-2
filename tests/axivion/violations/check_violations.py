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

"""Script that crates a custom reporting based on the Axivion analysis.
1. It checks, that certain types of already 'eliminated' violations in the
   entire codebase are not re-introduced.
2. It checks, that files that are violation free, stay violation free.
"""

import argparse
import json
import logging
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Tuple


@dataclass
class Analysis:
    """Stores the analysis report path and the report itself"""

    file: Path
    analysis: dict


def parse_config_file(config_file: Path) -> List[str]:
    """Parses the line separated configuration files (line comments are enabled
    by lines starting with '#')"""
    config_txt = config_file.read_text(encoding="utf-8")
    config = []
    for line in config_txt.splitlines():
        if not line or line.lstrip().startswith("#"):
            continue
        config.append(line.strip())
    return sorted(config)


def get_location(violation: dict) -> Tuple[str, int, int]:
    """Returns the location of a violation"""
    file_path = violation.get("path", "")
    if not file_path:
        file_path = violation.get("leftPath", "")
        if not file_path:
            file_path = "."
    line = violation.get("line", "") or violation.get("leftLine", 0)
    column = violation.get("column", 1)
    return (file_path, line, column)


def check_rule_violations(config_file: Path, report: Analysis) -> int:
    """Checks that certain violations are not present in the codebase."""
    err = 0
    forbidden_rules = parse_config_file(config_file)
    logging.info("Rules that shall be followed:\n%s", "\n".join(forbidden_rules))
    for violation in report.analysis:
        error_number = violation["errorNumber"]
        file_path, line, column = get_location(violation)
        logging.debug("Checking violation '%s' for file '%s'.", error_number, file_path)
        if error_number in forbidden_rules:
            msg = (
                f"Found forbidden violation {error_number} in "
                f"{file_path}:{line}:{column}."
            )
            if suppressed := violation["suppressed"]:
                justification = violation["justification"]
                msg += f" (justification: {justification}, suppressed: {suppressed})"
                logging.debug(msg)
            else:
                logging.critical(msg)
                err += 1
    return err


def check_violation_free_files_is_up_to_date(config_file: Path) -> int:
    """Check that the files list is up-to-date"""
    err = 0
    config_txt = config_file.read_text(encoding="utf-8")
    config = []
    for line in config_txt.splitlines():
        if not line:
            continue
        if line.lstrip().startswith("#src/"):
            pass
        elif line.lstrip().startswith("#"):
            continue
        config.append(line.strip().lstrip("#"))
    expected_files = sorted(config)
    cmd = ["git", "rev-parse", "--show-toplevel"]
    cwd = Path(__file__).parent
    root = ""
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=cwd) as p:
        root = p.communicate()[0].decode("utf-8").strip()
    if not root:
        sys.exit("Could not determine the repository root.")
    cmd = ["git", "ls-files", "src/app/*.c", "src/app/*.h"]
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=root) as p:
        stdout: bytes = p.communicate()[0]
    files = sorted(stdout.decode("utf-8").splitlines())
    if not expected_files == files:
        logging.error(
            "File '%s' is not up-to-date:\n%s",
            config_file,
            set(expected_files) - set(files),
        )
        err = 1
    else:
        logging.debug("Expected files match actual files.")
    return err


def check_file_violations(config_file: Path, report: Analysis) -> int:
    """Checks that files listed as violation free, do not introduce violations."""
    err = 0
    files_with_no_violations = parse_config_file(config_file)
    logging.info(
        "Files that are required to be violation free:\n%s",
        "\n".join(files_with_no_violations),
    )
    for violation in report.analysis:
        # if a violation is suppressed and justified, we can skip this violation
        if violation["suppressed"] and violation["justification"]:
            continue
        file_path, line, column = get_location(violation)
        if file_path in files_with_no_violations:
            msg = (
                f"'{file_path}' is listed as violation free, but introduces "
                f"'{violation['errorNumber']}' at '{file_path}:{line}:{column}'."
            )
            logging.critical(msg)
            err += 1
    return err


def main():
    """This script searches based on a json report of Axivion analysis for
    - violations that are not allowed anywhere in the code base
    - files that completely free of violations
    in order to prevent a regression.
    """
    parser = argparse.ArgumentParser(
        description="""This script searches in the json report of Axivion analysis for
    - violations that are not allowed anywhere in the code base
    - files that completely free of violations\nin order to prevent a regression.""",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="Set verbosity level",
    )
    parser.add_argument(
        "-r",
        "--forbidden-rule-violations",
        dest="forbidden_rule_violations",
        action="store",
        type=Path,
        help="Line separated file specifying a list of forbidden rules",
    )
    parser.add_argument(
        "-f",
        "--violation-free-files",
        dest="violation_free_files",
        action="store",
        type=Path,
        help="Line separated file specifying a list of violation free files",
    )
    parser.add_argument(
        "report",
        action="store",
        default=None,
        nargs="+",
        type=Path,
        help="Axivion report file",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    for k, v in args.__dict__.items():
        logging.debug("%s: %s", k, v)

    valid = True
    if not args.forbidden_rule_violations and not args.violation_free_files:
        parser.print_help(sys.stderr)
        sys.exit(
            "\nYou need to specify --forbidden-rule-violations or --violation-free-files."
        )
    # if provided, check if valid
    if args.forbidden_rule_violations and not args.forbidden_rule_violations.is_file():
        valid = False
        logging.error(
            "forbidden-rule-violations: File '%s' does not exist.",
            args.forbidden_rule_violations,
        )
    # if provided, check if valid
    if args.violation_free_files and not args.violation_free_files.is_file():
        valid = False
        logging.error(
            "violation-free-files: File '%s' does not exist.",
            args.violation_free_files,
        )
    for report in args.report:
        if not report.is_file():
            valid = False
            logging.error("report: File '%s' does not exist.", report)
    if not valid:
        sys.exit("Invalid script arguments.")

    reports = [
        Analysis(i, json.loads(i.read_text(encoding="utf-8"))) for i in args.report
    ]
    logging.info(
        "Successfully loaded JSON report files:\n%s",
        "\n".join(str(i) for i in args.report),
    )

    err = 0
    for report in reports:
        if args.forbidden_rule_violations:
            err += check_rule_violations(args.forbidden_rule_violations, report)
        if args.violation_free_files:
            err += check_violation_free_files_is_up_to_date(args.violation_free_files)
            err += check_file_violations(args.violation_free_files, report)
    sys.exit(err)


if __name__ == "__main__":
    main()
