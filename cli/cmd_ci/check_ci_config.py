#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Checks that the GitLab CI configuration adheres to some rules"""

import logging
import sys
from dataclasses import dataclass

import yaml

from ..helpers.ansi_colors import RED
from ..helpers.misc import PROJECT_ROOT, eprint

CI_CONFIG = PROJECT_ROOT / ".gitlab-ci.yml"


@dataclass
class Stage:
    """container for stage"""

    name: str
    prefix: str


class VerificationError:  # pylint:disable=too-few-public-methods
    """Error class"""

    def __init__(self) -> None:
        self.error_level = 0

    def log(self, msg) -> int:
        """Error increment and logging"""
        self.error_level += 1
        logging.error(msg)
        return self.error_level


def check_ci_config() -> int:
    """Reads the '.gitlab-ci.yml' file and validates it"""
    err = 0
    if not CI_CONFIG.is_file():
        eprint(f"File '{CI_CONFIG}' does not exist.", err=True, color=RED)
        err += 1
    if err:
        return err
    with open(CI_CONFIG, encoding="utf-8") as f:
        ci_config_txt = f.read()
        try:
            ci_config: dict = yaml.load(ci_config_txt, Loader=yaml.Loader)
        except yaml.YAMLError as exc:
            sys.exit(f"{CI_CONFIG} is not a valid yaml file {exc}).")

    stages = [
        Stage(i, "".join([j[0] for j in i.split("_")]))
        for i in ci_config.get("stages", [])
    ]
    if not stages:
        sys.exit("Could not determine stages.")

    # check that all job use the correct prefix
    special_keys = ["variables", "workflow", "stages", "before_script"]
    prefixes = tuple(f"{i.prefix}_" for i in stages)
    error = VerificationError()
    for k in ci_config.keys():
        if k.startswith(".") or k in special_keys:
            continue
        if not k.startswith(prefixes):
            error.log(f"Job '{k}' uses the wrong prefix.")

    # all files that are listed in '.parallel-build-bin-and-spa' shall be
    # gathered along with the respective binaries
    parallel_build_bin_and_spa_artifact_strings = []
    for i in ci_config[".parallel-build-bin-and-spa-template"]["parallel"]["matrix"]:
        artifact_parts = []
        for _, v in i.items():
            artifact_parts.append(str(v))
        artifact_string = ", ".join(artifact_parts)
        parallel_build_bin_and_spa_artifact_strings.append(artifact_string)

    # construct expected binaries so that none is missed.
    # creating jobs are 'bb_all_config_variants' and 'spa_build'
    expected = []
    for i in parallel_build_bin_and_spa_artifact_strings:
        expected.extend(
            [
                f"bb_all_config_variants_0: [{i}]",
                f"spa_build: [{i}]",
            ]
        )
    if not sorted(expected) == sorted(ci_config["spae_gather_spa_artifacts"]["needs"]):
        missing = set(expected) - set(ci_config["spae_gather_spa_artifacts"]["needs"])
        error.log(
            f"Some artifacts are missing in job 'spae_gather_spa_artifacts' ({missing}).",
        )

    # all testes that are run on the HIL shall be defined via the
    # ".parallel-build-bin-and-spa" key.
    # all HIL tests shall start with 'th_test', expect for the known helpers
    for k, v in ci_config.items():
        if not k.startswith("th_"):
            continue
        if k in ["th_ensure_power_supply_is_off", "th_switch_power_supply_off"]:
            continue
        needs = v.get("needs", [])
        if not needs:
            error.log(f"{k} does not specify any artifacts.")
            continue
        # first element now guaranteed
        if not needs[0] == "th_ensure_power_supply_is_off":
            error.log(
                f"The first entry for '{k}:needs:' shall be 'th_ensure_power_supply_is_off'.",
            )
        if len(needs) > 2:
            error.log(f"Too many artifacts specified in '{k}:needs:'.")
        if len(needs) < 2:
            error.log(f"Too few artifacts specified in '{k}:needs:'.")
            continue
        # second element now guaranteed
        if needs[1] not in [
            x for x in expected if x.startswith("bb_all_config_variants_0: [")
        ]:
            error.log(
                f"The test '{k}' specifies an artifact that is is not "
                "specified in '.parallel-build-bin-and-spa'",
            )

    return min(error.error_level, 255)
