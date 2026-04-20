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

"""Implements a waf tool to use TI ARM CGT (https://www.ti.com/tool/ARM-CGT)."""

# pylint: disable=too-many-lines

import binascii
import os
import re
import shutil
from hashlib import md5
from string import Template
from pathlib import Path


import crc_bootloader
import f_ti_arm_cgt_cc_options  # noqa: F401 pylint: disable=unused-import
import f_ti_arm_helper  # noqa: F401 pylint: disable=unused-import
import f_ti_arm_tools  # noqa: F401 pylint: disable=unused-import
import waflib.Tools.asm
from waflib import Context, Logs, Task, TaskGen, Utils
from waflib.Build import BuildContext
from waflib.Configure import conf
from waflib.Node import Node
from waflib.TaskGen import taskgen_method
from waflib.Tools import c_preproc
from waflib.Tools.ccroot import link_task


TOOL_DIR = os.path.dirname(os.path.realpath(__file__))


class AtTemplate(Template):
    """Custom 'Template'-string to support the '@{abc}' syntax"""

    delimiter = "@"


def remove_targets(task):
    """General helper function to remove targets"""
    for target in task.outputs:
        if os.path.exists(target.abspath()):
            os.remove(target.abspath())


def options(opt):
    """Configurable options of the :py:mod:`f_ti_arm_cgt` tool.

    Furthermore the default formatter gets replaced by
    :py:class:`f_ti_arm_cgt.armclFormatter`.
    """
    opt.load("f_ti_arm_cgt_cc_options", tooldir=TOOL_DIR)
    opt.load("f_hcg", tooldir=TOOL_DIR)


@TaskGen.extension(".asm")
def asm_hook(self, node):
    """Creates the compiled task for assembler sources
    :py:meth:`f_ti_arm_cgt.create_compiled_task_asm`.
    """
    return self.create_compiled_task_asm("asm", node)


@taskgen_method
def create_compiled_task_asm(self, name, node):
    """Creates the assembler task and binds it to the
    :py:class:`f_ti_arm_cgt.asm` class.

    The created tasks are appended to the list of ``compiled_tasks``.
    """
    out_bin = f"{node.name}.{self.idx}.obj"
    task = self.create_task(name, node, node.parent.find_or_declare(out_bin))
    try:
        self.compiled_tasks.append(task)
    except AttributeError:
        self.compiled_tasks = [task]
    return task


class asm(Task.Task):
    """class to run the TI ARM CGT compiler in compiler mode to create object
    files from assembler sources.
    """

    vars = ["CCDEPS"]
    ext_in = [".h"]
    scan = c_preproc.scan
    run_str = (
        "${CC} ${CFLAGS} ${CC_COMPILE_ONLY} "
        "${ASM_DIRECTORY}${TGT[0].parent.bldpath()} ${CPPPATH_ST:INCPATHS} "
        "${DEFINES_ST:DEFINES} ${SRC} ${CC_TGT_F}${TGT[0].relpath()}"
    )

    def keyword(self):  # noqa: D102
        return "Compiling"


@TaskGen.extension(".c")
def c_hook(self, node):
    """Creates all related task generators for C sources, which are

    - :py:meth:`create_compiled_task_c`,
    - :py:meth:`create_compiled_task_c_pp`,
    - :py:meth:`create_compiled_task_c_ppi`,
    - :py:meth:`create_compiled_task_c_ppd` and
    - :py:meth:`create_compiled_task_c_ppm`.
    """
    return (
        self.create_compiled_task_c("c", node),
        self.create_compiled_task_c_pp("c_pp", node),
        self.create_compiled_task_c_ppi("c_ppi", node),
        self.create_compiled_task_c_ppd("c_ppd", node),
        self.create_compiled_task_c_ppm("c_ppm", node),
    )


@taskgen_method
def create_compiled_task_c(self, name, node):
    """Creates the assembler task and binds it to the
    :py:class:`f_ti_arm_cgt.c` class.

    The created tasks are appended to the list of ``compiled_tasks``.
    """
    if self.idx > 1:
        # The TI CGT tools do not allow setting an output file name for aux,
        # crl and rl files, therefore we need to warn the user here as these
        # auxiliary files might be overwritten without anybody noticing.
        Logs.warn(
            f"{node.relpath()} Consistency of .aux, .crl and .rl output files "
            "can not be guaranteed."
        )
    out_obj = f"{node.name}.{self.idx}.obj"
    out_aux = f"{node.name.rsplit('.')[0]}.aux"
    out_crl = f"{node.name.rsplit('.')[0]}.crl"
    out_rl = f"{node.name.rsplit('.')[0]}.rl"
    task = self.create_task(
        name,
        src=node,
        tgt=[
            node.parent.find_or_declare(out_obj),
            node.parent.find_or_declare(out_aux),
            node.parent.find_or_declare(out_crl),
            node.parent.find_or_declare(out_rl),
        ],
    )

    try:
        self.compiled_tasks.append(task)
    except AttributeError:
        self.compiled_tasks = [task]
    return task


