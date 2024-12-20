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

"""Creates the README for the GitLab CI configuration documentation."""

import re
import sys

import yaml
from click import secho

from ..helpers.misc import PROJECT_ROOT

CI_CONFIG = PROJECT_ROOT / ".gitlab-ci.yml"
CI_README = PROJECT_ROOT / ".gitlab/README.md"


MARKER = (
    "###############################################################################"
)


def create_readme() -> int:  # pylint:disable=too-many-branches
    """Reads the '.gitlab-ci.yml' file and creates the 'README.md'."""
    err = 0
    if not CI_CONFIG.is_file():
        secho(f"File '{CI_CONFIG}' does not exist.", fg="red", err=True)
        err += 1
    if not CI_README.is_file():
        secho(f"File '{CI_README}' does not exist.", fg="red", err=True)
        err += 1
    if err:
        return err
    with open(CI_CONFIG, encoding="utf-8") as f:
        ci_config_txt = f.read()
        try:
            ci_config: dict = yaml.load(ci_config_txt, Loader=yaml.Loader)
        except yaml.YAMLError as exc:
            sys.exit(f"{CI_CONFIG} is not a valid yaml file {exc}).")
    if "include" in list(ci_config.keys()):
        for included_yml in ci_config["include"]:
            with open(included_yml["local"], encoding="utf-8") as f:
                included_ci_config_txt = f.read()
                start_position_of_block = included_ci_config_txt.find(MARKER)
                included_ci_config_txt = included_ci_config_txt[
                    start_position_of_block - 1 :
                ]
                ci_config_txt = ci_config_txt + included_ci_config_txt

    if not ci_config.get("stages", []):
        sys.exit("Could not determine stages.")

    in_block = False
    in_stages = False
    done = False
    stage_docs: list[str] = []
    markdown: list[str] = []
    lines = ci_config_txt.splitlines()
    for i, line in enumerate(lines):
        # start of a block is defined as 78 # symbols in a row
        if not in_block:
            if line == MARKER:
                in_block = True
            continue
        if line == MARKER:
            continue
        # special handling for stages
        if line == "stages:":
            in_stages = True
            continue
        if in_stages:
            if not (line.strip().startswith("-") or line.strip().startswith("#")):
                in_stages = False
                continue
            if line.startswith("  -") and len(line.split("#")) == 2:
                if not stage_docs:
                    stage_docs.append("\n| stage name | job prefix |description |\n")
                    stage_docs.append("| --- | --- | --- |\n")
                s_name, s_docs = line.split("#")
                s_name = s_name.split("-")[1].strip()
                s_job_prefix = "".join([j[0] for j in s_name.split("_")])
                s_docs = s_docs.strip()
                stage_docs.append(f"| {s_name} | {s_job_prefix} |{s_docs} |\n")
                continue
        if not done and not in_stages and len(stage_docs) > 1:  # we are done parsing
            markdown.extend(stage_docs)
            markdown.append("\n")
            done = True
            continue
        # we reached some part of the document, where documentation started
        while line.startswith("#") or line.startswith("  #"):
            if line.startswith("#"):
                markdown.append(line[2:].rstrip() + "\n")
            elif line.startswith("  #"):
                if line.startswith("  # ###"):
                    markdown.append(f"### `{lines[i-1][:-1]}`" + "\n")
                if markdown[-1].startswith("### "):
                    line = line[5:]
                markdown.append(line[3:].strip() + "\n")
            break
        if not line:
            markdown.append("\n")
    markdown = re.sub(r"[\r\n][\r\n]{2,}", "\n\n", "".join(markdown)).split("\n")
    # Fix newlines
    markdown_fixed = []
    for i, line in enumerate(markdown):
        markdown_fixed.append(line)
        # we are in a list or heading
        if line.startswith("#") and not markdown[i + 1] == "":
            markdown_fixed.append("")
    with open(CI_README, "w", encoding="utf-8") as f:
        f.write("\n".join(markdown_fixed))
    return err
