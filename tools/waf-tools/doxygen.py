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

# cSpell:ignore Doxyfile

"""Simple doxygen runner"""

import json
import os

import graphviz  # noqa: F401 pylint:disable=unused-import

from waflib import Context, Logs, Node, Task, TaskGen, Utils
from waflib.Configure import ConfigurationContext, conf

TOOL_DIR = os.path.dirname(os.path.realpath(__file__))


class CreateDoxygenConfiguration(Task.Task):
    """Doxygen task"""

    color = "BLUE"
    vars = ["DOXYGEN_CONFIG", "DOT", "APPNAME", "VERSION"]

    def run(self):  # noqa: D102
        txt: str = self.inputs[0].read()
        # pylint: disable-next=no-member
        for k, v in self.config.items():
            if isinstance(v, Node.Node):
                v = v.abspath()
            txt = txt.replace(f"@{k.upper()}@", v)
        self.outputs[0].write(txt)


class Doxygen(Task.Task):
    """Doxygen task"""

    color = "BLUE"

    vars = ["DOXYGEN_CONFIG", "DOT", "APPNAME", "VERSION"]

    def run(self):  # noqa: D102
        cmd = self.generator.env.DOXYGEN + [self.inputs[0].abspath()]
        # pylint: disable-next=no-member
        with Utils.subprocess.Popen(cmd, cwd=self.cwd) as p:
            p.communicate()
        if p.returncode:
            return 1

        self.outputs = self.generator.bld.path.get_bld().ant_glob(
            [
                "html/**/*.png",
                "html/**/*.js",
                "html/**/*.svg",
                "html/**/*.html",
                "html/**/*.css",
                "html/**/*.md5",
                "html/**/*.map",
            ],
        )
        self.generator.bld.raw_deps[self.uid()] = [self.signature()] + self.outputs
        return 0

    def runnable_status(self):  # noqa: D102
        ret = super().runnable_status()
        if ret == Task.SKIP_ME:
            lst = self.generator.bld.raw_deps[self.uid()]
            if lst[0] != self.signature():
                return Task.RUN_ME

            nodes = lst[1:]
            for x in nodes:
                try:
                    os.stat(x.abspath())
                except:  # noqa: E722 pylint: disable=bare-except
                    return Task.RUN_ME

            nodes = lst[1:]
            self.set_outputs(nodes)

        return ret

    def keyword(self):  # noqa: D102
        return "Compiling"


@TaskGen.feature("doxygen")
def process_doxygen(self):
    """Creates the doxygen task"""
    valid = True
    attrs = [
        "project_name",
        "project_number",
        "project_brief",
        "project_logo",
        "layout_file",
        "input",
        "image_path",
        "html_footer",
        "html_stylesheet",
        "html_extra_files",
        "exclude_patterns",
    ]
    config = {}
    for i in attrs:
        if not getattr(self, i, None):
            valid = False
            Logs.error(f"Setting '{i}' is missing or empty.")
    if not valid:
        self.bld.fatal("Some doxygen configuration is not correct.")
    tmp = ""
    for i in self.input.split(" "):
        i_tmp = self.path.find_node(i)
        if not i:
            i_tmp = self.path.find_dir(i)
        if not i_tmp:
            self.bld.fatal(f"Node {i} does not exist.")
        tmp = tmp + " " + i_tmp.abspath()
    config["input"] = tmp
    tmp = ""
    for i in getattr(self, "exclude", "").split(" "):
        i_tmp = self.path.find_node(i)
        if not i_tmp:
            self.bld.fatal(f"Node {i} does not exist.")
        tmp = tmp + " " + i_tmp.abspath()
    config["exclude"] = tmp
    config["predefined"] = getattr(self, "predefined", "")
    config["output_directory"] = "."
    src = [
        self.path.find_node(self.doxygen_conf),
        self.path.find_node(self.project_logo),
        self.path.find_node(self.layout_file),
        self.path.find_node(self.image_path),
        self.path.find_node(self.html_footer),
        self.path.find_node(self.html_stylesheet),
        self.path.find_node(self.html_extra_files),
    ]
    if not all(i if i else False for i in src):
        self.bld.fatal("Some input node does not exist.")

    config["project_name"] = self.project_name
    config["project_number"] = self.project_number
    config["project_brief"] = self.project_brief
    config["project_logo"] = self.path.find_node(self.project_logo)
    config["layout_file"] = self.path.find_node(self.layout_file)
    config["image_path"] = self.path.find_node(self.image_path)
    config["html_footer"] = self.path.find_node(self.html_footer)
    config["html_stylesheet"] = self.path.find_node(self.html_stylesheet)
    config["html_extra_files"] = self.path.find_node(self.html_extra_files)
    config["dot_path"] = self.dot_path

    tgt = self.bld.path.find_or_declare("Doxyfile")

    self.bld.env["DOXYGEN_CONFIG"] = json.dumps(
        str(config), ensure_ascii=True, sort_keys=True
    ).encode()
    self.create_doxygen_config = self.create_task(
        "CreateDoxygenConfiguration", src=src, tgt=tgt, config=config
    )
    self.create_task("Doxygen", src=tgt, cwd=self.bld.path.get_bld().abspath())


@conf
def get_doxygen_version(ctx: ConfigurationContext):
    """Determine 'doxygen' version"""
    cmd = ctx.env.DOXYGEN + ["--version"]
    std_out, std_err = ctx.cmd_and_log(cmd, output=Context.BOTH)
    if std_err:
        ctx.fatal(f"Could not successfully run '--version' on {ctx.env.DOXYGEN}.")
    std_out = std_out.strip()
    try:
        doxygen_version = std_out.split()[0]
    except IndexError:
        Logs.warn("Could not determine 'doxygen' version.")
        doxygen_version = "unknown"
    ctx.env.DOXYGEN_VERSION = doxygen_version


@conf
def find_doxygen(ctx: ConfigurationContext):
    """Find the 'doxygen' executable and determine its version."""
    ctx.find_program("doxygen", var="DOXYGEN")
    ctx.get_doxygen_version()


def configure(ctx: ConfigurationContext):
    """Find 'doxygen' and 'dot'"""
    ctx.find_doxygen()
    ctx.find_dot()
