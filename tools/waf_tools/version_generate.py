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

"""Add version information to build artifacts."""

import hashlib
from datetime import UTC, datetime
from typing import Literal

from c_codegen_template import render_template
from misc_helpers import map_python_bool_to_c_bool
from vcs import VcsInformation
from vcs_git import fast_get_version_info_git
from waflib.Configure import ConfigurationContext, conf
from waflib.Task import RUN_ME, SKIP_ME, Task
from waflib.TaskGen import after_method, feature, task_gen

VERSION_STRUCT_TEMPLATE = """const VER_VERSION_s ver_versionInformation VER_VERSION_INFORMATION = {{
    .underVersionControl = {under_version_control},
    .isDirty = {is_dirty},
    .major = {major}u,
    .minor = {minor}u,
    .patch = {patch}u,
    .distanceFromLastRelease = {distance}u,
    .commitHash = "{commit}",
    .remote = "{remote}",
}};
"""


class create_version_source(Task):
    """creates the version information file"""

    weight = 2
    color = "GREY"
    before = ["c"]
    ext_out = [".h"]
    always_run = True

    def create_version_hash(self) -> str:
        """Compute a hash from current VCS version information."""
        out = fast_get_version_info_git(self.generator.bld)
        return hashlib.sha256(repr(out).encode("utf-8")).hexdigest()

    def runnable_status(self) -> Literal[-1, -2, -3, -4]:
        """Skip regeneration if the persisted version hash is unchanged."""
        ret = super().runnable_status()
        if ret != RUN_ME:
            return ret

        new_version_hash = self.create_version_hash()
        old_version_hash = None

        out = self.outputs[0].parent.find_or_declare("version.hash")
        try:  # noqa: SIM105 performance is important here
            old_version_hash = out.read(encoding="utf-8")
        except FileNotFoundError:
            pass
        if new_version_hash == old_version_hash:
            return SKIP_ME
        out.write(new_version_hash, encoding="utf-8")
        return RUN_ME

    def run(self) -> None:  # pragma: no cover
        """Render and write the generated ``version.c`` and local clang-format file."""
        txt = self.inputs[0].read(encoding="utf-8")
        txt = self.generator.bld.create_version_c(txt)
        self.outputs[0].write(txt, encoding="utf-8")
        self.outputs[1].write("DisableFormat: true\nSortIncludes: false\n")

    def keyword(self) -> Literal["Creating"]:  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        """Return the generated output path."""
        return str(self.outputs[0].path_from(self.generator.bld.path))


@feature("cprogram")
@after_method("process_rule")
def create_version_file(self: task_gen) -> None:
    """Task generator for version information file"""
    if not getattr(self, "version", False):
        return
    generated_sources = []
    src = self.path.ctx.root.find_node(f"{self.env.PROJECT_ROOT[0]}/conf/tpl/c.c")
    version_c = self.path.find_or_declare("version.c")
    no_clang_node = self.path.find_or_declare(".clang-format")
    version_src_tsk = self.create_task(
        "create_version_source", src=src, tgt=[version_c, no_clang_node]
    )
    generated_sources.append(version_src_tsk.outputs[0])
    try:
        self.source.extend(generated_sources)
    except AttributeError:
        self.source = [self.source] + generated_sources


@conf
def create_version_c(ctx: ConfigurationContext, txt: str) -> str:
    """Create the version source code from a template.

    Args:
        ctx : Waf ConfigurationContext Waf
        txt : The template text to be rendered

    Returns:
        The generated C source code containing version information
    """
    version: VcsInformation = ctx.gather_and_validate_version_info()
    today = datetime.now(tz=UTC).date().strftime("%Y-%m-%d")

    version_block = VERSION_STRUCT_TEMPLATE.format(
        under_version_control=map_python_bool_to_c_bool(version.under_version_control),
        is_dirty=map_python_bool_to_c_bool(version.dirty),
        major=version.major,
        minor=version.minor,
        patch=version.patch,
        distance=version.distance,
        commit=version.short_hash,
        remote=version.remote,
    )

    includes = '#include "version.h"\n\n#include <stdbool.h>\n#include <stdint.h>'
    return render_template(
        txt,
        {
            "file_name": "version.c",
            "author": "foxBMS Team",
            "date": f"{today} (date of creation)",
            "updated": f"{today} (date of last update)",
            "version": f"v{version.major}.{version.minor}.{version.patch}",
            "ingroup": "GENERAL",
            "prefix": "VER",
            "brief": "Version information that is generated by the toolchain.",
            "details": "Version information that is generated by the toolchain.",
            "include_directives": includes,
            "static_constant_and_variable_definitions": version_block,
        },
    )
