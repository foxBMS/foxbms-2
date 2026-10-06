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

# cspell:ignore NETTCP,targetpath

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

from waflib import Task, TaskGen, Utils
from waflib.Configure import ConfigurationContext
from waflib.Options import OptionsContext

if Utils.is_win32:
    import win32com.client


class update_lauterbach_script(Task.Task):
    """Task create the CRC file from the .bin file"""

    color = "CYAN"
    after = ["link_task"]

    def run(self) -> None:
        """Write a Lauterbach script populated with current CRC and app metadata."""
        app_info = self.inputs[1].read_json()
        with open(self.inputs[0].abspath(), "rb") as f:
            try:  # catch OSError in case of a one line file
                f.seek(-2, os.SEEK_END)
                while f.read(1) != b"\n":
                    f.seek(-2, os.SEEK_CUR)
            except OSError:
                f.seek(0)
            last_line = f.readline().decode("utf-8")
        crc_8bytes = int(last_line.split(",")[-1])
        cmm = AtTemplate(self.inputs[2].read(encoding="utf-8"))
        cmm_txt = cmm.substitute(
            {
                "BOOT_PROGRAM_INFO_ADDRESS_BASE": "0x00018000",
                "BOOT_PROGRAM_INFO_MAGIC_NUM": "0xAAAAAAAA",
                "BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_LEN": "0x00018004",
                "BOOT_PROGRAM_INFO_PROGRAM_LEN": f"0x{app_info['app_size']:X}",
                "BOOT_PROGRAM_INFO_ADDRESS_CRC_8_BYTES": "0x0001800C",
                "BOOT_PROGRAM_INFO_CRC_8_BYTES": f"0x{crc_8bytes:X}",
                "BOOT_PROGRAM_INFO_ADDRESS_VECTOR_CRC_8_BYTES": "0x00018014",
                "BOOT_PROGRAM_INFO_VECTOR_CRC_8_BYTES": f"0x{app_info['vector_table_crc']:X}",
                "BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE": "0x0001801C",
                "BOOT_PROGRAM_IS_AVAILABLE": "0xCCCCCCCC",
                "BOOT_VECTOR_TABLE_BACKUP_ADDRESS_1": "0x00018064",
                "BOOT_VECTOR_TABLE_BACKUP_ADDRESS_2": "0x0001806C",
                "BOOT_VECTOR_TABLE_BACKUP_ADDRESS_3": "0x00018074",
                "BOOT_VECTOR_TABLE_BACKUP_ADDRESS_4": "0x0001807C",
            }
        )
        self.outputs[0].write(cmm_txt)


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_lauterbach_task(self: TaskGen.task_gen) -> None:
    """Add a task to update the Lauterbach script with the CRC values."""
    if not getattr(self, "app_build_cfg", False):
        return
    if not hasattr(self, "crc_task"):
        return
    in_file = "tools/debugger/lauterbach/update_program_information.cmm.in"
    src = self.crc_task.outputs + [self.bld.srcnode.find_resource(in_file)]

    tgt = self.bld.path.find_or_declare(
        self.bld.out_dir + "/update_program_information.cmm"
    )
    self.create_task("update_lauterbach_script", src=src, tgt=[tgt])


class AtTemplate(Template):
    """Custom 'Template'-string to support the '@{abc}' syntax"""

    delimiter = "@"


def options(opt: OptionsContext) -> None:
    """Lauterbach waf tool configuration options"""
    if Utils.is_win32:
        home = os.getenv("HOMEDRIVE", "C:") + os.sep
    else:
        home = os.getenv("HOME", "~")

    lauterbach_installation_directory = [os.path.join(home, "T32")]

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


