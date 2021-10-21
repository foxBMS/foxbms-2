#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a waf tool to configure a git repository to use the foxBMS
specific git hooks.
"""

import os
import shutil


def options(opt):
    """Options to enable using the foxBMS git hooks. The hook can either be
    globally enabled by defining the environment variable
    ``FOXBMS_USE_GIT_HOOKS`` or passing --use-git-hooks during the configure
    step."""
    opt.add_option(
        "--use-git-hooks",
        dest="USE_GIT_HOOKS",
        action="store_true",
        help="Apply foxBMS git hooks to that repository.",
    )


def configure(conf):  # pylint: disable=too-many-statements,too-many-branches
    """Use foxBMS specific git hooks in that repository."""
    # create a VS Code workspace if code is installed on this platform
    if not (os.environ.get("FOXBMS_USE_GIT_HOOKS") or conf.options.USE_GIT_HOOKS):
        return
    conf.start_msg("Applying git hooks.")
    hooks = [
        conf.path.find_node(os.path.join("tools", "utils", "git-hooks", "pre-commit"))
    ]
    for i in hooks:
        shutil.copyfile(i.abspath(), os.path.join(".git", "hooks", i.name))
    conf.end_msg("ok")