@taskgen_method
def create_compiled_task_c_pp(self, name, node):
    """Creates the pp task and binds it to the
    :py:class:`f_ti_arm_cgt.c_pp` class.
    """
    out = f"{node.name}.{self.idx}.pp"
    task = self.create_task(name, node, node.parent.find_or_declare(out))
    try:
        self.c_pp_tasks.append(task)
    except AttributeError:
        self.c_pp_tasks = [task]
    return task


@taskgen_method
def create_compiled_task_c_ppi(self, name, node):
    """Creates the ppi task and binds it to the
    :py:class:`f_ti_arm_cgt.c_ppi` class.
    """
    out = f"{node.name}.{self.idx}.ppi"
    return self.create_task(name, node, node.parent.find_or_declare(out))


@taskgen_method
def create_compiled_task_c_ppd(self, name, node):
    """Creates the ppd task and binds it to the
    :py:class:`f_ti_arm_cgt.c_ppd` class.
    """
    out = f"{node.name}.{self.idx}.ppd"
    return self.create_task(name, node, node.parent.find_or_declare(out))


@taskgen_method
def create_compiled_task_c_ppm(self, name, node):
    """Creates the ppm task and binds it to the
    :py:class:`f_ti_arm_cgt.c_ppm` class.
    """
    out = f"{node.name}.{self.idx}.ppm"
    return self.create_task(name, node, node.parent.find_or_declare(out))


class c(Task.Task):
    """This class implements the TI ARM CGT compiler in compiler mode to create
    object files from c sources. Additionally an aux file (user information
    file), a crl files (cross-reference listing file) and a rl file (output
    preprocessor listing) are generated.
    """

    vars = ["CCDEPS", "CMD_FILES_HASH"]
    ext_in = [".h"]
    scan = c_preproc.scan
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${CFLAGS_COMPILE_ONLY} ${CC_COMPILE_ONLY} "
        "${OBJ_DIRECTORY}${TGT[0].parent.bldpath()} ${CPPPATH_ST:INCPATHS} "
        "${DEFINES_ST:DEFINES} ${SRC[0].abspath()} ${CC_TGT_F}${TGT[0].abspath()}"
    )

    def keyword(self):  # noqa: D102
        """Displayed keyword when source files are compiled"""
        return "Compiling"


class c_pp(Task.Task):
    """class to run the TI ARM CGT compiler in "preproc_only" mode, to
    have a pp file that includes the preprocess information
    """

    vars = ["CCDEPS", "CMD_FILES_HASH"]
    ext_in = [".h"]
    scan = c_preproc.scan
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPO} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


class c_ppi(Task.Task):
    """class to run the TI ARM CGT compiler in "preproc_includes" mode, to
    have a ppi file that includes the include information
    """

    vars = ["CCDEPS", "CMD_FILES_HASH"]
    ext_in = [".h"]
    scan = c_preproc.scan
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPI} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


class c_ppd(Task.Task):
    """class to run the TI ARM CGT compiler in "preproc_dependency" mode, to
    have a ppd file that includes dependency information
    """

    vars = ["CCDEPS", "CMD_FILES_HASH"]
    ext_in = [".h"]
    scan = c_preproc.scan
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPD} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


class c_ppm(Task.Task):
    """class to run the TI ARM CGT compiler in "preproc_macros" mode, to have
    a ppm file that includes all compile time macros
    """

    vars = ["CCDEPS", "CMD_FILES_HASH"]
    ext_in = [".h"]
    scan = c_preproc.scan
    color = "CYAN"
    run_str = (
        "${CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${PPM} ${CC_TGT_F}${TGT[0].abspath()} "
        "${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC[0].abspath()}"
    )

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