def configure(ctx: ConfigurationContext) -> None:
    """Configuration step of the Lauterbach waf tool"""
    if not Utils.is_win32:
        return

    ctx.start_msg("Checking for Lauterbach installation")
    ctx.find_program("filecvt", var="FILECVT", mandatory=False)
    if not ctx.env.FILECVT:
        ctx.find_program(
            "filecvt",
            var="FILECVT",
            path_list=ctx.options.LAUTERBACH_BASE,
            mandatory=False,
        )
    ctx.find_program("t32marm", var="T32MARM", mandatory=False)
    if not ctx.env.T32MARM:
        lauterbach_bin_directories = [
            os.path.join(i, "bin", "windows64") for i in ctx.options.LAUTERBACH_BASE
        ]
        ctx.find_program(
            "t32marm",
            var="T32MARM",
            path_list=lauterbach_bin_directories,
            mandatory=False,
        )
    ctx.end_msg(ctx.env.get_flat("T32MARM"))

    if not ctx.env.T32MARM:
        return

    tmp_dir = ctx.path.get_bld().make_node("tmp")
    tmp_dir.mkdir()

    if ctx.env.FILECVT:
        t32_root = ctx.root.find_node(ctx.env.FILECVT[0]).parent.abspath()
        if t32_root.endswith(os.sep):
            t32_root = t32_root[:-1]
    else:
        # the executable is in '<root>/<platform>/bin/...'
        t32_root = ctx.root.find_node(ctx.env.T32MARM[0]).parent.parent.parent.abspath()
        if t32_root.endswith(os.sep):
            t32_root = t32_root[:-1]

    t32marm_root = ctx.root.find_node(ctx.env.T32MARM[0]).parent.abspath()
    if t32marm_root.endswith(os.sep):
        t32marm_root = t32marm_root[:-1]

    tcp = ""
    if ctx.options.lauterbach_use_tcp:
        tcp = "RCL=NETTCP\nPORT=20000"  # cspell:ignore NETTCP

    # configuration input files
    base = "tools/debugger/lauterbach"
    config_t32_in = ctx.path.find_node(f"{base}/config.t32.in")
    init_cmm_in = ctx.path.find_node(f"{base}/init.cmm.in")
    t32_cmm_in = ctx.path.find_node(f"{base}/t32.cmm.in")
    load_macro_values_in = ctx.path.find_node(f"{base}/load_macro_values.cmm.in")

    # actual configuration as text
    init_cmm = AtTemplate(init_cmm_in.read())
    config_t32 = AtTemplate(config_t32_in.read())
    t32_cmm = AtTemplate(t32_cmm_in.read())
    load_macro_values = AtTemplate(load_macro_values_in.read())

    # define configuration files
    config_t32_node = ctx.path.get_bld().make_node("config.t32")
    init_cmm_node = ctx.path.get_bld().make_node("init.cmm")
    t32_cmm_node = ctx.path.get_bld().make_node("t32.cmm")
    load_macro_values_node = ctx.path.get_bld().make_node("load_macro_values.cmm")

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
                "bootloader_ti_arm_cgt",
                "src",
                "bootloader",
                "main",
                "foxbms-bootloader.elf",
            ),
            "BOOTLOADER_ELF_SEARCHPATH": os.path.join(
                "bootloader_ti_arm_cgt", "src", "bootloader", "main", "*.elf"
            ),
            "APP_ELF_FILE": os.path.join(
                "app_ti_arm_cgt", "src", "app", "main", "foxbms.elf"
            ),
            "APP_ELF_SEARCHPATH": os.path.join(
                "app_ti_arm_cgt", "src", "app", "main", "*.elf"
            ),
            "T32_CMM_FILE": t32_cmm_node.abspath(),
            "UPDATE_PROGRAM_INFORMATION_SCRIPT": "update_program_information.cmm",
        }
    )

    # t32.cmm
    t32_cmm = t32_cmm.substitute({"INIT_FILE": init_cmm_node.abspath()})

    # load_macro_values.cmm
    load_macro_values = load_macro_values.substitute({"MACROS_AND_VALUES": ""})

    # write all configurations
    config_t32_node.write(config_t32)
    init_cmm_node.write(init_cmm)
    t32_cmm_node.write(t32_cmm)
    load_macro_values_node.write(load_macro_values)

    if Utils.is_win32:
        # create a shortcut
        path = os.path.join(ctx.path.get_bld().abspath(), "run_t32marm.lnk")
        shell = win32com.client.Dispatch("WScript.Shell")
        shortcut = shell.CreateShortCut(path)
        shortcut.Targetpath = ctx.env.T32MARM[0]  # cspell:ignore Targetpath
        shortcut.WorkingDirectory = t32marm_root
        shortcut.Arguments = " ".join(
            ["-c", config_t32_node.abspath(), "-s", t32_cmm_node.abspath()]
        )
        shortcut.WindowStyle = 3
        shortcut.save()
