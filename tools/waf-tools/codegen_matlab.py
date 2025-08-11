#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a waf tool to use MATLAB® code generators (https://www.mathworks.com/)"""

import os
from pathlib import Path

from waflib.Configure import ConfigurationContext


def configure(ctx: ConfigurationContext):
    """configuration step of the MATLAB® code generators.

    #. searches for the HALCoGen program
    #. further support is still TODO
    """
    ctx.env.MATLAB_AVAILABLE = False
    ctx.start_msg("Checking for program 'matlab'")
    try:
        ctx.find_program("matlab", mandatory=True)
    except ctx.errors.ConfigurationError:
        ctx.end_msg(
            "not found - MATLAB® features will not be available", color="YELLOW"
        )
        return
    ctx.end_msg(ctx.env.get_flat("MATLAB") or "not found")
    ctx.env.MATLAB_AVAILABLE = True

    matlab_root = Path(ctx.env.get_flat("MATLAB")).parents[1]
    matlab_includes = os.path.join(matlab_root, "extern", "include")
    ctx.env.append_unique("INCLUDES", os.path.join(matlab_includes, "include"))
