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

"""Implements a waf tool to check for foxBMS project guidelines"""

import os
import re
import pathlib
import collections
import datetime
import json
from codecs import BOM_UTF8, BOM_UTF16_BE, BOM_UTF16_LE, BOM_UTF32_BE, BOM_UTF32_LE
from enum import Enum

from waflib import Context, Task, TaskGen, Logs
from waflib.Build import BuildContext

# pylint: disable-msg=invalid-name
# pylint: disable=no-member

TOOLDIR = os.path.dirname(os.path.realpath(__file__))


class AutoNumberGuidelineErrors(Enum):
    """Auto numbering Enum

    Inherits from enum and provides an automatic numeration for all its
    members
    """

    def __new__(cls):
        """overridden __new__ class method

        Each time a member in an AutoNumberGuidelineErrors enum is assigned,
        the member value is set to the number of actual members. The counting
        starts with 0.
        """
        value = len(cls.__members__)
        obj = object.__new__(cls)
        obj._value_ = value
        return obj


class GuidelineViolations(AutoNumberGuidelineErrors):
    """Assign numbers to our error codes

    GuidelineViolations inherits from AutoNumberGuidelineErrors and numbers
    are automatically set for each error code. The counting starts with 0 for
    NO_VIOLATION.
    """

    NO_VIOLATION = ()
    GENERAL_FORBIDDEN_FILENAME = ()
    GENERAL_NOT_UNIQUE_FILENAME = ()
    GENERAL_EOF_NO_EMPTY_NEWLINE = ()
    GENERAL_EOF_TOO_MANY_EMPTY_NEWLINES = ()
    GENERAL_TRAILING_WHITESPACE = ()
    GENERAL_TABS = ()
    GENERAL_ENCODING = ()
    HEADER = ()
    C_GUARD = ()
    C_DOXYGEN = ()
    C_SECTION = ()
    C_COMMENT_STYLE = ()
    RST_MISSING_INCLUDE = ()
    RST_ORPHAN = ()
    RST_HEADING = ()


def options(opt):
    """Defines options that can be passed to waf"""
    opt.add_option(
        "--commit-year",
        action="store",
        default=datetime.datetime.now().year,
        dest="COMMIT_YEAR",
        help="Date to be checked",
    )
    opt.load("f_black", tooldir=TOOLDIR)
    opt.load("f_clang_format", tooldir=TOOLDIR)
    opt.load("f_pylint", tooldir=TOOLDIR)


def configure(conf):
    "Configures all sub-tools the guidelines tool needs"
    # black
    conf.load("f_black", tooldir=TOOLDIR)
    conf.env.BLACK_CONFIG = [
        conf.path.find_node(os.path.join("conf", "fmt", "pyproject.toml")).abspath()
    ]
    if not conf.env.BLACK_OPTIONS:
        conf.env.BLACK_OPTIONS = ["--config", conf.env.BLACK_CONFIG[0], "--quiet"]
    # clang-format
    conf.load("f_clang_format", tooldir=TOOLDIR)
    if not conf.env.CLANG_FORMAT_OPTIONS:
        conf.env.CLANG_FORMAT_OPTIONS = ["-i", "-style=file"]
    # pylint
    conf.load("f_pylint", tooldir=TOOLDIR)
    conf.env.PYLINT_CONFIG = [
        conf.path.find_node(os.path.join("conf", "spa", ".pylintrc")).abspath()
    ]
    if not conf.env.PYLINT_OPTIONS:
        conf.env.PYLINT_OPTIONS = [f"--rcfile={conf.env.PYLINT_CONFIG[0]}"]


class filenames(Task.Task):
    """Class to implement the filenames check

    A task is only executed if the filename of a file or the regular expression for
    correct filenames is changed.

    """

    # color (string): color in which the command line is displayed in the terminal
    color = "BLUE"

    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_GEN_001"]

    def run(self):
        """calls test function and prints the error message

        A list (in self.input) containing the name of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. Depending on
        the return value of the test function, an error message
        with incorrect filename is printed.

        Returns:
            int:
            returns 0 for no filename rule violation otherwise the value
            of GuidelineViolations.GENERAL_FORBIDDEN_FILENAME
        """

        error = self.test(self.inputs[0].name, self.regex)

        if error != GuidelineViolations.NO_VIOLATION:
            Logs.error(
                f"{self.rule_name}: File/directory '{self.inputs[0].abspath()}' "
                f"does not match to ({self.regex})."
            )
        return error.value

    @staticmethod
    def test(filename, reg):
        """Implements check that filename does not include forbidden
        characters

        Args:
            filename (string): the name of the to be checked file
            reg (regex): a compiled regular expression for a correct filename

        Returns:
            GuidelineViolations:
            GENERAL_FORBIDDEN_FILENAME or NO_VIOLATION depending on the test result
        """
        error = GuidelineViolations.NO_VIOLATION
        if not reg.search(filename):
            error = GuidelineViolations.GENERAL_FORBIDDEN_FILENAME
        return error

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking filename ({self.rule_name})"


