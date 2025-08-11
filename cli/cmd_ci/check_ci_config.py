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

"""Checks that the GitLab CI configuration adheres to some rules"""

import copy
import sys
from dataclasses import dataclass, field
from pathlib import Path

import yaml

from ..helpers.click_helpers import recho
from ..helpers.misc import PROJECT_ROOT

CI_MAIN_CONFIG = PROJECT_ROOT / ".gitlab-ci.yml"
CI_PIPELINE_DIR = PROJECT_ROOT / ".gitlab/ci_pipeline"


@dataclass(order=True)
class Stage:
    """container for stage"""

    sort_index: str = field(init=False)
    name: str
    prefix: str = ""

    def __post_init__(self) -> None:
        self.prefix = "".join([j[0] for j in self.name.split("_")])
        self.sort_index = self.name


def read_config() -> dict:
    """Reads the '.gitlab-ci.yml' file"""
    with open(CI_MAIN_CONFIG, encoding="utf-8") as f:
        ci_config_txt = f.read()
        try:
            ci_config: dict = yaml.load(ci_config_txt, Loader=yaml.Loader)
        except yaml.YAMLError:
            sys.exit(f"{CI_MAIN_CONFIG} is not a valid yaml file.")
    err = 0
    if not ci_config:
        sys.exit(f"'{CI_MAIN_CONFIG}' is empty.")
    if not ci_config.get("include", []):
        sys.exit(f"Expected 'include' key to be defined in '{CI_MAIN_CONFIG}'.")

    for included_yml in ci_config["include"]:
        tmp = Path(included_yml["local"])
        if not tmp.is_file():
            err += 1
            recho(f"Could not find file '{tmp}'.")
            continue

        included_ci_config_txt = tmp.read_text(encoding="utf-8")
        try:
            included_ci_config: dict = yaml.load(
                included_ci_config_txt, Loader=yaml.Loader
            )
        except yaml.YAMLError:
            err += 1
            recho(f"'{included_yml['local']}' is not a valid yml file.")
            continue
        ci_config = {**ci_config, **included_ci_config}

    if err:
        sys.exit(
            "Could not find some included file(s) or files are not valid yml files."
        )
    return ci_config


def get_stages(ci_config: dict) -> list[Stage]:
    """Returns a list of stage objects"""
    stages = [Stage(i) for i in ci_config.get("stages", [])]
    if not stages:
        sys.exit("Could not determine stages.")
    return stages


def match_stages_with_files(
    ci_config: dict, stages: list[Stage], stage_files: list[Path]
) -> int:
    """Check that the setup:
    defined stages <-> included stage files <-> existing stage files matches"""
    # the number of defined stages in the CI yml file needs to be the same as
    # the number of yml files found in the pipeline directory
    if len(stages) != len(stage_files):
        recho(
            f"Number of stages ({len(stages)}) does not match the number "
            f"stage files ({len(stage_files)}) found in '{CI_PIPELINE_DIR}'."
        )
        return 1

    # the number of files defines in the 'include' key in the CI yml file needs
    # to be the same as the number of yml files found in the pipeline directory
    if len(ci_config["include"]) != len(stage_files):
        recho(
            f"Number of included stages ({len(ci_config['include'])}) does "
            f"not match the number stage files ({len(stage_files)})."
        )
        recho(
            "included files: "
            + "\n".join(sorted(i["local"] for i in ci_config["include"]))
        )
        recho(
            "stage files: " + "\n".join(sorted(str(i) for i in stage_files)),
        )
        return 1
    return 0


def check_job_prefix(ci_config: dict, stages: list[Stage]) -> int:
    """check that all job use the correct prefix"""
    special_keys = ["variables", "workflow", "stages", "before_script", "include"]
    prefixes = tuple(f"{i.prefix}_" for i in stages)
    for k in ci_config.keys():
        if k.startswith(".") or k in special_keys:
            continue
        if not k.startswith(prefixes):
            recho(f"Job '{k}' uses the wrong prefix.")
            return 1
    return 0


def check_stage_order(ci_config: dict, stages: list[Stage]) -> int:
    """Check that the CI file is ordered consistently."""
    tmp = copy.deepcopy(stages)
    tmp.remove(Stage("configure", "c"))
    err = 0
    for i, (stage, stage_file) in enumerate(zip(tmp, ci_config["include"])):
        matched_stage_file = Path(stage_file["local"]).stem
        if not stage.name == matched_stage_file:
            err += 1
            recho(
                f"{i + 1}: 'stages' list and included documents are not in the"
                "  same order.\n"
                f" --> Stage '{stage.name}' maps to '{matched_stage_file}'."
            )
    return err


