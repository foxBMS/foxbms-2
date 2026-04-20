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

"""Implements a waf tool to use TI HALCoGen (https://www.ti.com/tool/HALCOGEN)"""

import binascii
import os
import pathlib
import re

import shutil
from dataclasses import dataclass
from xml.etree import ElementTree as ET

from yaml import YAMLError, Loader, load
from waflib import Errors, Logs, Task, TaskGen, Utils
from waflib.Node import Node


class ToolNotSupportedError(Exception):
    """Tool not supported by our toolchain"""


@dataclass
class NodeStructure:
    """Holds all relevant information on generated files based on a HALCoGen
    configuration file
    """

    def __init__(self, xml_file_path) -> None:
        self.root = ET.parse(xml_file_path).getroot()
        self.dil_file_name = self.root.find("DEVICE").find("dilfile").text

        self.uses_freertos = False
        self.headers = []
        self.sources = []

    def parse_xml(self):
        """Parses information on generated files from the HALCoGen
        configuration file.
        """
        for element in self.root:
            if element.tag == "DEVICE":
                self._parse_device(element.tag)
            else:
                self._parse_system(element.tag)

    def _parse_device(self, tag):
        for device_settings in self.root.iter(tag):
            for device_setting in device_settings:
                if device_setting.tag == "tools" and device_setting.text != "ti":
                    err_msg = "tool not supported"
                    raise ToolNotSupportedError(err_msg)
                if (
                    device_setting.tag == "device"
                    and device_setting.text.lower().endswith("_freertos")
                ):
                    self.uses_freertos = True

    def _parse_system(self, tag):
        # xml is otherwise not good parseable
        # pylint: disable-next=too-many-nested-blocks
        for hcg_setting in self.root.iter(tag):
            for hardware_type in list(hcg_setting[0]):
                for value_hal in list(hardware_type):
                    if value_hal.tag == "PATH":
                        if value_hal.text is not None:
                            if value_hal.text.endswith(".h"):
                                self.headers.append(value_hal.text)
                            elif value_hal.text.endswith((".c", ".asm")):
                                self.sources.append(value_hal.text)
        self.headers = sorted(set(self.headers))
        self.sources = sorted(set(self.sources))


