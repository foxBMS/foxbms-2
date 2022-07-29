# Copyright (c) 2022, Axivion GmbH
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


import itertools
import typing
from enum import Enum, auto

from bauhaus import analysis, ir
from bauhaus.analysis.config import SearchPattern
from bauhaus.ir.common.algorithms import symbol_declarations
from bauhaus.ir.common.output import unparse
from bauhaus.ir.common.scanner import comments


class LocationSpecifier(Enum):
    at_definition = auto()
    """The Doxygen comment shall be located before the definition."""
    at_any_declaration = auto()
    """The Doxygen comment shall be located before any (of potentially many) declaration of the entity.
    This includes the definition."""
    at_all_declarations = auto()
    """A Doxygen comment shall be located before all declarations of the entity or the definition
    if that is the only declaration."""
    at_all_decls_and_defs = auto()
    """A Doxygen comment shall be located before all declarations and definitions of the entity."""


@analysis.rule("IISB-DoxygenCommentAtDeclaration")
class DoxygenCommentRule(analysis.AnalysisRule):
    title = """Place Doxygen comments above declarations or definitions of entities."""

    languages = {analysis.Language.C, analysis.Language.CPP}

    node_types: typing.Dict[ir.LIR_Class_Name, LocationSpecifier] = {
        "Composite_Type": LocationSpecifier.at_definition,
        "Routine": LocationSpecifier.at_any_declaration,
        "General_Typedef_Type": LocationSpecifier.at_any_declaration,
        "Global_Static_Variable": LocationSpecifier.at_any_declaration,
        "Global_Normal_Variable": LocationSpecifier.at_any_declaration,
    }
    """Logical IR node types to check for preceding Doxygen comment. The comment shall be placed
    before the entities specified in the mapping."""

    doxygen_start: typing.Set[str] = {"/**", "///"}
    """Start of a valid Doxygen comment."""

    ignore_redefinitions = False
    """If True, method redefinitions are not checked as they can 'inherit'
    the comment from the redefined method."""

    ignore_deleted = False
    """If True, deleted function declarations are not checked for comments."""

    ignore_defaulted = False
    """If True, defaulted function declarations are not checked for comments."""

    allow_inherited = False
    """
    If True, a definition does not need documentation, if a corresponding
    declaration is documented. This includes definitions of virtual member function.
    """

    allow_missing_documentation_on_private = False
    """If True, a class-member definition does not need documentation, if it
    is `private`."""

    allow_missing_documentation_on_protected = False
    """If True, a class-member definition does not need documentation, if it
    is `protected`."""

    ignore_tool_comments: typing.Optional[SearchPattern] = None
    """
    An optional regular expression. Comments where this regex finds a matching
    substring are ignored in the search for a doxygen comment (e.g. control-comments of other tools).
    """

    _rule_description = """
        <p>Requires doxygen comments to appear before certain source
        code entities.</p>
        <p>The set of entities that should be documented can be controlled
        with the attribute <code>node_types</code>. A set of LIR node types can be selected
        and the places where the comment should be placed (before any declaration, before
        all declarations, before the definition or before all of the beforementioned). See
        configuration attribute <code>node_types</code>.</p>
    """

    _message_descriptions = {
        "comment_missing": "Need Doxygen comment for this entity.",
        "comment_potentially_missing": "Need Doxygen comment for any declaration of this entity.",
    }

    def is_allowed_comment(self, text):
        return text.startswith(tuple(self.doxygen_start))

    def is_relevant_node(self, lir_node):
        # for functions, optionally ignore = delete ones
        if lir_node.is_of_type("Routine") and lir_node.Attributes:
            if self.ignore_deleted and lir_node.Attributes.is_deleted:
                return False

            if self.ignore_defaulted and lir_node.Attributes.is_defaulted:
                return False

        if lir_node.is_of_type("Member"):
            if (
                self.allow_missing_documentation_on_protected
                and lir_node.Visibility == ir.visibility_protected
            ):
                return False

            if (
                self.allow_missing_documentation_on_private
                and lir_node.Visibility == ir.visibility_private
            ):
                return False

        # for functions, optionally allow inheriting from any declaration
        if lir_node.is_of_type("Routine") and self.allow_inherited:
            candidates = itertools.chain(
                lir_node.Decls_And_Defs,
                (
                    itertools.chain.from_iterable(
                        n.Decls_And_Defs for n in lir_node.Redefines
                    )
                    if lir_node.is_of_type("Method")
                    else ()
                ),
            )
            if any(
                comments.has_comment_before(
                    n,
                    self.is_allowed_comment,
                    ignore_matching=self.ignore_tool_comments,
                )
                for n in candidates
            ):
                return False

        # for method decl/def, optionally allow comment from redefined base
        # method
        if (
            lir_node.is_of_type("Method")
            and self.ignore_redefinitions
            and lir_node.Redefines
        ):
            return False
        return True

    def execute(self, ir_graph):
        for node_types, check in self.node_types.items():
            for lir_node in ir_graph.nodes_of_type(ir.Logical, node_types):
                pir_node = lir_node.Physical
                if (
                    pir_node
                    and lir_node
                    and not pir_node.Artificial
                    and not pir_node.In_Template_Instance
                    and self.is_relevant_node(lir_node)
                ):
                    all_required = list()
                    one_of = list()
                    if len(lir_node.Declarations) == 0:
                        all_required.extend(lir_node.Definitions)
                    else:
                        if check == LocationSpecifier.at_definition:
                            all_required.extend(lir_node.Definitions)
                        elif check == LocationSpecifier.at_any_declaration:
                            one_of.extend(lir_node.Declarations)
                        elif check == LocationSpecifier.at_all_declarations:
                            all_required.extend(lir_node.Declarations)
                        elif check == LocationSpecifier.at_all_decls_and_defs:
                            all_required.extend(lir_node.Decls_And_Defs)
                        else:
                            assert False

                    secondaries = symbol_declarations.sort_by_position(
                        one_of + all_required
                    )
                    if not any(
                        comments.has_comment_before(
                            n,
                            self.is_allowed_comment,
                            ignore_matching=self.ignore_tool_comments,
                        )
                        for n in one_of
                    ):
                        for primary in one_of:
                            self.add_message(
                                msg_key="comment_potentially_missing",
                                entity=unparse.entity(primary),
                                primary_sloc=primary,
                                secondary_slocs=filter(
                                    lambda n: n != primary, secondaries
                                ),
                            )
                    if not all(
                        comments.has_comment_before(
                            n,
                            self.is_allowed_comment,
                            ignore_matching=self.ignore_tool_comments,
                        )
                        for n in all_required
                    ):
                        for primary in all_required:
                            self.add_message(
                                msg_key="comment_missing",
                                entity=unparse.entity(primary),
                                primary_sloc=primary,
                                secondary_slocs=filter(
                                    lambda n: n != primary, secondaries
                                ),
                            )