class cprogram(link_task):
    """class to run the TI ARM CGT compiler in linker mode to create the target"""

    run_str = (
        "${LINK_CC} ${CFLAGS} ${CMD_FILES_ST:CMD_FILES} ${RUN_LINKER} "
        "${LINKFLAGS} ${MAP_FILE}${TGT[2].abspath()} "
        "${XML_LINK_INFO}${TGT[1].abspath()} ${CCLINK_TGT_F}${TGT[0].abspath()} "
        "${SRC} ${LINKER_SCRIPT} "
        "${LIBPATH_ST:LIBPATH} ${STLIBPATH_ST:STLIBPATH} "  # library search paths
        " ${TI_ARM_CGT_LINKER_START_GROUP} "
        "${LIB_ST:LIB} ${STLIB_ST:STLIB} "
        "${TI_ARM_CGT_LINKER_END_GROUP} "
        "${TARGETLIB_ST:TARGETLIB} ${LDFLAGS}"
    )

    vars = ["LINKDEPS", "CMD_FILES_HASH"]
    ext_out = [".elf"]

    # set inst_to to a dummy value so that waf knows that it can execute
    # an installation task
    inst_to = True

    def keyword(self):  # noqa: D102
        return "Linking"


class stlink_task(link_task):
    """static link task"""

    run_str = [
        remove_targets,  # cspell:disable-next-line
        "${AR} ${ARFLAGS} ${AR_TGT_F} ${TGT[0].abspath()} ${AR_SRC_F}${SRC}",
    ]

    def keyword(self):  # noqa: D102
        return "Linking"


class cstlib(stlink_task):
    """c static library"""


class copy_to_out_dir(Task.Task):
    """This class implements the copying of a node to another location
    in the build tree
    """

    weight = 3
    color = "CYAN"
    after = ["link_task"]

    def run(self):  # noqa: D102
        """Copy the generated elf file to build directory of the variant"""
        for in_file, out_file in zip(self.inputs, self.outputs, strict=False):
            shutil.copy2(in_file.abspath(), out_file.abspath())

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_copy_elf_task(self):
    """Creates a task to copy the elf file into the output root
    (task :py:class:`f_ti_arm_cgt.copy_to_out_dir`)
    """
    if self.bld.variant_dir == self.link_task.outputs[0].parent.abspath():
        return
    if not hasattr(self, "link_task"):
        return
    if self.bld.variant_dir:
        out_dir = self.bld.variant_dir
    else:
        out_dir = self.bld.path.get_bld()
    self.copy_elf_task = self.create_task(
        "copy_to_out_dir",
        src=self.link_task.outputs,
        tgt=[
            self.bld.path.find_or_declare(os.path.join(out_dir, i.name))
            for i in self.link_task.outputs
        ],
    )


@conf
def tiprogram(bld: BuildContext, *k, **kw):
    """Wrapper for bld.program for simpler target configuration.
    The linker script is added as env input/output dependency for the
    target.
    Based on the target name all related targets are created:

    - binary: <target>.<format>
    - linker information: <target>.<format>.xml
    - map information: <target>.<format>.map
    """
    if "target" not in kw:
        kw["target"] = "out"
    if "linker_script" not in kw:
        bld.fatal("linker script missing")
    if not isinstance(kw["linker_script"], Node):
        kw["linker_script"] = bld.path.find_node(kw["linker_script"])
    bld.env.LINKER_SCRIPT = kw["linker_script"].abspath()

    kw["features"] = "c cprogram"

    tgt_elf = bld.path.find_or_declare(f"{kw['target']}.{bld.env.DEST_BIN_FMT}")
    tgt_xml = bld.path.find_or_declare(tgt_elf.name + ".xml")
    tgt_map = bld.path.find_or_declare(tgt_elf.name + ".map")
    kw["target"] = [tgt_elf, tgt_xml, tgt_map]

    bld.add_manual_dependency(tgt_elf, kw["linker_script"])

    # if a hex file should be generated, we need to append the config
    if "linker_script_hex" in kw:
        kw["features"] += " hexgen"
        bld.env.LINKER_SCRIPT_HEX = kw["linker_script_hex"].abspath()
        bld.add_manual_dependency(tgt_elf, kw["linker_script_hex"])
        # get the file hash assuming Unix-style line endings
        elf_file_hash = binascii.hexlify(
            md5(
                kw["linker_script"].read().replace("\r\n", "\n").encode("utf-8")
            ).digest()
        )
        txt = kw["linker_script_hex"].read().strip().splitlines()[0]
        txt = re.search(r"\/\*(.*)\*\/", txt)
        try:
            txt = txt.group(1)
        except IndexError:
            bld.fatal("hashing error")
        known_hash = bytes(txt.strip(), encoding="utf-8")
        if not elf_file_hash == known_hash:
            bld.fatal(
                f"The hash of '{kw['linker_script'].abspath()}' has changed from "
                f"'{known_hash.decode('utf-8')}' to '{elf_file_hash.decode('utf-8')}'.\n"
                f"Reflect the changes in the elf file linker script "
                f"('{kw['linker_script'].name}') in the hex file linker script "
                f"('{kw['linker_script_hex'].name}') and then update the file hash "
                f"generated based on the content of the elf linker script in the "
                f"hex file linker script ('{kw['linker_script_hex'].abspath()}')."
            )

    return bld(*k, **kw)


