#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""foxBMS specific rules setting for Axivion analysis of include guards."""

from typing import Union
from pathlib import Path

import axivion.config  # pylint: disable=import-error
from bauhaus import ir  # pylint: disable=import-error
from bauhaus import scanner  # pylint: disable=import-error

ANALYSIS = axivion.config.get_analysis()


def create_include_guard(file_name: str) -> str:
    """Creates foxBMS project specific include guard"""
    prefix = "FOXBMS__"
    suffix = "_H_"
    sanitized_file_name = "".join(
        ch.upper() if ch.isalnum() else "_" for ch in file_name
    )
    return prefix + sanitized_file_name + suffix


def check_include_guard(
    node: ir.Node,
    define: scanner.Token,  # pylint: disable=unused-argument
    macro: Union[str, None],
) -> bool:
    """check that header file uses the correct include guard."""
    if node:
        expected_include_guard = create_include_guard(Path(node.Name).stem)
        if not expected_include_guard == macro:
            return f"Expected include guard is '{expected_include_guard}', but found {macro}."
    return None


RULE_NAME = "Generic-MissingIncludeGuard"
ANALYSIS[RULE_NAME].macro_name_restrictions.append((check_include_guard))
