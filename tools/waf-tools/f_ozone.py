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

"""Implements a waf tool to configure a Segger Ozone project to foxBMS specific
needs.

For information on Segger Ozone see the
`Segger website <https://www.segger.com/>`_.

This waf tool automatically generates a project configuration for Segger
Ozone, when the binary of Segger Ozone is found on the system. Check the
output of the configure step for whether the binary ``ozone`` has been
successfully located if you suspect any issues.

After successful configuration of the project with the configure task the
Ozone configuration file will be available as ``foxbms.jdebug`` in the build
directory of this project. When loading this file in Ozone everything should
be automatically configured and the debugger ready to start a debugging
session. Ozone will create a file ``foxbms.jdebug.user`` in the same
directory in order to store the layout of the debugging windows and
similar configuration. This file will normally not be disturbed by the
build system.
"""

import os

from waflib import Utils


def configure(conf):
    """configuration step of the Ozone waf tool:

    - Find the debugger software
    - configure a project if debugger software was found"""
    if Utils.is_win32:
        conf.start_msg("Checking for program 'ozone'")
        conf.find_program("Ozone", var="OZONE", mandatory=False)
        if not conf.env.OZONE:
            segger_base_installdir = os.path.join(os.environ["ProgramFiles"], "SEGGER")
            try:
                segger_installdir = os.listdir(segger_base_installdir)
            except FileNotFoundError:
                pass
            else:
                path_list = []
                for segger_version in [
                    os.path.join(segger_base_installdir, x) for x in segger_installdir
                ]:
                    path_list.append(segger_version)
                conf.find_program(
                    "Ozone", var="OZONE", path_list=path_list, mandatory=False
                )
        conf.end_msg(conf.env.get_flat("OZONE"))

    if not conf.env.OZONE:
        return

    # setup JLink project
    ozone_config = conf.path.find_node(
        os.path.join(os.path.join("tools", "debugger", "ozone", "foxbms.jdebug.config"))
    )
    config = ozone_config.read()
    config = config.replace(
        "@PROJECT_DIR@", conf.path.get_bld().abspath().replace("\\", "/")
    )
    config = config.replace(
        "@ELF_FILE@",
        os.path.join(conf.path.get_bld().abspath(), "bin", "foxbms.elf").replace(
            "\\", "/"
        ),
    )
    conf.path.get_bld().make_node("foxbms.jdebug").write(config)
