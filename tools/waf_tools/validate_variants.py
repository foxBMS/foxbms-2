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

"""Validate the variant names defined in the main build script."""

import re

from waflib.Options import OptionsContext

VALID_VARIANTS = re.compile(r"^(app_|bootloader_|docs$)")
RE_NORMAL = re.compile(r"(doxygen|ti_arm_cgt)$")
RE_UNIT_TEST = re.compile(r"(doxygen)|((spa_)?gcc)|(ti_arm_cgt)$")


def options(opt: OptionsContext) -> None:
    """Validate the variant names defined in the main build script"""
    for variant in opt.VARIANTS:
        if not VALID_VARIANTS.search(variant):
            err = f"{variant}: variant names must start with {VALID_VARIANTS.pattern}."
            opt.fatal(err)
        if variant == "docs":
            # there is no more to validate
            continue
        # check whether it's a unit test variant, so that the prefix needs to
        # adapted to xxx_unit_test
        if "unit_test" in variant:
            if RE_UNIT_TEST.search(variant):
                continue
            err = (
                f"{variant}: Unit test variant names must match {RE_UNIT_TEST.pattern}"
            )
            opt.fatal(err)
        if RE_NORMAL.search(variant):
            continue
        err = f"{variant}: variant names must match {RE_NORMAL.pattern}"
        opt.fatal(err)