@TaskGen.feature("c", "asm", "includes")
@TaskGen.after_method("propagate_uselib_vars", "process_source")
def apply_incpaths(self):
    """Adds the include paths"""
    lst = self.to_incnodes(
        self.to_list(getattr(self, "includes", [])) + self.env.INCLUDES
    )
    self.includes_nodes = lst
    cwd = self.get_cwd()
    self.env.INCPATHS = []
    for i in lst:
        if os.sep + "ti" + os.sep in i.abspath():
            self.env.INCPATHS.append(i)
        else:
            self.env.INCPATHS.append(i.path_from(cwd))


@TaskGen.feature("c")
@TaskGen.before_method("apply_incpaths")
def check_duplicate_and_not_existing_includes(self):
    """Check if include directories are included more than once and if they
    really exist on the file system. If include directories do not exist, or
    they are included twice, raise an error.
    """
    includes = self.to_incnodes(
        self.to_list(getattr(self, "includes", [])) + self.env.INCLUDES
    )
    not_existing = list(
        filter(
            None,
            [None if os.path.isdir(i.abspath()) else i.abspath() for i in includes],
        )
    )
    abs_incs = [[x.abspath(), x.relpath()] for x in includes]
    seen = {}
    duplicates = []
    for x_abs, x_rel in abs_incs:
        if x_abs not in seen:
            seen[x_abs] = 1
        else:
            if seen[x_abs] == 1:
                duplicates.append((x_abs, x_rel))
            seen[x_abs] += 1
    err = ""
    if duplicates or not_existing:
        err = (
            f"There are include errors when building '{self.target}' from "
            f"sources {self.source} in build file from '{self.path}{os.sep}'.\n"
        )
    if not_existing:
        err += f"Not existing include directories are: {not_existing}\n"
    if duplicates:
        duplicates = [
            item
            for t in duplicates
            for item in t
            if os.sep + "build" + os.sep not in item
        ]
        duplicates = list(set(duplicates))
        err += f"Duplicate include directories are: {duplicates}\n"
    if err:
        self.bld.fatal(err)


class hexgen(Task.Task):
    """Task create hex file from elf files"""

    color = "YELLOW"
    after = ["link_task"]

    run_str = (
        "${ARMHEX} -q ${HEXGENFLAGS} --map=${TGT[1].abspath()} ${LINKER_SCRIPT_HEX} "
        "${SRC[0].abspath()} -o ${TGT[0].abspath()}"
    )

    def keyword(self):  # noqa: D102
        return "Compiling"

    def __str__(self) -> str:
        return f"{self.inputs[0]} -> {self.outputs[0]}"


@TaskGen.feature("hexgen")
@TaskGen.after("apply_link")
def add_hexgen_task(self):
    """Creates a tasks to create a hex file from the linker output
    (task :py:class:`f_ti_arm_cgt.hexgen`)
    """
    if not hasattr(self, "link_task"):
        return
    self.hexgen = self.create_task(
        "hexgen",
        src=self.link_task.outputs[0],
        tgt=[
            self.link_task.outputs[0].change_ext(".hex"),
            self.link_task.outputs[0].change_ext(".hex.map"),
        ],
    )


class bingen(Task.Task):
    """Task create bin file from elf files"""

    weight = 3
    color = "CYAN"
    after = ["link_task"]

    run_str = (
        "${TIOBJ2BIN} ${SRC[0].abspath()} ${TGT[0].abspath()} ${ARMOFD} "
        "${ARMHEX} ${MKHEX4BIN}"
    )

    def keyword(self):  # noqa: D102
        return "Compiling"

    def __str__(self) -> str:
        return f"{self.inputs[0]} -> {self.outputs[0]}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_bingen_task(self):
    """Creates a task to create a bin file from the linker output
    (task :py:class:`f_ti_arm_cgt.bingen`)
    """
    if not hasattr(self, "link_task"):
        return
    self.bingen = self.create_task(
        "bingen",
        src=self.link_task.outputs[0],
        tgt=[self.link_task.outputs[0].change_ext(".bin")],
    )


