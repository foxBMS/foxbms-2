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

r"""Implements a waf tool to run `pylint <https://pylint.org/>`_.

:numref:`f-pylint-usage` shows how to use this tool.

.. code-block:: python
    :caption: f_pylint.py
    :name: f-pylint-usage
    :linenos:

    def options(opt):
        opt.load("pylint")

    def configure(conf):
        conf.load("pylint")

    def build:
        files = bld.path.ant_glob("\*\*/\*.py")
        bld(features="pylint", files=files)

"""

from waflib import Task, TaskGen


class pylint(Task.Task):  # pylint: disable-msg=invalid-name
    """Class to implement running the pylint static analysis tool on Python files"""

    #: str: color in which the command line is displayed in the terminal
    color = "BLUE"
    vars = ["PYLINT_OPTIONS"]
    after = ["black"]

    run_str = "${PYLINT} ${PYLINT_OPTIONS} ${SRC[0].abspath()}"

    def keyword(self):
        """displayed keyword when pylint is run"""
        return "Linting"


@TaskGen.feature("pylint")
def process_pylint(self):
    """creates pylint tasks for each input file"""
    if not getattr(self, "files", None):
        self.bld.fatal("No files given.")
    for src in self.files:
        self.create_task("pylint", src, cwd=self.path)


def options(opt):
    """Passing options to pylint"""
    opt.add_option(
        "--pylint-option",
        action="append",
        default=[],
        dest="PYLINT_OPTION",
        help="Options for pylint",
    )


def configure(conf):
    """configuration step of the pylint tool

    - searches for the program ``pylint``
    - applies configured options
    """
    conf.find_program("pylint", var="PYLINT")
    conf.env.append_unique("PYLINT_OPTIONS", conf.options.PYLINT_OPTION)
