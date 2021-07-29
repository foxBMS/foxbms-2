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

"""Implements a waf tool to flash binaries to the foxBMS MCU
"""

import os

from waflib import Context, Errors, Logs, Task, Utils
from waflib.Configure import conf


MINIMUM_J_FLASH_VERSION = "V7.22"
JFLASH_ENV_VAR = "JFLASH"


@conf
def check_j_flash_version(ctx):
    """
    Check if J-Flash has the correct minimum version for the features that we
    use.

    If not successful, J-Flash is discarded from the found programs
    """
    jflash = ctx.env.JFLASH
    if not jflash:
        Logs.debug("J-Flash not found; aborting version check")
        return
    Logs.debug("J-Flash found; checking version")
    # check if a stale version file is present. If so delete it.
    file_name = "jflash-version.log"
    stale_file = ctx.path.get_bld().find_node(file_name)
    if stale_file:
        Logs.debug(f"Found stale version file {stale_file}; deleting it")
        stale_file.delete()
    # Create a new node to store the version information output from J-Flash
    jflash_version_node = ctx.path.find_or_declare(file_name)

    cmd = [
        Utils.subst_vars("${JFLASH}", ctx.env),
        f"-jflashlog{jflash_version_node.abspath()}",
        "-hide",
        "-exit",
    ]

    try:
        ctx.cmd_and_log(cmd, output=Context.BOTH)
    except Errors.WafError as jflash_call:
        Logs.error(jflash_call.msg.strip())
        Logs.error("Checking the J-Flash version failed")
        return

    version = "V0.0"
    txt = jflash_version_node.read(encoding="utf-8")
    for line in txt.splitlines():
        if "j-flash v" in line.lower():
            # tokenize line
            split_line = line.split(" ")
            # extract the string that starts with V
            version = list(filter(lambda v: v.startswith("V"), split_line))[0]

    Logs.debug(f"Extracted version {version}")
    Logs.debug(
        f"Parsing and comparing version to threshold version {MINIMUM_J_FLASH_VERSION}"
    )

    def versiontuple(version_string):
        extract_version = version_string.lstrip()
        split_version = extract_version.split(".")
        major = split_version[0]
        minor = split_version[1]
        alpha = ""
        # versions may have a alphabetic character at the end
        if minor[-1].isalpha():
            alpha = minor[-1]
            minor = minor[:-1]
        Logs.debug(f"decoded a version to {major}, {minor}, {alpha}")
        return major, minor, alpha

    if versiontuple(version) >= versiontuple(MINIMUM_J_FLASH_VERSION):
        Logs.debug("Version of J-Flash is OK")
    else:
        Logs.warn(
            f"The installed version of J-Flash is too old (Installed: {version},"
            f" Required: {MINIMUM_J_FLASH_VERSION})."
        )
        Logs.debug(
            f"Removing (invalid) version of J-Flash ({JFLASH_ENV_VAR}) from env."
        )
        ctx.env.__delitem__(JFLASH_ENV_VAR)


def configure(ctx):
    """configuration step of the flash waf tool:

    - Find J-Flash
    - Check that version matches the minimum version
    """

    # search for J-Flash in path
    ctx.start_msg("Checking for program 'jflash'")
    if not Utils.is_win32:  # currently on support for Windows OS
        ctx.end_msg(False)
        return

    ctx.find_program("JFlash", var=JFLASH_ENV_VAR, mandatory=False)
    if ctx.env.JFLASH:  # We found jflash in Path - good to go!
        ctx.end_msg(ctx.env.get_flat(JFLASH_ENV_VAR))
        return

    # search for J-Flash in known paths
    base_installdir = os.path.join(os.environ["ProgramFiles"], "SEGGER")
    base_installdir_x86 = os.path.join(os.environ["ProgramFiles(x86)"], "SEGGER")
    segger_installdir = []
    segger_installdir_x86 = []
    try:
        segger_installdir = os.listdir(base_installdir)
        segger_installdir_x86 = os.listdir(base_installdir_x86)
    except FileNotFoundError:
        pass
    path_list = []
    for i in [os.path.join(base_installdir, x) for x in segger_installdir]:
        path_list.append(i)
    for i in [os.path.join(base_installdir_x86, x) for x in segger_installdir_x86]:
        path_list.append(i)
    ctx.find_program("JFlash", var=JFLASH_ENV_VAR, path_list=path_list, mandatory=False)
    ctx.end_msg(ctx.env.get_flat(JFLASH_ENV_VAR))
    ctx.check_j_flash_version()


class inst(Task.Task):  # pylint: disable=invalid-name
    """Installer class"""

    def __str__(self):
        return ""

    def uid(self):
        # pylint: disable=no-member
        lst = self.inputs + self.outputs + [self.link, self.generator.path.abspath()]
        return Utils.h_list(lst)

    def init_files(self):
        """Initialize files used by the task."""
        # pylint: disable=no-member
        inputs = self.generator.to_nodes(self.install_from)
        self.set_inputs(inputs)

    def runnable_status(self):
        """runnable status of the flash tool"""
        ret = super().runnable_status()
        if ret == Task.SKIP_ME and self.generator.bld.is_install:
            return Task.RUN_ME
        return ret

    def post_run(self):
        pass

    def run(self):
        """run J-Flash"""
        is_install = self.generator.bld.is_install
        if not is_install:
            return 0
        if not self.generator.bld.env.JFLASH:
            self.generator.bld.fatal("No debugger configured in the flash tool.")
        elf_file = list(filter(lambda x: x.suffix() == ".elf", self.inputs))[0]
        cmd = [
            Utils.subst_vars("${JFLASH}", self.generator.bld.env),
            "-openprjtools/debugger/ozone/foxbms.jflash",
            f"-open{elf_file}",
            "-auto",
            "-startapp",
            "-exit",
        ]

        try:
            self.generator.bld.cmd_and_log(
                cmd, output=Context.BOTH, cwd=self.generator.bld.path.abspath()
            )
        except Errors.WafError as jflash_call:
            Logs.error(jflash_call.msg.strip())
            Logs.error("Running J-Flash failed")
            return 1
        return 0

    def run_now(self):
        """implementation of waf run_now method"""
        status = self.runnable_status()
        if status not in (Task.RUN_ME, Task.SKIP_ME):
            raise Errors.TaskNotReady(f"Could not process {self}: status {status}")
        self.run()
        self.hasrun = Task.SUCCESS