class app_crc_gen(Task.Task):
    """Task create the CRC file from the .bin file"""

    color = "CYAN"
    after = ["link_task"]

    def run(self):  # noqa: D102
        try:
            crc_bootloader.BootloaderBinaryFile(
                app_file=Path(self.inputs[0].abspath()),
                crc64_table=Path(self.outputs[0].abspath()),
                info_file=Path(self.outputs[1].abspath()),
            )
        except SystemExit as exc:
            Logs.error(exc.code)
            return 1
        return 0

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs[0]} -> {self.outputs[0]}"


class update_lauterbach_script(Task.Task):
    """Task create the CRC file from the .bin file"""

    color = "CYAN"
    after = ["link_task"]

    def run(self):  # noqa: D102
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

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs[0]}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_crc_task(self):
    """TODO"""
    if not hasattr(self, "link_task"):
        return
    if self.bld.variant_dir == self.link_task.outputs[0].parent.abspath():
        return
    if not getattr(self, "app_build_cfg", False):
        return
    # create the CRC table
    self.crc_task = self.create_task(
        "app_crc_gen",
        src=self.bingen.outputs[0],
        tgt=[
            self.bingen.outputs[0].change_ext(".crc64.csv"),
            self.bingen.outputs[0].change_ext(".crc64.json"),
        ],
    )

    # update the Lauterbach script
    lauterbach_in = self.bld.path.find_node(
        "tools/debugger/lauterbach/update_program_information.cmm.in"
    )
    lauterbach_out = self.bld.path.find_or_declare(
        self.bld.out_dir + "/update_program_information.cmm"
    )
    self.create_task(
        "update_lauterbach_script",
        src=[self.crc_task.outputs[0], self.crc_task.outputs[1], lauterbach_in],
        tgt=[lauterbach_out],
    )

    if self.bld.variant_dir:
        out_dir = self.bld.variant_dir
    else:
        out_dir = self.bld.path.get_bld()
    self.create_task(
        "copy_to_out_dir",
        src=self.crc_task.outputs,
        tgt=[
            self.bld.path.find_or_declare(
                os.path.join(out_dir, self.crc_task.outputs[0].name)
            )
        ],
    )


@taskgen_method
def accept_node_to_link(self, node):  # pylint: disable=unused-argument
    """Filters which output files are not meant to be linked"""
    return not node.name.endswith((".aux", "crl", "rl"))


@TaskGen.feature("c", "cprogram")
@TaskGen.after("apply_link")
def process_sizes(self):
    """Creates size tasks for generated object and object-like files"""
    if getattr(self, "link_task", None) is None:
        return
    for node in self.link_task.inputs:
        out = node.change_ext(".size.log")
        self.create_task("size", node, out)
    for node in self.link_task.outputs:
        if node.suffix() in (".a", "." + self.bld.env.DEST_BIN_FMT):
            out = node.change_ext(".size.log")
            self.create_task("size", node, out)


class size(Task.Task):
    """Task to run size on all input files"""

    vars = ["ARMSIZE", "ARMSIZE_OPTS"]
    color = "BLUE"

    def run(self):  # noqa: D102
        cmd = (
            Utils.subst_vars("${ARMSIZE} ${ARMSIZE_OPTS}", self.env)
            + " "
            + self.inputs[0].abspath()
        )
        cmd = cmd.split()
        outfile = self.outputs[0].path_from(self.generator.path)
        env = self.env.env or None
        cwd = self.generator.bld.path.get_bld().abspath()
        out, err = self.generator.bld.cmd_and_log(
            cmd,
            output=waflib.Context.BOTH,
            quiet=waflib.Context.STDOUT,
            env=env,
            cwd=cwd,
        )
        self.generator.path.make_node(outfile).write(out)
        if err:
            Logs.error(err)

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


@TaskGen.feature("c", "cprogram")
@TaskGen.after("apply_link")
def process_nm(self):
    """Creates nm tasks for generated object files"""
    if getattr(self, "link_task", None) is None:
        return
    for node in self.link_task.inputs:
        out = node.change_ext(".nm.log")
        self.create_task("nm", node, out)
    for node in self.link_task.outputs:
        if node.suffix() in (".a", "." + self.bld.env.DEST_BIN_FMT):
            out = node.change_ext(".nm.log")
            self.create_task("nm", node, out)


