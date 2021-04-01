#!/usr/bin/env python
# -*- coding: utf-8 -*-

# @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
#   angewandten Forschung e.V. All rights reserved.
#
# BSD 3-Clause License
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1.  Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
# 3.  Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# &Prime;This product uses parts of foxBMS&reg;&Prime;
#
# &Prime;This product includes parts of foxBMS&reg;&Prime;
#
# &Prime;This product is derived from foxBMS&reg;&Prime;

"""Template for Python scripts"""
import sys
import os
import logging
import argparse
import json
import pathlib
import subprocess
import shutil


def main():
    """This script does this and that"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-f",
        "--file",
        dest="file",
        action="store",
        default="env_win32.json",
        help="Specify environemt file to test against",
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
    env_config = json.loads(env_file.read_text())
    logging.debug(env_config)
    conda = shutil.which("conda")
    if not conda:
        sys.exit("Could not conda binary.")
    cmd = [conda, "list", "--json"]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    current_config = json.loads(p.communicate()[0])
    if not current_config == env_config:
        raise BaseException("Environments differ.")
    d = os.path.dirname(os.path.realpath(__file__))
    test_scripts = [
        os.path.join(d, "packages", f"env_test_{i['name']}.py") for i in env_config
    ]
    cmd_list = zip([sys.executable] * len(test_scripts), test_scripts)
    test_processes = []
    for i in cmd_list:
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
