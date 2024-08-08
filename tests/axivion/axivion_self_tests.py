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

"""Template for Python scripts"""

import argparse
import json
import logging
import os
import shutil
import subprocess
import sys
from copy import deepcopy
from pathlib import Path

HERE = Path(__file__).parent
ROOT = HERE / "../.."

TEST_CONFIGS = HERE / f"{Path(__file__).stem}.json"


class TestCaseErrors:  # pylint: disable=too-few-public-methods
    """Axivion test case"""

    def __init__(self) -> None:
        self.error_level: int = 0
        self.details = []

    def add(self, test_case, error_level):
        """add a test case and increase the error level if it failed
        :param test_case: test case to be added
        :param error_level: error level of that test case"""
        logging.error("%s: %s", test_case, error_level)
        self.details.append((test_case, error_level))
        if error_level:
            self.error_level += 1


def set_logging_level(verbosity: int = 1) -> None:
    """sets the module logging level

    :param verbosity: verbosity level"""
    logging_levels = {
        0: logging.CRITICAL,
        1: logging.ERROR,
        2: logging.WARNING,
        3: logging.INFO,
        4: logging.DEBUG,
    }
    level = logging_levels[min(verbosity, max(logging_levels.keys()))]
    logging.basicConfig(
        level=level,
        format=f"%(asctime)s File: {__file__}:%(lineno)d %(levelname)-8s %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    logging.debug("Logging level: %s", level)


def check_for_run(only: dict):
    """Check if environment variables indicate that a test shall run"""
    if not only.get("env", {}):
        sys.exit("key 'env' must be defined.")
    for k, v in only["env"].items():
        var = os.environ.get(k)
        if not var:
            continue
        if str(v) in str(var).lower():
            return True
    return False


def main():  # pylint: disable=too-many-locals
    """Run Axivion self tests"""
    parser = argparse.ArgumentParser()
    try:
        choices = list(json.loads(TEST_CONFIGS.read_text(encoding="utf-8")).keys())
    except json.JSONDecodeError:
        sys.exit(f"Could not load test cases from {TEST_CONFIGS.absolute()}")
    all_choices = choices + ["all"]

    parser.add_argument(
        "--test-case",
        dest="test_case",
        required=True,
        choices=all_choices,
        action="append",
        help="Test case to run.",
    )
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    args = parser.parse_args()

    set_logging_level(args.verbosity)
    if "all" in args.test_case:
        logging.debug("Testing all test cases: %s", ", ".join(choices))
        args.test_case.remove("all")
        args.test_case = choices

    perform_tests_exe_name = "perform_tests"
    perform_tests = shutil.which(perform_tests_exe_name)
    if not perform_tests:
        search_path = ""
        paths = (ROOT / "conf/env/paths_win32.txt").read_text(encoding="utf-8")
        for path in paths:
            if "bauhaus" in path.lower():
                search_path = path
                break
        if not search_path:
            sys.exit(f"Could not find {perform_tests_exe_name}.")
        perform_tests = shutil.which(perform_tests_exe_name, path=search_path)
    if not perform_tests:
        sys.exit(f"Could not find {perform_tests_exe_name}.")
    logging.debug("Using %s", perform_tests)

    txt = TEST_CONFIGS.read_text(encoding="utf-8")
    txt = txt.replace("${HERE}", HERE.as_posix())

    err = TestCaseErrors()
    config: dict = json.loads(txt)

    for i in args.test_case:
        logging.debug("Test configuration:\n%s", json.dumps(config[i], indent=2))
        cmd = [perform_tests] + config[i].get("runner-arguments", [])
        logging.debug("cmd: %s", " ".join(cmd))

        cwd = config[i].get("cwd", HERE)
        logging.debug("cwd: %s", cwd)

        env = deepcopy(os.environ)
        for var, value in config[i].get("env", {}).items():
            env[var] = value
            logging.debug("var: %s=%s", var, value)

        for j in config[i].get("pre-build", []):
            action = j["action"]
            logging.debug("Running pre-build step...")
            if action == "copy":
                logging.debug("Copy '%s' to '%s'.", j["source"], j["dest"])
                shutil.copyfile(j["source"], j["dest"])

        needs_to_run = True
        if only := config[i].get("only", False):
            needs_to_run = check_for_run(only)

        if needs_to_run:
            logging.info("Running test '%s'", i)
            with subprocess.Popen(cmd, cwd=cwd, env=env) as p:
                p.communicate()
            err.add(i, p.returncode)
        else:
            logging.warning("Skipped test '%s'", i)
            err.add(i, 0)

    if err.error_level:
        logging.error("\nThe following test cases failed:\n----")
        for i in err.details:
            logging.error("%s: %s", i[0], i[1])
    sys.exit(err.error_level)


if __name__ == "__main__":
    main()
