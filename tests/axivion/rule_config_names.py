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

"""foxBMS specific rules settings for Axivion analysis."""

import logging
import re
import sys
from pathlib import Path

import axivion.config  # pylint: disable=import-error
from bauhaus import ir  # pylint: disable=import-error
from bauhaus import style  # pylint: disable=import-error
from bauhaus.ir.common.scanner import comments  # pylint: disable=import-error

if "perform_tests" in sys.argv[0]:
    logging.basicConfig(format="%(levelname)s:%(message)s", level=logging.DEBUG)
    logging.getLogger().setLevel(logging.DEBUG)


ANALYSIS = axivion.config.get_analysis()

UNKNOWN_PREFIX_ERROR_MESSAGE = "Unknown which module prefix to check."

VALID_SUFFIXES = (
    "degC",  # degree Celsius
    "ddegC",  # deci degree Celsius
    "dK",  # deci Kelvin
    "ohm",  # ohm
    "kOhm",  # kilo ohm
    "ms",  # milliseconds
    "us",  # microseconds
    "perc",  # percentage
    "mV",  # millivolt
    "V",  # volt
    "mA",  # milliampere
    "A",  # ampere
    "mAs",  # milliampere seconds
    "mAh",  # milliampere hours
    "As",  # ampere seconds
    "Wh",  # watt hours
    "t",  # "typedef"
    "Hz",  # Hertz
)


def validate_suffix(used_suffix: str = ""):
    """Validate used suffix against list of permitted suffixes"""
    valid_suffix = True
    if used_suffix:
        valid_suffix = False
        if used_suffix in VALID_SUFFIXES:
            valid_suffix = True
    return valid_suffix


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

    def file_sort_key(f):
        """sort by key"""
        if f.is_of_type("User_Include_File"):
            index = 1
        elif f.is_of_type("System_Include_File"):
            index = 10
        elif f.is_of_type("Preinclude_File"):
            index = 1000
        elif f.is_of_type("PCH_File"):
            index = 20
        elif f.is_of_type("Primary_File"):
            index = 50
        else:
            index = 100
        return (
            index,
            f.Full_Name,
        )

    if node.part() == ir.Logical:
        files = (pir_node.enclosing_of_type("File") for pir_node in node.Decls_And_Defs)
        try:
            yield next(iter(sorted(files, key=file_sort_key)))
        except StopIteration:
            pass
    else:
        file = node.enclosing_of_type("File")
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


def check_macro_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """check that the macro starts with the upper-case module prefixed followed by
    an underscore (exception: include guard macro 'FOXBMS__{FILENAME}_H_')."""
    if not node.Name:
        return True
    # ignore compiler builtins
    if (
        not node.type() == "Predefined_Object_Macro_Definition"
        and not node.Name.startswith("__")
    ):
        logging.debug(f"node.Name: {node.Name}")
    else:
        logging.getLogger().setLevel(logging.FATAL)  # ignore output

    for file in containing_modules(node):
        if file in module_prefixes:
            logging.debug(f"found prefix: {module_prefixes[file]}")
            if not node.Name.startswith(module_prefixes[file].upper() + "_"):
                logging.debug(f"Check if '{node.Name}' is a include guard.")
                # check that we are not hitting an include guard, as
                # include guards use the pattern 'FOXBMS__{FILENAME}_H_'
                file_name_to_guard = "".join(
                    ch.upper() if ch.isalnum() else "_" for ch in Path(file.Name).stem
                )
                define_guard = f"FOXBMS__{file_name_to_guard}_H_"
                if node.Name == define_guard:
                    logging.debug(f"found include guard ('{node.Name}').")
                    logging.debug(f"Done for '{node.Name}'.\n")
                    return True
                logging.debug(f"'{node.Name}' is not an include guard.")
            # we have not hit an include guard, now check that the macro starts
            # with the prefix is all uppercase and optionally ends with one of
            # the valid suffixes
            if not node.Name.startswith(module_prefixes[file].upper() + "_"):
                # macro does not use the module prefix
                logging.error(f"'{node.Name}' is missing the prefix.")
                logging.debug(f"Done for '{node.Name}'.\n")
                return False
            if node.Name == node.Name.upper():
                logging.debug(f"'{node.Name}' uses a valid name.")
                logging.debug(f"Done for '{node.Name}'.\n")
                return True
            # we start with the prefix, but the macro is not all uppercase,
            # so maybe we appended a valid suffix
            for suffix in VALID_SUFFIXES:
                if node.Name.endswith(f"_{suffix}"):
                    # remove the suffix, then we need to be all uppercase
                    # we need to subtract additionally -1 because of the underscore
                    without_suffix = node.Name[: len(node.Name) - len(suffix) - 1]
                    if without_suffix == without_suffix.upper():
                        logging.debug(f"Done for '{node.Name}'.\n")
                        return True
    logging.error(f"'{node.Name}' uses an invalid macro name.")
    logging.debug(f"Done for '{node.Name}'.\n")
    logging.getLogger().setLevel(logging.DEBUG)  # reset logging level
    return False


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
    try:  # TODO: will not work if suffix contains underscore
        function_name_suffix = base_name_splitted[1]
    except IndexError:
        function_name_suffix = None

    idx = []
    for i, val in enumerate(function_name):
        if val.isupper():
            idx.append(i)

    valid_pascal_case = True
    # any valid PascalCase word needs to start with an uppercase letter
    if not function_name[0].upper() == function_name[0]:
        valid_pascal_case = False
    for i in range(1, len(idx)):
        # if the difference between two indexes is 1, we are not seeing PascalCase
        if idx[i] - idx[i - 1] == 1:
            valid_pascal_case = False
            break

    valid_suffix = validate_suffix(function_name_suffix)
    return valid_pascal_case and valid_suffix


