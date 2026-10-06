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

# cspell:ignore BUILDERNAME,CONFDIR,DOCTREEDIR,doctrees,SRCDIR,OUTDIR

"""Implements a waf tool to use
`Sphinx <https://www.sphinx-doc.org/en/master/>`_.
"""

import os
import sys
from pathlib import Path
from typing import Literal

import drawio  # noqa: F401 pylint:disable=unused-import
import graphviz  # noqa: F401 pylint:disable=unused-import
from waflib import Context, Logs, Task, TaskGen, Utils
from waflib.Build import CleanContext
from waflib.Configure import ConfigurationContext, conf
from waflib.Node import Node

TOOL_DIR = str(Path(__file__).parent)


class create_version_file(Task.Task):
    """Create a version macro replacement file for Sphinx."""

    color = "GREY"

    vars = ["VERSION"]

    def run(self) -> int:
        """Create the version macro replacement."""
        self.outputs[0].parent.mkdir()
        self.outputs[0].write(f".. |version_foxbms| replace:: ``{self.env.VERSION}``")
        return 0


class sphinx_task(Task.Task):
    """Render rst-source files through a conf.py configuration file"""

    color = "BLUE"
    always_run = True
    after = ["create_version_file", "drawioToSvg"]

    def run(self) -> int:
        """Run the sphinx build command"""
        verbosity = ""
        if Logs.verbose:
            verbosity = "-" + Logs.verbose * "v"
        cmd = " ".join(
            [
                sys.executable,
                "-m sphinx",
                "-b ${BUILDERNAME}",
                "-n",  # run in nit-picky mode
                "-W",  # raise warnings to errors
                "--keep-going",  # continue building even if there are errors
                "-c ${CONFDIR}",
                "-D ${VERSION}",
                "-D ${RELEASE}",
                "-D ${PROJECT}",
                "-D graphviz_dot=${DOT}",
                "-d ${DOCTREEDIR}",
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
            outdir = Utils.subst_vars("${OUTDIR}", self.env)
            Logs.info(f"Index file: {outdir}{os.sep}index.html.")
        return proc.returncode

    def __str__(self) -> str:
        """Return the builder name and input paths."""
        return (
            self.env["BUILDERNAME"] + " " + " ".join([a.relpath() for a in self.inputs])
        )

    def keyword(self) -> Literal["Compiling"]:  # noqa: D102
        return "Compiling"


@TaskGen.feature("sphinx")
@TaskGen.before_method("process_source")
def apply_sphinx(self: TaskGen.task_gen) -> None:
    """Set up the task generator with a Sphinx instance and create a task."""
    # get sphinx config (conf.py) and derive the srcdir from it.
    if not getattr(self, "conf_py", None):
        err_msg = "Path to the sphinx config must be specified ('conf.py')."
        raise ValueError(err_msg)
    if not isinstance(self.conf_py, Node):
        self.conf_py = self.path.find_node(self.conf_py)
    src_dir = self.conf_py.parent.abspath()

    # set the output directory
    if not getattr(self, "out_dir", None):
        err_msg = "out_dir must be specified."
        raise ValueError(err_msg)
    self.out_dir_node = self.path.find_or_declare(self.out_dir).get_bld()

    # add builders, build at least html documentation
    builders = []
    if not getattr(self, "builders", None):
        builders.append("html")
    else:
        builders.extend(Utils.to_list(self.builders))

    for builder in builders:
        outfile = self.path.get_bld().make_node(builder)
        outfile.parent.mkdir()
        outfile.write(builder)
        version_file = outfile.parent.make_node("version_macro.txt")
        self.create_task("create_version_file", tgt=version_file)
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
        try:
            _version = self.version
        except AttributeError:
            _version = self.env.VERSION
        try:
            _release = self.release
        except AttributeError:
            _release = self.env.VERSION
        try:
            _project = self.project
        except AttributeError:
            _project = self.env.APPNAME
        builder_task.env["OUTDIR"] = self.out_dir_node.abspath()
        builder_task.env["VERSION"] = f"version={_version}"
        builder_task.env["RELEASE"] = f"release={_release}"
        builder_task.env["PROJECT"] = f"project={_project}"
        builder_task.outputs.append(self.out_dir_node)


class CleanDocs(CleanContext):
    """Custom clean command that removes generated documentation artifacts."""

    cmd = "clean_docs"
    variant = "docs"

    def clean(self) -> None:
        """Remove build outputs and generated documentation artifacts."""
        super().clean()

        docs_src = self.srcnode.find_dir("docs")
        for node in docs_src.ant_glob("**/*_autosummary", src=False, dir=True):
            node.delete()

        for node in docs_src.ant_glob("**/*_autosummary_hash.json"):
            node.delete()


@conf
def get_sphinx_build_version(ctx: ConfigurationContext) -> None:
    """Determine 'sphinx-build' version"""
    cmd = ctx.env.SPHINX_BUILD + ["--version"]
    std_out, std_err = ctx.cmd_and_log(cmd, output=Context.BOTH)
    if std_err:
        ctx.fatal(f"Could not successfully run '--version' on {ctx.env.SPHINX_BUILD}.")
    std_out = std_out.strip()
    try:
        sphinx_build_version = std_out.split()[1]
    except IndexError:
        Logs.warn("Could not determine 'sphinx-build' version.")
        sphinx_build_version = "unknown"
    ctx.env.SPHINX_BUILD_VERSION = sphinx_build_version


@conf
def find_sphinx_build(ctx: ConfigurationContext) -> None:
    """Find the 'sphinx-build' executable and determine its version."""
    ctx.find_program("sphinx-build", var="SPHINX_BUILD")
    ctx.get_sphinx_build_version()


def configure(ctx: ConfigurationContext) -> None:
    """Find 'sphinx-build' executable."""
    ctx.find_sphinx_build()
    ctx.find_dot()
    ctx.find_drawio()
