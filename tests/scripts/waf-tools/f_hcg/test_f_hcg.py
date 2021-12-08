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

"""Implements tests for the waf tool ``f_hcg``.
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


try:
    import f_hcg
except ImportError:
    sys.path.extend(
        [
            os.path.join(ROOT, "tools", "waf-tools"),
            os.path.join(ROOT, "tools", "waf3-2.0.22-1241519b19b496207abef1f72bbf61c2"),
        ]
    )
    import f_hcg


class TestHcgMethods(unittest.TestCase):
    """Unit test class for the ``f_hcg`` tool"""

    def test_hcg(self):
        """Checks that generated sources, headers and have-to-be-removed  files
        from a HALCoGen configuration file are the same as the ones that are
        expected."""
        script_dir = os.path.join(ROOT, "tests", "scripts", "waf-tools", "f_hcg")
        nr_of_tests = 4
        for i in range(nr_of_tests):
            with open(
                os.path.join(script_dir, "tests", f"test_hcg{i}.json"),
                "r",
                encoding="utf-8",
            ) as hcg_file:
                valid = json.load(hcg_file)
            hcg_configuration = f_hcg.NodeStructure(
                os.path.join(script_dir, "tests", f"test_hcg{i}.hcg")
            )
            hcg_configuration.parse_xml()
            self.assertEqual(hcg_configuration.sources, valid["sources"])
            self.assertEqual(hcg_configuration.headers, valid["headers"])
            self.assertEqual(hcg_configuration.removes, valid["removes"])

        with open(
            os.path.join(script_dir, "tests", "test_hcg_raises.json"),
            "r",
            encoding="utf-8",
        ) as hcg_file:
            valid = json.load(hcg_file)
        hcg_configuration = f_hcg.NodeStructure(
            os.path.join(script_dir, "tests", "test_hcg_raises.hcg")
        )
        with self.assertRaises(BaseException):
            hcg_configuration.parse_xml()


if __name__ == "__main__":
    unittest.main()
