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

"""Implements a waf tool to configure Cppcheck

For information on Cppcheck see http://cppcheck.sourceforge.net/.
"""

import os
import pathlib
from waflib import Node, Utils, Task, TaskGen


class compile_cppcheck_config(Task.Task):  # pylint: disable=invalid-name
    """Compiles the cppcheck template for the provided setup"""

    def run(self):
        """Replaces the configuration variables"""
        txt = self.inputs[0].read()
        txt = txt.replace("@root@", self.generator.root_val)
        txt = txt.replace("@paths@", self.generator.paths_val)
        txt = txt.replace("@exclude@", self.generator.exclude_val)
        txt = txt.replace("@addons@", self.generator.addons_val)
        self.outputs[0].write(txt)


class copy_cppcheck_suppressions(Task.Task):  # pylint: disable=invalid-name
    """Copies the suppression file to the build directory"""

    def run(self):
        """Copies the file"""
        self.outputs[0].write(self.inputs[0].read())


class configure_misra(Task.Task):  # pylint: disable=invalid-name
    """Configures the misra addon"""

    vars = ["RULES_FILE", "MISRA_ARGS"]

    def run(self):
        """Actual configuration step"""
        misra_args = self.misra_args  # pylint: disable=no-member
        if self.env.RULES_FILE:
            misra_args = misra_args + [  # pylint: disable=no-member
                f"--rule-texts={self.env.RULES_FILE[0]}".replace("\\", "\\\\")
            ]
        tmp = ""
        if len(misra_args) == 1:
            tmp = "".join(misra_args)
        else:
            for i, val in enumerate(misra_args):
                if i == 0:
                    tmp += f'{val}",\n'
                elif i == len(misra_args) - 1:
                    tmp += f'        "{val}'
                else:
                    tmp += f'        "{val}",\n'
        tmp = tmp.rstrip()
        misra_args = tmp

        txt = self.inputs[0].read()
        txt = txt.replace("@misra_args@", misra_args)
        self.outputs[0].write(txt)


class cppcheck(Task.Task):  # pylint: disable=invalid-name
    """The actual cppcheck task"""

    vars = ["CPPCHECK_OPTIONS", "RULES_FILE", "MISRA_ARGS"]

    def run(self):
        """Runs cppcheck with the provided configuration."""
        cmd = self.generator.bld.env.CPPCHECK + [
            f"--project={self.inputs[0].abspath()}",
            f"--cppcheck-build-dir={self.generator.bld.bldnode.abspath()}",
        ]
        if hasattr(self.generator, "suppressions"):
            cmd.extend(
                [
                    f"--exitcode-suppressions={self.generator.suppressions.abspath()}",
                    f"--suppressions-list={self.generator.suppressions.abspath()}",
                ]
            )
        if hasattr(self.generator, "exit_code"):
            cmd.append(f"--error-exitcode={self.generator.exit_code}")
        if hasattr(self.generator, "misra") and hasattr(self, "comp_misra"):
            cmd.append(f"--addon={self.comp_misra}")  # pylint: disable=no-member
        cmd.extend(self.generator.options)
        return self.exec_command(cmd, cwd=self.generator.bld.bldnode.abspath())


