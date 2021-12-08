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

"""Test regular expressions that are used to validate function, variable etc.
names in the Axivion analysis."""

import json
import os
import re
import sys
import unittest
from pathlib import Path


class TestNamingConventionRegularExpressions(unittest.TestCase):
    """Unit tests for foxBMS regular expressions for names in the embedded code"""

    test_files_to_rules = {
        "function": "CodingStyle-Naming.Function",
        "global-variable": "CodingStyle-Naming.GlobalVariable",
        "local-variable": "CodingStyle-Naming.LocalVariable",
        "macro": "CodingStyle-Naming.Macro",
        "typedefed-enum": "CodingStyle-Naming.TypedefedEnum",
        "typedefed-struct": "CodingStyle-Naming.TypedefedStruct",
    }

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.script_root = Path(os.path.abspath(os.path.dirname(__file__)))
        self.axivion_dir = (
            Path(os.path.abspath(os.path.dirname(__file__))).parent / "axivion"
        )
        self.axivion_config = self._load_ax_config()

    def _load_ax_config(self) -> dict:
        """validates and returns the Axivion configuration as dict."""
        try:
            ax_config = (self.axivion_dir / "rule_config_names.json").read_text()
        except FileNotFoundError:
            sys.exit("Could not find Axivion configuration.")
        try:
            tmp = json.loads(ax_config)
        except json.decoder.JSONDecodeError:
            sys.exit("Provided Axivion configuration is not a valid json file.")
        assert tmp["_Format"] == "1.0", "Expected format version '1.0'."

        if not tmp.get("Analysis", None):
            sys.exit("Could not find 'Analysis section'.")

        return tmp["Analysis"]

    def _load_test(self, test_name: str) -> dict:
        """returns the test as dict."""
        test_config = (self.script_root / test_name).read_text()
        return json.loads(test_config)

    def _run_test(self, test, test_config, test_re):
        """Checks the regex against the matching and not-matching test cases."""
        # Axivion uses 'fullmatch', therefore we need to test it the same way
        msg = (
            f"Checking that '{self.test_files_to_rules[test.stem]}: "
            f"'{test_re.pattern}' DOES %s match '%s'."
        )
        for i in test_config["valid"]:
            m = test_re.fullmatch(i)
            found = False
            if m:
                found = True
            with self.subTest(msg=" ".join((msg % ("", i)).split())):
                self.assertEqual(found, True)
        for i in test_config["invalid"]:
            m = test_re.fullmatch(i)
            found = False
            if m:
                found = True
            with self.subTest(msg=msg % ("NOT", i)):
                self.assertEqual(found, False)

    def test_naming_regexes(self):
        """Tests for the naming rules regular expressions."""
        for i in self.script_root.glob("*.json"):
            test_config = self._load_test(i.resolve())
            rule = self.test_files_to_rules[i.stem]
            test_re = re.compile(self.axivion_config[rule]["naming_convention"])
            self._run_test(i, test_config, test_re)


def main():
    """Gets the test and runs them through python's unit test module."""
    unittest.main()


if __name__ == "__main__":
    main()
