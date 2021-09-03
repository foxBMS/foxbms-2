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

"""Implements the compiler options for the TI ARM CGT
(https://www.ti.com/tool/ARM-CGT) Tool.
"""

import os

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
def load_cc_options(conf):  # pylint: disable-msg=redefined-outer-name
    """configuration step of the TI ARM CGT compiler tool"""
    cc_spec_file = conf.path.find_node(conf.options.CC_OPTIONS)
    if not cc_spec_file:
        conf.fatal(f"'{cc_spec_file}' does not exist.")
    cc_spec_txt = cc_spec_file.read(encoding="utf-8")
    try:
        conf.env.cc_options = yaml.load(cc_spec_txt, Loader=yaml.Loader)
    except yaml.YAMLError as exc:
        conf.fatal(exc)

    include_paths = conf.env.cc_options["INCLUDE_PATHS"][
        Utils.unversioned_sys_platform()
    ]
    library_paths = conf.env.cc_options["LIBRARY_PATHS"][
        Utils.unversioned_sys_platform()
    ]
    libraries_st = conf.env.cc_options["LIBRARIES"]["ST"]
    libraries_target = conf.env.cc_options["LIBRARIES"]["TARGET"]
    cflags_common = conf.env.cc_options["CFLAGS"]["common"]
    cflags_common_compile_only = conf.env.cc_options["CFLAGS"]["common_compile_only"]
    cflags_foxbms = conf.env.cc_options["CFLAGS"]["foxbms"]
    cflags_hal = conf.env.cc_options["CFLAGS"]["hal"]
    cflags_os = conf.env.cc_options["CFLAGS"]["operating_system"]
    linkflags = conf.env.cc_options["LINKFLAGS"]
    hexgenflags = conf.env.cc_options["HEXGENFLAGS"]
    nmflags = conf.env.cc_options["NMFLAGS"]

    if linkflags:
        conf.env.append_unique("LINKFLAGS", linkflags)
    if hexgenflags:
        conf.env.append_unique("HEXGENFLAGS", hexgenflags)
    if nmflags:
        conf.env.append_unique("NMFLAGS", nmflags)

    if include_paths:
        conf.env.append_unique("INCLUDES", include_paths)
    if library_paths:
        conf.env.append_unique("STLIBPATH", library_paths)
    if libraries_st:
        conf.env.append_unique("STLIB", libraries_st)
    if libraries_target:
        conf.env.append_unique("TARGETLIB", libraries_target)
    if cflags_common:
        conf.env.append_unique("CFLAGS", cflags_common)

    if cflags_common_compile_only:
        conf.env.append_unique("CFLAGS_COMPILE_ONLY", cflags_common_compile_only)
    conf.env.append_unique(
        "CFLAGS_COMPILE_ONLY",
        [
            "--gen_cross_reference_listing",
            "--gen_func_info_listing",
            "--gen_preprocessor_listing",
        ],
    )

    if cflags_foxbms:
        conf.env.append_unique("CFLAGS_FOXBMS", cflags_foxbms)
    if cflags_hal:
        conf.env.append_unique("CFLAGS_HAL", cflags_hal)
    if cflags_os:
        conf.env.append_unique("CFLAGS_OS", cflags_os)