@TaskGen.extension(".hcg")
def process_hcg(self: TaskGen.task_gen, node: Node):
    """Creates HALCoGen task if a hcg input file and a corresponding dil file
    exists and binds it to the :py:class:`f_hcg.hcg_compiler` class.

    See :numref:`hcg-io-dep` for a simplified representation of how the input
    output relation is calculated.

    .. graphviz::
        :caption: Input-output relation of hcg and dil file and corresponding
            output files
        :name: hcg-io-dep

        digraph HCG_AND_DIL_TO_SOURCES {
            compound=true;
            rankdir=LR;
            nd_halcogen [label="HALCoGen.exe", style=filled, fillcolor=green];
            nd_hcg          [label="*.hcg", style=filled];
            nd_dil          [label="*.dil", style=filled];
            nd_gen_sources  [label="source/*.c", style=filled];
            nd_gen_headers  [label="include/*.h", style=filled];
            nd_cpu_freq     [label="include/config_cpu_clock_hz.h", style=filled];
            nd_log          [label="*.log", style=filled];
            subgraph cluster_cmd {
                label = "Command Line";
                rank=same;
                nd_input           [label="HALCOGEN_SRC_INPUT"];
            }
            nd_halcogen -> nd_input         [lhead=cluster_cmd];
            nd_hcg      -> nd_input         [lhead=cluster_cmd];
            nd_dil      -> nd_input         [lhead=cluster_cmd];
            nd_input    -> nd_gen_sources   [ltail=cluster_cmd];
            nd_input    -> nd_gen_headers   [ltail=cluster_cmd];
            nd_input    -> nd_cpu_freq      [ltail=cluster_cmd];
            nd_input    -> nd_log           [ltail=cluster_cmd];
        }


    A copy of the FreeRTOS sources is provided in the repository. Therefore
    the generated FreeRTOS sources are not needed and these files must be
    removed as otherwise the wrong files may be included.

    The only information needed from the generated sources is the
    ``configCPU_CLOCK_HZ``. This information is extracted from the sources
    (see :py:meth:`hcg_compiler.run` for details) and written into a
    configuration header. This configuration header is *config_cpu_clock_hz.h*.

    Raises:
        ConfigurationError: Raises an error that stops the build if no
            corresponding .dil file is found.
    """
    hcg = NodeStructure(node.abspath())
    try:
        hcg.parse_xml()
    except ToolNotSupportedError:
        self.bld.fatal("Unsupported tools selected in HALCoGen configuration file")
        return
    if not self.path or not hcg.dil_file_name:
        self.bld.fatal("object error")
        return
    base_path = node.path_from(self.path)
    if not base_path:
        self.bld.fatal(f"Could determine base path of {self.path}")
        return
    dil_path = os.path.join(os.path.dirname(base_path), hcg.dil_file_name)
    dil_node = self.path.find_node(dil_path)
    if not dil_node:
        self.bld.fatal(
            f"No dil file '{hcg.dil_file_name}' to hcg file '{node.abspath()}'."
        )

    hcg_node = node
    hcg_sources = [hcg_node, dil_node]

    # we need a copy of the input sources
    copy_hcg_files = [
        self.path.find_or_declare(node.name),
        self.path.find_or_declare(dil_node.name),
    ]
    log_file = [self.path.find_or_declare(node.name.replace(node.suffix(), ".log"))]
    cpu_clock_config_header = []
    if hcg.uses_freertos:
        cpu_clock_config_header = [
            self.path.find_or_declare("include/config_cpu_clock_hz.h")
        ]
    gen_headers = [self.path.find_or_declare(i) for i in hcg.headers]
    gen_sources = [self.path.find_or_declare(i) for i in hcg.sources]
    preliminary_tgt = copy_hcg_files + log_file
    if hcg.uses_freertos:
        preliminary_tgt.extend(cpu_clock_config_header)
    preliminary_tgt.extend(gen_headers + gen_sources)

    # define remove files
    remove_attr = getattr(self, "remove", "")
    remove = []
    with open(remove_attr.abspath(), encoding="utf-8") as f:
        remove_txt = f.read()
        try:
            remove: list = load(remove_txt, Loader=Loader)
        except YAMLError:
            self.bld.fatal("Could not load remove configuration.")
        except (FileNotFoundError, TypeError):
            self.bld.fatal("Could not load remove configuration file.")
    remove = [self.path.find_or_declare(i) for i in remove]
    err = 0
    for i in remove:
        if not isinstance(i, Node):
            Logs.error(f"{i} is not a 'waflib.Node.Node'.")
            err += 1
    if err:
        self.bld.fatal("Could not find all need-to-be-removed files.")
    # create the actual target list
    tgt = [i for i in preliminary_tgt if i not in remove]

    self.create_task(
        "hcg_compiler",
        src=hcg_sources,
        tgt=tgt,
        remove_files=remove,
        uses_freertos=hcg.uses_freertos,
    )
    no_clang_node = self.path.find_or_declare(".clang-format")
    if not no_clang_node.exists():
        no_clang_node.write(
            f"DisableFormat: true{os.linesep}SortIncludes: false{os.linesep}"
        )

    if not hasattr(self, "unit_test"):
        for i in tgt:
            if i.suffix() == ".c" or i.suffix() == ".asm":
                if i not in self.source:
                    self.source.append(i)
                else:
                    pass
        try:
            self.includes.append("include")
        except AttributeError:
            self.includes = ["include"]


