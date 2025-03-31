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

"""Implements the compiler options for the TI ARM CGT
(https://www.ti.com/tool/ARM-CGT) Tool.
"""

import os
import re
from pathlib import Path

import yaml
from waflib import Utils
from waflib.Configure import conf


def options(opt):
    """Configurable options of the :py:mod:`f_ti_arm_cgt` tool. The options are

    - ``--cc-options=conf/cc/cc-options.yaml``

    """
    #: option to overwrite the default location of the cc-options file
    opt.add_option(
        "--cc-options",
        action="store",
        default=os.path.join("conf", "cc", "cc-options.yaml"),
        dest="CC_OPTIONS",
        help="Path to cc options specification file",
    )


@conf
def load_cc_options(ctx):  # pylint: disable=too-many-locals,too-many-branches
    """configuration step of the TI ARM CGT compiler tool"""
    cc_spec_file = ctx.path.find_node(ctx.options.CC_OPTIONS)
    if not cc_spec_file:
        ctx.fatal(f"'{cc_spec_file}' does not exist.")
    cc_spec_txt = cc_spec_file.read(encoding="utf-8")
    try:
        ctx.env.cc_options = yaml.load(cc_spec_txt, Loader=yaml.Loader)
    except yaml.YAMLError as exc:
        ctx.fatal(exc)

    include_paths = ctx.env.cc_options["INCLUDE_PATHS"][
        Utils.unversioned_sys_platform()
    ]
    library_paths = ctx.env.cc_options["LIBRARY_PATHS"][
        Utils.unversioned_sys_platform()
    ]
    libraries_st = ctx.env.cc_options["LIBRARIES"]["ST"]
    libraries_target = ctx.env.cc_options["LIBRARIES"]["TARGET"]
    cflags_common = ctx.env.cc_options["CFLAGS"]["common"]
    cflags_common_compile_only = ctx.env.cc_options["CFLAGS"]["common_compile_only"]
    cflags_foxbms = ctx.env.cc_options["CFLAGS"]["foxbms"]
    cflags_hal = ctx.env.cc_options["CFLAGS"]["hal"]
    cflags_os = ctx.env.cc_options["CFLAGS"]["operating_system"]
    linkflags = ctx.env.cc_options["LINKFLAGS"]
    hexgenflags = ctx.env.cc_options["HEXGENFLAGS"]
    nmflags = ctx.env.cc_options["NMFLAGS"]

    if linkflags:
        ctx.env.append_unique("LINKFLAGS", linkflags)
    if hexgenflags:
        ctx.env.append_unique("HEXGENFLAGS", hexgenflags)
    if nmflags:
        ctx.env.append_unique("NMFLAGS", nmflags)

    if include_paths:
        ctx.env.append_unique("INCLUDES", include_paths)
    if library_paths:
        ctx.env.append_unique("STLIBPATH", library_paths)
    if libraries_st:
        ctx.env.append_unique("STLIB", libraries_st)
    if libraries_target:
        ctx.env.append_unique("TARGETLIB", libraries_target)
    if cflags_common:
        ctx.env.append_unique("CFLAGS", cflags_common)

    if cflags_common_compile_only:
        ctx.env.append_unique("CFLAGS_COMPILE_ONLY", cflags_common_compile_only)
    ctx.env.append_unique(
        "CFLAGS_COMPILE_ONLY",
        [
            "--gen_cross_reference_listing",
            "--gen_func_info_listing",
            "--gen_preprocessor_listing",
        ],
    )

    if cflags_foxbms:
        ctx.env.append_unique("CFLAGS_FOXBMS", cflags_foxbms)
    if cflags_hal:
        ctx.env.append_unique("CFLAGS_HAL", cflags_hal)
    if cflags_os:
        ctx.env.append_unique("CFLAGS_OS", cflags_os)

    ctx.env.append_unique(
        "FOXBMS_2_CCS_VERSION_STRICT",
        ctx.env.cc_options.get("FOXBMS_2_CCS_VERSION_STRICT", []),
    )
    ccs_versions = []
    if Utils.is_win32:
        prefix = "C:"
    else:
        prefix = "/opt"
    for i in list(Path(f"{prefix}/ti/").glob("ccs*")):
        m = re.search(r".*ccs(\d{1,})", i.as_posix())
        if m:
            ccs_versions.append(int(m.group(1)))
    sorted_ccs_versions = list(reversed(sorted(ccs_versions)))
    search_path_group = []
    for i in sorted_ccs_versions:
        comp_base = Path(f"{prefix}/ti/ccs{i}/ccs/tools/compiler")
        comp_bins = list(comp_base.glob("ti-cgt-arm_*/bin"))
        comp_libs = list(comp_base.glob("ti-cgt-arm_*/lib"))
        if not all((comp_libs, comp_libs)):
            continue
        tmp = []
        tmp.append(str(comp_bins[0]))

        tmp.append(str(comp_libs[0]))
        tmp.extend(
            [
                str(Path(f"{prefix}/ti/ccs{i}/ccs/utils/bin")),
                str(Path(f"{prefix}/ti/ccs{i}/ccs/utils/tiobj2bin")),
            ]
        )
        if Utils.is_win32:
            tmp.append(str(Path(f"{prefix}/ti/ccs{i}/ccs/utils/cygwin")))
        search_path_group.append(tmp)

    ctx.env.append_unique("CCS_SEARCH_PATH_GROUP", list(search_path_group))
