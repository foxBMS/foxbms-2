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

"""Colorize the output of TI ARM CGT"""

import os
import sys

from waflib import Logs

WARNING_INDICATORS = [
    "warning #",
    ": warning",
    ": remark",
    "remark #",
]
ERROR_INDICATORS = [
    "error #",
    ": error",
    ": fatal error",
    "catastrophic error",
]

# pylint: disable-msg=invalid-name,too-few-public-methods
class armclFormatter(Logs.formatter):
    """Custom formatter for armcl output

    - warnings printed in yellow
    - errors printed in red

    This formatter overwrites the default formatter in
    :py:meth:`f_ti_color_arm_cgt.options`
    """

    def __init__(self):
        """Initialize from base logger"""
        Logs.formatter.__init__(self)

    def format(self, rec):
        """Overwrite the default formatter with the custom coloring formatter"""
        frame = sys._getframe()  # pylint: disable-msg=protected-access
        while frame:
            if frame.f_code.co_name == "exec_command":
                cmd = frame.f_locals.get("cmd")
                if isinstance(cmd, list) and ("armcl" in cmd[0]):
                    rec.msg = armclFormatter.colorize(rec.msg)
            frame = frame.f_back
        return Logs.formatter.format(self, rec)

    @staticmethod
    def colorize(txt):
        """Colorizes the input text for console output"""
        lines = []
        for line in txt.splitlines():
            if any(x in line for x in WARNING_INDICATORS):
                lines.append(Logs.colors.YELLOW + line + Logs.colors.NORMAL)
            elif any(x in line for x in ERROR_INDICATORS):
                lines.append(Logs.colors.RED + line + Logs.colors.NORMAL)
            else:
                lines.append(line)
        return os.linesep.join(lines)


def options(opt):  # pylint: disable=unused-argument
    """Colorize TI ARM CGT output

    Furthermore the default formatter gets replaced by
    :py:class:`f_ti_color_arm_cgt.armclFormatter`.
    """
    Logs.log.handlers[0].setFormatter(armclFormatter())
