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


import typing
from collections import defaultdict
from pathlib import Path

from bauhaus import analysis, ir
from bauhaus.ir.common.algorithms import symbol_declarations
from bauhaus.ir.common.output import unparse

from . import iisb_base


@analysis.rule("IISB-UniqueFileNameCheck")
class IisbUniqueFileNameRule(iisb_base.IISBRule, analysis.AnalysisRule):
    title = """Report equally named files in a project."""

    ignore_casing: bool = True
    """Treat file names that differ only in casing as equal"""

    _rule_description = """
        <p>No two user-defined files in a project should have the same base name.</p>
    """

    _message_descriptions = {
        "not_unique": "File names are too similar",
    }

    def execute(self, ir_graph: ir.Graph):
        all_files: typing.DefaultDict[str, typing.List[ir.Node]] = defaultdict(list)
        for file in ir_graph.nodes_of_type(ir.Physical, "File"):
            name = Path(file.Full_Name).name
            if self.ignore_casing:
                name = name.lower()
            all_files[name].append(file)
        for bucket in filter(lambda b: len(b) > 1, all_files.values()):
            bucket = list(symbol_declarations.sort_by_position(bucket))
            for i, file in enumerate(bucket):
                # do not report system include files as primary sloc as we probably cannot change them
                if file.is_of_type(
                    ("Primary_File", "Preinclude_File", "User_Include_File")
                ):
                    self.add_message(
                        msg_key="not_unique",
                        primary_sloc=file,
                        secondary_slocs=bucket[:i] + bucket[i + 1 :],
                        entity=unparse.entity(file),
                    )