class unique_filenames(Task.Task):
    """Class to implement the filename uniqueness check

    A task is only executed if a filename of a file is changed.
    A filename is not allowed to occur twice or more in the repository.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_GEN_002"]

    def run(self):
        """calls test function and prints the error message

        A list (in self.input) containing the name of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes all filenames in the repository. Depending on
        the return value of the test function, an error message with all
        duplicates is printed.

        Returns:
            int:
            returns 0 for no unique filename rule violation otherwise the value
            of GuidelineViolations.GENERAL_NOT_UNIQUE_FILENAME
        """

        error, duplicates = self.test(
            [pathlib.Path(i.abspath()).name for i in self.inputs]
        )
        if error != GuidelineViolations.NO_VIOLATION:
            for i in duplicates:
                ith_dup = self.generator.path.ant_glob(f"**/{i}", quiet=True)
                Logs.error(
                    f"{self.rule_name}: The file {i} exists multiple times: {ith_dup}"
                )
        return error.value

    @staticmethod
    def test(all_filenames):
        """checks whether there are files with the same name in the repository

        The collections.Counter puts all filenames with their count
        in a dictionary. If the count is bigger than one, the filename is
        inserted into the list of duplicates.

        Args:
            all_filenames (list): list with all filenames in the repository

        Returns:
            GuidelineViolations:
            If the duplicates list is not empty a GuidelineViolations
            GENERAL_NOT_UNIQUE_FILENAME is returned with the duplicates list
        """

        error = GuidelineViolations.NO_VIOLATION
        duplicates = [
            i for i, count in collections.Counter(all_filenames).items() if count > 1
        ]
        if duplicates:
            error = GuidelineViolations.GENERAL_NOT_UNIQUE_FILENAME
        return error, duplicates

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking filename uniqueness ({self.rule_name})"


class encoding(Task.Task):
    """Class to implement the utf-8 coding check

    A task is only executed if a file is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # invalid (tuple): lists all forbidden BOM
    invalid = (BOM_UTF8, BOM_UTF16_BE, BOM_UTF16_LE, BOM_UTF32_BE, BOM_UTF32_LE)
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_GEN_003"]

    def run(self):
        """Implements check that a file can be opened in utf-8 mode and has
        no BOM markers

        A list (in self.input) containing the path of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes file path, rule name and the encoding style (utf-8).
        Depending on the return value of the test function, an error message is
        printed.

        Returns:
            int:
            returns 0 for no encoding violation otherwise the value
            of GuidelineViolations.GENERAL_ENCODING
        """

        error, error_message = self.test(
            self.inputs[0].abspath(),
            self.rule_name,
            self.encoding,
        )
        Logs.error(error_message)
        return error.value

    @staticmethod
    def test(file_path, rulename, encoding_type="utf-8"):
        """checks whether a file is correct encoded

        file in file_path is first opened and read with utf-8 encoding and
        afterwards checked for not allowed BOM

        Args:
            file_path (string): path to file
            encoding_type (string): expected file encoding (default: "utf8")

        Raises:
            UnicodeDecodeError: DecodeError is thrown if the file can not be read
                                with the specified encoding

        Returns:
            GuidelineViolations:
            If the file can be opened with the specified encoding and contains
            no BOM, GuidelineViolations.NO_VIOLATION is returned.
            Otherwise GuidelineViolations.GENERAL_ENCODING is returned along with
            a more detailed error message
        """
        err = GuidelineViolations.NO_VIOLATION
        err_msg = ""
        try:
            with open(file_path, "r", encoding=encoding_type) as f:
                f.read()
        # UnicodeDecodeError if text can not be decoded
        # LookupError if the encoding is not known
        except (UnicodeDecodeError, LookupError) as err:
            err_msg = f"{rulename}: File '{file_path}' "
            err_msg += f"can not be read with encoding {encoding_type}.\n"
            err_msg += f"{err}"
            err = GuidelineViolations.GENERAL_ENCODING
            return err, err_msg
        # BOM are only part of utf-8
        if encoding_type == "utf-8":
            with open(file_path, "r+b") as f:
                try:
                    txt = f.readlines()[0]
                except IndexError:
                    pass  # file is empty
                else:
                    if txt.startswith(encoding.invalid):
                        err_msg = f"{rulename}: File"
                        err_msg += f" '{file_path}' uses BOM markers"
                        err = GuidelineViolations.GENERAL_ENCODING
        return err, err_msg

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking encoding ({self.rule_name})"


class posix_3_206(Task.Task):
    """Class to implement the POSIX 3.206 check

    A task is only executed if a file or the check itself is changed.

    """

    # color (string): color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_GEN_004"]

    def run(self):
        """Implements check that the file contains one empty line at the end

        A list (in self.input) containing the text of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes the text of the to be checked file. Depending on
        the return value of the test function, an error message is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If a file contains
            no empty line or too many empty lines an error message the is
            printed and the value of
            GuidelineViolations.GENERAL_EOF_NO_EMPTY_NEWLINE or
            GuidelineViolations.GENERAL_EOF_TOO_MANY_EMPTY_NEWLINES is returned
        """

        error = self.test(self.inputs[0].read())
        if error != GuidelineViolations.NO_VIOLATION:
            base_error_msg = f"{self.rule_name}: File '{self.inputs[0].abspath()}'"
            if error == GuidelineViolations.GENERAL_EOF_NO_EMPTY_NEWLINE:
                Logs.error(
                    f"{base_error_msg} misses an empty line at the end of the file."
                )
            elif error == GuidelineViolations.GENERAL_EOF_TOO_MANY_EMPTY_NEWLINES:
                Logs.error(
                    f"{base_error_msg} adds unnecessary newlines line at the end of the file."
                )
        return error.value

    @staticmethod
    def test(txt):
        """Implements check that text ends with a single empty line

        Args:
            txt (string): contains text that should have only one single
                empty line at the end

        Returns:
            GuidelineViolations:
            GuidelineViolations.NO_VIOLATION if the text ends with an
            empty line, GuidelineViolations.GENERAL_EOF_NO_EMPTY_NEWLINE if a
            newline is missing at the end of the text or
            GuidelineViolations.GENERAL_EOF_TOO_MANY_EMPTY_NEWLINES if there is
            more than one empty line at the end of the text.
        """

        error = GuidelineViolations.NO_VIOLATION
        if not txt == "":
            if not txt.endswith(os.linesep):
                error = GuidelineViolations.GENERAL_EOF_NO_EMPTY_NEWLINE
            if txt and txt.splitlines()[-1] == "":
                error = GuidelineViolations.GENERAL_EOF_TOO_MANY_EMPTY_NEWLINES
        return error

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking POSIX 3.206 ({self.rule_name})"


class trailing_whitespace(Task.Task):
    """Class to implement the trailing whitespace check

    A task is only executed if a file is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_GEN_005"]

    def run(self):
        """calls test function and prints the line number with trailing whitespace

        A list (in self.input) containing the path of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes the text of the to be checked file. Depending
        on the return value of the test function, an error message is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If a line in the
            file contains whitespace at the end
            GuidelineViolations.GENERAL_TRAILING_WHITESPACE is returned
        """
        errors = self.test(self.inputs[0].read())

        if errors:
            for err in errors:
                Logs.error(
                    f"{self.rule_name}: File '{self.inputs[0].abspath()}' "
                    f"adds trailing whitespace in {self.inputs[0].abspath()}:{err[1]}"
                )
            return GuidelineViolations.GENERAL_TRAILING_WHITESPACE.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt):
        """checks line by line the text whether there are trailing whitespace

        Args:
            txt (string): text of the to be checked file

        Returns:
            list:
            A list of tuples with GuidelineViolations members and the line
            number containing trailing whitespace.
        """

        errors = []
        txt_list = txt.splitlines()
        for i, val in enumerate(txt_list):
            if val.endswith(" "):
                errors.append((GuidelineViolations.GENERAL_TRAILING_WHITESPACE, i + 1))
        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking trailing whitespace ({self.rule_name})"


class tabs(Task.Task):
    """Class to implement running the trailing whitespace check

    A task is only executed if a file is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_GEN_006"]

    def run(self):
        """calls test function and prints out the line with tabs in it

        A list (in self.input) containing the path of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes the text of the to be checked file. Depending
        on the return value of the test function, an error message is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If a line in the
            file contains tabs the value of GuidelineViolations.GENERAL_TABS is returned
        """

        errors = self.test(self.inputs[0].read())

        if errors:
            for err in errors:
                Logs.error(
                    f"{self.rule_name}: File {self.inputs[0].abspath()}:{err[1]} "
                    f"forbidden tabs found"
                )
            return GuidelineViolations.GENERAL_TABS.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt):
        """Implements check that lines in text do not have tabs

        Args:
            txt (string): text of the to be checked file

        Returns:
            list:
            A list of tuples with GuidelineViolations members and the line
            number containing tabs.
        """
        errors = []
        txt_list = txt.splitlines()
        for i, val in enumerate(txt_list):
            if "\t" in val:
                errors.append((GuidelineViolations.GENERAL_TABS, i + 1))
        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking tabs ({self.rule_name})"