# pylint: disable=unused-argument
def check_parameter_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Checks that a parameter name is a camelName base name and an
    optional suffix that indicates the unit."""
    if not node.Name:
        return True
    base_name = str(node.Name)
    base_name_splitted = base_name.split("_", maxsplit=1)
    parameter_name = base_name_splitted[0]
    if not parameter_name:
        # a="_bla"; a.split("_"); ['', 'bla']
        return False

    try:  # TODO: will not work if suffix contains underscore
        parameter_name_suffix = base_name_splitted[1]
    except IndexError:
        parameter_name_suffix = None

    idx = []
    for i, val in enumerate(parameter_name):
        if val.isupper():
            idx.append(i)

    valid_camel_case = True
    # any valid camelCase word needs to start with an lowercase letter
    if not parameter_name[0].lower() == parameter_name[0]:
        valid_camel_case = False
    for i in range(1, len(idx)):
        # if the difference between two indexes is 1, we are not seeing camelCase
        if idx[i] - idx[i - 1] == 1:
            valid_camel_case = False
            break

    valid_suffix = validate_suffix(parameter_name_suffix)
    return valid_camel_case and valid_suffix


def check_global_variable_name(
    node: ir.Node, module_prefixes: ModulePrefixLookup
) -> bool:
    """Checks that a variable name is a camelName base name and an
    optional suffix that indicates the unit."""
    if not node.Name:
        return True
    prefix = None
    if node.Name:
        for file in containing_modules(node):
            if file in module_prefixes:
                prefix = module_prefixes[file].lower() + "_"
    if not prefix:  # something went wrong
        return False

    base_name = ""
    if node.Name.startswith(prefix):
        base_name = node.Name[len(prefix) :]
    else:
        return False

    base_name_splitted = base_name.split("_", maxsplit=1)
    variable_name = base_name_splitted[0]
    try:
        variable_name_suffix = base_name_splitted[1]
    except IndexError:
        variable_name_suffix = None

    idx = []
    for i, val in enumerate(variable_name):
        if val.isupper():
            idx.append(i)

    valid_camel_case = True
    # any valid camelCase word needs to start with an lowercase letter
    if not variable_name[0].lower() == variable_name[0]:
        valid_camel_case = False
    for i in range(1, len(idx)):
        # if the difference between two indexes is 1, we are not seeing camelCase
        if idx[i] - idx[i - 1] == 1:
            valid_camel_case = False
            break

    valid_suffix = validate_suffix(variable_name_suffix)
    return valid_camel_case and valid_suffix


NAMING = "CodingStyle-Naming"
prefix_check = {
    "lower": (
        f"{NAMING}.GlobalVariable",
        f"{NAMING}.LocalVariable",
    ),
    "upper": (
        f"{NAMING}.Function",
        f"{NAMING}.TypedefedFuncPtr",
        f"{NAMING}.TypedefedEnum",
        f"{NAMING}.TypedefedStruct",
    ),
}

for casing, applicable_rules in prefix_check.items():
    for rule in applicable_rules:
        RULE_TYPE = get_rule_type_from_name(rule)
        error_message = f"Please use {casing}case module prefix for {RULE_TYPE}."
        ANALYSIS[rule].additional_checks.extend(
            [
                (check_prefix_known, UNKNOWN_PREFIX_ERROR_MESSAGE),
                (globals()[f"check_{casing}case_prefix"], error_message),
            ]
        )

FUNCTION_NAME_ERROR_MESSAGE = "The function name is not 'PascalCase'."
GLOBAL_VARIABLE_ERROR_MESSAGE = "The global variable name is not 'camelCase'."
MACRO_ERROR_MESSAGE = "The macro name is not 'ALL_CAPS'."
PARAMETER_ERROR_MESSAGE = "The parameter name is not 'camelCase'."
STRUCT_MEMBER_NAME_ERROR_MESSAGE = "The struct member name is not 'camelCase'."
naming_errors = {
    "Function": (check_function_name, FUNCTION_NAME_ERROR_MESSAGE),
    "GlobalVariable": (check_global_variable_name, GLOBAL_VARIABLE_ERROR_MESSAGE),
    "Macro": (check_macro_name, MACRO_ERROR_MESSAGE),
    "Parameter": (check_parameter_name, PARAMETER_ERROR_MESSAGE),
    # struct member and parameter follow the same convention
    "StructField": (check_parameter_name, STRUCT_MEMBER_NAME_ERROR_MESSAGE),
}

for applicable_rule, handling in naming_errors.items():
    ANALYSIS[f"{NAMING}.{applicable_rule}"].additional_checks.append(handling)


# Only check function pointers with rule CodingStyle-Naming.TypedefedFuncPtr
def is_function_pointer_type(type_node: ir.Node) -> bool:
    """Filter function to only check function pointers"""
    original_type = type_node.skip_typedefs(True)
    if original_type.is_of_type("Pointer_Type"):
        element_type = original_type.Pointed_To_Type.skip_typedefs(True)
        return element_type.is_of_type("Routine_Type")
    return False


ANALYSIS[f"{NAMING}.TypedefedFuncPtr"].filter_predicate = is_function_pointer_type