class hcg_compiler(Task.Task):
    """Class to implement running the HALCoGen code generator"""

    #: list of string: variables this task depends on
    vars = ["HALCOGEN", "HALCOGEN_SRC_INPUT"]

    #: str: color in which the command line is displayed in the terminal
    color = "PINK"

    #: list of string: task produces headers therefore it must run before c-tasks
    ext_out = [".h", ".c", ".asm"]

    def keyword(self):
        """Displayed keyword when generating sources from a HALCoGen project"""
        return "Compiling"

    def run(self):
        """Runs the HALCoGen code generator

        #. Copy the HALCoGen sources to the build directory
        #. Run HALCoGen
        #. Get CPU clock frequency that is set by HALCoGen from
           FreeRTOSConfig.h and copy it to our configuration file
        #. Copy the HALCoGen sources again to the output directory. This needs
           to be done, as HALCoGen re-writes the timestamp of the HALCoGen
           file when the tool is run. With this step the sources and build
           directory are synchronized.

        """
        for src, tgt in zip(self.inputs[:2], self.outputs[:2], strict=False):
            shutil.copy2(src.abspath(), tgt.abspath())

        cmd = Utils.subst_vars(
            "${HALCOGEN} ${HALCOGEN_SRC_INPUT} " + self.outputs[0].abspath(),
            self.generator.env,
        ).split()
        try:
            self.generator.bld.exec_command(cmd)
        except Errors.WafError:
            self.generator.bld.fatal("Could not generate HAL sources.")
        # now we have all files that HALCoGen created, including the ones we
        # do actually not need

        # get clock info from generated source 'FreeRTOSConfig.h'
        if self.uses_freertos:  # pylint: disable=no-member
            try:
                # pylint: disable-next=no-member
                freertos_file_id = self.remove_files.index(
                    self.generator.path.find_resource("include/FreeRTOSConfig.h")
                )
            except ValueError:
                self.generator.bld.fatal("Could not find 'FreeRTOSConfig.h'.")

            # pylint: disable-next=no-member
            freertos_config = self.remove_files[freertos_file_id].read()
            frequency = None
            for line in freertos_config.splitlines():
                mach = re.search(
                    r"#define configCPU_CLOCK_HZ.*\( \( unsigned portLONG \) ([0-9]+) \)",
                    line,
                )
                if mach:
                    frequency = mach.group(1)
                    break
            if not frequency:
                self.generator.bld.fatal("Could not determine clock frequency.")
            define_guard = (
                self.outputs[3].name.replace(self.outputs[3].suffix(), "").upper()
                + "_H_"
            )
            self.outputs[3].write(
                f"#ifndef {define_guard}\n"
                f"#define {define_guard}\n"
                f"#define HALCOGEN_CPU_CLOCK_HZ ({frequency})\n"
                f"#endif /* {define_guard} */\n"
            )

        startup_node = self.generator.path.find_resource("source/HL_sys_startup.c")
        if not startup_node:
            self.generator.bld.fatal("Could not find startup source.")

        hl_sys_startup_file = None
        try:
            hl_sys_startup_file = self.outputs.index(startup_node)
            tmp = self.outputs
        except ValueError:
            # pylint: disable=no-member
            try:
                hl_sys_startup_file = self.remove_files.index(startup_node)
            except ValueError:
                # if the startup node is not in 'outputs' or 'remove_files'
                # then something went wrong.
                self.generator.bld.fatal("Could not find 'FreeRTOSConfig.h'.")
            tmp = self.remove_files
            # pylint: enable=no-member
        if not hl_sys_startup_file:
            self.generator.bld.fatal("Could not find 'HL_sys_startup.c'.")

        generated_file_hash = binascii.hexlify(
            Utils.h_file(tmp[hl_sys_startup_file].abspath())
        )
        known_hash = bytes(self.generator.startup_hash.read().strip(), encoding="utf-8")
        if not generated_file_hash == known_hash:
            Logs.error(
                "The auto-generated file 'HL_sys_startup.c' has changed due to "
                "a configuration change in the HALCoGen project.\nThe "
                f"expected hash is {known_hash} but the generated hash is "
                f"{generated_file_hash}.\nCompare '{startup_node}' with "
                "'fstartup.c' and see if changes need to be applied to to "
                "'fstartup.c'.\nIf everything is changed as needed, updated "
                f"the hash in '{self.generator.startup_hash}' and build "
                "again.\nFor more information see the documentation "
                " (Configuration/HALCoGen)."
            )
            return 1

        # remove un-wanted generated sources
        for src in self.remove_files:  # pylint: disable=no-member
            src.delete()

        # HALCoGen alters the timestamp hardcoded in the copied file after it
        # generated the sources, we do not want that, therefore overwrite the
        # altered HALCoGen files with the "original" ones
        for src, tgt in zip(self.inputs[:2], self.outputs[:2], strict=False):
            shutil.copy2(src.abspath(), tgt.abspath())
        return 0


@TaskGen.feature("c", "asm")
@TaskGen.after_method("apply_incpaths")
def fix_gen_hal_incs(self):
    """Add path to HALCoGen generated header files to every build"""
    if self.env.HCG_GEN_HAL_INC_PATHS:
        inc_paths = [os.path.join(i, "include") for i in self.env.HCG_GEN_HAL_INC_PATHS]
        if inc_paths not in self.env.INCPATHS:
            self.env.append_unique("INCPATHS", inc_paths)


def configure(ctx):
    """Configuration step of the TI HALCoGen Code Generator.

    #. checks whether the platform is Win32 or not, as HALCoGen is only
       supported on Win32.
    #. searches for the HALCoGen program
    #. Adds the include path of the F021 Flash API to ``INCLUDES``

    """
    ctx.start_msg("Checking for TI Code Generator (HALCoGen)")
    ctx.find_program("HALCOGEN", var="HALCOGEN", mandatory=False)
    if ctx.env.HALCOGEN:
        include_path_halcogen = os.path.join(
            pathlib.Path(ctx.env.HALCOGEN[0]).parent.parent.parent,
            "F021 Flash API",
            "02.01.01",
            "include",
        )
        if os.path.exists(include_path_halcogen):
            ctx.env.append_unique("INCLUDES", include_path_halcogen)

        libpath_halcogen = os.path.join(
            pathlib.Path(ctx.env.HALCOGEN[0]).parent.parent.parent,
            "F021 Flash API",
            "02.01.01",
        )
        if os.path.exists(libpath_halcogen):
            ctx.env.append_unique("STLIBPATH", libpath_halcogen)
        ctx.env.HALCOGEN_SRC_INPUT = ["-i"]
    else:
        Logs.warn(
            "HALCogen is not available  and therefore the code generator can not run."
        )
    ctx.end_msg(ctx.env.get_flat("HALCOGEN"))
