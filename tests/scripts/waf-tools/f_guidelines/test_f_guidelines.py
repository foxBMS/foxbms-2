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

"""Implements tests for the waf tool ``f_guidelines``.
"""

import unittest
import sys
import os
import re
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
    if HAVE_GIT:
        try:
            repo = Repo(path, search_parent_directories=True)
            root = repo.git.rev_parse("--show-toplevel")
        except InvalidGitRepositoryError:
            pass
    return root


ROOT = get_git_root(os.path.realpath(__file__))
SCRIPT_PATH = os.path.abspath(os.path.dirname(__file__))
TEST_FILES_PATH = os.path.join(SCRIPT_PATH, "tests")

try:
    import f_guidelines
    from f_guidelines import GuidelineViolations
except ImportError:
    sys.path.extend(
        [
            os.path.join(ROOT, "tools", "waf-tools"),
            os.path.join(ROOT, "tools", "waf3-2.0.22-1241519b19b496207abef1f72bbf61c2"),
        ]
    )
    import f_guidelines
    from f_guidelines import GuidelineViolations

rules_file = os.path.join(ROOT, "conf", "guidelines", "rules.json")
with open(rules_file, encoding="utf-8") as open_file:
    rules = json.loads(open_file.read())


def get_results(tests_folder, result_json_file):
    """returns result json

    Args:
        tests_folder (string): folder name of the test
    """
    result_path = os.path.join(TEST_FILES_PATH, tests_folder, result_json_file)
    with open(result_path, encoding="utf-8") as file:
        results = json.loads(file.read())
        return results


def get_txt(f):
    """returns the content of file"""
    txt = None
    with open(
        os.path.join(TEST_FILES_PATH, f),
        "r",
        newline=os.linesep,
        encoding="utf-8",
    ) as text_file:
        txt = text_file.read()
    return txt


