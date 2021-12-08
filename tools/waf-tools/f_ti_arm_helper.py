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

"""A tool to write the ARM CGT compiler builtin defines (based on the given
flags) into a separate file"""

import os
import traceback

from waflib import Context, Errors, Logs, Utils
from waflib.Configure import conf


@conf
def run_build_for_defines(self, *k, **kw):  # pylint: disable-msg=unused-argument
    """Runs a build during configuration time. The build is based on
    :func:`waflib.Configure.run_build`. In contrast to a test build during
    configuration the output is persistent.
    This output is used to determine the predefined compiler defines.

    This implementation is based on ``waflib.Configure.run_build``:
    We do the same as in run_build, except, that we hard code the output
    directory and change the return value to return the build success/failure
    and the path of the build directory

    Args:
        out_name(string): name of the output directory. Needs to be passed as kw.

    Returns:
        tuple: A tuple containing the success of the build and the path to the output directory
    """
    buf = []
    for key in sorted(kw.keys()):
        v = kw[key]  # pylint: disable-msg=invalid-name
        if hasattr(v, "__call__"):
            buf.append(Utils.h_fun(v))
        else:
            buf.append(str(v))
    h = Utils.h_list(buf)  # pylint: disable-msg=invalid-name,W0612
    _dir = (
        self.bldnode.abspath()
        + os.sep
        + (not Utils.is_win32 and "." or "")
        + kw["out_name"]
    )

    try:
        os.makedirs(_dir)
    except OSError:
        pass

    try:
        os.stat(_dir)
    except OSError:
        self.fatal(f"cannot use the configuration test folder {_dir}")

    bdir = os.path.join(_dir, "build")

    if not os.path.exists(bdir):
        os.makedirs(bdir)

    cls_name = kw.get("run_build_cls") or getattr(self, "run_build_cls", "build")
    self.test_bld = bld = Context.create_context(cls_name, top_dir=_dir, out_dir=bdir)
    bld.init_dirs()
    bld.progress_bar = 0
    bld.targets = "*"

    bld.logger = self.logger
    bld.all_envs.update(self.all_envs)
    bld.env = kw["env"]

    bld.kw = kw
    bld.conf = self
    kw["build_fun"](bld)
    ret = -1

    try:
        bld.compile()
    except Errors.WafError:
        ret = f"Test does not build: {traceback.format_exc()}"
        self.fatal(ret)
    else:
        ret = getattr(bld, "retval", 0)
    return (ret, bdir)


@conf
def get_defines(self, *k, **kw):
    """Wrapper function to get all predefined compiler defines. Based on
    :func:`waflib.Tools.c_config.check`. This function uses
    :py:meth:`run_build_for_defines` to perform the actual build

    This implementation is based on ``waflib.Tools.c_config.check``:
    We do the same, as in c_config.check, except, that we use
    :py:meth:`run_build_for_defines`, which is based on waf's run_build to
    create a persistent build directory.
    This is required as we need to parse the testbuild output in order to get
    the list of predefined defines of the compiler. We could have also used
    waf's default --confcache option and set --confcache's default to 1, but
    this would create a really cluttered output directory (Every test build of
    a configure command would be persistent). With this implementation we can
    still use waf's default check(...) feature along with it's option
    --confcache to have a good debug experience on the build process while not
    cluttering the output directory.
    """

    testfile_basename = "predefined_defines"
    kw["features"] = "c"
    kw["idx"] = 0
    kw["fragment"] = "int main() {\n    return 0;\n}\n"
    kw["out_name"] = testfile_basename
    kw["compile_filename"] = f"{testfile_basename}.c"
    kw["msg"] = "Getting predefined compiler defines"

    self.validate_c(kw)  # TODO try to remove this
    self.start_msg(kw["msg"], **kw)
    ret = None
    out_dir = None
    try:
        (ret, out_dir) = self.run_build_for_defines(*k, **kw)
    except self.errors.ConfigurationError:
        self.end_msg(kw["errmsg"], "YELLOW", **kw)
        if Logs.verbose > 1:  # pylint: disable-msg=R1720
            raise
        else:
            self.fatal("The configuration failed")
    else:
        defines_file = os.path.join(
            out_dir, kw["compile_filename"] + "." + str(kw["idx"]) + ".ppm"
        )
        kw["success"] = ret
        kw["okmsg"] = defines_file

    ret = self.post_check(*k, **kw)  # TODO try to remove this
    if not ret:
        self.end_msg(kw["errmsg"], "YELLOW", **kw)
        self.fatal(f"The configuration failed {ret}")
    else:
        self.end_msg(self.ret_msg(kw["okmsg"], kw), **kw)
    return defines_file