class c_check_doxygen(Task.Task):
    """Class to implement the doxygen comment check

    A task is only executed if a file or the regular expression C_004_REGEX
    is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_C_004"]

    def run(self):
        """
        calls test function and prints out the line with a wrong file level
        doxygen comment

        A list (in self.input) containing the path of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes the filename, the text of the to be checked
        file and regular expression from rules.json. Depending on the return
        value of the test function, an error message is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If a line is not
            conform with the FILE LEVEL Doxygen rule the
            GuidelineViolations.C_DOXYGEN value is returned

        """
        errors = self.test(
            self.inputs[0].name,
            self.inputs[0].read(),
            self.regex,
        )
        if errors:
            for err in errors:
                Logs.error(
                    f"{self.rule_name}: In file: "
                    f"{self.inputs[0].abspath()}:{err[1]} {err[2]}"
                )
            return GuidelineViolations.C_DOXYGEN.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(filename, txt, regex):
        """Implements check that the file level doxygen comments are correct

        This function checks whether the FILE LEVEL Doxygen comment
        format is existent and in the correct order in the file. The first for
        loop compiles the needed regular expressions for the FILE LEVEL Doxygen
        comment style and the second loop goes line by line over the whole file.
        doxygen_regex_number is the current searched regular expression and
        doxygen_regex_found_line is the line of the match of the last searched
        regular expression. In the second loop the beginning of the C
        comment is searched first. A second occurrence of the C comment start sets
        doxygen_regex_found_line to the actual line, because not all C comments
        are FILE LEVEL Doxygen comments. The next if clause looks for the end
        of the Doxygen comment. At last it is checked whether all necessary
        comments exists in the right order.

        Args:
            filename (string): name of the actual file
            txt (string): text of the to be checked file
            config (dict): dictionary containing regular expressions

        Returns:
            list:
            A list of tuple with GuidelineViolations member and the error
            message
        """
        errors = []
        txt_list = txt.splitlines()
        doxygen_regex = []
        for i, regex_string in enumerate(regex):  # compile all doxygen regex
            regex_string = regex_string.replace("@FILENAME@", filename)
            compiled_regex = re.compile(regex_string)
            doxygen_regex.append(compiled_regex)

        doxygen_regex_number = 0  # which doxygen regex is searched at the moment
        doxygen_regex_found_line = -1  # in which line the last doxygen regex was found
        # Search the start of the File Level Doxygen
        for i, line in enumerate(txt_list):  # searching line by line
            if re.match(doxygen_regex[0], line):  # try to find \**
                if doxygen_regex_number == 0:
                    doxygen_regex_number = 1
                    doxygen_regex_found_line = i
                    continue
                if doxygen_regex_number == 1:
                    # If \* was found a second time found line is set to actual line
                    doxygen_regex_found_line = i
                    continue

            # Search end mark of File Level Doxygen
            if doxygen_regex_number == len(doxygen_regex) - 2:
                # if all doxygen regex were found, except the last two
                if re.match(doxygen_regex[len(doxygen_regex) - 1], line):
                    # if */ was found end search loop
                    break
                if re.match(doxygen_regex[len(doxygen_regex) - 2], line):
                    # if [ ]\\*.* was found jump into next loop step
                    continue
                errors.append(  # If the last two doxygen regex were not found
                    (
                        GuidelineViolations.C_DOXYGEN,
                        i + 1,
                        "The File Level Doxygen ends without end mark */",
                    )
                )
                break

            # Searching File Level Doxygen pattern between start and end mark
            if not doxygen_regex_number == 0:  # If at least \* was found
                if re.match(doxygen_regex[doxygen_regex_number], line):
                    # If actual search doxygen regex was found
                    if doxygen_regex_found_line == i - 1:
                        # Is the next doxygen regex found after the previous regex
                        doxygen_regex_number += 1
                        doxygen_regex_found_line = i
                        continue
                if (
                    doxygen_regex_number > 1
                ):  # If \* and @file were found but no other doxygen regex
                    regex_pattern = doxygen_regex[doxygen_regex_number].pattern
                    regex_pattern = re.findall(r"\(\D*\)", regex_pattern)[0][1:-1]
                    errors.append(
                        (
                            GuidelineViolations.C_DOXYGEN,
                            i + 1,
                            f"Line does not match with required File Level Doxygen "
                            f"{regex_pattern} comment",
                        )
                    )
                    doxygen_regex_number += 1
                    doxygen_regex_found_line = i

        if doxygen_regex_number == 1:
            errors.append(
                (
                    GuidelineViolations.C_DOXYGEN,
                    0,
                    f"@file {filename} was not found",
                )
            )
        if doxygen_regex_number == 0:
            errors.append(
                (
                    GuidelineViolations.C_DOXYGEN,
                    0,
                    r"The start \* of the doxygen header was not found",
                )
            )
        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking filelevel doxygen ({self.rule_name})"


class c_check_define_guard(Task.Task):
    """Class to implement define guard check

    A task is only executed if a file or the regular expression
    C_005_REGEX is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_C_005"]

    def run(self):
        """calls test function to search after the include guard

        A list (in self.input) containing the path and the name of the to be
        checked file is passed as first argument in the task generation. The
        second argument self.rule_name contains the name of the style guide rule.
        run calls the test function and passes the filename, the text of the
        to be checked file and regular expression from rules.json. Depending
        on the return value of the test function, an error message is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If a line in the
            file contains tabs GuidelineViolations.GUARD is returned
        """
        errors = self.test(
            self.inputs[0].name,
            self.inputs[0].read(),
            self.regex,
        )
        if errors:
            for err in errors:
                Logs.error(f"{self.rule_name}: {self.inputs[0].abspath()} {err[1]}")
            return GuidelineViolations.C_GUARD.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(filename, txt, config):
        """Implements checks that the include guard exists

        First it is searched after #ifndef, #define and #endif. Afterwards it
        is checked with the line number whether everything is at the correct
        position.

        Args:
            filename (string): name of the actual file
            txt (string): text of the to be checked file
            config (dict): dictionary containing regular expressions

        Returns:
            list:
            A list of tuple with GuidelineViolations member and the error
            message

        """
        stem = pathlib.Path(filename).stem
        errors = []
        found_ifndef_guard = (False, -2)
        found_define_guard = (False, -2)
        found_endif_guard = (False, -2)
        txt_list = txt.splitlines()
        prefix = config["prefix"]
        suffix = config["suffix"]
        infix = stem.upper()
        last_line_nr = 0
        for rep in config["replacements"]:
            for key, value in rep.items():
                infix = infix.replace(key, value)
        guard = f"{prefix}{infix}{suffix}"
        for i, line in enumerate(txt_list):
            last_line_nr = i
            if line == f"#ifndef {guard}":
                found_ifndef_guard = (True, i + 1)
            if line == f"#define {guard}":
                found_define_guard = (True, i + 1)
            if line == f"#endif /* {guard} */":
                found_endif_guard = (True, i + 1)

        if not found_ifndef_guard[0]:
            errors.append(
                (
                    GuidelineViolations.C_GUARD,
                    f"The file misses '#ifndef {guard}'",
                )
            )
        if not found_define_guard[0]:
            errors.append(
                (
                    GuidelineViolations.C_GUARD,
                    f"No '#define {guard}' found",
                )
            )
        else:
            if (
                found_ifndef_guard[0]
                and found_ifndef_guard[1] != found_define_guard[1] - 1
            ):
                errors.append(
                    (
                        GuidelineViolations.C_GUARD,
                        f"No '#define {guard}' found " "directly after ifndef",
                    )
                )

        if not found_endif_guard[0]:
            errors.append(
                (
                    GuidelineViolations.C_GUARD,
                    "No '#endif /* " f"{guard} */' found in the whole file",
                )
            )
        else:
            if (
                found_define_guard[0]
                and found_define_guard[1] > found_endif_guard[1] - 2
            ):
                errors.append(
                    (
                        GuidelineViolations.C_GUARD,
                        "'#endif /* "
                        f"{guard} */' not found at least two "
                        "lines after #define",
                    )
                )
            if found_endif_guard[1] != last_line_nr + 1:
                errors.append(
                    (GuidelineViolations.C_GUARD, "Found '#endif' is not the last line")
                )

        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking define guard ({self.rule_name})"


