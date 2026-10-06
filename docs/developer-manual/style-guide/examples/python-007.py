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

"""Example for platform independent code"""

# ruff: noqa: D103,E402,F841
# pylint: disable=unused-argument,unused-variable
import sys
from pathlib import Path

# we need this here to set up the WAF directory before importing waflib;
# DO NOT USE THIS SNIPPET SOMEWHERE ELSE
_WAF_VERSION = "2.1.9-beba77c244731800bf15a003232e7040"
_WAF_DIR_NAME = f"waf3-{_WAF_VERSION}"
if sys.platform.lower() != "win32":
    _WAF_DIR_NAME = f".{_WAF_DIR_NAME}"
_WAF_DIR_REL = f"tools/{_WAF_DIR_NAME}"
_WAF_DIR = Path(__file__).parents[4].resolve() / _WAF_DIR_REL
sys.path.append(str(_WAF_DIR))
from waflib.Build import BuildContext  # pylint: disable=wrong-import-position


# start-include-in-docs
def build(bld: BuildContext) -> None:
    # fmt: off
    includes = [
        # ...
        "some/very/long/path/that/leads/to/very/long/code/lines",
        "some/very/long/path/that/leads/to/very/long/code/lines/subpath0/subpath1",
        "some/very/long/path/that/leads/to/very/long/code/lines/subpath0/subpath1/subpath2",
        # ...
    ]
    # fmt: on