def check_emb_spa_build_alignment(ci_config: dict) -> int:
    """Check that target and SPA build are aligned."""
    if not ci_config.get(".parallel-build_app_embedded-template"):
        recho("key '.parallel-build_app_embedded-template' is not defined.")
        return 1
    if not ci_config.get(".parallel-build_app_spa-template"):
        recho("key '.parallel-build_app_spa-template' is not defined.")
        return 1

    if not (
        ci_config[".parallel-build_app_embedded-template"]
        == ci_config[".parallel-build_app_spa-template"]
    ):
        err_msg = (
            "Error:The build matrix defined in\n"
            f"  {CI_PIPELINE_DIR / 'build_app_embedded.yml'}: "
            "key '.parallel-build_app_embedded-template'\nand\n"
            f"  {CI_PIPELINE_DIR / 'static_program_analysis.yml'}: "
            "key '.parallel-build_app_spa-template'\n"
            "need to be equal."
        )
        recho(err_msg)
        return 1
    return 0


def validate_build_app_matrix(ci_config: dict) -> int:
    """Validate the application build matrix is set."""
    try:
        ci_config[".parallel-build_app_embedded-template"]["parallel"]["matrix"]
    except (KeyError, TypeError):
        recho(
            "Key '.parallel-build_app_embedded-template:↳parallel:↳matrix' is "
            "missing or uses a wrong format."
        )
        return 1
    return 0


def get_expected_artifacts(ci_config: dict) -> list[str]:
    """Return the list of artifacts we expect from the target and the SPA build."""
    # all files that are listed in '.parallel-build_app_embedded-template'
    # shall be gathered along with the respective binaries
    parallel_build_app_embedded_and_spa_artifact_strings: list[str] = []
    for i in ci_config[".parallel-build_app_embedded-template"]["parallel"]["matrix"]:
        artifact_parts: list[str] = []
        for _, v in i.items():
            artifact_parts.append(str(v))
        artifact_string: str = ", ".join(artifact_parts)
        parallel_build_app_embedded_and_spa_artifact_strings.append(artifact_string)

    # construct expected binaries so that none is missed.
    # creating jobs are 'bb_all_config_variants' and 'spa_build'
    expected: list[str] = []
    for i in parallel_build_app_embedded_and_spa_artifact_strings:
        expected.extend(
            [
                f"bae_all_config_variants: [{i}]",
                f"spa_build: [{i}]",
            ]
        )
    return expected


def validate_spae_artifacts(ci_config: dict, expected) -> int:
    """Validate the artifacts that are required for the SPA evaluation artifact
    package"""
    try:
        ci_config["spae_gather_spa_artifacts"]["needs"]
    except (KeyError, TypeError):
        recho(
            "Key 'spae_gather_spa_artifacts:↳needs' is missing or uses a wrong format."
        )
        return 1

    if not sorted(expected) == sorted(ci_config["spae_gather_spa_artifacts"]["needs"]):
        found = str(sorted(ci_config["spae_gather_spa_artifacts"]["needs"]))
        recho(
            "Some artifacts are missing in job 'spae_gather_spa_artifacts':\n"
            f"Expected: {sorted(expected)}\nFound: {found}",
        )
        return 1
    return 0