class c_check_sections(Task.Task):
    """Class to check existence of section comments

    A task is only executed if a file or the regular expression
    C_006_SECTION_STRING is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_C_006"]

    def run(self):
        """calls test function to search section comments

        A list (in self.input) containing the path of the to be checked file
        is passed as first argument in the task generation. The second argument
        self.rule_name contains the name of the style guide rule. run calls the
        test function and passes the text of the to be checked
        file and regular expression from rules.json. Depending on the return
        value of the test function, an error message is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If the sections are
            not correct the value of GuidelineViolations.C_SECTION is returned
        """

        errors = self.test(
            self.inputs[0].read(),
            self.section_strings,
        )
        if errors:
            for err in errors:
                Logs.error(f"In file: {self.inputs[0].abspath()}:{err[1]} {err[2]}")
            return GuidelineViolations.C_SECTION.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt, section_strings):
        """Implements check that all section comments exists in the right order

        This function checks whether the section comments are existent in the
        file. Furthermore it is checked whether the section comments are in the
        correct order which is checked by a current_section_searched.

        Args:
            txt (string): text of the to be checked file
            section_strings (list): list containing regular expressions for the
                               section comments

        Returns:
            list:
            A list with tuple out of a GuidelineViolations member and the error
            messages
        """
        found_section = []
        errors = []
        previous_line = ""
        for i, line in enumerate(txt.splitlines()):
            for j, comment_string in enumerate(section_strings):
                if comment_string == line:
                    found_section.append((j, i, previous_line))
            previous_line = line.rstrip(os.linesep)

        found_section_bool = [False] * len(section_strings)
        previous_section_index = -1

        for i, sec in enumerate(found_section):
            # sec[0] is the number of the section string
            # found_section_bool[sec[0]] can only be true if the section was
            # already found earlier
            if found_section_bool[sec[0]]:
                previous_section_index = sec[0]
                errors.append(
                    (
                        GuidelineViolations.C_SECTION,
                        sec[1] + 1,
                        f"{section_strings[sec[0]]} is a duplicate",
                    )
                )
            else:
                found_section_bool[sec[0]] = True
                # previous_section_index is only smaller if the actual section
                # index is at least 2 larger, therefore other sections are missing
                # between them
                if (
                    previous_section_index < (sec[0] - 1)
                    and previous_section_index != -1
                ):
                    errors.append(
                        (
                            GuidelineViolations.C_SECTION,
                            sec[1] + 1,
                            f"Between {section_strings[sec[0]]} and the previous "
                            f"section should be {sec[0]-1-previous_section_index}"
                            " other sections",
                        )
                    )
                # previous_section is only bigger if the actual section should
                # have come earlier
                if previous_section_index > (sec[0] - 1):
                    errors.append(
                        (
                            GuidelineViolations.C_SECTION,
                            sec[1] + 1,
                            f"{section_strings[sec[0]]} must be before the "
                            "previous section",
                        )
                    )
                previous_section_index = sec[0]
                # sec[2] is the string of the previous line
                if sec[2]:
                    errors.append(
                        (
                            GuidelineViolations.C_SECTION,
                            sec[1] + 1,
                            f"Ahead of {section_strings[sec[0]]} must be one "
                            "empty line",
                        )
                    )

        for i, found_bool in enumerate(found_section_bool):
            if not found_bool:
                errors.append(
                    (
                        GuidelineViolations.C_SECTION,
                        0,
                        f"{section_strings[i]} is missing",
                    )
                )
        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking existence of sections ({self.rule_name})"


class c_check_comment_style(Task.Task):
    """Class to implement c comment style check

    A task is only executed if a file is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_C_029"]

    def run(self):
        """calls test function and prints the error message

        A list (in self.input) containing the path of the to be
        checked file is passed as first argument in the task generation. The
        second argument self.rule_name contains the name of the style guide rule.
        run calls the test function and passes the text of the to be checked
        file with regular expression for the comment style. Depending on
        the return value of the test function an error message with the
        filename and the line number is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If the file contains
            the forbidden comment style the value of
            GuidelineViolations.C_COMMENT_STYLE is returned
        """

        errors = self.test(self.inputs[0].read(), self.regex)
        if errors:
            for err in errors:
                Logs.error(
                    f"{self.rule_name}: File: {self.inputs[0].abspath()}:{err}:"
                    f" {self.comment_style} comment style is not allowed"
                )
                return GuidelineViolations.C_COMMENT_STYLE.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt, regex):
        """Implements check which searches for forbidden comment style

        Args:
            txt (string): text of the to be checked file
            regex (compiled pattern):
            contains regular expression for the forbidden comment style

        Returns:
            list:
            A list with line numbers in which the forbidden comment style was
            found.
        """
        errors = []
        for i, line in enumerate(txt.splitlines()):
            if re.search(regex, line):
                errors.append(i + 1)
        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking c comment style ({self.rule_name})"


