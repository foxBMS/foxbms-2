#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Generate a json file based on an RFG input file."""

import argparse
import json
import logging
import os
import sys
from pathlib import Path

HAVE_GIT = False
try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    pass


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of test_f_guidelines

    Returns:
        root (string): root path of the git repository
    """
    root = os.path.join(os.path.dirname(path), "..", "..", "..")
    if HAVE_GIT:
        try:
            repo = Repo(path, search_parent_directories=True)
            root = repo.git.rev_parse("--show-toplevel")
        except InvalidGitRepositoryError:
            pass
    return root


ROOT = Path(get_git_root(os.path.realpath(__file__)))


def find_axivion():
    """Checks for Axivion Bauhaus installation."""
    axivion_scripts = ""
    default_path = Path(
        os.path.join(
            os.environ.get(
                "ProgramFiles(x86)", os.path.join("C:", "Program Files (x86)")
            ),
            "Bauhaus",
            "lib",
            "scripts",
        )
    )
    if default_path.is_dir():
        axivion_scripts = default_path
    else:
        versioned_base_path = os.path.join("C:", os.sep, "Bauhaus")
        all_installs = []
        if os.path.exists(versioned_base_path):
            all_installs = os.listdir(versioned_base_path)
        all_install_paths = []
        for i in all_installs:
            all_install_paths.append(os.path.join(versioned_base_path, i))
        latest_version = ""
        if all_install_paths:
            latest_version = os.path.join(all_install_paths[-1], "lib", "scripts")
            axivion_scripts = latest_version
    return axivion_scripts


ax_install = find_axivion()
if not ax_install:
    sys.exit("Could not find an Axivion installation.")

sys.path.append(ax_install)
from bauhaus import rfg  # pylint: disable=import-error,wrong-import-position


class RfgParsing:
    """Helper class to parse the rfg file"""

    def __init__(self, rfg_graph) -> None:
        self.rfg_graph = rfg_graph
        self.graph_to_json = {}

    @staticmethod
    def get_name(node):
        """Returns the name of the source"""
        try:
            return node["Source.Name"]
        except Exception:  # pylint: disable=broad-except
            return None

    @staticmethod
    def get_sloc(elem):
        """Returns the source code location"""
        try:
            return f"{elem['Source.Path']}{elem['Source.File']}:{elem['Source.Line']}"
        except Exception:  # pylint: disable=broad-except
            return None

    def export_dependencies(self, ignore):
        """Generates a dictionary based on the graph"""
        for edge in self.rfg_graph.view("Code Facts").edges(
            edge_matcher=lambda e: not e.is_of_subtype("Belongs_To")
        ):
            dependency_type = edge.edge_type().name()
            entity_name = self.get_name(edge.target())
            entity_sloc = self.get_sloc(edge.target())
            usage_sloc = self.get_sloc(edge)
            if (
                entity_name
                and entity_sloc
                and usage_sloc
                and not os.path.isabs(entity_sloc)
            ):
                if "HL_" in entity_sloc or any(ext in usage_sloc for ext in ignore):
                    continue
                if self.graph_to_json.get(entity_name, None):
                    self.graph_to_json[entity_name]["usage"].append(usage_sloc)
                else:
                    self.graph_to_json[entity_name] = {
                        "entity": entity_name,
                        "type": dependency_type,
                        "impl": entity_sloc,
                        "usage": [usage_sloc],
                    }

    def save(self, out=Path("out.json")):
        """Saves the json output to a file."""
        with open(out.resolve(), "w", encoding="utf-8") as outfile:
            outfile.write(json.dumps(self.graph_to_json, indent=4, sort_keys=True))
            outfile.write("\n")


def main():
    """This script transforms some Axivion rfg information into a json file."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i",
        "--rfg-file",
        dest="rfg_file",
        action="store",
        default=Path(
            os.path.join(
                os.environ["USERPROFILE"],
                ".bauhaus",
                "localbuild",
                "projects",
                "foxbms-2.rfg",
            )
        ),
        help="Axivion rfg file to analysed.",
    )
    parser.add_argument(
        "-o",
        dest="out",
        action="store",
        default=Path(os.path.join(ROOT, "build", "axivion", "foxbms-2.rfg.json")),
        help="Output file",
    )
    parser.add_argument(
        "--ignore-locs",
        dest="ignore_locs",
        default=["axivion_preinc.h", "/os/", "HL_"],
        nargs="+",
        help="usage locs to be ignored",
    )
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    args.rfg_file = Path(args.rfg_file)
    if not args.rfg_file.is_file():
        sys.exit(f"Could not find input file '{args.rfg_file.resolve()}'.")
    logging.debug(f"Found input file '{args.rfg_file.resolve()}'.")
    logging.info("Analysing rfg file...")
    rfg_analysis = RfgParsing(rfg.Graph(args.rfg_file.resolve()))
    rfg_analysis.export_dependencies(ignore=args.ignore_locs)
    logging.info("Done...")
    logging.info(f"Saving output to '{args.out.resolve()}'")
    rfg_analysis.save(args.out.resolve())
    logging.info("Done...")


if __name__ == "__main__":
    main()
