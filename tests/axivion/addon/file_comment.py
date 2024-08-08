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


import dataclasses
import io
import re
import typing
from enum import Enum, auto

from bauhaus import analysis, ir
from bauhaus.ir.common.files import files_by_language
from bauhaus.ir.common.scanner import comments
from bauhaus.shared import MessageSLoc
from bauhaus.style import excludes

from . import iisb_base


class MatchResult(Enum):
    ok = auto()
    wrong_prefix = auto()
    wrong_tag = auto()
    wrong_value = auto()
    unmatched = auto()


@dataclasses.dataclass
class RequirementMatchResult:
    state: MatchResult = MatchResult.unmatched
    tag_index: typing.Optional[int] = None
    value_index: typing.Optional[int] = None
    continue_option: typing.Optional["LineRequirement"] = None

    def __bool__(self):
        return self.state == MatchResult.ok


@dataclasses.dataclass
class LineRequirement:
    tag_name: typing.Optional[str]
    match_tag: bool = True
    prefix: str = " *"
    pattern_min_index: typing.Optional[int] = None
    pattern: typing.Optional[typing.Pattern[str]] = None
    multiline: bool = False

    @classmethod
    def empty_line(cls) -> "LineRequirement":
        return LineRequirement(tag_name=None, prefix=" *", pattern=re.compile("\\s*"))

    def expected_arguments(self) -> typing.Dict[str, str]:
        return {
            "tag": self.tag_name if self.tag_name else "",
            "prefix": self.prefix if self.prefix else "",
            "pattern": self.pattern.pattern if self.pattern is not None else "",
        }

    def match(self, line: str) -> RequirementMatchResult:
        """Checks if line matches self's requirements. Returns a triple of match status,
        index in line where the pattern starts (if there is any) and optionally a continue pattern to match
        succeeding lines for a multiline pattern."""
        index = 0
        if not line.startswith(self.prefix):
            return RequirementMatchResult(state=MatchResult.wrong_prefix)
        index += len(self.prefix)
        line = line[len(self.prefix) :]
        while len(line) > 0 and line[0].isspace():
            index += 1
            line = line[1:]
        tag_start_index = index
        if self.tag_name is not None:
            if line.startswith(self.tag_name):
                index += len(self.tag_name)
                line = line[len(self.tag_name) :]
            elif self.match_tag:
                return RequirementMatchResult(
                    state=MatchResult.wrong_tag, tag_index=tag_start_index
                )
        while (
            len(line) > 0
            and line[0].isspace()
            and (self.pattern_min_index is None or index < self.pattern_min_index)
        ):
            index += 1
            line = line[1:]
        value_start_index = index
        # for multiline comments, create requirements for the next line containing
        # only a continuation of the value. Verify correct value_start_index!
        continue_option: typing.Optional["LineRequirement"] = (
            None
            if not self.multiline
            else LineRequirement(
                prefix=self.prefix,
                tag_name=self.tag_name,
                match_tag=False,
                pattern_min_index=value_start_index
                if self.pattern_min_index is None
                else self.pattern_min_index,
                pattern=self.pattern,
                multiline=True,
            )
        )
        if line == "" and self.multiline:
            # accept empty lines in multiline mode
            pass
        elif (
            line != ""
            and self.pattern_min_index is not None
            and value_start_index < self.pattern_min_index
        ):
            # need proper indent of the value to accept it
            return RequirementMatchResult(
                state=MatchResult.wrong_value,
                tag_index=tag_start_index,
                value_index=value_start_index,
                continue_option=continue_option,
            )
        elif self.pattern is not None:
            m = self.pattern.fullmatch(line)
            if not m:
                return RequirementMatchResult(
                    state=MatchResult.wrong_value,
                    tag_index=tag_start_index,
                    value_index=value_start_index,
                    continue_option=continue_option,
                )
        return RequirementMatchResult(
            state=MatchResult.ok,
            tag_index=tag_start_index,
            value_index=value_start_index,
            continue_option=continue_option,
        )


@dataclasses.dataclass
class LineDescription:
    sloc: MessageSLoc
    text: str
    pattern_no: typing.Optional[int] = None