class check_header(Task.Task):
    """Class to implement running the checks for headers in files

    A task is only executed if a file or the regular expression HEADER_REGEX is changed.
    This task is executed for C, Python, Shell, Batch and YAML files.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_HEADER"]

    def run(self):
        """calls test function and errors in the headers of the files

        A list (in self.input) containing the path of the to be
        checked file is passed as first argument in the task generation. The
        second argument self.rule_name contains the name of the style guide rule.
        run calls the test function and passes the text of the to be checked
        file with the specific header of a file type. Depending on the return
        value of the test function an error message with the filename and line
        is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If a line in the
            file contains whitespace at the end
            GuidelineViolations.HEADER is returned
        """
        errors = self.test(
            self.inputs[0].read(), self.generator.env.HEADER_REGEX[self.lang_type]
        )
        if errors:
            for err in errors:  # pylint: disable=unused-variable
                Logs.error(
                    f"{self.rule_name}: File: {self.inputs[0].abspath()}:{err[1]}"
                    f" Line is not matching with {self.lang_type} header"
                )
            return GuidelineViolations.HEADER.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt, header):
        """Implements check that header is contained in text

        For each file type it is defined a specific header which
        has to be contained in the file. test function checks each line for
        this header.

        Args:
            txt (string): text of the to be checked file
            header (list): contains all lines of the header

        Returns:
            list:
            A list of tuples with GuidelineViolations members and the line
            number containing differences in the headers.
        """
        errors = []
        txt_list = txt.splitlines()
        for i, line in enumerate(header):
            try:
                if not line == txt_list[i]:
                    errors.append((GuidelineViolations.HEADER, i + 1))
            except IndexError:
                errors.append((GuidelineViolations.HEADER, i + 1))
                break
        return errors

    def keyword(self):
        """Displayed keyword for check depending on the file type"""
        word = "Checking headers"

        if self.lang_type == "C":
            word = f"{self.rule_name} in C-file)"
        elif self.lang_type == "Python":
            word = f"{self.rule_name} in python-file)"
        elif self.lang_type == "YAML":
            word = f"{self.rule_name} in YAML-file)"
        elif self.lang_type == "batch":
            word = f"{self.rule_name} in batch-file)"
        elif self.lang_type == "shell":
            word = f"{self.rule_name} in shell-file)"
        return word


class rst_check_include(Task.Task):
    """Class to implement the macro include check

    A task is only executed if a file or the INCLUDE_REGEX is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_RST_003"]

    def run(self):
        """calls test function and prints the error message

        A list (in self.input) containing the path of the to be
        checked file is passed as first argument in the task generation. The
        second argument self.rule_name contains the name of the style guide rule.
        run iterates over all specific excludes of the included files.
        Afterwards run calls the test function and passes the text of the to
        be checked file with regular expression for to be included file. Depending on
        the return value of the test function an error message with the
        filename is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If the file does not
            include the necessary file the value of
            GuidelineViolations.RST_MISSING_INCLUDE is returned
        """

        err = GuidelineViolations.NO_VIOLATION.value
        for i, regex in enumerate(self.regex_list):
            error = self.test(
                self.inputs[0].read(),
                regex,
            )
            if error != GuidelineViolations.NO_VIOLATION:
                Logs.error(
                    f"{self.rule_name}: File: {self.inputs[0].abspath()}:"
                    f" {self.include_name[i]} has to be included"
                )
                err = error.value
        return err

    @staticmethod
    def test(txt, regex):
        """Implements check that all necessary files are included

        Each .rst file has to include specific files

        Args:
            txt (string): text of the to be checked file
            regex (compiled pattern): contains regular expression for the include

        Returns:
            GuidelineViolations:
            if a file includes necessary file, GuidelineViolations.NO_VIOLATION is
            returned otherwise GuidelineViolations.RST_MISSING_INCLUDE
        """
        txt = txt.splitlines()
        error = GuidelineViolations.RST_MISSING_INCLUDE
        for line in txt:
            if re.match(regex, line):
                error = GuidelineViolations.NO_VIOLATION
                break
        return error

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking rst include ({self.rule_name})"


