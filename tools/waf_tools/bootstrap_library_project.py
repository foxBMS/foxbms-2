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


# cspell:ignore arcname

"""Implement a waf tool to bootstrap a library project.

This module **must** be loaded in the 'options' method.
"""

import os
import tarfile
from pathlib import Path
from tempfile import NamedTemporaryFile

from git import Repo
from git.exc import InvalidGitRepositoryError
from waflib import Context
from waflib.Build import BuildContext

README_TEMPLATE = """# Minimal Library Project for foxBMS

This is a minimal project to build a library for foxBMS (based on {}).

For details visit https://foxbms.org.
"""


def bootstrap_library_project(ctx: BuildContext) -> None:
    """creates a library project"""  # noqa: D403
    misc = [
        ctx.path.find_node(".gitignore"),
        ctx.path.find_node("BSD-3-Clause.txt"),
        ctx.path.find_node("CC-BY-4.0.txt"),
        ctx.path.find_node("LICENSE.md"),
    ]
    tools = (
        [
            ctx.path.find_node("fox.ps1"),
            ctx.path.find_node("fox.py"),
            ctx.path.find_node("fox.sh"),
        ]
        + list(ctx.path.ant_glob("cli/**/*.py"))
        + list(ctx.path.ant_glob("conf/env/**"))
        + [
            ctx.path.find_node("conf/cc/remarks.txt"),
            ctx.path.find_node("tools/waf"),
        ]
        + list(ctx.path.ant_glob("tools/waf_tools/*.py"))
    )
    base = ctx.path.find_node("docs/software/build-process/misc")
    lib_wscript = base.find_node("wscript")
    example_source = base.find_node("libproject-example.c")
    example_header = base.find_node("libproject-example.h")

    commit_id = ""
    try:
        repo = Repo(search_parent_directories=True)
        commit_id = repo.head.object.hexsha
    except InvalidGitRepositoryError:
        pass
    version = ctx.env.VERSION + commit_id[:7]
    readme_txt = README_TEMPLATE.format(version)
    with tarfile.open("library-project.tar.gz", mode="w:gz") as tar:
        for i in tools + misc:
            tar.add(i.relpath())
        tar.add(lib_wscript.relpath(), arcname="wscript")
        tar.add(
            example_source.relpath(), arcname=os.path.join("src", example_source.name)
        )
        tar.add(
            example_header.relpath(), arcname=os.path.join("src", example_header.name)
        )
        with NamedTemporaryFile(mode="w", delete=False, encoding="utf-8") as tmp:
            tmp.write(readme_txt)
            tmp.flush()
            tar.add(tmp.name, arcname="README.md")
        try:  # noqa: SIM105
            Path(tmp.name).unlink()
        except FileNotFoundError:
            pass


class bootstrap_context(BuildContext):
    """Helper class to bind the bootstrap context to an waf argument"""

    cmd = "bootstrap_library_project"
    fun = "bootstrap_library_project"


# inject command into top-level wscript. This requires that g_module is available
if Context.g_module:
    Context.g_module.__dict__["bootstrap_library_project"] = bootstrap_library_project
