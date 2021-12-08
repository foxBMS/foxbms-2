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

"""Implements a waf tool to use
`Sphinx <https://www.sphinx-doc.org/en/master/>`_.
"""

import os
import re
import sys

from waflib import Logs, Node, Task, TaskGen, Utils


class sphinx_task(Task.Task):  # pylint: disable=invalid-name
    """class to compile a conf.py file into documentation using Sphinx.

    .. graphviz::
        :caption: Input-output relation for conf.py
        :name: confpy-to-docs

        digraph ASM_TO_OBJECT {
            compound=true;
            rankdir=LR;
            nd_sphinx [label="sphinx_build", style=filled, fillcolor=green];
            nd_confpy  [label="conf.py", style=filled];
            nd_outdir  [label="OUTDIR/index.html", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_buildername  [label="BUILDERNAME"];
                nd_version      [label="VERSION"];
                nd_release      [label="RELEASE"];
                nd_dot          [label="DOT"];
                nd_doctreedir   [label="DOCTREEDIR"];
                nd_srcdir       [label="SRCDIR"];
            }
            nd_sphinx       -> nd_buildername   [lhead=cluster_cmd];
            nd_confpy       -> nd_buildername   [lhead=cluster_cmd];
            nd_buildername  -> nd_outdir        [ltail=cluster_cmd];
        }
    """

    #: str: color in which the command line is displayed in the terminal
    color = "BLUE"

    #: str: Sphinx handles the need for a re-run, so always run this task
    always_run = True

    def run(self):
        """Creates a command line processed by ``Utils.subprocess.Popen`` in
        order to build the sphinx documentation. See :numref:`confpy-to-docs`
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
        proc = Utils.subprocess.Popen(
            cmd.split(),
            stdin=Utils.subprocess.PIPE,
            stdout=Utils.subprocess.PIPE,
            stderr=Utils.subprocess.PIPE,
            env=env,
            cwd=cwd,
        )

        std_out, std_err = proc.communicate()
        std_out = std_out.decode(errors="ignore")
        std_err = std_err.decode(errors="ignore")
        ret = getattr(self, "check_output_" + self.env.BUILDERNAME)(std_out, std_err)
        if ret:
            self.generator.bld.fatal(f"Could not build {self.env.BUILDERNAME}")

    def check_output_spelling(self, std_out, std_err):
        """check if the spelling task generates any real errors"""
        err_bit = 0
        re_splitter = r":([0-9]{1,}):"
        re_err = re.compile(r"\.rst" + re_splitter)
        for line in std_out.strip().splitlines():
            # if the output line contains ".rst:" it indicates that an error
            # has been found. The path is relative to the source directory
            # therefore we need to construct the path to the source file with
            # the error to generate a meaningfull errormessage
            match = re_err.search(line)
            if match:
                err_bit += 1
                err_msg = re.split(re_splitter, line)
                err_file = os.path.normpath(os.path.join(self.env.SRCDIR, err_msg[0]))
                err_line = err_msg[1]
                err_word = err_msg[2]
                Logs.error(f"{err_file}:{err_line}:error:{err_word}")
            else:
                if Logs.verbose:
                    print(line)
        for line in std_err.strip().splitlines():
            if self.removedinsphinx30warning(line):
                continue
            Logs.error(line)
            if not err_bit:
                err_bit = 1

        return err_bit

    def check_output_linkcheck(self, std_out, std_err):
        """check if the linkcheck task generates any real errors"""
        err_bit = 0
        re_splitter = r"(\[[ ]{0,2}\d{1,3}\%\])"
        re_err = r"line\s{0,}(\d{1,})\s{0,}\)(.*)"
        current_file = ""
        for line in std_out.strip().splitlines():
            try:
                current_file = os.path.normpath(
                    os.path.join(
                        self.env.SRCDIR, re.split(re_splitter, line)[2].strip() + ".rst"
                    )
                )
            except IndexError:
                pass
            if "broken " in line:
                line = re.sub(r"\s\s+", " ", line)
                err_bit += 1
                try:
                    line, msg = re.split(re_err, line)
                    Logs.error(f"{current_file}:{line}:{msg}")
                except ValueError:
                    Logs.error(line)
            else:
                if Logs.verbose:
                    print(line)
        for line in std_err.strip().splitlines():
            if self.removedinsphinx30warning(line):
                continue
            Logs.error(line)
            if not err_bit:
                err_bit = 1

        return err_bit

    def check_output_html(self, std_out, std_err):
        """check if the html task generates any real errors"""
        err_bit = 0
        for line in std_out.strip().splitlines():
            if Logs.verbose:
                print(line)
        for line in std_err.strip().splitlines():
            if self.removedinsphinx30warning(line):
                continue
            Logs.error(line)
            if not err_bit:
                err_bit = 1

        return err_bit

    @staticmethod
    def removedinsphinx30warning(_str):
        """The warning ``RemovedInSphinx30Warning`` is not a valid warning in
        our build therefore it can skipped to fail the build."""
        ret = False
        if "RemovedInSphinx30Warning".lower() in _str.lower():
            Logs.warn(_str)
            ret = True
        return ret

    def __str__(self):
        """for printing"""
        return " ".join([a.relpath() for a in self.inputs])

    def keyword(self):
        """displayed keyword when the sphinx configuration file is compiled"""
        return f"Compiling {self.env['BUILDERNAME']}"


@TaskGen.extension(".rst", ".txt", ".csv")
def rst(self, node):  # pylint: disable=unused-argument
    """dummy function to be able to use
    ``bld(features="sphinx", source="abc*.rst", ...)``."""


@TaskGen.feature("sphinx")
@TaskGen.before_method("process_source")
def apply_sphinx(self):
    """Set up the task generator with a Sphinx instance and create a task."""
    inputs = []
    for i in Utils.to_list(self.source):
        if not isinstance(i, Node.Node):
            node = self.path.find_node(i)
        else:
            node = i
        if not node:
            raise ValueError(f"Source file '{i}'' not found.")
        inputs.append(node)

    # get sphinx config (conf.py) and derive the srcdir from it.
    if not getattr(self, "confpy", None):
        raise ValueError("Path to the sphinx config must be specified ('conf.py').")
    src_dir = self.confpy.parent.abspath()

    # set the output directory
    if not getattr(self, "outdir", None):
        raise ValueError("outdir must be specified.")
    self.outdir_node = self.path.find_or_declare(getattr(self, "outdir")).get_bld()

    # add builders, build at least html documentation
    builders = []
    if not getattr(self, "builders", None):
        builders.append("html")
    else:
        builders.extend(Utils.to_list(self.builders))

    for builder in builders:
        outfile = self.path.get_bld().make_node(builder)
        outfile.write(builder)
        builder_task = self.create_task("sphinx_task", inputs + [outfile])
        builder_task.inputs.append(self.confpy)
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
                os.path.join(self.outdir_node.abspath(), ".doctrees"),
            )
        else:
            builder_task.env["DOCTREEDIR"] = os.path.join(
                self.outdir_node.abspath(), f".doctrees-{builder}"
            )
            builder_task.no_errcheck_out = True
        builder_task.env["OUTDIR"] = self.outdir_node.abspath()
        builder_task.env["VERSION"] = f"version={self.env.VERSION}"
        builder_task.env["RELEASE"] = f"release={self.env.VERSION}"
        builder_task.outputs.append(self.outdir_node)


def configure(conf):
    """Check if the following programs are available

    - ``sphinx-build``,
    - ``dot``
    """
    conf.find_program("sphinx-build", var="SPHINX_BUILD")
    conf.find_program("dot", var="DOT")
