#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

# TODOs:
# - variables can be pointers/const etc.
# - struct fields can be pointers/const etc.
# - parameter check needs to be refactored

import logging
import re
import sys
from pathlib import Path
from typing import Optional, Literal


# disable import checking for Python modules that come with Axivion
# pylint: disable=import-error
import axivion.config  # pylint: disable=no-name-in-module
from bauhaus import ir
from bauhaus import style
from bauhaus.ir.common.scanner import comments
from bauhaus.ir.common.output.unparse import unparse_type

# pylint: enable=import-error

DatabaseVariable = Literal["database", "other"]

if "perform_tests" in sys.argv[0]:
    logging.basicConfig(format="%(levelname)s:%(message)s", level=logging.DEBUG)
    logging.getLogger().setLevel(logging.DEBUG)


ANALYSIS = axivion.config.get_analysis()  # pylint: disable=c-extension-no-member


@style.workitem(inputs=(ir.Graph, comments.scan_and_cache_comments), repeat=True)
class ModulePrefixLookup(style.WorkItem):  # pylint: disable=abstract-method
    """Fetches the module prefix based on a regex from the provided source file"""

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


# foxBMS specific check implementations


def check_prefix_known(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Check that a module prefix exists for the current node."""
    if node.Name:
        for file in containing_modules(node):
            if file in module_prefixes:
                return True
        return False
    return True


VALID_SUFFIXES = (
    "degC",  # degree Celsius
    "ddegC",  # deci degree Celsius
    "dK",  # deci Kelvin
    "ohm",  # ohm
    "kOhm",  # kilo ohm
    "kHz",  # kilo Hertz
    "ms",  # milliseconds
    "us",  # microseconds
    "perc",  # percentage
    "perm",  # per mill
    "mV",  # millivolt
    "V",  # volt
    "mA",  # milliampere
    "A",  # ampere
    "mAs",  # milliampere seconds
    "mAh",  # milliampere hours
    "As",  # ampere seconds
    "W",  # watt
    "Wh",  # watt hours
    "t",  # "typedef"
    "Hz",  # Hertz
)


def validate_suffix(suffix: str = ""):
    """Validate used suffix against list of permitted suffixes"""
    if not suffix:
        return True
    if suffix in VALID_SUFFIXES:
        return True
    return False


def remove_prefix(name: str, prefix: str | tuple[str,]) -> str:
    """Removes a prefix from a given name."""
    if isinstance(prefix, str):
        return name[len(prefix) :]
    if isinstance(prefix, tuple):
        for i in prefix:
            if name.startswith(i):
                return name[len(i) :]
    raise RuntimeError(
        f"Could not remove prefix '{prefix}', as it does not exist for '{name}'."
    )


def get_prefix_for_node(
    node: ir.Node, module_prefixes: ModulePrefixLookup
) -> Optional[tuple[str,]]:
    """gets"""
    prefix: tuple[str] = ()
    for file in containing_modules(node):
        if file in module_prefixes:
            prefix = (module_prefixes[file].upper() + "_",)
    return prefix


def split_name_and_unit(name: str) -> tuple[str, str]:
    """Splits a name at the underscore to determine name and unit (if existing)"""
    # foxBMS coding conventions do not use underscores (except for the prefix),
    # therefore its safe to split on underscore and then assume that the first
    # element is the name and the second part if the unit.
    if name.endswith(tuple("_" + i for i in VALID_SUFFIXES)):
        for i in VALID_SUFFIXES:
            tmp = name.rsplit("_" + i, maxsplit=1)
            if len(tmp) == 2:
                base_name = tmp[0]
                # replace the base_name from the name, then we only have left
                # _<suffix>; remove the leading underscore, then we have just
                # the suffix
                suffix = name.replace(tmp[0], "", 1)[1:]
                return (base_name, suffix)
    return (name, "")


def is_camel_case(name: str) -> bool:
    """Checks whether a string is camelCase or not"""
    # any valid camelCase word needs to start with an lowercase letter
    if not name[0].lower() == name[0]:
        return False

    idx = []
    for i, val in enumerate(name):
        if val.isupper():
            idx.append(i)

    for i in range(1, len(idx)):
        # if the difference between two indexes is 1, we are not seeing camelCase
        if idx[i] - idx[i - 1] == 1:
            return False

    # if we come here, the name is camelCase
    return True


def is_pascal_case(name: str) -> bool:
    """Checks whether a string is PascalCase or not"""
    # any valid PascalCase word needs to start with an uppercase letter
    if not name[0].upper() == name[0]:
        return False

    idx = []
    for i, val in enumerate(name):
        if val.isupper():
            idx.append(i)
    for i in range(1, len(idx)):
        # if the difference between two indexes is 1, we are not seeing PascalCase
        if idx[i] - idx[i - 1] == 1:
            return False

    # if we come here, the name is PascalCase
    return True


def valid_database_variable(name: str, db_type: DatabaseVariable) -> bool:
    """Database variables shall always start with 'table'."""
    if db_type == "database":
        start = "block"
    elif db_type == "other":
        start = "table"
    else:
        raise RuntimeError("Something went really wrong.")
    if not name.startswith(start):
        return False
    return True


# pylint: disable-next=unused-argument
def check_enumerator_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Check that all enumerators inside an enum use the module prefix and are
    all uppercase"""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False

    # Enumerator prefixes need to be uppercase
    prefix = tuple(i.upper() for i in prefix)
    if not name.startswith(prefix):
        return False

    # we have a name, that starts with a valid prefix, remove the prefix and
    # check rest of the name
    name = remove_prefix(name, prefix)

    name, unit = split_name_and_unit(name)

    if not validate_suffix(unit):  # Enumerator uses an invalid unit as suffix
        return False

    # at this point, 'name' must only consist of alphanumerics and underscores
    if not name.replace("_", "").isalnum():
        return False

    # Finally, enumerator names need to be entirely uppercase and this is the
    # last check to be performed and we can exit with this checks return value
    return name.isupper()


def check_function_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Checks that a function name is PascalCase and uses a valid prefix."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False

    # Now we have the default prefix
    # If its a test file, the prefix 'test_' is also fine, as Ceedling requires
    # a 'test' prefix.
    # If its not a test file, than it's a source file; source files may
    # externalize static functions for  unit testing, then the prefix is
    # 'TEST_<PREFIX>_'
    if node.Physical.SLoc.Full_Name.startswith("test_"):
        prefixes = prefix + ("test",)
    else:
        prefixes = prefix + (f"TEST_{prefix[0]}",)
    if not name.startswith(prefixes):
        return False

    # we have a name, that starts with a valid prefix, remove the prefix and
    # check rest of the name
    name = remove_prefix(name, prefixes)

    # at this point, 'name' must only consist of alphanumerics as we have
    # removed the prefix (that includes an underscore)
    if not name.isalnum():
        return False

    # Finally, function names need to be PascalCase and this is the last check
    # to be performed and we can exit with this checks return value
    return is_pascal_case(name)


# pylint: disable-next=too-many-return-statements
def check_global_variable_name(
    node: ir.Node, module_prefixes: ModulePrefixLookup
) -> bool:
    """Checks that a variable name is a camelName base name and an
    optional suffix that indicates the unit."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False

    # global variable prefixes need to be lowercase
    prefix = tuple(i.lower() for i in prefix)
    if not name.startswith(prefix):
        return False

    # we have a name, that starts with a valid prefix, remove the prefix and
    # check rest of the name
    name = remove_prefix(name, prefix)

    name, unit = split_name_and_unit(name)

    if not validate_suffix(unit):  # Variable uses an invalid unit as suffix
        return False

    # at this point, 'name' must only consist of alphanumerics as we have
    # removed the prefix (that includes an underscore) and the suffix (which
    # could tool include a suffix)
    if not name.isalnum():
        return False

    # data blocks follow a specific pattern
    if unparse_type(node.Its_Type).startswith("DATA_BLOCK_"):
        if "data_" in prefix:  # prefixes are here already lowercase
            start = "database"
        else:
            start = "other"
        if not valid_database_variable(name, start):
            return False

    # Finally, global variable names need to be camelCase and this is the last
    # check to be performed and we can exit with this checks return value
    return is_camel_case(name)


# pylint: disable-next=too-many-return-statements
def check_local_variable_name(
    node: ir.Node, module_prefixes: ModulePrefixLookup
) -> bool:
    """Checks that a variable name is a camelName base name and an
    optional suffix that indicates the unit."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False
    # we need to distinguish between static and normal variables within the
    # function
    require_prefix = False
    if node.type() == "Local_Static_Regular_Variable":
        require_prefix = True

    if require_prefix:
        # local static variable prefixes need to be lowercase
        prefix = tuple(i.lower() for i in prefix)
        if not name.startswith(prefix):
            return False

        # we have a name, that starts with a valid prefix, remove the prefix
        # and check rest of the name
        name = remove_prefix(name, prefix)

    name, unit = split_name_and_unit(name)

    if not validate_suffix(unit):  # Variable uses an invalid unit as suffix
        return False

    # at this point, 'name' must only consist of alphanumerics as we have
    # removed the prefix (that includes an underscore) and the suffix (which
    # could tool include a suffix)
    if not name.isalnum():
        return False

    # data blocks follow a specific pattern
    if unparse_type(node.Its_Type).startswith("DATA_BLOCK_"):
        if "data_" in prefix:  # prefixes are here already lowercase
            start = "database"
        else:
            start = "other"
        if not valid_database_variable(name, start):
            return False

    # Finally, all local variable names need to be camelCase and this is the
    # last check to be performed and we can exit with this checks return value
    return is_camel_case(name)


# pylint: disable-next=too-many-return-statements
def check_macro_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Check that the macro starts with the upper-case module prefixed followed by
    an underscore (exception: include guard macro 'FOXBMS__{FILENAME}_H_')."""
    # ignore compiler builtins
    if (
        not node.type() == "Predefined_Object_Macro_Definition"
        and not node.Name.startswith("__")
    ):
        logging.debug("node.Name: %s", node.Name)
    else:
        return True

    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    temp = str(node.Original_Position).rsplit(":", maxsplit=2)[0]
    # Check if we found a define guard
    guard = "".join(ch.upper() if ch.isalnum() else "_" for ch in Path(temp).stem)
    if node.Name == f"FOXBMS__{guard}_H_":
        return True

    # it's not a define guard, so we need to check the details

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False

    # macro prefixes need to be uppercase
    prefix = tuple(i.upper() for i in prefix)
    if not name.startswith(prefix):
        return False

    # we have a name, that starts with a valid prefix, remove the prefix and
    # check rest of the name
    name = remove_prefix(name, prefix)

    name, unit = split_name_and_unit(name)

    if not validate_suffix(unit):  # Macro uses an invalid unit as suffix
        return False

    # at this point, 'name' must only consist of alphanumerics and underscores
    if not name.replace("_", "").isalnum():
        return False

    # Finally, macro names need to be entirely uppercase and this is the
    # last check to be performed and we can exit with this checks return value
    return name.isupper()


# pylint: disable-next=unused-argument
def check_parameter_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Checks that a parameter name is a camelName base name and an
    optional suffix that indicates the unit."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    # check for const and pointer prefixes
    expected_prefix = ""
    node_type: str = unparse_type(node.Its_Type)

    for i in node_type.split()[::-1]:  # Clockwise/Spiral Rule
        if i.endswith("*"):
            expected_prefix += "p"
        if i == "const":
            expected_prefix += "k"
    base_name = str(node.Name)
    base_name_splitted = base_name.split("_", maxsplit=1)
    parameter_name = base_name_splitted[0]
    if not parameter_name:
        # a="_bla"; a.split("_"); ['', 'bla']
        return False
    valid_indicator = True
    if not parameter_name.startswith(expected_prefix):
        valid_indicator = False
    if expected_prefix:
        try:
            # if we can not index, the name is invalid in any case
            first_letter_after_prefix = parameter_name[len(expected_prefix)]
        except IndexError:
            return False
        # 'pabc' would otherwise be valid, but it needs to be 'pAbc'
        if not first_letter_after_prefix == first_letter_after_prefix.upper():
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
    return all([valid_indicator, valid_camel_case, valid_suffix])


# pylint: disable-next=unused-argument
def check_struct_field_name(node: ir.Node, module_prefixes: ModulePrefixLookup) -> bool:
    """Checks that..."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    name, unit = split_name_and_unit(name)

    if not validate_suffix(unit):  # Variable uses an invalid unit as suffix
        return False

    # at this point, 'name' must only consist of alphanumerics as we have
    # removed the prefix (that includes an underscore) and the suffix (which
    # could tool include a suffix)
    if not name.isalnum():
        return False

    # Finally, all local variable names need to be camelCase and this is the
    # last check to be performed and we can exit with this checks return value
    return is_camel_case(name)


def check_typedefed_enum_name(
    node: ir.Node, module_prefixes: ModulePrefixLookup
) -> bool:
    """Check that all typedefed enums use the module prefix, are
    all uppercase, and end with '_e'."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False

    if not name.startswith(prefix):
        return False

    if not name.endswith("_e"):
        return False

    if not name[:-2].isupper():
        return False

    return True


def check_typedefed_struct_name(
    node: ir.Node, module_prefixes: ModulePrefixLookup
) -> bool:
    """Check that all typedefed strucs use the module prefix, are
    all uppercase, and end with '_s'."""
    name: str = node.Name
    if not name:  # we only need to do something, when we have any name
        return True

    prefix = get_prefix_for_node(node, module_prefixes)
    if not prefix:  # something went really wrong; prefix must be defined
        return False

    if not name.startswith(prefix):
        return False

    if not name.endswith("_s"):
        return False

    if not name[:-2].isupper():
        return False

    return True


UNKOWN_PREFIX = "Unknown which module prefix to check."
ERROR_MESSAGES = {
    "Enumerator": (
        "Enumerator names shall (1) use the uppercase module prefix followed "
        "by an underscore, (2) be all uppercase, (3) separate words by "
        "underscores, and optional (4) use a phyiscal unit as suffix."
    ),
    "Function": (
        "Function names shall (1) use the uppercase module prefix followed by "
        "an underscore and then (2) use PascalCase."
    ),
    "GlobalVariable": (
        "Global variables names shall (1) use the lowercase module prefix "
        "followed by an underscore and then (2) use camelCase, and optional "
        "(3) use a phyiscal unit as suffix."
    ),
    "LocalVariable": (
        "Local variables names shall (1) use the lowercase module prefix "
        "followed by an underscore if they are static otherwise omit the "
        "prefix and then (2) use camelCase, and optional (3) use a phyiscal "
        "unit as suffix."
    ),
    "Macro": (
        "Macro names shall (1) use the uppercase module prefix followed by an "
        "underscore, (2) be all uppercase, (3) separate words by underscores, "
        "and optional (4) use a phyiscal unit as suffix."
    ),
    "Parameter": (
        "Paramater names shall (1) use camelCase, and optional (2) use a "
        "phyiscal unit as suffix."
    ),
    "StructField": (
        "Struct field names shall (1) use camelCase, and optional (2) use a "
        "phyiscal unit as suffix."
    ),
    "TypedefedEnum": (
        "Typedefed enum names shall (1) use the uppercase module prefix "
        "followed by an underscore, (2) be all uppercase, (3) separate words "
        "by underscores, and (3) end with '_e'."
    ),
    "TypedefedStruct": (
        "Typedefed struct names shall (1) use the uppercase module prefix "
        "followed by an underscore, (2) be all uppercase, (3) separate words "
        "by underscores, and (3) end with '_s'."
    ),
}


naming_errors = {
    "Enumerator": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_enumerator_name, ERROR_MESSAGES["Enumerator"]),
    ],
    "Function": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_function_name, ERROR_MESSAGES["Function"]),
    ],
    "GlobalVariable": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_global_variable_name, ERROR_MESSAGES["GlobalVariable"]),
    ],
    "LocalVariable": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_local_variable_name, ERROR_MESSAGES["LocalVariable"]),
    ],
    "Macro": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_macro_name, ERROR_MESSAGES["Macro"]),
    ],
    "Parameter": [(check_parameter_name, ERROR_MESSAGES["Parameter"])],
    "StructField": [(check_struct_field_name, ERROR_MESSAGES["StructField"])],
    "TypedefedEnum": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_typedefed_enum_name, ERROR_MESSAGES["TypedefedEnum"]),
    ],
    "TypedefedStruct": [
        (check_prefix_known, UNKOWN_PREFIX),
        (check_typedefed_struct_name, ERROR_MESSAGES["TypedefedStruct"]),
    ],
}


NAMING = "CodingStyle-Naming"
for applicable_rule, handling in naming_errors.items():
    ANALYSIS[f"{NAMING}.{applicable_rule}"].additional_checks.extend(handling)


# Only check function pointers with rule CodingStyle-Naming.TypedefedFuncPtr
def is_function_pointer_type(type_node: ir.Node) -> bool:
    """Filter function to only check function pointers"""
    original_type = type_node.skip_typedefs(True)
    if original_type.is_of_type("Pointer_Type"):
        element_type = original_type.Pointed_To_Type.skip_typedefs(True)
        return element_type.is_of_type("Routine_Type")
    return False


ANALYSIS[f"{NAMING}.TypedefedFuncPtr"].filter_predicate = is_function_pointer_type