@analysis.rule("IISB-DoxygenFileComment")
class IisbDoxygenFileCommentRule(iisb_base.IISBRule, analysis.AnalysisRule):
    title = """Verify header comments in files."""

    file_node_kinds: typing.Set[ir.PIR_Class_Name] = {
        "Primary_File",
        "User_Include_File",
    }

    _iso_date_pattern = r"(2[0-9]{3}-[01][0-9]-[0-3][0-9])"

    required_tags: typing.List[LineRequirement] = [
        LineRequirement(prefix="/**", tag_name=None),
        LineRequirement(tag_name="@file", pattern=re.compile(r"\S.*")),
        LineRequirement(tag_name="@author", pattern=re.compile(r"\S.*")),
        LineRequirement(
            tag_name="@date",
            pattern=re.compile(_iso_date_pattern + r"(:?|\s*\(date\s+of\s+creation\))"),
        ),
        LineRequirement(
            tag_name="@updated",
            pattern=re.compile(
                _iso_date_pattern + r"(:?|\s*\(date\s+of\s+last\s+update\))"
            ),
        ),
        LineRequirement(
            tag_name="@version",
            pattern=re.compile(r"v((\d{1,}|x)\.(\d{1,}|y)\.(\d{1,}|z))"),
        ),
        LineRequirement(tag_name="@ingroup", pattern=re.compile(r"\S.*")),
        LineRequirement(tag_name="@prefix", pattern=re.compile(r"\S.*")),
        LineRequirement.empty_line(),
        LineRequirement(tag_name="@brief", pattern=re.compile(r"\S.*"), multiline=True),
        # Do not add an empty line here, because previous is a multiline comment,
        # which itself allows empty line already
        LineRequirement(
            tag_name="@details", pattern=re.compile(r"\S.*"), multiline=True
        ),
        LineRequirement(prefix=" */", tag_name=None),
    ]
    """List of tags to appear inside the comment. The exact order of this list
    will be enforced. Each entry consists of a pair of 'tag' and regular
    expression restricting allowed values for the tag. Only a single line of text
    is allowed for each tag."""

    _rule_description = """
        <p>Each file should begin with a header comment. The
        header comment should be in doxygen format and contain a defined list
        of tags in an exact order. See option <code>required_tags</code>.</p>
    """

    _message_descriptions = {
        "no_header_comment": "No Doxygen file header comment found",
        "first_comment_is_not_header_comment": "The only comment before the first source code token is not a correct file comment. Add license header to get detailed message",
        "wrong_prefix": "Line should start with prefix '{prefix}'",
        "wrong_tag": "Expected tag '{tag}'",
        "wrong_value": "Value of tag '{tag}' does not match expected regex '{pattern}'",
        "unmatched": "Unexpected text in comment",
        "not_doxygen_comment": "Expect a Doxygen comment here",
        "content_missing": "Unexpected end of comment",
        "tags_missing": "Missing tags in comment: {}",
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
                    license_header = None
                    file_header = None
                    for comment in cached_comments[src_file]:
                        if comment.at_beginning:
                            if license_header is None:
                                license_header = comment
                            elif file_header is None:
                                file_header = comment
                                break
                    yield src_file, license_header, file_header

    def execute(self, ir_graph, cached_comments):
        for src_file, license_comment, header_comment in self.header_comments(
            ir_graph=ir_graph,
            file_node_kinds=self.file_node_kinds,
            lang=self.languages,
            cached_comments=cached_comments,
        ):
            if license_comment is None:
                self.add_message(msg_key="no_header_comment", primary_sloc=src_file)
            elif header_comment is None:
                if not self.verify(comment=license_comment, silent=True):
                    self.add_message(
                        msg_key="first_comment_is_not_header_comment",
                        primary_sloc=src_file,
                    )
            else:
                self.verify(comment=header_comment)

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

    def verify(self, comment: comments.CachedComment, silent: bool = False):
        comment_stream = io.StringIO(initial_value=comment.Value)
        line_no = 0
        filename = comment.filename()
        issue_key: typing.Optional[str] = None
        issue_arguments = None
        slocs: typing.List[MessageSLoc] = list()
        pattern_no = -1
        continuation: typing.Optional[LineRequirement] = None
        for line_str in comment_stream:
            line_str = line_str.rstrip()
            if line_no == 0:
                line_no = comment.line()
            else:
                line_no += 1
            match = (
                RequirementMatchResult()
                if continuation is None
                else continuation.match(line=line_str)
            )
            if match:
                continuation = match.continue_option
            else:
                pattern_no += 1
                if pattern_no >= len(self.required_tags):
                    break
                    # required should be configured up to the end of the comment
                match = self.required_tags[pattern_no].match(line_str)
                if match:
                    continuation = match.continue_option
                else:
                    if issue_key is None:
                        if pattern_no == 0 and match.state == MatchResult.wrong_prefix:
                            issue_key = "not_doxygen_comment"
                            column_no = 1
                            slocs = [
                                MessageSLoc(
                                    filename=filename, line=line_no, column=column_no
                                )
                            ]
                            break
                        elif pattern_no + 1 < len(
                            self.required_tags
                        ) and self.required_tags[-1].match(line_str):
                            issue_key = "content_missing"
                        else:
                            issue_key = match.state.name
                        issue_arguments = self.required_tags[
                            pattern_no
                        ].expected_arguments()
                    if match.state == MatchResult.wrong_prefix:
                        column_no = 1
                    elif match.state == MatchResult.wrong_tag:
                        column_no = match.tag_index + 1
                    elif match.state == MatchResult.wrong_value:
                        column_no = match.value_index + 1
                    else:
                        column_no = 1
                    slocs.append(
                        MessageSLoc(filename=filename, line=line_no, column=column_no)
                    )

        if issue_key is None and pattern_no + 1 < len(self.required_tags):
            issue_key = "content_missing"
        if issue_key is not None and not silent:
            if issue_key == "content_missing":
                end_sloc = MessageSLoc(
                    line=comment.End_Line,
                    column=comment.End_Column,
                    filename=comment.filename(),
                )
                missing_tags: typing.List[str] = list(
                    expected.tag_name
                    for expected in filter(
                        lambda e: e.tag_name and e.match_tag,
                        self.required_tags[pattern_no:],
                    )
                )
                if len(missing_tags) > 0:
                    self.add_message(
                        msg_key="tags_missing",
                        primary_sloc=end_sloc,
                        message_arguments=(", ".join(missing_tags),),
                    )
                else:
                    self.add_message(
                        msg_key="content_missing",
                        primary_sloc=end_sloc,
                    )
            else:
                self.add_message(
                    msg_key=issue_key,
                    message_arguments=issue_arguments,
                    primary_sloc=slocs[0],
                    secondary_slocs=slocs[1:],
                )
        return issue_key is None
