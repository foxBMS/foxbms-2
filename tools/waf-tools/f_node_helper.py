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

"""Helper methods to work with waf node objects"""


from waflib import Errors, Node, TaskGen, Utils


@TaskGen.taskgen_method
# pylint: disable=missing-function-docstring
def convert_to_node(self, _node, path=None):
    # see f_node_helper.rst
    path = path or self.path

    if not isinstance(_node, Node.Node):
        _node = path.find_node(_node)
    return _node


@TaskGen.taskgen_method
# pylint: disable=missing-function-docstring
def convert_to_nodes(self, lst, path=None):
    # see f_node_helper.rst

    # implementation is based on
    # https://gitlab.com/ita1024/waf/-/blob/3536dfecf8061c6d99bac338837997c4862ee89b/waflib/TaskGen.py#L495-527
    tmp = []
    path = path or self.path

    if isinstance(lst, Node.Node):
        lst = [lst]

    for i in Utils.to_list(lst):
        if isinstance(i, str):
            node = path.find_node(i)
        elif hasattr(i, "name"):
            node = i
        else:
            tmp.extend(self.convert_to_nodes(i))
            continue
        if not node:
            raise Errors.WafError(f"source/directory not found: {i} in {self}")
        tmp.append(node)
    return tmp
