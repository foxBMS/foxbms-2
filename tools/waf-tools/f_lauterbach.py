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

"""Implements a waf tool to configure a Lauterbach to foxBMS specific needs.

For information on Lauterbach see the
`Lauterbach website <https://www.lauterbach.com/frames.html?home.html>`_.

This waf tool automatically generates a project configuration for Lauterbach
Trace32, when the binary of a compatible Lauterbach Trace32 is found on
the system. Check the output of the configure step for whether a
Lauterbach installation has been found if you suspect any issues.

After successful configuration of the project with the configure task the
Lauterbach Trace32 configuration files will be available in the build
directory of this project. In order to run the debugger simply use the
created link called ``run_t32marm`` in the build directory which will start
a new instance of Trace32.
"""

import os
from string import Template

from waflib import Utils
from waflib.Configure import ConfigurationContext

if Utils.is_win32:
    import win32com.client  # pylint: disable=import-error


class AtTemplate(Template):
    """Custom 'Template'-string to support the '@{abc}' syntax"""

    delimiter = "@"


def options(opt):
    """Lauterbach waf tool configuration options"""
    homedrive = os.getenv("HOMEDRIVE", "C:")
    lauterbach_installation_directory = [
        os.path.join(homedrive, os.sep, "T32-tms"),
        os.path.join(homedrive, os.sep, "T32"),
    ]
    opt.add_option(
        "--lauterbach-installation-directory",
        action="append",
        default=lauterbach_installation_directory,
        dest="LAUTERBACH_BASE",
        help="Installation directory of Lauterbach tools",
    )
    opt.add_option(
        "--lauterbach-use-tcp",
        action="store_true",
        dest="lauterbach_use_tcp",
        help="Use TCP as connection for the debugger.",
    )


def configure(conf: ConfigurationContext):
    """configuration step of the Lauterbach waf tool"""
    if not Utils.is_win32:
        return

    conf.start_msg("Checking for Lauterbach installation")
    conf.find_program("filecvt", var="FILECVT", mandatory=False)
    if not conf.env.FILECVT:
        conf.find_program(
            "filecvt",
            var="FILECVT",
            path_list=conf.options.LAUTERBACH_BASE,
            mandatory=False,
        )
    conf.find_program("t32marm", var="T32MARM", mandatory=False)
    if not conf.env.T32MARM:
        lauterbach_bin_directories = [
            os.path.join(i, "bin", "windows64") for i in conf.options.LAUTERBACH_BASE
        ]
        conf.find_program(
            "t32marm",
            var="T32MARM",
            path_list=lauterbach_bin_directories,
            mandatory=False,
        )
    conf.end_msg(conf.env.get_flat("T32MARM"))

    if not conf.env.FILECVT or not conf.env.T32MARM:
        return

    tmp_dir = conf.path.get_bld().make_node("tmp")
    tmp_dir.mkdir()

    t32_root = conf.root.find_node(conf.env.FILECVT[0]).parent.abspath()
    if t32_root.endswith(os.sep):
        t32_root = t32_root[:-1]

    t32marm_root = conf.root.find_node(conf.env.T32MARM[0]).parent.abspath()
    if t32marm_root.endswith(os.sep):
        t32marm_root = t32marm_root[:-1]

    tcp = ""
    if conf.options.lauterbach_use_tcp:
        tcp = "RCL=NETTCP\nPORT=20000"

    # configuration input files
    base = "tools/debugger/lauterbach"
    config_t32_in = conf.path.find_node(f"{base}/config.t32.in")
    init_cmm_in = conf.path.find_node(f"{base}/init.cmm.in")
    t32_cmm_in = conf.path.find_node(f"{base}/t32.cmm.in")
    load_macro_values_in = conf.path.find_node(f"{base}/load_macro_values.cmm.in")

    # actual configuration as text
    init_cmm = AtTemplate(init_cmm_in.read())
    config_t32 = AtTemplate(config_t32_in.read())
    t32_cmm = AtTemplate(t32_cmm_in.read())
    load_macro_values = AtTemplate(load_macro_values_in.read())

    # define configuration files
    config_t32_node = conf.path.get_bld().make_node("config.t32")
    init_cmm_node = conf.path.get_bld().make_node("init.cmm")
    t32_cmm_node = conf.path.get_bld().make_node("t32.cmm")
    load_macro_values_node = conf.path.get_bld().make_node("load_macro_values.cmm")

    # config.t32
    config_t32 = config_t32.substitute(
        {
            "TMP": os.getenv("TMP"),
            "SYS": t32_root,
            "TCP": tcp,
        }
    )

    # init.cmm
    init_cmm = init_cmm.substitute(
        {
            "BOOTLOADER_ELF_FILE": os.path.join(
                "bootloader_embedded", "foxbms-bootloader.elf"
            ),
            "BOOTLOADER_ELF_SEARCHPATH": os.path.join("bootloader_embedded", "*.elf"),
            "APP_ELF_FILE": os.path.join("app_embedded", "foxbms.elf"),
            "APP_ELF_SEARCHPATH": os.path.join("app_embedded", "*.elf"),
            "T32_CMM_FILE": t32_cmm_node.abspath(),
            "UPDATE_PROGRAM_INFORMATION_SCRIPT": "update_program_information.cmm",
        }
    )

    # t32.cmm
    t32_cmm = t32_cmm.substitute(
        {
            "INIT_FILE": init_cmm_node.abspath(),
        }
    )

    # load_macro_values.cmm
    replacements = ""
    load_macro_values = load_macro_values.substitute(
        {
            "MACROS_AND_VALUES": replacements,
        }
    )

    # write all configurations
    config_t32_node.write(config_t32)
    init_cmm_node.write(init_cmm)
    t32_cmm_node.write(t32_cmm)
    load_macro_values_node.write(load_macro_values)

    if Utils.is_win32:
        # create a shortcut
        path = os.path.join(conf.path.get_bld().abspath(), "run_t32marm.lnk")
        shell = win32com.client.Dispatch("WScript.Shell")
        shortcut = shell.CreateShortCut(path)
        shortcut.Targetpath = conf.env.T32MARM[0]
        shortcut.WorkingDirectory = t32marm_root
        shortcut.Arguments = " ".join(
            ["-c", config_t32_node.abspath(), "-s", t32_cmm_node.abspath()]
        )
        shortcut.WindowStyle = 3
        shortcut.save()
