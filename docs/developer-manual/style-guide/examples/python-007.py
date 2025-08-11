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

"""Example for platform independent code"""

# ruff: noqa: E402,F841
# pylint: disable=invalid-name,unused-argument,unused-variable,wrong-import-position
import os
import sys

WAF_DIR_BASE_NAME = "waf3-2.1.5-7e89fb078ab3c46cf09c8f74bbcfd16d"
sys.path = [
    os.path.abspath(f"./../../tools/{WAF_DIR_BASE_NAME}"),
    os.path.abspath(f"./../../tools/.{WAF_DIR_BASE_NAME}"),
] + sys.path

# start-include-in-docs
from waflib.Build import BuildContext


def build(bld: BuildContext):
    """builds something..."""
    # fmt: off
    # pylint: disable=line-too-long
    includes = [
        # ...
        "some/very/long/path/that/leads/to/very/long/code/lines",
        "some/very/long/path/that/leads/to/very/long/code/lines/subpath0/subpath1",
        "some/very/long/path/that/leads/to/very/long/code/lines/subpath0/subpath1/subpath2",
        # ...
    ]
    # pylint: enable=line-too-long
    # fmt: on
