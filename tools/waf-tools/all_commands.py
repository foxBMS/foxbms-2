#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implement a 'build_all' and its counterpart 'clean_all' commands.

The actual 'build_all' and 'clean_all' commands are derived from two lists
named 'BUILD_VARIANTS' and 'CLEAN_VARIANTS' respectively, that need to be
defined in the top-level wscript.
The fall back options are 'build' and 'clean' respectively.

This module **must** be loaded in the 'options' method.
"""

from waflib import Context, Options
from waflib.Build import BuildContext


def build_all(_: Context.Context) -> None:
    """builds all variants"""  # noqa: D403
    variants = [
        f"build_{var}" for var in getattr(Context.g_module, "VARIANT_CONFIGS", {})
    ]
    Options.commands.extend(variants)


def clean_all(_: Context.Context) -> None:
    """cleans all variants"""  # noqa: D403
    variants = [
        f"clean_{var}" for var in getattr(Context.g_module, "VARIANT_CONFIGS", {})
    ]
    Options.commands.extend(variants)


class BuildAllContext(BuildContext):
    """Build all targets"""

    cmd = "build_all"
    fun = "build_all"


class CleanAllContext(BuildContext):
    """Clean all targets"""

    cmd = "clean_all"
    fun = "clean_all"


# inject command into top-level wscript. This requires that g_module is available
if Context.g_module:
    Context.g_module.__dict__["build_all"] = build_all
    Context.g_module.__dict__["clean_all"] = clean_all
