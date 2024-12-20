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

"""This script combines all provided analysis reports to on large report."""

import argparse
import json
import logging
import sys
import os
from pathlib import Path
from dataclasses import dataclass
from typing import List, Tuple
import tabulate


@dataclass
class Analysis:
    """Stores the analysis report path and the report itself"""

    file: Path
    analysis: dict


INDEX_TEMPLATE = """<!DOCTYPE html>
<html>
  <head>
    <title>Custom Axivion Reports</title>
  </head>
  <body>
    <ul>
      <li><a href="files.html">Combined report</a></li>
@REPORTS@
    </ul>
  </body>
</html>
"""


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


# pylint: disable-next=too-many-locals
def combine_analysis_reports(reports: List[Analysis]) -> int:
    """Creates a custom HTML report based on the JSON analysis report.
    It further creates one HTML report for all builds that combines all
    violations."""
    all_errors = []
    header = ("Location", "Violation", "Entity", "Message", "Entity")
    for report in reports:
        errors = []
        for violation in report.analysis:
            if not violation.get("isViolation", True):
                continue
            if not violation.get("suppressed", False):
                continue
            file_path, line, column = get_location(violation)
            error_number = violation.get("errorNumber", "Unknown")
            if error_number == "Architecture-ArchitectureCheck":
                entity = (
                    violation.get("architectureSourceLinkName", "")
                    + " --> "
                    + violation.get("architectureTargetLinkName", "")
                )
            else:
                entity = violation.get("entity", None)
            message = violation.get("message", None)
            errors.append((file_path, line, column, error_number, entity, message))

        sorted_errors = sorted(errors, key=lambda x: (x[0], x[1]))
        formatted_errors = []
        for file_path, line, column, error_number, entity, message in sorted_errors:
            location = f"{file_path}:{line}:{column}"
            entry = (location, entity, error_number, message)
            formatted_errors.append(entry)
            if entry not in all_errors:
                all_errors.append(entry)

        with open(f"files_{report.file.stem}.html", "w", encoding="utf-8") as f:
            out_txt = tabulate.tabulate(
                formatted_errors, tablefmt="html", showindex=True, headers=header
            )
            f.write(out_txt)
    with open("files.html", "w", encoding="utf-8") as f:
        out_txt = tabulate.tabulate(
            all_errors, tablefmt="html", showindex=True, headers=header
        )
        f.write(out_txt)
    with open("index.html", "w", encoding="utf-8") as f:
        links = ""
        for i in reports:
            link_name = i.file.stem.replace("foxbms_", "").replace("_spa_report", "")
            link_name = " : ".join(link_name.split("_"))
            links += f'      <li><a href="files_{i.file.stem}.html">{link_name}</li>\n'
        index_txt = INDEX_TEMPLATE.replace("@REPORTS@", links.rstrip())
        f.write(index_txt)
    return 0


def main():
    """TODO"""
    parser = argparse.ArgumentParser(
        description="""This script combines several analysis reports to a single report.""",
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
        "report_files",
        action="store",
        default=None,
        nargs="+",
        type=Path,
        help="Axivion report file(s)",
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
    for report in args.report_files:
        if not report.is_file():
            valid = False
            logging.error("report: File '%s' does not exist.", report)
    if not valid:
        sys.exit("Invalid script arguments.")

    reports = [
        Analysis(i, json.loads(i.read_text(encoding="utf-8")))
        for i in args.report_files
    ]
    logging.info(
        "Successfully loaded JSON report files:\n%s",
        os.linesep.join(str(i) for i in args.report_files),
    )

    err = combine_analysis_reports(reports)
    sys.exit(err)


if __name__ == "__main__":
    main()