class TestGuidelineMethods(unittest.TestCase):
    """Unit test class for the ``f_guidelines`` tool"""

    def general_tester(self, result, function_result):
        """function for reiterative tests

        Args:
            result (list): list containing the defined results of one test
            function_result (list): list containing the results from the called
                                   test function

        """

        if not result:
            self.assertEqual(result, function_result)
        else:
            output = []
            for res in result:
                if len(res) == 2:
                    output.append((GuidelineViolations[res[0]], res[1]))
                if len(res) == 3:
                    output.append((GuidelineViolations[res[0]], res[1], res[2]))
            self.assertEqual(output, function_result)

    def test_filenames(self):
        """Unit test for valid filenames

        test_filenames uses python unittest to check the return value of the
        f_guidelines filename test function with bad and good filenames.

        """
        bad_names = ["BAD_NAME", "Bad%name", "bad name"]
        regex = re.compile(rules["file_names"]["regex"])
        for i in bad_names:
            self.assertEqual(
                GuidelineViolations.GENERAL_FORBIDDEN_FILENAME,
                f_guidelines.filenames.test(i, regex),
            )
        good_names = ["good_name", "good-name"]
        for i in good_names:
            self.assertEqual(
                GuidelineViolations.NO_VIOLATION,
                f_guidelines.filenames.test(i, regex),
            )

    def test_posix_3_206(self):
        """Unit test for empty line at end of text

        test_posix_3_206 uses python unittest to check the return value of the
        f_guidelines posix_3_206 test function. Only one empty line is allowed
        at the end of each text.

        """

        results = get_results("general_tests", "general_test_results.json")[
            "test_rule"
        ][1]
        for test_file, result in results.items():
            txt = get_txt(os.path.join("general_tests", test_file))
            self.assertEqual(
                GuidelineViolations[result[0]],
                f_guidelines.posix_3_206.test(txt),
            )

    def test_trailing_whitespace(self):
        """Unit test for no trailing whitespace in lines of text

        test_trailing_whitespace uses python unittest to check the return value
        of the f_guidelines trailing_whitespace test function. There should be no
        trailing whitespace at the end of a line.

        """

        results = get_results("general_tests", "general_test_results.json")[
            "test_rule"
        ][2]
        for test_file, result in results.items():
            txt = get_txt(os.path.join("general_tests", test_file))
            self.general_tester(result, f_guidelines.trailing_whitespace.test(txt))

    def test_tabs(self):
        """Unit test for no tabs in lines of text

        test_tabs uses python unittest to check the return value
        of the tabs test function. Tabs are not allowed. Whitespace is used
        for indentation.

        """

        results = get_results("general_tests", "general_test_results.json")[
            "test_rule"
        ][3]

        for test_file, result in results.items():
            txt = get_txt(os.path.join("general_tests", test_file))
            self.general_tester(result, f_guidelines.tabs.test(txt))

    def test_unique_filename(self):
        """Unit test for unique filenames

        test_unique_filename uses python unittest to check the return value
        of the f_guidelines unique_filenames test function. In the repository
        a filename can not exit more than one time.

        """

        self.assertEqual(
            (GuidelineViolations.NO_VIOLATION, []),
            f_guidelines.unique_filenames.test(["testE234aQ.py", "testE23haQ.py"]),
        )
        self.assertEqual(
            (
                (GuidelineViolations.GENERAL_NOT_UNIQUE_FILENAME),
                ["test_f_guidelines.py"],
            ),
            f_guidelines.unique_filenames.test(
                ["test_f_guidelines.py", "test_f_guidelines.py"]
            ),
        )

    def test_encoding(self):
        """test guideline-check that files MUST be UTF-8"""

        results = get_results("general_tests", "general_test_results.json")[
            "test_rule"
        ][0]
        rule_name = "encoding"
        for test_file, result in results.items():
            test_file_path = os.path.join(TEST_FILES_PATH, "general_tests", test_file)
            # result[1][0] is NO_VIOLATION or GENERAL_ENCODING
            # result[0] is utf-8 or ascii
            self.assertEqual(
                GuidelineViolations[result[1][0]],
                f_guidelines.encoding.test(test_file_path, rule_name, result[0])[0],
            )

    def test_c_define_guard(self):
        """checks the define guard"""

        guard_config = rules["languages"]["C"]["define_guard"]
        results = get_results("c-005_tests", "c-005_test_results.json")
        for test_file, result in results.items():
            txt = get_txt(os.path.join("c-005_tests", test_file))
            self.general_tester(
                result,
                f_guidelines.c_check_define_guard.test(test_file, txt, guard_config),
            )

    def test_header(self):
        """Unit test to check the header

        test_header uses python unittest to check the return value
        of the f_guidelines check_header test function. Depending of the file type
        the file has to start with a specific header.

        """

        python_header = rules["languages"]["Python"]["header"]["text"]
        results = get_results("header_tests", "header_test_results.json")
        for test_file, result in results.items():
            test_header = get_txt(os.path.join("header_tests", test_file))
            # First line is skipped because file should start with header not
            # with comment
            test_header = os.linesep.join(test_header.splitlines()[1:]) + os.linesep
            self.general_tester(
                result, f_guidelines.check_header.test(test_header, python_header)
            )

    def test_doxygen(self):
        """Unit test for FILE LEVEL doxygen comment style in c and h files

        test_doxygen uses python unittest to check the return value
        of the f_guidelines c_check_doxygen test function. There has to be
        a doxygen comment in a specific style.

        """
        config = rules["languages"]["C"]["doxygen"]["regex"]
        results = get_results("c-004_tests", "c-004_test_results.json")
        for test_file, result in results["default_regex"].items():
            txt = get_txt(os.path.join("c-004_tests", test_file))
            self.general_tester(
                result, f_guidelines.c_check_doxygen.test(test_file, txt, config)
            )

        # Tests for changed doxygen regular expressions
        config = get_results("c-004_tests", "c-004_test_configured_rules.json")
        config = config["regex"]
        for test_file, result in results["changed_regex"].items():
            txt = get_txt(os.path.join("c-004_tests", test_file))
            self.general_tester(
                result, f_guidelines.c_check_doxygen.test(test_file, txt, config)
            )

    def test_c_sections(self):
        """Unit test for c section check in c and h files

        test_c_sections uses python unittest to check the return value
        of the f_guidelines c_check_sections test function. There have to be
        section comments in the right order for c and h files.

        """
        c_section_rules = rules["languages"]["C"]["sections"]
        section_strings = []
        section_strings.append(c_section_rules["source"]["sections"])
        results = get_results("c-006_tests", "c-006_test_results.json")
        for test_file, result in results.items():
            txt = get_txt(os.path.join("c-006_tests", test_file))
            if not result:
                self.assertEqual(
                    result,
                    f_guidelines.c_check_sections.test(txt, section_strings[0]),
                )
            else:
                output = []
                for res_list in result:
                    output.append(
                        (
                            GuidelineViolations[res_list[0]],
                            res_list[1],
                            "".join(res_list[2]),
                        )
                    )
                self.assertEqual(
                    output,
                    f_guidelines.c_check_sections.test(txt, section_strings[0]),
                )

    def test_c_comment_style(self):
        """Unit test for c comment style check in c and h files

        test_c_comment_style uses python unittest to check the return value
        of the f_guidelines c_check_comment_style test function. There are forbidden
        comment styles which has to be checked

        """
        c_comment_style_regex = re.compile(r"(?<!:)//\s{0,}\S+")
        results = get_results("c-029_tests", "c-029_test_results.json")
        for test_file, result in results.items():
            txt = get_txt(os.path.join("c-029_tests", test_file))
            self.assertEqual(
                result,
                f_guidelines.c_check_comment_style.test(txt, c_comment_style_regex),
            )

    def test_rst_macro(self):
        """Unit test for macro including check in rst files

        test_rst_macro uses python unittest to check the return value
        of the f_guidelines rst_check_include test function. Each rst files has to
        include macros.txt

        """

        regex_string = rules["languages"]["reStructuredText"]["include"][
            "include_files"
        ][0]["macros.txt"]["regex"]
        macro_regex = re.compile(regex_string)
        results = get_results("rst-003_tests", "rst-003_test_results.json")
        for test_file, result in results.items():
            txt = get_txt(os.path.join("rst-003_tests", test_file))
            self.assertEqual(
                GuidelineViolations[result],
                f_guidelines.rst_check_include.test(txt, macro_regex),
            )

    def test_rst_orphan(self):
        """Unit test for macro including check in rst files

        test_rst_orphan uses python unittest to check the return value
        of the f_guidelines rst_check_orphan test function. rst files which are
        not included in other rst files must have :orphan: in the first line

        """
        self.assertEqual(
            GuidelineViolations.NO_VIOLATION,
            f_guidelines.rst_check_orphan.test(":orphan:"),
        )
        self.assertEqual(
            GuidelineViolations.RST_ORPHAN,
            f_guidelines.rst_check_orphan.test("no orphan"),
        )

    def test_rst_heading(self):
        """Unit test for macro including check in rst files

        test_rst_heading uses python unittest to check the return value
        of the f_guidelines rst_check_heading test function. Each rst files must
        have a heading

        """
        heading_regex = []
        heading_regex.append(re.compile(r"\.\. _(\S+):"))
        heading_regex.append(re.compile(r"[=#*^\-\"]+"))
        heading_regex.append(re.compile(r"[#*]+"))
        results = get_results("rst-007_tests", "rst-007_test_results.json")
        for test_file, result in results.items():
            txt = get_txt(os.path.join("rst-007_tests", test_file))
            if not result:
                self.assertEqual(
                    result, f_guidelines.rst_check_heading.test(txt, heading_regex)
                )
            else:
                output = []
                for res in result:
                    output.append((res[0], res[1]))
                self.assertEqual(
                    output, f_guidelines.rst_check_heading.test(txt, heading_regex)
                )


def main():
    """main function"""
    unittest.main()


if __name__ == "__main__":
    main()