class rst_check_heading(Task.Task):
    """Class to implement the rst heading check

    A task is only executed if a file is changed.

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_RST_005"]

    def run(self):
        """calls test function and prints the error message

        A list (in self.input) containing the path of the to be
        checked file is passed as first argument in the task generation. The
        second argument self.rule_name contains the name of the style guide rule.
        run calls the test function and passes the text of the to be checked
        file. Depending on the return value of the test function an error
        message with the filename is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If the file does not
            have a heading the value of GuidelineViolations.RST_HEADING is returned

        """
        errors = self.test(self.inputs[0].read(), self.heading_regex_list)
        if errors:
            for error in errors:
                Logs.error(
                    f"{self.rule_name}: File: {self.inputs[0].abspath()}:{error[1]}"
                    f" {error[0]}"
                )
            return GuidelineViolations.RST_HEADING.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt, regex):  # pylint: disable=too-many-branches
        """Implements the check that rst files have a heading

        First it is iterated over the whole file to search for the file label,
        overline, file caption and underline. Afterwards there are returned
        error messages depending on the case.

        Args:
            txt (string): text of the to be checked file

        Returns:
            GuidelineViolations:
            if a file has a heading two lines after the file label
            GuidelineViolations.NO_VIOLATION is returned,
            otherwise GuidelineViolations.RST_HEADING
        """
        label_link_regex = regex[0]
        underlines_regex = regex[1]
        overlines_regex = regex[2]
        errors = []
        heading = ""
        found_label = -1
        found_caption = -1
        found_overline = -1
        found_underline = -1
        overline_length = 0
        underline_length = 0
        caption_length = 0
        wrong_overline_symbols = False
        overline = ""
        # loops over whole file and saves the line number of the found headings regex
        for i, line in enumerate(txt.splitlines()):
            label_found = re.match(label_link_regex, line)
            if label_found and not heading:
                heading = label_found.group(1)
                heading = "".join(ch.upper() if ch.isalnum() else "_" for ch in heading)
                found_label = i
            if found_caption < 0 and found_overline < 0:
                if re.match(overlines_regex, line):
                    found_overline = i
                    overline = line
                    overline_length = len(line)
                elif re.match(underlines_regex, line):
                    wrong_overline_symbols = True
            if heading and found_caption < 0:
                caption = "".join(ch.upper() if ch.isalnum() else "_" for ch in line)
                if caption == heading:
                    found_caption = i
                    caption_length = len(caption)
            if found_caption > 0 > found_underline:
                if found_overline > 0:
                    if line == overline:
                        found_underline = i
                        underline_length = len(line)
                elif re.match(underlines_regex, line):
                    found_underline = i
                    underline_length = len(line)

        # error handling for the returned errors
        if found_label > 0:
            if found_caption < 0:
                errors.append((f"Caption {heading} was not found", 0))
            else:
                if found_overline > 0 and found_overline != found_label + 2:
                    errors.append(
                        (
                            "Overline of the caption was not found two lines"
                            " after the file label",
                            found_overline + 1,
                        )
                    )
                if found_overline > 0:
                    if found_label != found_caption - 3:
                        errors.append(
                            (
                                f"The caption {heading} was not found three lines after "
                                "the file label",
                                found_caption + 1,
                            )
                        )

                    if found_underline < 0:
                        errors.append(
                            (
                                "Caption underline equal to overline is missing",
                                0,
                            )
                        )
                    elif found_underline > 0 and found_underline - 1 != found_caption:
                        errors.append(
                            (
                                "Underline was not found directly after caption",
                                found_underline + 1,
                            )
                        )
                else:
                    if wrong_overline_symbols:
                        errors.append(
                            (
                                "Overline contains not allowed symbols"
                                " (allowed symbols as '#*)",
                                0,
                            )
                        )
                    if found_label != found_caption - 2:
                        errors.append(
                            (
                                f"The caption {heading} was not found two lines after "
                                "the file label",
                                found_caption + 1,
                            )
                        )
                    if found_underline < 0:
                        errors.append(
                            (
                                "Caption underline (allowed symbols as '#*=-^\"')"
                                " is missing",
                                0,
                            )
                        )
                    elif found_underline > 0 and found_underline - 1 != found_caption:
                        errors.append(
                            (
                                "Underline was not found directly after caption",
                                found_underline + 1,
                            )
                        )
        else:
            errors.append(("File label not found at the beginning of the file", 0))

        if underline_length not in (0, caption_length) or overline_length not in (
            0,
            caption_length,
        ):
            errors.append(
                (
                    "Length of underline/overline is not equal to the length of the caption",
                    0,
                )
            )
        return errors

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking rst heading ({self.rule_name})"


class rst_check_orphan(Task.Task):
    """Class to implement declaration of rst file orphans

    A task is only executed if a file is changed

    """

    # color (string):  color in which the command line is displayed in the terminal
    color = "BLUE"
    # vars (list): contains a condition which triggers the execution of the task.
    # The condition is the change of the corresponding part in the
    # configuration file
    vars = ["CHECK_CONFIG_RST_006"]

    def run(self):
        """calls test function and prints the error message

        A list (in self.input) containing the path of the to be
        checked file is passed as first argument in the task generation. The
        second argument self.rule_name contains the name of the style guide rule.
        run calls the test function and passes the first line of the to be checked
        file. Depending on the return value of the test function an error
        message with the filename is printed.

        Returns:
            int:
            If no violation is found the value of
            GuidelineViolations.NO_VIOLATION is returned. If the file does not
            have orphan in the first line the value of
            GuidelineViolations.RST_ORPHAN is returned
        """
        error = self.test(self.inputs[0].read())
        if error != GuidelineViolations.NO_VIOLATION:
            Logs.error(
                f"{self.rule_name}: File: {self.inputs[0].abspath()}:"
                f" The first line has to be ':orphan:'"
            )
            return GuidelineViolations.RST_ORPHAN.value
        return GuidelineViolations.NO_VIOLATION.value

    @staticmethod
    def test(txt):
        """Implements check that the file is declared as orphan

        Args:
            txt (string): first line of the to be checked file

        Returns:
            GuidelineViolations:
            if a file includes orphan in the first line,
            GuidelineViolations.NO_VIOLATION is returned,
            otherwise GuidelineViolations.ORPHAN
        """
        txt = txt.splitlines()[0]
        if txt == ":orphan:":
            return GuidelineViolations.NO_VIOLATION
        return GuidelineViolations.RST_ORPHAN

    def keyword(self):
        """Displayed keyword for check"""
        return f"Checking rst orphan ({self.rule_name})"


@TaskGen.feature("guidelines")
def process_guidelines(self):
    # pylint: disable=too-many-statements,too-many-branches,too-many-locals
    """creates guideline tasks for the repository

    process_guidelines is decorated with TaskGen.feature which specifies when
    the tasks are executed. feature guidelines is defined in wscript.

    """

    if not getattr(self, "config", None):
        self.bld.fatal("No configuration given.")
    excl_global = self.config["global"]
    excl_binary = [f"**/*.{i}" for i in self.config["binary"]]

    # check for valid filenames
    rule = self.config["file_names"]
    excl_filenames = rule.get("exclude", []) + excl_global
    filenames_to_check = self.bld.path.ant_glob(
        "**", excl=excl_filenames, dir=True, quiet=True
    )
    self.env.CHECK_CONFIG_GEN_001 = str(excl_filenames)
    filename_regex = re.compile(r"" + rule["regex"])
    rn = rule["name"]
    for i in filenames_to_check:
        self.create_task("filenames", src=i, rule_name=rn, regex=filename_regex)

    # check that filenames are unique
    rule = self.config["unique_filenames"]
    excl_unique = rule.get("exclude", []) + excl_global
    self.env.CHECK_CONFIG_GEN_002 = str(excl_unique)
    unique_to_check = self.bld.path.ant_glob("**", excl=excl_unique, quiet=True)
    self.create_task("unique_filenames", src=unique_to_check, rule_name=rule["name"])

    # check that text files can be opened in UTF-8 mode
    rule = self.config["encoding"]
    excl_encoding = excl_binary + excl_global
    # value of vars to trigger task execution
    self.env.CHECK_CONFIG_GEN_003 = str(excl_encoding) + str(rule)
    default_excl = excl_encoding
    # saves for each encoding all files in a list
    encoding_to_check = {}
    for exception in rule["exceptions"]:
        for key, value in exception.items():
            # key is glob path expression
            # value is encoding
            if value in encoding_to_check:
                # if encoding already part of the dict
                encoding_to_check[value].extend(
                    self.bld.path.ant_glob(key, excl=excl_encoding, quiet=True)
                )
                default_excl.append(key)
            else:
                encoding_to_check[value] = self.bld.path.ant_glob(
                    key, excl=excl_encoding, quiet=True
                )
                default_excl.append(key)
    encoding_to_check[rule["default"]] = self.bld.path.ant_glob(
        "**", excl=default_excl, quiet=True
    )
    for key, value in encoding_to_check.items():
        for i in value:
            self.create_task("encoding", src=i, rule_name=rule["name"], encoding=key)

    # check that text files adhere to POSIX 3.206
    rule = self.config["posix_3.206"]
    excl_posix_3_206 = rule.get("exclude", []) + excl_binary + excl_global
    posix_3_206_to_check = self.bld.path.ant_glob(
        "**", excl=excl_posix_3_206, quiet=True
    )
    self.env.CHECK_CONFIG_GEN_004 = str(excl_posix_3_206)
    for i in posix_3_206_to_check:
        self.create_task("posix_3_206", src=i, rule_name=rule["name"])

    # check that text files do not have trailing whitespace
    rule = self.config["trailing_whitespace"]
    excl_trailing_whitespace = rule.get("exclude", []) + excl_binary + excl_global
    trailing_whitespace_to_check = self.bld.path.ant_glob(
        "**", excl=excl_trailing_whitespace, quiet=True
    )
    self.env.CHECK_CONFIG_GEN_005 = str(excl_trailing_whitespace)
    for i in trailing_whitespace_to_check:
        self.create_task("trailing_whitespace", src=i, rule_name=rule["name"])

    # check that text files do not use tabs
    rule = self.config["tabs"]
    excl_tabs = rule.get("exclude", []) + excl_binary + excl_global
    tabs_to_check = self.bld.path.ant_glob("**", excl=excl_tabs, quiet=True)
    self.env.CHECK_CONFIG_GEN_006 = str(excl_tabs)
    for i in tabs_to_check:
        self.create_task("tabs", src=i, rule_name=rule["name"])

    # language checks : C
    c_rules = self.config["languages"]["C"]
    excl_c = c_rules.get("exclude", []) + excl_global
    incl_c = c_rules.get("files", ["**/*.c", "**/.h"])

    # doxygen
    excl_c_doxygen = c_rules["doxygen"].get("exclude", []) + excl_c
    c_to_check_doxygen = self.bld.path.ant_glob(incl_c, excl=excl_c_doxygen, quiet=True)
    self.env.CHECK_CONFIG_C_004 = str(excl_c) + str(incl_c) + str(c_rules["doxygen"])
    C_004_REGEX = c_rules["doxygen"]["regex"]
    rn = c_rules["doxygen"]["name"]
    for i in c_to_check_doxygen:
        self.create_task("c_check_doxygen", src=i, rule_name=rn, regex=C_004_REGEX)

    # define guard
    excl_c_define_guard = c_rules["define_guard"].get("exclude", []) + excl_c
    incl_c_define_guard = c_rules["define_guard"].get("include", ["**/*.h"])
    c_to_check_define_guard = self.bld.path.ant_glob(
        incl_c_define_guard, excl=excl_c_define_guard, quiet=True
    )
    self.env.CHECK_CONFIG_C005 = (
        str(incl_c) + str(excl_c) + str(c_rules["define_guard"])
    )
    C_005_REGEX = c_rules["define_guard"]
    rn = c_rules["define_guard"]["name"]
    for i in c_to_check_define_guard:
        self.create_task("c_check_define_guard", src=i, rule_name=rn, regex=C_005_REGEX)

    # section
    excl_sections = c_rules["sections"].get("exclude", []) + excl_c
    excl_c_header_section = (
        excl_sections + ["**/*.c"] + c_rules["sections"]["header"]["exclude"]
    )
    c_to_check_header_section = self.bld.path.ant_glob(
        incl_c, excl=excl_c_header_section, quiet=True
    )

    excl_c_source_section = (
        excl_sections + ["**/*.h"] + c_rules["sections"]["source"]["exclude"]
    )
    c_to_check_source_section = self.bld.path.ant_glob(
        incl_c, excl=excl_c_source_section, quiet=True
    )

    excl_c_test_header_section = (
        excl_sections + ["**/*.c"] + c_rules["sections"]["test_header"]["exclude"]
    )
    c_to_check_test_header_section = self.bld.path.ant_glob(
        incl_c, excl=excl_c_test_header_section, quiet=True
    )

    excl_c_test_source_section = (
        excl_sections + ["**/*.h"] + c_rules["sections"]["test_source"]["exclude"]
    )
    c_to_check_test_source_section = self.bld.path.ant_glob(
        incl_c, excl=excl_c_test_source_section, quiet=True
    )

    section_to_check_file = [
        c_to_check_header_section,
        c_to_check_source_section,
        c_to_check_test_header_section,
        c_to_check_test_source_section,
    ]
    section_list = []
    section_list.append(c_rules["sections"]["header"]["sections"])
    section_list.append(c_rules["sections"]["source"]["sections"])
    section_list.append(c_rules["sections"]["test_header"]["sections"])
    section_list.append(c_rules["sections"]["test_source"]["sections"])
    self.env.CHECK_CONFIG_C_006 = str(incl_c) + str(excl_c) + str(c_rules["sections"])
    rn = c_rules["sections"]["name"]
    for i, file_list in enumerate(section_to_check_file):
        for f in file_list:
            self.create_task(
                "c_check_sections", src=f, section_strings=section_list[i], rule_name=rn
            )

    # c comment style
    excl_c_comment_style = c_rules["comment-style"].get("exclude", []) + excl_global
    self.env.CHECK_CONFIG_C_029 = (
        str(incl_c) + str(excl_c) + str(c_rules["comment-style"])
    )
    c_to_check_comment_style = self.bld.path.ant_glob(
        incl_c, excl=excl_c_comment_style, quiet=True
    )
    forbidden_style = c_rules["comment-style"]["forbidden"]
    self.comment_style = forbidden_style
    if forbidden_style != "C99":
        self.bld.fatal(f"{forbidden_style} is not implement yet")
    comment_regex = re.compile(r"(?<!:)//\s{0,}\S+")
    rn = c_rules["comment-style"]["name"]
    for i in c_to_check_comment_style:
        self.create_task(
            "c_check_comment_style",
            src=i,
            regex=comment_regex,
            comment_style=forbidden_style,
            rule_name=rn,
        )

    # C sources formatting
    excl_c_formatting = c_rules["formatting"].get("exclude", []) + excl_global
    c_to_check_formatting = self.bld.path.ant_glob(
        incl_c, excl=excl_c_formatting, quiet=True
    )
    c_formatting_provider = c_rules["formatting"]["provider"]
    if c_formatting_provider != "clang-format":
        self.bld.fatal("Only clang-format is supported.")
    if self.bld.env.CLANG_FORMAT:
        for i in c_to_check_formatting:
            self.create_task("clang_format", src=i, cwd=self.path)
    else:
        Logs.warn("clang-format is not available.")

    year_replace = ("@YEAR@", str(self.bld.options.COMMIT_YEAR))
    # language checks : header
    # rules with excludes and includes for each language
    c_rules = self.config["languages"]["C"]
    excl_c = c_rules.get("exclude", []) + excl_global + c_rules["header"]["exclude"]
    incl_c = c_rules.get("files", ["**/*.c", "**/*.h"])

    python_rules = self.config["languages"]["Python"]
    excl_python = (
        python_rules.get("exclude", [])
        + excl_global
        + python_rules["header"]["exclude"]
    )
    incl_python = python_rules.get("files", ["**/*.py", "**/wscript"])

    yaml_rules = self.config["languages"]["YAML"]
    excl_yaml = (
        yaml_rules.get("exclude", []) + excl_global + yaml_rules["header"]["exclude"]
    )
    incl_yaml = yaml_rules.get("files", ["**/*.yaml", "**/*.yml"])

    batch_rules = self.config["languages"]["batch"]
    excl_batch = (
        batch_rules.get("exclude", []) + excl_global + batch_rules["header"]["exclude"]
    )
    incl_batch = batch_rules.get("files", ["**/*.bat", "**/*.cmd"])

    shell_rules = self.config["languages"]["shell"]
    excl_shell = (
        shell_rules.get("exclude", []) + excl_global + shell_rules["header"]["exclude"]
    )
    incl_shell = shell_rules.get("files", ["**/*.sh"])

    self.env.CHECK_CONFIG_HEADER = str(incl_c) + str(excl_c)
    self.env.CHECK_CONFIG_HEADER = str(incl_python) + str(excl_python)
    self.env.CHECK_CONFIG_HEADER = str(incl_yaml) + str(excl_yaml)
    self.env.CHECK_CONFIG_HEADER = str(incl_batch) + str(excl_batch)
    self.env.CHECK_CONFIG_HEADER = str(incl_shell) + str(excl_shell)
    self.env.CHECK_CONFIG_HEADER += str(c_rules["header"])
    self.env.CHECK_CONFIG_HEADER += str(python_rules["header"])
    self.env.CHECK_CONFIG_HEADER += str(yaml_rules["header"])
    self.env.CHECK_CONFIG_HEADER += str(batch_rules["header"])
    self.env.CHECK_CONFIG_HEADER += str(shell_rules["header"])

    # C
    c_to_check = self.bld.path.ant_glob(incl_c, excl=excl_c, quiet=True)
    c_header = [i.replace(*year_replace) for i in c_rules["header"]["text"]]
    # Python
    python_to_check = self.bld.path.ant_glob(incl_python, excl=excl_python, quiet=True)
    python_header = [i.replace(*year_replace) for i in python_rules["header"]["text"]]
    # YAML
    yaml_to_check = self.bld.path.ant_glob(incl_yaml, excl=excl_yaml, quiet=True)
    yaml_header = [i.replace(*year_replace) for i in yaml_rules["header"]["text"]]
    # batch
    batch_to_check = self.bld.path.ant_glob(incl_batch, excl=excl_batch, quiet=True)
    batch_header = [i.replace(*year_replace) for i in batch_rules["header"]["text"]]
    # shell
    shell_to_check = self.bld.path.ant_glob(incl_shell, excl=excl_shell, quiet=True)
    shell_header = [i.replace(*year_replace) for i in shell_rules["header"]["text"]]
    # Combined
    header = {
        "Python": python_header,
        "YAML": yaml_header,
        "batch": batch_header,
        "shell": shell_header,
        "C": c_header,
    }
    check_files = {
        "Python": python_to_check,
        "YAML": yaml_to_check,
        "batch": batch_to_check,
        "shell": shell_to_check,
        "C": c_to_check,
    }
    self.env.HEADER_REGEX = header
    rn = "Check header"
    for key, files in check_files.items():
        for src in files:
            self.create_task("check_header", src=src, rule_name=rn, lang_type=key)

    # Python sources formatting
    excl_python_formatting = python_rules["formatting"].get("exclude", []) + excl_global
    python_to_check_formatting = self.bld.path.ant_glob(
        incl_python, excl=excl_python_formatting, quiet=True
    )
    python_formatting_provider = python_rules["formatting"]["provider"]
    if python_formatting_provider != "black":
        self.bld.fatal("Only black is supported.")
    self.create_task(
        "black", src=python_to_check_formatting, cwd=self.path, shell=False
    )

    # Python sources static program analysis
    py_spa = "static_program_analysis"
    excl_python_spa = python_rules[py_spa].get("exclude", []) + excl_global
    python_to_check_spa = self.bld.path.ant_glob(
        incl_python, excl=excl_python_spa, quiet=True
    )
    python_spa_provider = python_rules[py_spa]["provider"]
    if python_spa_provider != "pylint":
        self.bld.fatal("Only pylint is supported.")
    for i in python_to_check_spa:
        self.create_task("pylint", src=i, cwd=self.path)

    # rst include
    rst_rules = self.config["languages"]["reStructuredText"]
    excl_rst = rst_rules.get("exclude", []) + excl_global
    incl_rst = rst_rules.get("files")
    excl_rst_include = rst_rules["include"].get("exclude", []) + excl_rst
    rst_to_check_include = self.bld.path.ant_glob(
        incl_rst, excl=excl_rst_include, quiet=True
    )
    regex_list = []
    include_names = []
    excl_specific_rst = []
    # collect all regex, include_names and specific exclusion
    for i, include_file in enumerate(rst_rules["include"]["include_files"]):
        # obtains key by converting dict_keys object into list
        key = list(include_file.keys())[0]
        # obtains value by converting dict_values object into list
        values = list(include_file.values())[0]
        include_names.append(key)
        regex_list.append(re.compile(values["regex"]))

        if values["exclude"]:
            excl_specific_rst.append(
                self.bld.path.ant_glob(values["exclude"], quiet=True)
            )
        else:
            excl_specific_rst.append([])

    self.env.CHECK_CONFIG_RST_003 = (
        str(incl_rst) + str(excl_rst) + str(rst_rules["include"])
    )
    passed_regex_list = []
    passed_include_names = []
    # generate for each file the specific regular expression that has to be
    # checked with their include_names
    rn = rst_rules["include"]["name"]
    for rst_file in rst_to_check_include:
        for i, exclude in enumerate(excl_specific_rst):
            if not rst_file in exclude:
                passed_regex_list.append(regex_list[i])
                passed_include_names.append(include_names[i])
        self.create_task(
            "rst_check_include",
            src=rst_file,
            regex_list=passed_regex_list,
            rule_name=rn,
            include_name=passed_include_names,
        )
        passed_regex_list = []
        passed_include_names = []

    # rst heading
    heading_regex = []
    # r"\.\. _(\S+):" is used to search for link to the file like ".. _HOW_TO:"
    heading_regex.append(re.compile(r"\.\. _(\S+):"))
    # r"[=#*^\-\"]+" is used to search for the underlining of the heading
    heading_regex.append(re.compile(r"[=#*^\-\"]+"))
    # r"[#*]+" is used to check for overline and underline in headings
    heading_regex.append(re.compile(r"[#*]+"))
    excl_heading = rst_rules["heading"].get("exclude", []) + excl_rst
    rst_to_check_heading = self.bld.path.ant_glob(
        incl_rst, excl=excl_heading, quiet=True
    )
    self.env.CHECK_CONFIG_RST_005 = (
        str(incl_rst) + str(excl_rst) + str(rst_rules["heading"])
    )
    rn = rst_rules["heading"]["name"]
    for rst_file in rst_to_check_heading:
        self.create_task(
            "rst_check_heading",
            src=rst_file,
            rule_name=rn,
            heading_regex_list=heading_regex,
        )

    # rst orphan
    incl_orphan = rst_rules["orphan"].get("include", [])
    rst_to_check_orphan = []
    if incl_orphan:
        rst_to_check_orphan = self.bld.path.ant_glob(
            incl_orphan, excl=excl_rst, quiet=True
        )
    self.env.CHECK_CONFIG_RST_006 = (
        str(incl_rst) + str(excl_rst) + str(rst_rules["orphan"])
    )
    rn = rst_rules["orphan"]["name"]
    for rst_file in rst_to_check_orphan:
        self.create_task("rst_check_orphan", src=rst_file, rule_name=rn)


def check_guidelines(ctx):
    """Calls the guidelines check"""
    rules_node = ctx.path.find_node("conf/guidelines/rules.json")
    if not rules_node:
        ctx.fatal("Could not find guidelines file")
    rules = json.loads(rules_node.read())
    ctx(features="guidelines", config=rules)


class guidelines_context(BuildContext):
    """Helper class to bind the guidelines check to an waf argument"""

    cmd = "check_guidelines"
    fun = "check_guidelines"


# inject command into top-level wscript. This requires that g_module is available
if Context.g_module:
    Context.g_module.__dict__["check_guidelines"] = check_guidelines
