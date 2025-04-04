#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a waf tool to bootstrap a library project.

This tool adds the command ``bootstrap_library_project`` to the wscript in
which you are loading it.
"""

import os
import tarfile
from tempfile import NamedTemporaryFile
from git import Repo
from git.exc import InvalidGitRepositoryError

from waflib import Context
from waflib.Build import BuildContext


def bootstrap_library_project(ctx):
    """Creates a zip that includes a library project"""
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
            ctx.path.find_node("tools/waf"),
            ctx.path.find_node("tools/waf-tools/crc_bootloader.py"),
            ctx.path.find_node("tools/waf-tools/crc64_ti_impl.py"),
            ctx.path.find_node("tools/waf-tools/f_hcg.py"),
        ]
        + ctx.path.ant_glob("conf/env/** conf/cc/remarks.txt tools/utils/**")
        + ctx.path.ant_glob("cli/**/*.py")
        + ctx.path.ant_glob("tools/waf-tools/f_ti_*.py")
    )
    lib_cc_options = ctx.path.find_node(
        "docs/software/build-process/misc/libcc-options.yaml"
    )
    lib_wscript = ctx.path.find_node("docs/software/build-process/misc/wscript")
    example_source = ctx.path.find_node(
        "docs/software/build-process/misc/libproject-example.c"
    )
    example_header = ctx.path.find_node(
        "docs/software/build-process/misc/libproject-example.h"
    )
    commit_id = "unknown"
    try:
        repo = Repo(search_parent_directories=True)
        commit_id = repo.head.object.hexsha
    except InvalidGitRepositoryError:
        pass
    readme_txt = (
        "# Minimal Library Project for foxBMS\n\nThis is a minimal project to "
        f"build a library for foxBMS (based on {commit_id}).\n\nFor details "
        "visit https://foxbms.org.\n"
    )
    with tarfile.open("library-project.tar.gz", mode="w:gz") as tar:
        for i in tools + misc:
            tar.add(i.relpath())
        tar.add(
            lib_cc_options.relpath(),
            arcname=os.path.join("conf", "cc", "cc-options.yaml"),
        )
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
        try:
            os.remove(tmp.name)
        except FileNotFoundError:
            pass


class bootstrap_context(BuildContext):  # pylint: disable=invalid-name
    """Helper class to bind the bootstrap context to an waf argument"""

    cmd = "bootstrap_library_project"
    fun = "bootstrap_library_project"


# inject command into top-level wscript. This requires that g_module is available
if Context.g_module:
    Context.g_module.__dict__["bootstrap_library_project"] = bootstrap_library_project
