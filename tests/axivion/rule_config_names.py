#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2022, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""foxBMS specific rules settings for Axivion analysis."""

import re

import axivion.config  # pylint: disable=import-error
from bauhaus import ir  # pylint: disable=import-error
from bauhaus import style  # pylint: disable=import-error
from bauhaus.ir.common.scanner import comments  # pylint: disable=import-error

ANALYSIS = axivion.config.get_analysis()

VALID_FUNCTION_SUFFIXES = [
    "us",  # microseconds
    "perc",  # percentage
    "mV",  # millivolt
    "V",  # volt
    "mA",  # milliampere
    "A",  # ampere
    "t",  # "typedef"
]


def get_rule_type_from_name(rule_name: str) -> str:
    """returns the rule in a nicer human readable format."""
    cs_type_name = re.sub(
        r"([A-Z])", r" \1", rule_name.rsplit(".", maxsplit=1)[-1]
    ).split()
    return " ".join([i.lower() for i in cs_type_name])


@style.workitem(inputs=(ir.Graph, comments.scan_and_cache_comments), repeat=True)
class ModulePrefixLookup(style.WorkItem):
    """Fetches the module prefix based on a regex from the provided source file

    hint: check that this regex is consistent with
    'conf/guidelines/rules.json:languages:C:doxygen:regex:{xy}'
    setting
    """

    prefix_re = re.compile(r"@prefix\s+(\S+)(?:\s|\n|$)", flags=re.MULTILINE)

    # pylint: disable=too-many-nested-blocks
    def __init__(self, ir_graph, cached_comments):
        self._map = {}
        for file_node in ir_graph.nodes_of_type(
            ir.Physical,
            (
                "Primary_File",
                "User_Include_File",
            ),
        ):
            for comment in cached_comments.get(file_node, ()):
                if not comment.at_beginning:
                    break
                match = self.prefix_re.search(comment.Value)
                if match:
                    self._map[file_node] = match[1]
                    break

    def __contains__(self, item):
        return item in self._map

    def __getitem__(self, item):
        return self._map[item]


def containing_modules(node):
    """yields the file"""
    if node.part() == ir.Logical:
        nodes = node.Definitions
        # if no definitions available, go for the declarations
        if len(nodes) == 0:
            # use declarations if available, otherwise fallback
            nodes = node.Declarations
            if len(nodes) == 0:
                nodes = (node.Physical,)
    else:
        nodes = (node,)

    for pir_node in nodes:
        if pir_node:
            file = pir_node.enclosing_of_type("File")
            if file:
                yield file


def check_lowercase_prefix(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """check that node starts with the lower-case module prefixed followed by
    an underscore."""
    if node.Name:
        for file in containing_modules(node):
            if file in module_prefixes:
                if not node.Name.startswith(module_prefixes[file].lower() + "_"):
                    return False
    return True


def check_uppercase_prefix(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """check that node starts with the upper-case module prefixed followed by
    an underscore."""
    if node.Name:
        for file in containing_modules(node):
            if file in module_prefixes:
                if not node.Name.startswith(module_prefixes[file].upper() + "_"):
                    return False
    return True


def check_prefix_known(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """check that a module prefix exists for the current node."""
    if node.Name:
        for file in containing_modules(node):
            if file in module_prefixes:
                return True
        return False
    return True


def check_function_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Checks that a function name consists of PascalName base name and an
    optional suffix that indicates the unit."""
    prefix = None
    if node.Name:
        for file in containing_modules(node):
            if file in module_prefixes:
                prefix = module_prefixes[file].upper() + "_"
    if not prefix:  # something went wrong
        return False

    base_name = ""
    if node.Name.startswith(prefix):
        base_name = node.Name[len(prefix) :]
    else:
        return False

    base_name_splitted = base_name.split("_", maxsplit=1)
    function_name = base_name_splitted[0]
    try:
        function_name_suffix = base_name_splitted[1]
    except IndexError:
        function_name_suffix = None

    idx = []
    for i, val in enumerate(function_name):
        if val.isupper():
            idx.append(i)

    valid_pascal_case = True
    for i in range(1, len(idx)):
        # if the difference between two indexes is 1, we are not seeing PascalCase
        if idx[i] - idx[i - 1] == 1:
            valid_pascal_case = False
            break

    valid_suffix = True
    if function_name_suffix:
        valid_suffix = False
        if function_name_suffix in VALID_FUNCTION_SUFFIXES:
            valid_suffix = True
    return valid_pascal_case and valid_suffix


#: list: List of things that MUST start with the upper-case module prefix
upper_case = (
    "CodingStyle-Naming.Function",
    "CodingStyle-Naming.Macro",
    "CodingStyle-Naming.TypedefedFuncPtr",
    "CodingStyle-Naming.TypedefedEnum",
    "CodingStyle-Naming.TypedefedStruct",
)

for name in upper_case:
    ANALYSIS[name].additional_checks.append(
        (
            check_uppercase_prefix,
            f"Please use upper-case module prefix for {get_rule_type_from_name(name)}.",
        )
    )
    ANALYSIS[name].additional_checks.append(
        (check_prefix_known, "Unknown which module prefix to check.")
    )

ANALYSIS["CodingStyle-Naming.Function"].additional_checks.append(
    (check_function_name, "The function name is not PascalCase.")
)

#: list: List of things that MUST start with the lower-case module prefix
lower_case = (
    "CodingStyle-Naming.GlobalVariable",
    "CodingStyle-Naming.LocalVariable",
)
for name in lower_case:
    ANALYSIS[name].additional_checks.append(
        (
            check_lowercase_prefix,
            f"Please use lower-case module prefix for {get_rule_type_from_name(name)}.",
        )
    )
    ANALYSIS[name].additional_checks.append(
        (check_prefix_known, "Unknown which module prefix to check.")
    )


# Only check function pointers with rule CodingStyle-Naming.TypedefedFuncPtr
def is_function_pointer_type(type_node: ir.Node) -> bool:
    """Filter function to only check function pointers"""
    original_type = type_node.skip_typedefs(True)
    if original_type.is_of_type("Pointer_Type"):
        element_type = original_type.Pointed_To_Type.skip_typedefs(True)
        return element_type.is_of_type("Routine_Type")
    return False


ANALYSIS[
    "CodingStyle-Naming.TypedefedFuncPtr"
].filter_predicate = is_function_pointer_type
