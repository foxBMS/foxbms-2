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

# cspell:ignore ARMHEX,HEXFLAGS

"""Generation of Intel HEX files from linked ELF outputs.

This waf tool converts a linked ELF executable into an Intel HEX file using
the TI ``armhex`` utility (``ARMHEX``). The HEX generation task is
automatically appended after the link step for task generators with the
``cprogram`` feature when the TI ARM CGT toolchain is active and a hex
linker script (``linker_script_hex``) is provided.

Before creating the task, the tool validates the integrity of the ELF linker
script by comparing its MD5 hash against a known hash embedded in the first
line of the hex linker script. This ensures that changes to the ELF linker
script are intentionally reflected in the hex linker script.

Usage
-----
1. The tool is loaded and configured by :func:`waf_tools.ti_arm_cgt.find_hexgen`::

    # in ti_arm_cgt.py
    @conf
    def find_hexgen(ctx):
        ctx.load("hexgen", tooldir=TOOL_DIR)

2. In a build script, declare a task generator with a hex linker script::

    def build(bld):
        bld(
            features="c cprogram",
            source="main.c",
            target="main",
            linker_script=bld.path.find_node("app.cmd"),
            linker_script_hex=bld.path.find_node("app_hex.cmd"),
        )

   The ``linker_script_hex`` attribute points to the hex file linker script
   that controls how sections are mapped into the HEX output. The first line
   of this file must contain a comment with the MD5 hash of the ELF linker
   script content.

Dependencies
------------
- TI ``armhex`` utility (``HEX`` environment variable)
- TI ARM CGT toolchain (``CC_NAME == "armcl"``)
"""

import binascii
import re
from hashlib import md5

from waflib import Task, TaskGen
from waflib.Build import BuildContext
from waflib.Configure import ConfigurationContext
from waflib.Node import Node


class hexgen(Task.Task):
    """Task that converts a linked ELF file into an Intel HEX file.

    Executes ``ARMHEX`` with the configured flags and the hex linker script
    to produce a ``.hex`` file from the linker output.

    Environment variables used:
        - **ARMHEX**: Path to the ``armhex`` executable.
        - **HEXFLAGS**: Flags passed before the source file argument.
        - **HEX_TGT_F**: Flag prefix for the output file argument.
    """

    color = "PINK"
    run_str = "${ARMHEX} ${HEXFLAGS} ${SRC} ${HEX_TGT_F}${TGT[0].abspath()}"


@TaskGen.feature("cprogram")
@TaskGen.after("apply_link")
def add_hexgen_task(self: TaskGen.task_gen) -> None:
    """Append a HEX-file generation task after the link step.

    Creates a :class:`hexgen` task that converts the linked ELF output into
    an Intel HEX file using the hex linker script specified by
    ``self.linker_script_hex``.

    The task is only created when the ``linker_script_hex`` attribute
    is defined.

    Before task creation, the integrity of the ELF linker script is verified
    via :func:`check_linker_script_hash`.
    """
    if getattr(self.env, "CC_NAME", "") != "armcl":
        return
    if not hasattr(self, "link_task"):
        return
    if not hasattr(self, "linker_script_hex"):
        return

    if not isinstance(self.linker_script_hex, Node):
        self.linker_script_hex = self.path.find_node(self.linker_script_hex)

    if hasattr(self, "linker_script"):
        linker_script_node = self.linker_script
        check_linker_script_hash(self.bld, linker_script_node, self.linker_script_hex)

    src = [self.linker_script_hex] + self.link_task.outputs
    tgt = [self.link_task.outputs[0].change_ext(".hex")]
    self.hexgen = self.create_task("hexgen", src=src, tgt=tgt)


def check_linker_script_hash(
    bld: BuildContext, linker_script: Node, linker_script_hex: Node
) -> None:
    """Verify that the ELF linker script content matches the expected hash.

    Computes the MD5 hash of the ELF linker script content and compares
    it against the hash stored in the first line of the hex linker script
    (embedded as a C-style comment ``/* <hash> */``).

    Args:
        bld: The active build context.
        linker_script: The ELF linker script node whose content is hashed.
        linker_script_hex: The hex linker script node containing the
            expected hash in its first line.

    Raises:
        SystemExit: Via ``bld.fatal()`` if the hashes do not match or the
            hash comment cannot be parsed.
    """
    elf_file_hash = binascii.hexlify(
        md5(
            linker_script.read().replace("\r\n", "\n").encode("utf-8"),
            usedforsecurity=False,
        ).digest()
    )
    txt = linker_script_hex.read().strip().splitlines()[0]
    txt = re.search(r"\/\*(.*)\*\/", txt)
    try:
        txt = txt.group(1)
    except IndexError:
        bld.fatal("hashing error")
    known_hash = bytes(txt.strip(), encoding="utf-8")
    if elf_file_hash != known_hash:
        bld.fatal(
            f"The hash of '{linker_script.abspath()}' has changed from "
            f"'{known_hash.decode('utf-8')}' to '{elf_file_hash.decode('utf-8')}'.\n"
            f"Reflect the changes in the elf file linker script "
            f"('{linker_script.name}') in the hex file linker script "
            f"('{linker_script_hex.name}') and then update the file hash "
            f"generated based on the content of the elf linker script in the "
            f"hex file linker script ('{linker_script_hex.abspath()}')."
        )


def configure(ctx: ConfigurationContext) -> None:
    """Configure the Intel HEX generation tool.

    Locates the ``HEX`` executable and initializes the flag variables.

    The following environment variables are set:
        - **HEX**: Path to the ``armhex`` executable.
        - **HEXFLAGS**: Flags passed to ``armhex``.
    """
    if not ctx.env.HEX:
        ctx.find_program("HEX", var="HEX")
    if not ctx.env.HEXFLAGS:
        ctx.env.HEXFLAGS = []
