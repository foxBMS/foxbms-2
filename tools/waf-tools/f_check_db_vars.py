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

r"""Implements a waf tool to check the database initialization in foxBMS.

:numref:`f-check-db-vars` shows how to use this tool.

.. code-block:: python
    :caption: f_check_db_vars.py
    :name: f-check-db-vars
    :linenos:

    def configure(conf):
        conf.load("f_check_db_vars")

    def build:
        bld(
            features="db_check",
            files=bld.path.ant_glob("tests/unit/**/*.c"),
        )

"""

import os
import re
import tabulate

from waflib import Logs, Task, TaskGen


class check_db_vars(Task.Task):  # pylint: disable=invalid-name
    """checks that database variables are initialized"""

    color = "BLUE"

    before = ["c", "c_ppi", "c_ppm", "c_ppd"]

    # static/extern or left out + whitespace
    base_regex = r"^[\s]{0,}(?:static[\s]{1,}|extern[\s]{1,})?[\s]{0,}"

    # regex to find the usage of database variables.
    # Database variable identifiers need to start with `DATA_BLOCK_` followed
    # by some name (group 1). After that one or more whitespace may be used and
    # the variables name comes (group 2).
    db_simple_regex = re.compile(
        base_regex + r"(DATA_BLOCK_)([A-za-z_0-9]{1,})_s[ ]{0,}([A-za-z0-9_]{1,})"
    )
    # regex to match the exact way we want database variables initialized.
    # Database variable identifiers need to start with `DATA_BLOCK_` followed
    # by some name (group 1). After that one whitespace must be used and the
    # variables name comes (group 2). Next the uniqueID of the database entry
    # must be initialized.
    db_var_regex = (
        base_regex
        + r"DATA_BLOCK_([A-Z_0-9]{1,})_s[\s]{1,}"  # type needs prefix + alphanumeric and _
        r"([A-za-z0-9_]{0,})[\s]{0,}=[\s]{0,}"  # variable name
        r"\{[\s]{0,}(\.header\.uniqueId)"  # this is the member we initialize
        r"[\s]{0,}=[\s]{0,}"  # the assignment
        # we need a specific ID with might have a suffix
        r"(DATA_BLOCK_ID_)(@VAR@)(_BASE|_REDUNDANCY[0-9]{1,})?"
        r"[\s]{0,}\}[\s]{0,};[\s]{0,}$"
    )

    def run(self):
        """does the actual check"""
        source = self.inputs[0].read()
        lines = source.splitlines()
        for i, _line in enumerate(lines):
            line = _line.strip()
            if "DATA_BLOCK_HEADER" in line:
                continue
            is_db_var = self.db_simple_regex.match(line)
            if is_db_var:
                db_regex = r"" + check_db_vars.db_var_regex.replace(
                    "@VAR@", is_db_var.group(2)
                )
                correct_init = re.match(db_regex, line)
                if not correct_init:
                    # recovery attempt: initializer might be broken into next line
                    next_line = lines[i + 1].strip()
                    line_and_next = "".join([line, next_line])
                    correct_init = re.match(db_regex, line_and_next)
                if correct_init:
                    if Logs.verbose:
                        info = tabulate.tabulate(
                            [
                                ["Source", f"{self.inputs[0].abspath()}:{i+1}"],
                                ["Type", correct_init.group(1)],
                                ["Variable", correct_init.group(2)],
                                ["ID member", correct_init.group(3)],
                                ["uniqueID", correct_init.group(4)],
                            ]
                        )
                        print(info)
                else:
                    correct_init_base = (
                        f"{is_db_var.group(1)}{is_db_var.group(2)}_s "
                        f"{is_db_var.group(3)} = {{.header.uniqueId = "
                        f"DATA_BLOCK_ID_{is_db_var.group(2)}"
                    )

                    err_details = (
                        f"{self.inputs[0].abspath()}:{i+1} uses database "
                        f"variable {is_db_var.group(3)} (type: '"
                        f"{is_db_var.group(1)}{is_db_var.group(2)}_s') without "
                        f"correct initialization.\n\nSomething like\n\n\t"
                        + correct_init_base
                        + "};\nor\n\t"
                        + correct_init_base
                        + "_BASE};\nor\n\t"
                        + correct_init_base
                        + "_REDUNDANCY0};\n\n"
                        f"is required, where the ID is from typedef enum "
                        f"DATA_BLOCK_ID_e.\nDetails: Regex\n\t{db_regex}\n"
                        f"did not match on\n\t{is_db_var.string}\n."
                    )
                    Logs.pprint("YELLOW", err_details)
                    self.generator.bld.fatal(
                        "Project will not compile with that error."
                    )

    def keyword(self):  # pylint: disable=no-self-use
        """displayed keyword when this check is run"""
        return "Checking for database variables in"


@TaskGen.feature("c", "db_check")
@TaskGen.after_method("process_source")
def check_data_base_init(self):
    """Task creator for check_db_vars"""
    if self.bld.variant == "unit_test":
        for c_source in self.files:
            self.create_task("check_db_vars", c_source)
    else:
        for c_source in self.source:
            if os.path.join("src", "app") in c_source.relpath():
                self.create_task("check_db_vars", c_source)
