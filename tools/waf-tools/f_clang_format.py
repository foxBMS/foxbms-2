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

r"""Implements a waf tool to run
`clang-format <https://clang.llvm.org/docs/ClangFormat.html>`_.

:numref:`f-clang-format-usage` shows how to use this tool.

.. code-block:: python
    :caption: f_clang-format.py
    :name: f-clang-format-usage
    :linenos:

    def options(opt):
        opt.load("clang_format")

    def configure(conf):
        conf.load("clang_format")

    def build:
        files = bld.path.ant_glob("\*\*/\*.c")
        bld(features="clang-format", files=files)

"""

import os
from waflib import Task, TaskGen, Utils


class clang_format(Task.Task):  # pylint: disable-msg=invalid-name
    """Task to run clang-format on all given source files"""

    #: str: color in which the command line is displayed in the terminal
    color = "BLUE"

    vars = ["CLANG_FORMAT_OPTIONS"]

    run_str = "${CLANG_FORMAT} ${CLANG_FORMAT_OPTIONS} ${SRC[0].abspath()}"


@TaskGen.feature("clang-format")
def process_clang_format(self):
    """creates clang-format task for each input source"""
    if not getattr(self, "files", None):
        self.bld.fatal("No files given.")
    for src in self.files:
        self.create_task("clang_format", src, cwd=self.path)


def options(opt):
    """Passing options to clang-format"""
    opt.add_option(
        "--clang-format-option",
        action="append",
        default=[],
        dest="CLANG_FORMAT_OPTION",
        help="Options for clang-format",
    )


def configure(conf):
    """configuration step of the clang-format tool

    - searches for the program ``clang-format``
    - applies configured options
    """
    # check first for clang-format in the PATH. If it is not present search in
    # the default installation directory
    conf.start_msg("Checking for program 'clang-format'")
    conf.find_program("clang-format", mandatory=False)
    if not conf.env.CLANG_FORMAT:
        if Utils.is_win32:
            conf.find_program(
                "clang-format",
                path_list=[os.path.join(os.environ["ProgramFiles"], "LLVM", "bin")],
            )
    conf.end_msg(conf.env.get_flat("CLANG_FORMAT"))
    conf.env.append_unique("CLANG_FORMAT_OPTIONS", conf.options.CLANG_FORMAT_OPTION)
