#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Utility functions and waf tasks to generate cleaned header files from preprocessed headers.

This waf tool provides the preprocessing pipeline required by the ``mock`` tool.
It takes raw C header files, runs them through the C preprocessor (``cc -E``),
and then extracts only the lines that belong to the original header - stripping
all content pulled in via transitive ``#include`` directives.
The result is a "clean" header suitable for consumption by CMock.

Usage
-----
This module is not loaded directly via ``ctx.load()``. Instead it is imported
by the ``mock`` tool::

    import preprocess

    preprocessed_nodes = preprocess.create_preprocessed_files(self, headers, test_file)
    for node in preprocessed_nodes:
        clean_node = preprocess.create_clean_header(self, node)

Dependencies
------------
- Host C compiler (``env.CC``) capable of ``-E`` preprocessing
- Used exclusively by the ``mock`` tool in this toolchain
"""

import re
from pathlib import Path
from typing import Literal

from waflib import Logs, Utils
from waflib.Node import Node
from waflib.Task import Task
from waflib.TaskGen import task_gen
from waflib.Tools import c as c_tools
from waflib.Tools import c_preproc

# cspell:ignore DINC tgen


class clean_header(Task):
    """Waf task that creates a cleaned header file from a preprocessed header input."""

    def run(self) -> int:
        """Reads the preprocessed output and identifies lines that originate
        from the target header via GCC line markers (``# <line> "<file>"``).
        Only those lines are written to the cleaned output file.
        Reconstructed ``#include`` directives are prepended separately.
        """
        includes, extracted_array = extract_file_as_array_from_expansion(self.inputs[0])
        output = includes + extracted_array
        self.outputs[0].write("\n".join(output))
        return 0

    def keyword(self) -> Literal["Creating"]:
        """Return the short task label used in build logs."""
        return "Cleaning"


class HPreproc(Task):
    """Task that preprocesses header files."""

    vars = c_tools.c.vars
    scan = c_preproc.scan

    def run(self) -> None:
        """Runs ``cc -E`` on a header file to produce a fully-expanded
        preprocessed output.

        Include paths and defines are inherited from the build environment.
        For ``app`` variants the macro ``INC_FREERTOS_H`` is defined
        automatically.
        """
        cmd = (
            self.env.CC
            + self.env.CFLAGS
            + [f"-I{i}" for i in self.env.INCPATHS]
            + [f"-D{d}" for d in self.env.DEFINES]
        )
        if "app" in self.generator.bld.variant:
            cmd.append("-DINC_FREERTOS_H")
        return self.exec_command(
            cmd + ["-E", self.inputs[0].abspath(), "-o", self.outputs[0].abspath()],
            env=self.env.env,
        )

    def keyword(self) -> Literal["Processing"]:
        """Return the short task label used in build logs."""
        return "Preprocessing"


def clean_encoding(line: str) -> str:
    """Normalize a text line to UTF-8, ignoring invalid byte sequences."""
    return line.encode("utf-8", "ignore").decode("utf-8")


def extract_file_as_array_from_expansion(
    prep_node: Node,
) -> tuple[list[str], list[str]]:
    """Extract logical source lines for the given header from a preprocessed file.

    The function scans the preprocessed file, tracks line-marker directives,
    and collects only the lines that belong to the original header file
    identified by the name of prep_node.
    """
    directive = re.compile(r'^# \d+ "')
    line_marker = re.compile(r"^#\s\d+\s\"(.+)\"")
    include = re.compile(r'^#\s+\d+\s+"([^"]+)"\s+(.*)')
    extract = False
    lines: list[str] = []
    includes: list[str] = []

    for raw_line in prep_node.read().splitlines():
        line = clean_encoding(raw_line)
        if extract and not directive.search(line):
            _line = line.strip()
            if _line:
                _line = line.rstrip()
            lines.append(_line)
        else:
            extract = False

        m = line_marker.match(line)
        if m and len(m.groups()) >= 1:
            marker_path = Path(m.group(1).strip())
            if marker_path.name == prep_node.name:
                extract = True

        inc = include.match(line)
        if inc:
            flags = inc.group(2).strip().split()
            if "1" in flags:
                name = Path(inc.group(1)).name
                formatted = (
                    f"#include <{name}>" if "3" in flags else f'#include "{name}"'
                )
                if formatted not in includes:
                    includes.append(formatted)
    return includes, lines


def create_clean_header(self: task_gen, prep_node: Node) -> Node:
    """Create a clean_header waf task for the given preprocessed node and return its output node."""
    clean_node = prep_node.parent.parent.make_node(prep_node.name)
    self.create_task("clean_header", prep_node, clean_node)
    return clean_node


def create_preprocessed_files(
    self: task_gen, nodes: list[Node], test_file: Node
) -> list[Node]:
    """Create preprocessing tasks for the given nodes, ensuring they run after
    the appropriate HAL build tasks, and return the resulting preprocessed output nodes.
    """
    test_name = test_file.name.replace(test_file.suffix(), "")
    preprocessed_nodes = []
    hal_tg = []
    hal_tasks = []
    if Utils.is_win32:
        hal_tg = self.bld.get_tgen_by_name(f"{self.bld.env.APPNAME.lower()}-hal")
        hal_tg.post()
        hal_tasks = hal_tg.tasks
    for node in nodes:
        if not isinstance(node, Node):
            Logs.warn(node, test_file)
            continue
        pp_out = self.bld.path.find_or_declare(
            f"preprocess/{test_name}/full_expansion/{node.name}"
        )
        t = self.create_task("HPreproc", src=node, tgt=pp_out)
        pp_out.parent.mkdir()
        preprocessed_nodes.append(pp_out)
        for task in hal_tasks:
            t.set_run_after(task)
    return preprocessed_nodes