@TaskGen.feature("cppcheck")
def add_cppcheck_task(self):
    """Task creator for cppcheck
    Attributes to be processed are:

    - config: str or node where the cppcheck configuration is stored
    - root (optional): str or node where the project root is (default: '.')
    - paths (optional): list of str or nodes that should be checked
      (default: '.')
    - exclude (optional): list of str or nodes that should be excluded
      (default: none)
    - addons (optional): list of addon that should be used
    - options (optional): list of options that should be passed to tool
    - misra (optional): configuration of the misra tool
    - suppressions (optional): str or node that includes warnings that should
      be suppressed
    - exit_code: int that should be used as exit code in error case"""

    # validate that all required settings have been and fill optional inputs
    # with defaults
    if not hasattr(self, "config"):
        self.bld.fatal("A cppcheck configuration must be provided.")
    self.config = self.convert_to_node(self.config)
    if not hasattr(self, "config"):
        self.bld.fatal("Could not find cppcheck configuration.")

    if not hasattr(self, "root"):
        self.root = self.path.find_node(".")
    self.root = self.convert_to_node(self.root)
    if not self.root:
        self.bld.fatal("invalid root directory.")

    root_pattern = '    <root name="{}"/>'
    self.root_val = root_pattern.format(pathlib.Path(self.root.abspath()).as_posix())

    if not hasattr(self, "paths"):
        self.paths = self.path.find_node(".")
    self.paths = self.convert_to_nodes(self.paths)
    if not self.paths:
        self.fatal("invalid paths.")

    dir_pattern = '        <dir name="{}"/>\n'
    self.paths_val = ""
    for i in self.paths:
        self.paths_val += dir_pattern.format(pathlib.Path(i.abspath()).as_posix())
    self.paths_val = self.paths_val.rstrip()

    self.exclude_val = ""
    if hasattr(self, "exclude"):
        self.exclude = self.convert_to_nodes(self.exclude)
        for i in self.exclude:
            self.exclude_val += dir_pattern.format(pathlib.Path(i.abspath()).as_posix())
        self.exclude_val = self.exclude_val.rstrip()

    if not hasattr(self, "addons"):
        self.addons = ""
    self.addons = Utils.to_list(self.addons)
    addon_pattern = "        <addon>{}</addon>\n"
    self.addons_val = ""
    for i in self.addons:
        self.addons_val += addon_pattern.format(i)
    self.addons_val = self.addons_val.rstrip()

    if hasattr(self, "options"):
        self.options = Utils.to_list(self.options)
    else:
        self.options = [""]
    self.env.CPPCHECK_OPTIONS = hash("".join(self.options))
    self.compile_cppcheck_config = self.create_task(
        "compile_cppcheck_config",
        src=self.config,
        tgt=self.bld.path.find_or_declare(self.config.name),
    )
    src = self.compile_cppcheck_config.outputs
    if hasattr(self, "suppressions"):
        if not isinstance(self.suppressions, Node.Node):
            self.suppressions = self.path.find_node(self.suppressions)
        if not self.suppressions:
            self.bld.fatal("invalid suppressions file.")
        self.copy_cppcheck_suppressions = self.create_task(
            "copy_cppcheck_suppressions",
            src=self.suppressions,
            tgt=self.bld.path.find_or_declare(self.suppressions.name),
        )
        src = src + self.copy_cppcheck_suppressions.outputs
    if hasattr(self, "misra"):
        if not isinstance(self.misra, Node.Node):
            self.misra = self.path.find_node(self.misra)
        if getattr(self, "misra_args", ""):
            if isinstance(self.misra_args, list):
                self.misra_args = " ".join(self.misra_args)
            self.env.MISRA_ARGS = hash(self.misra_args)
        self.configure_misra = self.create_task(
            "configure_misra",
            src=self.misra,
            tgt=self.bld.path.find_or_declare(self.misra.name),
            misra_args=Utils.to_list(getattr(self, "misra_args", "")),
        )
        src = src + self.configure_misra.outputs
    self.create_task(
        "cppcheck",
        src=src,
        comp_misra=self.configure_misra.outputs[0].name,
    )


def options(opt):
    """Defines options that can be passed to cppcheck tool"""
    opt.add_option(
        "--misra-rules-text",
        action="store",
        dest="misra_rules_text",
        help="Sets the path to the MISRA rules file for cppcheck",
    )


def configure(conf):
    """configuration step of the Cppcheck waf tool

    - Find cppcheck
    - Search for the MISRA-C rules text"""
    # for static analysis
    conf.find_program("cppcheck", mandatory=False)
    if not conf.env.CPPCHECK:
        return

    conf.start_msg("Checking for MISRA-C rules file")

    rules_file = None
    if conf.options.misra_rules_text:
        if not conf.root.find_node(os.path.abspath(conf.options.misra_rules_text)):
            conf.fatal(
                f"{os.path.abspath(conf.options.misra_rules_text)} does not exist."
            )
        rules_file = [
            conf.root.find_node(os.path.abspath(conf.options.misra_rules_text))
        ]
    else:
        if Utils.is_win32:
            doc_paths = [
                os.path.join(os.path.expanduser("~"), "Documents", "MISRA-C"),
                os.path.join(os.environ["PUBLIC"], "Documents", "MISRA-C"),
            ]
        else:
            doc_paths = [
                os.path.join(os.path.expanduser("~"), "MISRA-C"),
            ]
        rules_file = [
            conf.root.find_node(os.path.join(x, "rules-2012.txt")) for x in doc_paths
        ]
    rules_file = list(filter(None, rules_file))
    if any(rules_file):
        conf.env.append_unique("RULES_FILE", rules_file[0].abspath())
    conf.end_msg(conf.env.get_flat("RULES_FILE"))
