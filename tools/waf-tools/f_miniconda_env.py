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

"""Implements a waf tool to check that all requirements for the development
workflow are installed properly.

:numref:`f-miniconda-env-usage` shows how to use this tool.

.. code-block:: python
    :caption: f_miniconda_env.py
    :name: f-miniconda-env-usage
    :linenos:

    def options(opt):
        opt.load("f_miniconda_env")

    def configure(conf):
        conf.load("f_miniconda_env")

"""
import sys
import os
import json
import yaml

from waflib import Context, Errors, Utils, Logs

if Utils.is_win32:
    CONDA_BASE = [
        os.path.join(os.environ["USERPROFILE"], "miniconda3"),
        os.path.join(os.environ["LOCALAPPDATA"], "Continuum", "miniconda3"),
        os.path.join(os.environ["ProgramData"], "Miniconda3"),
        os.path.join(os.environ["SystemDrive"], os.sep, "miniconda3"),
    ]
    ENV_DIRS = [
        os.path.join(i, "envs")
        for i in CONDA_BASE + [os.path.join(os.environ["USERPROFILE"], ".conda")]
    ]

else:
    CONDA_BASE = [
        os.path.join("~", "miniconda3"),
        os.path.join(os.sep, "opt", "miniconda3"),
    ]
    ENV_DIRS = [
        os.path.join(i, "envs") for i in CONDA_BASE + [os.path.join("~", ".conda")]
    ]


def options(opt):
    """Configuration options for the miniconda environment tool"""
    opt.add_option(
        "--ignore-env-check",
        action="store_true",
        default=False,
        dest="IGNORE_ENV_CHECK",
        help="Disable environment checking (not recommended)",
    )
    opt.add_option(
        "--conda-env-file",
        action="store",
        default=os.path.join(
            "conf", "env", f"conda_env_{Utils.unversioned_sys_platform()}.yaml"
        ),
        dest="CONDA_ENV_FILE",
        help="Path to conda environment specification file",
    )

    opt.add_option(
        "--conda-base-env",
        action="append",
        default=CONDA_BASE,
        dest="CONDA_BASE_ENV",
        help="Installation directory of the miniconda base environment",
    )
    opt.load("python")


def configure(conf):  # pylint: disable=too-many-statements,too-many-branches
    """configuration step of the miniconda environment tool"""
    try:
        conf.env.PYTHON[0]
    except IndexError:
        conf.load("python")
    if conf.options.IGNORE_ENV_CHECK:
        conf.env.ENV_CHECK = False
        return

    # check that all python packages are available
    if Utils.is_win32:
        a_dir = "Scripts"
    else:
        a_dir = "bin"
    path_list = [os.path.join(i, a_dir) for i in conf.options.CONDA_BASE_ENV]
    conf.find_program("conda", mandatory=False, path_list=path_list)
    try:
        conf.env.CONDA[0]
    except IndexError:
        conf.fatal(
            "A conda base installation is required at "
            f"{conf.options.CONDA_BASE_ENV}.\nAlternatively you can specify "
            "an installed conda base environment by passing it via "
            "'--conda-base-env'."
        )

    cmd = Utils.subst_vars("${CONDA} env list --json", conf.env).split()

    try:
        std = conf.cmd_and_log(cmd, output=Context.BOTH)
        available_conda_envs = json.loads(std[0])
    except Errors.WafError as env_search:
        Logs.error(env_search.msg.strip())
        conf.fatal("Searching for conda environments failed.")

    conda_env_spec_file = conf.path.find_node(conf.options.CONDA_ENV_FILE)
    with open(conda_env_spec_file.abspath(), "r", encoding="utf-8") as stream:
        try:
            conda_spec = yaml.load(stream, Loader=yaml.Loader)
        except yaml.YAMLError as exc:
            conf.fatal(exc)
    env = None
    for env in available_conda_envs["envs"]:
        if env.lower().endswith(conda_spec["name"]):
            conf.env.CONDA_DEVEL_ENV = conda_spec["name"]
            break
    if not conf.env.CONDA_DEVEL_ENV:
        conf.fatal(f"Development environment '{conf.env.CONDA_DEVEL_ENV}' not found.")

    correct_env = False
    # now we are sure that: at least a string (we found at least *something*),
    # is returned from shutil.which() and conf.env.PYTHON[0] exist.
    # Therefore the following comparisons are (from a type perspective) safe.
    if Utils.is_win32:
        if (
            # NTFS on Windows is case insensitive, so don't be too
            # strict on string comparison when we check paths
            sys.executable.lower() == conf.env.PYTHON[0].lower()
            and sys.executable.lower() == os.path.join(env.lower(), "python.exe")
        ):
            correct_env = True
    else:
        if sys.executable == conf.env.PYTHON[0] and sys.executable == os.path.join(
            env.lower(), "bin", "python"
        ):
            correct_env = True
    if not correct_env:
        Logs.error(f"The development environment {conda_spec['name']} is not active.")
        conf.fatal(
            f"Run 'conda activate {conda_spec['name']}' and configure the project again."
        )

    found_devel_env = False
    for e in ENV_DIRS:  # pylint: disable=invalid-name
        search_path = os.path.join(e, conf.env.get_flat("CONDA_DEVEL_ENV"))
        cmd = conf.env.CONDA + ["env", "export", "-p", search_path]
        try:
            std = conf.cmd_and_log(
                cmd, output=Context.BOTH, quiet=Context.BOTH, input=os.linesep.encode()
            )
        except Errors.WafError as env_export:
            Logs.error(env_export.msg.strip())
            conf.fatal(
                f"Could not export dependencies from environment {conda_spec['name']}"
            )
        tmp_env = yaml.load(std[0], Loader=yaml.Loader)
        if not tmp_env.get("dependencies", None):
            # if there are no dependencies, then the environment does not exist.
            continue

        conda_env_yaml = conf.path.get_bld().make_node(
            f"{conf.env.CONDA_DEVEL_ENV}_environment.yaml"
        )
        conda_env_yaml.write(std[0])
        with open(conda_env_yaml.abspath(), "r", encoding="utf-8") as stream:
            try:
                current_conda_env = yaml.load(stream, Loader=yaml.Loader)
            except yaml.YAMLError as exc:
                conf.fatal(exc)

        for i in current_conda_env["dependencies"]:
            if isinstance(i, dict):
                pips = i["pip"]

        pkg_error = False
        for _pkg in conda_spec["dependencies"]:
            if isinstance(_pkg, str):
                if _pkg in current_conda_env["dependencies"]:
                    Logs.debug(f"Found {_pkg.split('=')}")
                else:
                    Logs.warn(f"Could not find {_pkg.split('=')}")
                    pkg_error = True
            elif isinstance(_pkg, dict):
                for pip_pkg in _pkg["pip"]:
                    if pip_pkg in pips:
                        Logs.debug(f"Found {pip_pkg.split('==')}")
                    else:
                        Logs.warn(f"Could not find pip-package {pip_pkg.split('==')}")
                        pkg_error = True
        if pkg_error:
            Logs.warn(
                "There are package errors in environment "
                f"{conf.env.CONDA_DEVEL_ENV} at {search_path}.\n"
                "Proceeding with next search path."
            )
        else:
            # found a development environment, that satisfies the dependencies
            found_devel_env = True
            break

    if not found_devel_env:
        conf.fatal(
            f"No development environment found in {ENV_DIRS} that satisfies "
            "the dependencies."
        )