class nm(Task.Task):
    """Task to run armnm on all input files"""

    color = "PINK"
    run_str = "${ARMNM} ${NMFLAGS} --output=${TGT} ${SRC}"

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


@TaskGen.feature("c")
@TaskGen.after("c_pp")
def remove_stuff_from_pp(self):
    """Creates pp tasks for generated object files"""
    for node in getattr(self, "c_pp_tasks", []):
        outs = [node.outputs[0].change_ext(".ppr"), node.outputs[0].change_ext(".pprs")]
        self.create_task("clean_pp_file", node.outputs[0], outs)


class clean_pp_file(Task.Task):
    """Task to remove some information from the preprocessed files"""

    color = "PINK"
    remove_str = ("#define", "#pragma", "# pragma", "_Pragma")
    replace_str = [(r"__attribute__\(.*\)", "")]

    def run(self):  # noqa: D102
        """Removes empty lines and strips some intrinsics that should not be
        included in the postprocessed file
        """
        # read file, split text in list by lines and remove all empty entries
        txt = list(filter(str.rstrip, self.inputs[0].read().splitlines()))
        # join all lines without right side whitespace and write it to file
        txt = os.linesep.join(line.rstrip() for line in txt) + os.linesep
        self.outputs[0].write(txt, encoding="utf-8")
        txt = os.linesep.join(
            line.rstrip()
            for line in txt.split(os.linesep)
            if (not line.startswith(clean_pp_file.remove_str))
        )
        for rep in clean_pp_file.replace_str:
            txt = re.sub(rep[0], rep[1], txt)
        self.outputs[1].write(txt, encoding="utf-8")

    def keyword(self):  # noqa: D102
        return "Processing"

    def __str__(self) -> str:
        return f"{self.inputs} -> {self.outputs}"


class create_version_source(Task.Task):
    """creates the version information file"""

    weight = 2
    color = "BLUE"
    before = ["c"]
    ext_out = [".h"]
    always_run = True

    def run(self):  # noqa: D102
        txt = self.inputs[0].read(encoding="utf-8")
        txt = self.generator.bld.create_version_c(txt)
        self.outputs[0].write(txt, encoding="utf-8")
        self.outputs[1].write("DisableFormat: true\nSortIncludes: false\n")

    def keyword(self):  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        return str(self.outputs[0].path_from(self.generator.bld.path))


class create_app_build_cfg_source(Task.Task):
    """creates the app build configuration information file"""

    weight = 2
    color = "BLUE"
    before = ["c"]
    ext_out = [".h"]
    always_run = True

    def run(self):  # noqa: D102
        txt = self.inputs[0].read(encoding="utf-8")
        txt = self.generator.bld.create_app_build_cfg_c(txt)
        self.outputs[0].write(txt, encoding="utf-8")

    def keyword(self):  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        return str(self.outputs[0].path_from(self.generator.bld.path))


@TaskGen.feature("cprogram")
@TaskGen.after_method("process_rule")
def create_version_file(self):
    """Task generator for version information file"""
    generated_sources = []

    src = self.path.ctx.root.find_node(f"{self.env.PROJECT_ROOT[0]}/conf/tpl/c.c")
    version_c = self.path.find_or_declare("version.c")
    no_clang_node = self.path.find_or_declare(".clang-format")

    version_src_tsk = self.create_task(
        "create_version_source", src=src, tgt=[version_c, no_clang_node]
    )
    generated_sources.append(version_src_tsk.outputs[0])

    if getattr(self, "app_build_cfg", False):
        app_build_cfg_c = self.path.find_or_declare("app_build_cfg.c")
        app_cfg_tsk = self.create_task(
            "create_app_build_cfg_source", src=src, tgt=[app_build_cfg_c]
        )
        generated_sources.append(app_cfg_tsk.outputs[0])

    try:
        self.source.extend(generated_sources)
    except AttributeError:
        self.source = [self.source] + generated_sources


