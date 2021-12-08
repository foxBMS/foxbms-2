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

"""Basic test of all packages in conda environment"""
import sys
import os
import logging
import argparse
import glob
import json
import pathlib
import subprocess
import shutil

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))


def main():
    """This script tests our conda environment"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-f",
        "--file",
        dest="file",
        action="store",
        default=os.path.join(SCRIPT_DIR, f"conda_env_{sys.platform}.json"),
        help="Specify environment file to test against",
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

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    env_file = pathlib.Path(args.file)
    logging.info(env_file)
    env_config = json.loads(env_file.read_text(encoding="utf-8"))
    logging.debug(env_config)
    conda = shutil.which("conda")
    logging.info(f"Conda instance at: {conda}")
    if not conda:
        sys.exit("Could not find conda binary.")
    cmd = [conda, "list", "--json"]
    # pylint: disable=consider-using-with
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    current_config = json.loads(p.communicate()[0])
    if not current_config == env_config:
        raise BaseException("Environments differ.")
    test_scripts = []
    for i in env_config:
        tmp = os.path.join(SCRIPT_DIR, "packages", f"env_test_{i['name']}.py")
        if not os.path.isfile(tmp):
            tmp = os.path.join(
                SCRIPT_DIR,
                "packages",
                f"env_test_{i['name']}_{sys.platform.lower()}.py",
            )
        if not os.path.isfile(tmp):
            raise FileNotFoundError(f"Could not find expected test file'{tmp}'.")
        test_scripts.append(tmp)
    packages = pathlib.Path(os.path.join(SCRIPT_DIR, "packages")).as_posix()
    globbed_test_scripts = glob.glob(packages + "/env_test_**.py")
    fixed_globbed_test_scripts = []
    for i in globbed_test_scripts:
        if sys.platform.lower() == "linux":
            if not i.endswith("_win32.py"):
                fixed_globbed_test_scripts.append(i)
            else:
                continue
        elif sys.platform.lower() == "win32":
            if not i.endswith("_linux.py"):
                fixed_globbed_test_scripts.append(i)
            else:
                continue
        else:
            sys.exit("Unsupported platform.")
        logging.debug(f"Added package '{i}.")

    stemmed_globbed_test_scripts = set(
        map(lambda x: pathlib.Path(x).stem, fixed_globbed_test_scripts)
    )
    stemmed_test_scripts = set(map(lambda x: pathlib.Path(x).stem, test_scripts))
    difference = stemmed_globbed_test_scripts.difference(stemmed_test_scripts)
    if difference:
        logging.error(
            f"Difference in env test python scripts detected: {sorted(difference)}"
        )
        sys.exit("Exiting.")
    cmd_list = zip([sys.executable] * len(test_scripts), test_scripts)
    test_processes = []
    for i in cmd_list:
        # pylint: disable=consider-using-with
        proc = subprocess.Popen(i, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        test_processes.append(proc)

    for i in test_processes:
        i.wait()
    exit_code = 0
    for i in test_processes:
        if i.returncode:
            print(i.returncode, i.args, i.stderr.read().decode("utf-8"))
            exit_code += 1

    sys.exit(exit_code)


if __name__ == "__main__":
    main()
