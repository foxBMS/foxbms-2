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

"""Searches the additional TI ARM CGT tools that come with TI CCS and do not
have a separate tool where they get configured.

This tool is loaded in the configure step of the main compiler tool
:py:meth:`f_ti_arm_cgt.configure`. and is not meant to be used standalone."""

from waflib.Configure import conf

TI_CCS_ARM_CGT_TOOLS = [
    "armabs",
    "armacpia",
    "armadv",
    "armcg",
    "armcl",
    "armclist",
    "armdem",
    "armdis",
    "armembed",
    "armilk",
    "armlibinfo",
    "armlnk",
    "armopt",
    "armpdd",
    "armpprof",
    "armstrip",
    "armsize",
    "armasm",
    "armnm",
    # arm-none-eabi- tools distributed with cgt
    "armobjcopy",
    "armobjdump",
    "armreadelf",
    # tools needed to build the hex and bin files
    "armhex",
    "tiobj2bin",
    "mkhex4bin",
    "armofd",
    # these tools are needed to build the runtime support libraries
    "mklib",
    "sh",
    "unzip",
    "gmake",
]


@conf
def find_arm_tools(conf):  # pylint: disable-msg=redefined-outer-name
    """Configures additional tools related to the compiler."""
    for i in TI_CCS_ARM_CGT_TOOLS:
        conf.find_program(i)
