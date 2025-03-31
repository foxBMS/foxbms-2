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

"""Implements a waf tool to use
`Sphinx <https://www.sphinx-doc.org/en/master/>`_.
"""

import os
import sys

from waflib import Logs, Task, TaskGen, Utils
from waflib.Configure import ConfigurationContext


class sphinx_task(Task.Task):  # pylint: disable=invalid-name
    """class to compile a conf.py file into documentation using Sphinx.

    .. graphviz::
        :caption: Input-output relation for conf.py
        :name: conf_py-to-docs

        digraph ASM_TO_OBJECT {
            compound=true;
            rankdir=LR;
            nd_sphinx [label="sphinx_build", style=filled, fillcolor=green];
            nd_conf_py  [label="conf.py", style=filled];
            nd_out_dir  [label="OUTDIR/index.html", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_builder_name [label="BUILDERNAME"];
                nd_version      [label="VERSION"];
                nd_release      [label="RELEASE"];
                nd_dot          [label="DOT"];
                nd_doctreedir   [label="DOCTREEDIR"];
                nd_srcdir       [label="SRCDIR"];
            }
            nd_sphinx       -> nd_builder_name   [lhead=cluster_cmd];
            nd_conf_py      -> nd_builder_name   [lhead=cluster_cmd];
            nd_builder_name -> nd_out_dir        [ltail=cluster_cmd];
        }
    """

    #: str: color in which the command line is displayed in the terminal
    color = "BLUE"

    #: str: Sphinx handles the need for a re-run, so always run this task
    always_run = True

    def run(self):
        """Creates a command line processed by ``Utils.subprocess.Popen`` in
        order to build the sphinx documentation. See :numref:`conf_py-to-docs`
        for a simplified representation"""
        verbosity = ""
        if Logs.verbose:
            verbosity = "-" + Logs.verbose * "v"
        cmd = " ".join(
            [
                sys.executable,
                "-m sphinx",
                "-b ${BUILDERNAME}",
                "-c ${CONFDIR}",
                "-D ${VERSION}",
                "-D ${RELEASE}",
                "-D graphviz_dot=${DOT}",
                "-d ${DOCTREEDIR}",
                "-W",
                "--keep-going",
                "${SRCDIR}",
                "${OUTDIR}",
                verbosity,
            ]
        )
        cmd = " ".join(cmd.split())
        cmd = Utils.subst_vars(cmd, self.env)
        Logs.info(cmd)
        env = self.env.env or None
        cwd = self.generator.bld.path.get_bld().abspath()
        proc = Utils.subprocess.Popen(cmd.split(), env=env, cwd=cwd)
        proc.communicate()
        if not proc.returncode:
            print(f"Index file: {os.path.join(self.env.OUTDIR, 'index.html')}.")
        return proc.returncode

    def __str__(self):
        """for printing"""
        return self.env["BUILDERNAME"] + " ".join([a.relpath() for a in self.inputs])

    def keyword(self):
        """displayed keyword when the sphinx configuration file is compiled"""
        return "Compiling"


@TaskGen.feature("sphinx")
@TaskGen.before_method("process_source")
def apply_sphinx(self):
    """Set up the task generator with a Sphinx instance and create a task."""

    # get sphinx config (conf.py) and derive the srcdir from it.
    if not getattr(self, "conf_py", None):
        raise ValueError("Path to the sphinx config must be specified ('conf.py').")
    src_dir = self.conf_py.parent.abspath()

    # set the output directory
    if not getattr(self, "out_dir", None):
        raise ValueError("out_dir must be specified.")
    self.out_dir_node = self.path.find_or_declare(getattr(self, "out_dir")).get_bld()

    # add builders, build at least html documentation
    builders = []
    if not getattr(self, "builders", None):
        builders.append("html")
    else:
        builders.extend(Utils.to_list(self.builders))

    for builder in builders:
        outfile = self.path.get_bld().make_node(builder)
        outfile.write(builder)
        builder_task = self.create_task("sphinx_task", [outfile])
        builder_task.inputs.append(self.conf_py)
        builder_task.env["BUILDERNAME"] = builder
        builder_task.env["SRCDIR"] = src_dir
        builder_task.env["CONFDIR"] = src_dir
        # we set an additional output node, in order to have a unique output
        # sequence, because otherwise waf complains about not unique tasks,
        # but these tasks are unique.
        if builder == "html":
            builder_task.env["DOCTREEDIR"] = getattr(
                self,
                "doctreedir",
                os.path.join(self.out_dir_node.abspath(), ".doctrees"),
            )
        else:
            builder_task.env["DOCTREEDIR"] = os.path.join(
                self.out_dir_node.abspath(), f".doctrees-{builder}"
            )
            builder_task.no_errcheck_out = True
        builder_task.env["OUTDIR"] = self.out_dir_node.abspath()
        builder_task.env["VERSION"] = f"version={self.env.VERSION}"
        builder_task.env["RELEASE"] = f"release={self.env.VERSION}"
        builder_task.outputs.append(self.out_dir_node)


def configure(conf: ConfigurationContext):
    """Check that sphinx-build and dot are available"""
    conf.find_program("sphinx-build", var="SPHINX_BUILD")
    if Utils.is_win32:
        conf.find_program("draw.io", var="DRAW_IO")
    else:
        conf.find_program("drawio", var="DRAW_IO")
    conf.find_program("dot", var="DOT")