@TaskGen.feature("c")
@TaskGen.before_method("apply_incpaths")
def hash_cmd_files(self):
    """Calculate hashes for command files, before running the c-related tasks,
    as c-related tasks might rely on these files
    """
    if not isinstance(getattr(self.env, "CMD_FILES", []), list):
        self.bld.fatal("'ctx.env.CMD_FILES' must be a list.")
    if not isinstance(getattr(self, "cmd_files", []), list):
        self.bld.fatal("keyword argument 'cmd_files' must be a list.")
    # now we sure to have at least lists or empty
    self.env.CMD_FILES = getattr(self, "cmd_files", []) + getattr(
        self.env, "CMD_FILES", []
    )
    if not self.env.CMD_FILES:
        return
    if not all(os.path.isabs(i) for i in self.env.CMD_FILES):
        self.bld.fatal(
            "Keyword argument 'cmd_files' only accepts absolute paths (use "
            "'ctx.path.find_node('xyz.txt').abspath()' in the list."
        )
    self.env.append_unique(
        "CMD_FILES_HASH", [Utils.h_file(i) for i in self.env.CMD_FILES]
    )


class get_stack(Task.Task):
    """gathers all stack information in one file"""

    after = ["link_task"]

    def run(self):  # noqa: D102
        """Gathers the stack usage information"""

        def parse(txt, line_number):
            stack_usage_re = re.compile(r"Stack usage:\s+(\d+)\s+bytes")
            out = {"Stack usage": -1, "Called functions": [], "Indirect calls": False}
            found_stack = False
            found_functions = False
            called_functions = []
            for line in txt[line_number:]:
                if not found_stack:
                    m = stack_usage_re.match(line)
                    if m and m.group(1):
                        out["Stack usage"] = int(m.group(1))
                        found_stack = True
                elif line == "Function contains indirect calls.":
                    out["Indirect calls"] = True
                elif line == "Called functions:":
                    found_functions = True
                elif found_functions:
                    if not line:
                        break
                    called_functions.append(line.strip())
            out["Called functions"] = called_functions
            return out

        out = {}
        func_re = re.compile(r"\|\s+FUNCTION:\s+(\w+)\s+|")
        sources = self.generator.bld.bldnode.ant_glob("**/*.aux", quiet=True)
        for src in sources:
            txt_lines = src.read().splitlines()
            for i, line in enumerate(txt_lines):
                m = func_re.match(line)
                if m and m.group(1):
                    out[m.group(1)] = parse(txt_lines, i)

        self.outputs[0].write_json(out)


@TaskGen.feature("cprogram")
@TaskGen.after_method("process_source")
def test_exec_fun(self):
    """Get stack usage"""
    tgt = os.path.join(
        self.bld.bldnode.abspath(), f"{self.env.APPNAME.lower()}.stacks.json"
    )
    self.create_task(
        "get_stack",
        src=self.link_task.inputs,
        tgt=self.path.find_or_declare(tgt),
    )


@conf
def find_armcl(ctx):
    """Configures the compiler, determines the compiler version, and sets the
    default include paths.
    """
    found_versions = []
    err = 0
    for i, path_list in enumerate(ctx.env.CCS_SEARCH_PATH_GROUP):
        ctx.env.stash()
        err = 0
        cc = ctx.find_program(["armcl"], var="CC", path_list=path_list)
        ctx.env.CC_NAME = "cgt"
        cc_path = Path(cc[0])
        ctx.env.append_unique(
            "INCLUDES", os.path.join(cc_path.parent.parent.absolute(), "include")
        )
        ctx.env.append_unique(
            "STLIBPATH", os.path.join(cc_path.parent.parent.absolute(), "lib")
        )
        ctx.find_program(["armcl"], var="LINK_CC", path_list=path_list)
        cmd = Utils.subst_vars("${CC} --compiler_revision", ctx.env).split(" ")
        std_out, std_err = ctx.cmd_and_log(cmd, output=Context.BOTH)
        if std_err:
            Logs.warn(f"Could not successfully run '--compiler_revision' on {cc}")
            err += 1
            ctx.env.revert()
            continue
        ctx.env.CC_VERSION = std_out.strip()
        cmd = Utils.subst_vars("${CC} -version", ctx.env).split(" ")
        std_out, std_err = ctx.cmd_and_log(cmd, output=Context.BOTH)
        if std_err:
            Logs.warn(f"Could not successfully run '-version' on {cc}")
            err += 1
        version_pattern = re.compile(r"(v\d{1,}\.\d{1,}\.\d{1,}\.(LTS|STS))")
        for line in std_out.splitlines():
            full_ver = version_pattern.search(line)
            if full_ver:
                ctx.env.append_unique("CC_VERSION_FULL", full_ver.group(1))
        if not ctx.env.CC_VERSION or not ctx.env.CC_VERSION_FULL:
            Logs.warn(f"Could not determine compiler version for '{cc}'")
            err += 1
            ctx.env.revert()
            continue

        # we are searching for the newest compiler first, if we find a working
        # one and strict version checking is not active, then just use this one
        # If we enforce a strict version just go on until the correct version
        # is found
        if not err and not ctx.env.FOXBMS_2_CCS_VERSION_STRICT:
            ctx.env.CCS_SEARCH_PATH_GROUP_ID = i
            break

        found_versions.append((ctx.env.CC_VERSION_FULL[0], cc))

        # we found a compiler, check if it reported the expected, i.e., pinned
        # version. If it does not match, continue the search.
        if ctx.env.FOXBMS_2_CCS_VERSION_STRICT:
            if ctx.env.FOXBMS_2_CCS_VERSION_STRICT[0] != ctx.env.CC_VERSION_FULL[0]:
                err += 1
                ctx.env.revert()
                continue
            ctx.env.CCS_SEARCH_PATH_GROUP_ID = i
            break

    if err:
        ctx.fatal(
            "Strict CCS version checking was set, and compiler version does not "
            "match.\n"
            f"(searched for {ctx.env.FOXBMS_2_CCS_VERSION_STRICT[0]}, but only "
            f"found {found_versions})."
        )


