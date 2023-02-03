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


import io
import re
import typing

from bauhaus import analysis, ir
from bauhaus.ir.common.files import files_by_language
from bauhaus.ir.common.scanner import comments
from bauhaus.shared import MessageSLoc
from bauhaus.style import excludes

from . import iisb_base


@analysis.rule("IISB-FileLicenseComment")
class IisbFileLicenseRule(iisb_base.IISBRule, analysis.AnalysisRule):
    title = """Verify license comments in files."""

    file_node_kinds: typing.Set[ir.PIR_Class_Name] = {
        "Primary_File",
        "User_Include_File",
    }

    license_comment: typing.List[typing.Pattern[str]] = list(
        re.compile(re.escape(s))
        for s in """/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */""".splitlines(
            keepends=False
        )
    )

    _rule_description = """
        <p>Every file must start with a license header comment, see <code>license_comment</code>.
         Exceptions can be configured in the attribute <code>alternative_licenses</code>.</p>
    """

    _message_descriptions = {
        "no_license_comment": "No license header found",
        "license_incorrect": "License header is incorrect",
        "license_short": "License header is too short",
    }

    def get_inputs(self, ctx):
        return ir.Graph, comments.comment_cache_work_item(ctx)

    @staticmethod
    def header_comments(
        ir_graph: ir.Graph,
        file_node_kinds: typing.Iterable[ir.PIR_Class_Name],
        lang: excludes.Language,
        cached_comments: typing.Dict[ir.Node, typing.List[comments.CachedComment]],
    ) -> typing.Iterable[
        typing.Tuple[
            ir.Node,
            typing.Optional[comments.CachedComment],
            typing.Optional[comments.CachedComment],
        ]
    ]:
        selected_files = set()
        if lang:
            selected_files = set(
                node.Parent
                for node in files_by_language.files_by_language(ir_graph, lang)
            )

        def is_of_language(filenode):
            return not lang or filenode in selected_files

        if cached_comments is not None:
            for src_file in cached_comments:
                if src_file.is_of_type(file_node_kinds) and is_of_language(src_file):
                    for comment in cached_comments[src_file]:
                        if comment.at_beginning:
                            yield src_file, comment
                            break
                    else:
                        # have no comment in src_file
                        yield src_file, None

    def execute(self, ir_graph, cached_comments):
        for src_file, license_comment in self.header_comments(
            ir_graph=ir_graph,
            file_node_kinds=self.file_node_kinds,
            lang=self.languages,
            cached_comments=cached_comments,
        ):
            if license_comment is None:
                self.add_message(msg_key="no_license_comment", primary_sloc=src_file)
            else:
                _ok = self.verify_license(comment=license_comment)

    def verify_license(self, comment: comments.CachedComment) -> bool:
        comment_stream = io.StringIO(initial_value=comment.Value)
        line_no = 0
        pattern_no = -1
        for line_str in comment_stream:
            if line_no == 0:
                line_no = comment.line()
                column_no = comment.column()
            else:
                line_no += 1
                column_no = 1
            pattern_no += 1
            line_str = line_str.splitlines(keepends=False)[0]
            match = self.license_comment[pattern_no].fullmatch(line_str)
            if not match:
                self.add_message(
                    msg_key="license_incorrect",
                    primary_sloc=MessageSLoc(
                        filename=comment.filename(), line=line_no, column=column_no
                    ),
                )
                pattern_no = None
                return False
        if pattern_no is not None and pattern_no + 1 < len(self.license_comment):
            self.add_message(
                msg_key="license_short",
                primary_sloc=MessageSLoc(
                    filename=comment.filename(),
                    line=comment.End_Line,
                    column=comment.End_Column,
                ),
            )
            return False
        return True
