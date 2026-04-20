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

"""Validate the consistency of all version specifiers in the project."""

import linecache
from waflib.Configure import conf


@conf
def version_consistency_checker(ctx):
    """Checks that all version strings in the repository are synced"""
    changelog_file = ctx.path.find_node("docs/general/changelog.rst")
    changelog_txt = changelog_file.read(encoding="utf-8")
    if changelog_txt.find(f"[{ctx.env.VERSION}]") < 0:
        ctx.fatal(
            f"The version information in {changelog_file} is different "
            f"from the specified version {ctx.env.VERSION}."
        )
    all_c_sources = ctx.path.ant_glob(
        "docs/**/*.c docs/**/*.h src/**/*.c src/**/*.c tests/**/*.c tests/**/*.c",
        excl=[
            "tests/cli/pre_commit_scripts/test_check_doxygen/invalid-version.c",
            "tests/cli/pre_commit_scripts/test_check_doxygen/ignore-version-comment.c",
            "tests/unit/build/**",
            "tests/unit/gen_hcg/**",
            "tests/waf-tools/fixtures/create_app_build_cfg/expected-app_build_cfg.c",
            "tests/waf-tools/fixtures/create_version/expected-version.c",
        ],
    )
    version_line = -1
    main_txt = ctx.path.find_node("src/app/main/main.c").read()
    for i, line in enumerate(main_txt.splitlines()):
        if line.startswith(" * @version "):
            version_line = i + 1
            break
    expected_line = f"* @version v{ctx.env.VERSION}"
    for i in all_c_sources:
        version_line_txt = linecache.getline(i.abspath(), version_line)
        if version_line_txt.startswith(" * @version "):
            if version_line_txt.strip() != expected_line:
                ctx.fatal(
                    f"Version information in {i.abspath()}:{version_line} is "
                    f"not correct (expected '{expected_line}', but found "
                    f"'{version_line_txt.strip()}')."
                )
