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

"""Implements tests for the waf tool ``f_ti_arm_cgt``.
"""

import unittest
import sys
import os
import json

HAVE_GIT = False
try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    pass


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of test_f_guidelines

    Returns:
        root (string): root path of the git repository
    """
    root = os.path.join(os.path.dirname(path), "..", "..", "..", "..")
    try:
        repo = Repo(path, search_parent_directories=True)
        root = repo.git.rev_parse("--show-toplevel")
    except InvalidGitRepositoryError:
        pass
    return root


ROOT = get_git_root(os.path.realpath(__file__))
SCRIPT_PATH = os.path.abspath(os.path.dirname(__file__))
TEST_FILES_PATH = os.path.join(SCRIPT_PATH, "tests")


def get_txt(f):
    """returns the content of file"""
    txt = None
    with open(os.path.join(TEST_FILES_PATH, f), "r", encoding="utf-8") as text_file:
        txt = text_file.read()
    return txt


try:
    import f_ti_arm_cgt
except ImportError:
    sys.path.extend(
        [
            os.path.join(ROOT, "tools", "waf-tools"),
            os.path.join(ROOT, "tools", "waf3-2.0.22-1241519b19b496207abef1f72bbf61c2"),
        ]
    )
    import f_ti_arm_cgt


class TestArmcl(unittest.TestCase):
    """Unit test class for the ``f_ti_arm_cgt`` tool"""

    def test_linkercheck(self):
        """Unit test for valid linker symbols

        test_linkercheck uses python unittest to check the return value of the
        f_ti_arm_cgt cprogram parse_output function.

        """
        result_path = os.path.join(TEST_FILES_PATH, "f_ti_arm_cgt_result.json")
        with open(result_path, encoding="utf-8") as file:
            results = json.loads(file.read())
        for key, output in results.items():
            test_text = get_txt(key)
            linker_output_path = key.split(".")[0] + ".txt"
            linker_output_text = get_txt(linker_output_path)
            self.assertEqual(
                (output[0], output[1]),
                f_ti_arm_cgt.cprogram.parse_output(
                    test_text, "test_path", linker_output_text
                ),
            )


if __name__ == "__main__":
    unittest.main()
