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

"""Implements a waf tool to configure Cppcheck

For information on Cppcheck see http://cppcheck.sourceforge.net/.
"""

import os

from waflib import Utils, Task, TaskGen, Logs


class cppcheck(Task.Task):  # pylint: disable=invalid-name
    """Call cppcheck"""

    #: str: color in which the command line is displayed in the terminal
    color = "GREEN"

    #: str: Cppcheck handles the need for a re-run, so always run this task
    always_run = True

    run_str = (
        "${CPPCHECK} --project=${CPPCHECK_MAIN_PROJECT_FILE} --cppcheck-build-dir=. "
        "--exitcode-suppressions=${CPPCHECK_RULE_SUPPRESSION_FILE} -f "
        "--std=c99 --enable=warning,style,performance,portability,information,unusedFunction "
        "--addon=${CPPCHECK_ADDON_CNF_MISRA} --error-exitcode=${CPPCHECK_EXITCODE_FAIL} "
        "--suppressions-list=${CPPCHECK_RULE_SUPPRESSION_FILE}",
    )
    """str: string to be interpolated to create the command line to run
    cppcheck."""


@TaskGen.feature("cppcheck")
def add_cppcheck_task(self):
    """Task creator for cppcheck"""
    self.create_task("cppcheck")


def options(opt):
    """Defines options that can be passed to cppcheck tool"""
    if Utils.is_win32:
        doc_paths = [
            os.path.join(os.path.expanduser("~"), "Documents", "MISRA-C"),
            os.path.join(os.environ["PUBLIC"], "Documents", "MISRA-C"),
        ]
    else:
        doc_paths = [
            os.path.join(os.path.expanduser("~"), "MISRA-C"),
        ]
    rules_files = [
        opt.root.find_node(os.path.join(x, "rules-2012.txt")) for x in doc_paths
    ]
    rules_file = False
    if any(rules_files):
        rules_file = list(filter(None, rules_files))[0].abspath()

    opt.add_option(
        "--misra-rules-file",
        action="store",
        default=rules_file,
        dest="misra_rules_file",
        help="Sets the path to the MISRA rules file for cppcheck",
    )


def configure(conf):
    """configuration step of the Cppcheck waf tool

    - Find cppcheck
    - Search for the MISRA-C rules text
    """
    # check first for cppcheck in the PATH. If it is not present search in
    # the default installation directory
    conf.start_msg("Checking for program 'cppcheck'")
    conf.find_program("cppcheck", mandatory=False)
    if not conf.env.CPPCHECK:
        if Utils.is_win32:
            conf.find_program(
                "cppcheck",
                path_list=[os.path.join(os.environ["ProgramFiles"], "Cppcheck")],
                mandatory=False,
            )
    conf.end_msg(conf.env.get_flat("CPPCHECK"))

    if not conf.env.CPPCHECK:
        return

    conf.start_msg("Checking for MISRA-C rules file")

    rules_file = []
    if conf.options.misra_rules_file:
        rules_file = conf.root.find_node(os.path.abspath(conf.options.misra_rules_file))
        if not rules_file:
            Logs.warn(
                f"{os.path.abspath(conf.options.misra_rules_file)} does not exist. Ignoring input."
            )
        else:
            conf.env.append_unique("RULES_FILE", rules_file.abspath())
    conf.end_msg(conf.env.get_flat("RULES_FILE"))
