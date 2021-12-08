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

"""Axivion rules configured for the foxBMS 2 project."""

import axivion.config  # pylint: disable=import-error

analysis = axivion.config.get_analysis()

analysis["Generic-LocalInclude"].msg["add_include"].disabled = True
analysis["Generic-LocalInclude"].msg["add_symbol_declaration"].disabled = True
analysis["Generic-LocalInclude"].msg["bad_pch_use"].disabled = True
analysis["Generic-LocalInclude"].msg["circular_include"].disabled = False
analysis["Generic-LocalInclude"].msg["contents_covered_include"].disabled = True
analysis["Generic-LocalInclude"].msg["covered_include"].disabled = True
analysis["Generic-LocalInclude"].msg["local_decl_instead_of_include"].disabled = True
analysis["Generic-LocalInclude"].msg["local_def_instead_of_include"].disabled = True
analysis["Generic-LocalInclude"].msg["more_precise_include"].disabled = True
analysis["Generic-LocalInclude"].msg["move_include"].disabled = True
analysis["Generic-LocalInclude"].msg["move_include_with_many_clients"].disabled = True
analysis["Generic-LocalInclude"].msg["unused_include"].disabled = False

analysis["Generic-ForbiddenFunctions"].blacklist = {
    "stdlib.h": ["malloc", "free", "calloc", "realloc"],
    "stdio.h": ["*printf*"],
    "string.h": ["str[!n]*"],
}

analysis["Parallelism-UnsafeVarAccess"].partitions = {
    "TaskEngine": {
        "entries": ["FTSK_CreateTaskEngine"],
        "priority": 9,
    },
    "Task1ms": {
        "entries": ["FTSK_CreateTaskCyclic1ms"],
        "priority": 6,
    },
    "Task10ms": {
        "entries": ["FTSK_CreateTaskCyclic10ms"],
        "priority": 5,
    },
    "Task100ms": {
        "entries": ["FTSK_CreateTaskCyclic100ms"],
        "priority": 4,
    },
    "Task_100ms_Algorithm": {
        "entries": ["FTSK_CreateTaskCyclicAlgorithm100ms"],
        "priority": 3,
    },
}


def disallow_unjustified_deviations(issue):
    """forbid suppressing without justification"""
    if issue.disabled:
        if not issue.justification:
            issue.disabled = False


for rule_name in analysis.get_active_rules():
    if hasattr(analysis[rule_name], "justification_checker"):
        analysis[rule_name].justification_checker = disallow_unjustified_deviations