# pylint: disable=too-many-branches
def validate_hil_test_config(ci_config: dict, expected: list[str]) -> int:
    """Validate the configuration of the HIL setup."""
    error = 0
    # all testes that are run on the HIL shall be defined via the
    # ".parallel-build_app_embedded-template" key.
    # all HIL tests shall start with 'th_test', expect for the known helpers
    for k, v in ci_config.items():
        if not k.startswith("th_"):
            continue
        if k == "th_ensure_power_supply_is_off":
            continue
        if k == "th_switch_power_supply_off":
            jobs = list(
                i for i in copy.deepcopy(ci_config).keys() if i.startswith("th_")
            )
            jobs.remove("th_switch_power_supply_off")
            if not ci_config[k].get("needs", []) == jobs:
                recho("Key 'th_switch_power_supply_off:↳needs' is missing test jobs.")
                error += 1
            continue
        try:
            needs = v["needs"]
        except (AttributeError, KeyError, TypeError):
            needs = []
        if not needs:
            recho(f"{k} does not specify any artifacts.")
            error += 1
            continue
        # first element now guaranteed
        if not needs[0] == "th_ensure_power_supply_is_off":
            recho(
                f"The first entry for '{k}:↳needs:' shall be "
                "'th_ensure_power_supply_is_off'."
            )
            error += 1

        if k == "th_bootload_on_hardware_unit_tests":
            if not needs[-1] == "bbe_tms570_on_hardware_unit_test":
                recho(
                    f"The last entry for '{k}:↳needs:' shall be "
                    "'bbe_tms570_on_hardware_unit_test'."
                )
                error += 1
            continue
        if k == "th_flash_and_test_bootloader":
            if not needs[-1] == "bbe_tms570":
                recho(f"The last entry for '{k}:↳needs:' shall be 'bbe_tms570'.")
                error += 1
            if not needs[-2] == "th_bootload_on_hardware_unit_tests":
                recho(
                    f"The second last entry for '{k}:↳needs:' shall be "
                    "'th_bootload_on_hardware_unit_tests'."
                )
                error += 1
            continue
        if not needs[-1] == "th_flash_and_test_bootloader":
            # ensure that the bootloader has been flashed
            recho(
                f"The last entry for '{k}:needs:' shall be "
                "'th_flash_and_test_bootloader'."
            )
            error += 1

        #  check that all needed dependencies are there
        if k == "th_test_bootloader":
            expected_needs = [
                "th_ensure_power_supply_is_off",
                "bae_all_config_variants: "
                "[freertos, adi, ades1830, vbb, cc, cc, tr, none, none, no-imd, 1, 2, 16]",
                "th_test_debug_simple_tests",
                "th_flash_and_test_bootloader",
            ]
            if needs != expected_needs:
                recho(
                    f"{k}:needs:' needs to list the following dependencies:\n{expected_needs}."
                )
                error += 1
            continue
        if k == "th_freertos_plus_tcp":
            expected_needs = [
                "th_ensure_power_supply_is_off",
                "bae_use_freertos_plus_tcp",
                "th_flash_and_test_bootloader",
            ]
            if needs != expected_needs:
                recho(
                    f"{k}:needs:' needs to list the following dependencies:\n{expected_needs}."
                )
                error += 1
            continue
        if len(needs) > 3:
            recho(f"Too many artifacts specified in '{k}:needs:'.")
            error += 1
        if len(needs) < 3:
            recho(f"Too few artifacts specified in '{k}:needs:'.")
            error += 1
            continue
        # second element now guaranteed
        if needs[1] not in [
            x for x in expected if x.startswith("bae_all_config_variants: [")
        ]:
            recho(
                f"The test '{k}' specifies an artifact that is is not "
                "specified in '.parallel-build_app_embedded-template'."
            )
            error += 1
    return error


# pylint: disable-next=too-many-return-statements
def analyze_config(ci_config: dict, stage_files: list[Path]) -> int:
    """Analyze the loaded configuration."""
    stages = get_stages(ci_config)
    tmp = []
    for stage in stages:  # configure is notdefined an own file
        if stage.name != "configure":
            tmp.append(stage)

    if err := match_stages_with_files(ci_config, tmp, stage_files):
        return err

    # check that all job use the correct prefix
    if err := check_job_prefix(ci_config, stages):
        return err

    # order in the 'stages' list and the list of files to be included shall be
    # in the same order
    if err := check_stage_order(ci_config, stages):
        return err

    # check that target and SPA build are aligned
    if err := check_emb_spa_build_alignment(ci_config):
        return err

    if err := validate_build_app_matrix(ci_config):
        return err

    expected = get_expected_artifacts(ci_config)

    if err := validate_spae_artifacts(ci_config, expected):
        return err

    return validate_hil_test_config(ci_config, expected)


def check_ci_config() -> int:
    """Reads the '.gitlab-ci.yml' file and validates it."""
    if not CI_MAIN_CONFIG.is_file():
        recho(f"File '{CI_MAIN_CONFIG}' does not exist.")
        return 1

    ci_config = read_config()
    stage_files = list(CI_PIPELINE_DIR.glob("*.yml"))

    return analyze_config(ci_config, stage_files)