@conf
def find_armar(ctx):
    """Configures the archive tool"""
    path_list = ctx.env.CCS_SEARCH_PATH_GROUP[ctx.env.CCS_SEARCH_PATH_GROUP_ID]
    ctx.find_program(["armar"], var="AR", path_list=path_list)


@conf
def cgt_flags(ctx):
    """Sets flags and related configuration options of the compiler."""
    env = ctx.env
    env.DEST_BIN_FMT = "elf"
    env.AR_TGT_F = ["rq"]
    env.CC_COMPILE_ONLY = ["--compile_only"]
    env.CC_TGT_F = ["--output_file="]
    env.CCLINK_TGT_F = ["--output_file="]
    env.RUN_LINKER = ["-qq", "--run_linker"]
    env.DEFINES_ST = "-D%s"
    env.CMD_FILES_ST = "--cmd_file=%s"
    env.LIB_ST = "--library=lib%s.a"
    env.TARGETLIB_ST = "--library=%s.lib"
    env.LIBPATH_ST = "--search_path=%s"
    env.STLIB_ST = "--library=lib%s.a"
    env.STLIBPATH_ST = "--search_path=%s"
    env.CPPPATH_ST = "--include_path=%s"
    env.cprogram_PATTERN = "%s"
    env.cstlib_PATTERN = "lib%s.a"
    env.MAP_FILE = "--map_file="
    env.XML_LINK_INFO = "--xml_link_info="
    env.OBJ_DIRECTORY = "--obj_directory="
    env.ASM_DIRECTORY = "--asm_directory="
    env.PPO = "--preproc_only"
    env.PPA = "--preproc_with_compile"
    env.PPM = "--preproc_macros"
    env.PPI = "--preproc_includes"
    env.PPD = "--preproc_dependency"
    env.ARMSIZE_OPTS = [
        "--common",
        "--arch=arm",
        "--format=berkeley",
        "--totals",
    ]
    env.TI_ARM_CGT_LINKER_END_GROUP = "--end-group"
    env.TI_ARM_CGT_LINKER_START_GROUP = "--start-group"


def configure(ctx):
    """Configuration step of the TI ARM CGT compiler tool"""
    ctx.load_special_tools("c_*.py")
    ctx.load("create_version", tooldir=TOOL_DIR)
    ctx.load("create_app_build_cfg", tooldir=TOOL_DIR)
    ctx.start_msg("Checking for TI ARM CGT compiler and tools")
    ctx.load_cc_options()
    ctx.find_armcl()
    ctx.find_armar()
    ctx.find_arm_tools()
    ctx.cgt_flags()
    ctx.link_add_flags()
    ctx.env.COMPILER_BUILTIN_DEFINES_FILE = [
        ctx.root.find_node(ctx.get_defines()).abspath()
    ]
    ctx.env.DEST_OS = ["EMBEDDED"]
    ctx.env.COMPILER_CC = "ti_arm_cgt"
    ctx.end_msg(ctx.env.get_flat("CC"))
    ctx.load("f_hcg", tooldir=TOOL_DIR)
