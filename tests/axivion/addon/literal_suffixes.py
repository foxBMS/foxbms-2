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


import re
import typing

from bauhaus import analysis, ir
from bauhaus.ir.common.preprocessor import preprocessor_ifs
from bauhaus.ir.common.scanner import literals
from bauhaus.ir.common.types import integer_types

from . import iisb_base


@analysis.rule("IISB-LiteralSuffixesCheck")
class IisbLiteralSuffixesRule(iisb_base.IISBRule, analysis.AnalysisRule):
    title = """Force Source Files to be decodable with specific encoding."""

    _message_descriptions = {
        "expect_no_literal_suffix": "Literal constant should have no suffix",
        "unexpected_int_literal_suffix": 'Integer literal constant of size {} requires "{}" suffix',
        "unexpected_float_literal_suffix": 'Floating point literal constant of size {} requires "{}" suffix',
        "wrong_casing_in_literal_suffix": 'Literal constant suffix has wrong casing, please use "{}"',
    }

    _rule_description = """
        <p>Force literal suffixes for numerical assignment
         (force <code>LL</code> for <code>int64</code> and force it to be uppercase,
          force lowercase <code>u</code> for unsigned, force lowercase <code>f</code> for float, …)</p>
        <ul>
          <li><code>int64_t</code>: <code>LL</code></li>
          <li><code>uint</code><emph>X</emph><code>_t</code>: <code>u</code></li>
          <li><code>uint64_t</code>: <code>uLL</code></li>
          <li><code>float</code>: <code>f</code></li>
        </ul>
    """

    consider_int_sign_conversion = False
    """Selects whether literals with an implicit conversion from signed to unsigned are reported."""

    consider_pp_expressions = False
    """Whether literals in #if conditions are checked."""

    unsigned_int_suffixes: typing.Dict[int, str] = {8: "u", 16: "u", 32: "u", 64: "uLL"}
    """Mapping of bit-size of unsigned int types to expected suffixes for literals
    of the corresponding type."""

    signed_int_suffixes: typing.Dict[int, str] = {8: "", 16: "", 32: "", 64: "LL"}
    """Mapping of bit-size of signed int types to expected suffixes for literals
    of the corresponding type."""

    float_suffixes: typing.Dict[int, str] = {32: "f", 64: ""}
    """Mapping of bit-size of floating point types to expected suffixes for literals
    of or immediately converted to the corresponding type."""

    def check_suffix(
        self, node: ir.Node, suffix: str, expected_suffix: str, size: int, type_str: str
    ):
        if suffix != expected_suffix:
            if expected_suffix == "":
                self.add_message(
                    msg_key="expect_no_literal_suffix",
                    entity=node.External_Form,
                    primary_sloc=node,
                )
            elif suffix.lower() != expected_suffix.lower():
                self.add_message(
                    msg_key=f"unexpected_{type_str}_literal_suffix",
                    message_arguments=(size, expected_suffix),
                    entity=node.External_Form,
                    primary_sloc=node,
                )
            else:
                self.add_message(
                    msg_key="wrong_casing_in_literal_suffix",
                    message_arguments=(expected_suffix,),
                    entity=node.External_Form,
                    primary_sloc=node,
                )

    _hex_literal_pattern: typing.Pattern[str] = re.compile(
        (
            r"("
            r"(0[xX](?P<hex_significand>['0-9A-Fa-f]+(\.['0-9A-Fa-f]+)?)(?P<hex_exponent>[pP][-+]?['0-9]+))"
            r"|"
            r"((?P<decimal_significand>['0-9]+(\.['0-9]+)?)(?P<decimal_exponent>[eE][-+]?['0-9]+)?)"
            r")"
            r"(?P<suffix>[fFlL])?"
        )
    )

    def get_floating_point_suffix(self, text: str):
        match = self._hex_literal_pattern.fullmatch(text.strip())
        if match:
            suffix = match["suffix"]
            if suffix is not None:
                return suffix
        return ""

    def execute(self, ir_graph: ir.Graph):
        for node in ir_graph.nodes_of_type(
            ir.Physical,
            "Floating_Point_Literal",
            lambda n: n.Position and n.External_Form,
        ):
            suffix = self.get_floating_point_suffix(node.External_Form)
            its_type = node.Its_Type.skip_typedefs(True)
            if node.Parent.is_of_type("Conversion"):
                parent_type = node.Parent.Its_Type.skip_typedefs(True)
                if parent_type.is_of_type("Floating_Point_Type"):
                    its_type = parent_type
            size = its_type.Size_In_Bits
            self.check_suffix(
                node=node,
                suffix=suffix,
                expected_suffix=self.float_suffixes.get(size, ""),
                size=size,
                type_str="float",
            )
        for node in ir_graph.nodes_of_type(
            ir.Physical,
            "Integer_Literal",
            lambda n: (
                n.Position
                and n.External_Form
                and (self.consider_pp_expressions or not preprocessor_ifs.is_pp_expr(n))
            ),
        ):
            suffix = literals.get_suffix(node)
            its_type = node.Its_Type.skip_typedefs(True)
            if (
                not integer_types.is_unsigned(its_type)
                and (
                    self.consider_int_sign_conversion
                    and node.Parent.is_of_type("Conversion")
                    and integer_types.is_unsigned(node.Parent.Its_Type)
                )
                and not node.Parent.is_of_type("Bit_Field_Definition")
            ):
                its_type = node.Parent.Its_Type.skip_typedefs(True)
            size = its_type.Size_In_Bits
            self.check_suffix(
                node=node,
                suffix=suffix,
                expected_suffix=(
                    self.unsigned_int_suffixes.get(size, "")
                    if integer_types.is_unsigned(its_type)
                    else self.signed_int_suffixes.get(size, "")
                ),
                size=size,
                type_str="int",
            )
