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

"""Generates the list of conda packages for the documentation"""

from typing import Dict, List, Tuple, Union

import logging
import argparse
import os
import shutil
import sys
import subprocess
from datetime import date
from io import StringIO
import json
import pandas as pd


NAME_COLUMN_WIDTH = 36
VERSION_COLUMN_WIDTH = 20
BUILD_COLUMN_WIDTH = 24
CHANNEL_COLUMN_WIDTH = 12
LICENSE_COLUMN_WIDTH = 76

NAME_COLUMN_NAME = "Name"
VERSION_COLUMN_NAME = "Version"
BUILD_COLUMN_NAME = "Build"
CHANNEL_COLUMN_NAME = "Channel"
LICENSE_COLUMN_NAME = "License"
LATEST_LICENSE_COLUMN_NAME = "Latest license check"

MAGIC_KEYWORD_UNKNOWN_LICENSE = "UNKNOWN"


class PackageEntry:
    """class for representing a package entry"""

    # pylint:disable=too-many-arguments
    def __init__(
        self,
        name: str = None,
        version: str = None,
        build: str = None,
        channel: str = None,
        license_type: Union[str, None] = None,
        latest_license_check: Union[str, None] = None,
    ) -> None:
        super().__init__()
        self.name = name
        self.version = version
        self.build = build
        self.channel = channel
        self.license_type = license_type
        self.latest_license_check = latest_license_check

    def __repr__(self):
        output = f"{self.name};".ljust(NAME_COLUMN_WIDTH)
        output += f"{self.version};".ljust(VERSION_COLUMN_WIDTH)
        output += f"{self.build};".ljust(BUILD_COLUMN_WIDTH)
        output += f"{self.channel};".ljust(CHANNEL_COLUMN_WIDTH)
        if self.license_type:
            output += f"{self.license_type.replace(';',',')};".ljust(
                LICENSE_COLUMN_WIDTH
            )
        else:
            output += f"{self.license_type};".ljust(LICENSE_COLUMN_WIDTH)
        output += f"{self.latest_license_check}"
        return output

    @staticmethod
    def date_get_today() -> str:
        """returns the datestring for the output"""
        today = date.today()
        return today.strftime("%Y-%m-%d")

    def touch_latest_license_check(self) -> None:
        """sets the date of the license check to today"""
        self.latest_license_check = self.__class__.date_get_today()


class CondaParser:  # pylint:disable=too-few-public-methods
    """Helper class for parsing conda output"""

    @staticmethod
    def _parse_package_list(conda_output: str = None) -> List[PackageEntry]:
        """parses the conda output into a list"""
        csv = pd.read_csv(
            StringIO(conda_output),
            sep=r"\s+",
            engine="python",
            header=None,
            comment="#",
            names=[
                NAME_COLUMN_NAME,
                VERSION_COLUMN_NAME,
                BUILD_COLUMN_NAME,
                CHANNEL_COLUMN_NAME,
            ],
        )
        logging.debug(csv)

        package_list: List[PackageEntry] = []

        for index, row in csv.iterrows():
            package = PackageEntry(
                name=row[NAME_COLUMN_NAME],
                version=row[VERSION_COLUMN_NAME],
                build=row[BUILD_COLUMN_NAME],
                channel=row[CHANNEL_COLUMN_NAME],
            )
            logging.debug(f"package from line {index}: {package}")
            package_list.append(package)

        return package_list

    @staticmethod
    def get_package_list_from_conda() -> List[PackageEntry]:
        """calls conda and obtains a list of packages"""
        conda = shutil.which("conda")
        logging.info(f"Conda instance at: {conda}")
        if not conda:
            sys.exit("Could not find conda binary.")
        cmd = [conda, "list", "--show-channel-urls"]
        with subprocess.Popen(cmd, stdout=subprocess.PIPE) as p:
            stdout: bytes = p.communicate()[0]
        package_list: List[PackageEntry] = CondaParser._parse_package_list(
            stdout.decode("utf-8")
        )
        logging.debug(package_list)
        return package_list


class PipParser:  # pylint:disable=too-few-public-methods
    """Helper class for parsing pip packages"""

    @staticmethod
    def _retrieve_pip_licenses() -> List[Dict[str, str]]:
        """retrieves the pip licenses by calling pip-licenses"""
        piplicenses = shutil.which("pip-licenses")
        logging.info(f"Conda instance at: {piplicenses}")
        if not piplicenses:
            sys.exit("Could not find pip-licenses binary.")
        cmd = [piplicenses, "--format=json"]
        # pylint: disable=consider-using-with
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        pippackages = json.loads(p.communicate()[0])
        logging.debug(pippackages)
        return pippackages

    @staticmethod
    def enhance_with_pip_licenses(
        input_package_list: List[PackageEntry] = None,
    ) -> List[PackageEntry]:
        """returns the licenses of all pip packages"""
        package_list = []
        if input_package_list is not None:
            package_list = input_package_list

        pip_json = PipParser._retrieve_pip_licenses()
        logging.info("Scanning pip license list for bad entries and dropping them.")
        pip_json = list(
            filter(
                lambda x: x[LICENSE_COLUMN_NAME] != MAGIC_KEYWORD_UNKNOWN_LICENSE,
                pip_json,
            )
        )

        logging.info("Iterating over licenses in pip JSON")
        for package in pip_json:
            logging.debug(package)
            hit = False

            # handle special names
            if package[NAME_COLUMN_NAME] == "libarchive-c":
                package[NAME_COLUMN_NAME] = "python-libarchive-c"
            elif package[NAME_COLUMN_NAME] == "ruamel.yaml":
                package[NAME_COLUMN_NAME] = "ruamel-yaml"
            elif package[NAME_COLUMN_NAME] == "ruamel.yaml.clib":
                package[NAME_COLUMN_NAME] = "ruamel-yaml-clib"
            else:
                # normalize package name to lower-case
                package[NAME_COLUMN_NAME] = package[NAME_COLUMN_NAME].lower()

            def match_package_name(
                package_list: List[PackageEntry], package: Dict[str, str]
            ) -> Tuple[bool, List[PackageEntry]]:
                """copy data into package_list if it can be matched with package"""
                package_name = package[NAME_COLUMN_NAME]
                hit = False
                for listed_package in package_list:
                    if listed_package.name == package_name:
                        logging.debug(
                            f"Found hit in package list for package {listed_package.name}"
                        )
                        assert listed_package.version == package[VERSION_COLUMN_NAME]
                        logging.debug(
                            f"Version in JSON and package list match: {listed_package.version}"
                        )
                        listed_package.license_type = package[LICENSE_COLUMN_NAME]
                        listed_package.touch_latest_license_check()
                        logging.debug(
                            f"Assigning license {package[LICENSE_COLUMN_NAME]}"
                            f" (checked {listed_package.latest_license_check}) to {package_name}."
                        )

                        hit = True
                return hit, package_list

            hit, package_list = match_package_name(
                package_list=package_list, package=package
            )
            if not hit:
                # try with canonized name
                package[NAME_COLUMN_NAME] = package[NAME_COLUMN_NAME].replace("-", "_")
                hit, package_list = match_package_name(
                    package_list=package_list, package=package
                )
            if not hit:
                logging.error(f"No match found for {package[NAME_COLUMN_NAME]}.")

        return package_list


class OldFileParser:
    """Helper class for parsing existing license files"""

    @staticmethod
    def turn_old_file_into_list_of_entries(
        old_file: Union[os.PathLike[str], os.PathLike[bytes]] = None,
    ) -> List[PackageEntry]:
        """Takes as argument the path to an old csv, tries to read it and
        turn into list of packages"""
        package_list = []
        if old_file is not None:
            with open(old_file, mode="r", encoding="utf-8") as file_handle:
                csv = pd.read_csv(file_handle, sep=";", skipinitialspace=True)
            logging.debug(csv)
            for index, row in csv.iterrows():
                package = PackageEntry(
                    name=row[NAME_COLUMN_NAME],
                    version=row[VERSION_COLUMN_NAME],
                    build=row[BUILD_COLUMN_NAME],
                    channel=row[CHANNEL_COLUMN_NAME],
                    license_type=row[LICENSE_COLUMN_NAME],
                    latest_license_check=row[LATEST_LICENSE_COLUMN_NAME],
                )
                logging.debug(f"Extracted old package from entry {index}: {package}")
                package_list.append(package)

        return package_list

    @staticmethod
    def prefill_license_data_from_old_package_list(
        old_list: List[PackageEntry] = None, new_list: List[PackageEntry] = None
    ) -> List[PackageEntry]:
        """prefills the license data in a new list from the old list"""
        if (old_list is None) or (new_list is None):
            sys.exit("Package List is None even though it should not be.")

        for package in new_list:
            for old_package in old_list:
                if package.name == old_package.name:
                    package.license_type = old_package.license_type
                    package.latest_license_check = old_package.latest_license_check
                    logging.info(f"Prefilled package {package.name} from old list.")

        return new_list


def generate_output(package_list: List[PackageEntry]) -> None:
    """generates the output to stdout"""

    # print with hardcoded columns for consistency
    name_text = f"{NAME_COLUMN_NAME};".ljust(NAME_COLUMN_WIDTH)
    version_text = f"{VERSION_COLUMN_NAME};".ljust(VERSION_COLUMN_WIDTH)
    build_text = f"{BUILD_COLUMN_NAME};".ljust(BUILD_COLUMN_WIDTH)
    channel_text = f"{CHANNEL_COLUMN_NAME};".ljust(CHANNEL_COLUMN_WIDTH)
    license_text = f"{LICENSE_COLUMN_NAME};".ljust(LICENSE_COLUMN_WIDTH)
    latest_license_text = LATEST_LICENSE_COLUMN_NAME
    print(
        name_text
        + version_text
        + build_text
        + channel_text
        + license_text
        + latest_license_text
    )
    for package in package_list:
        print(package)


def main():
    """This script generates a list of conda packages and gathers license
    information on stdout.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    parser.add_argument(
        "-f",
        "--file",
        dest="old_file",
        action="store",
        default=None,
        help="path to an existing csv file",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    old_package_list = OldFileParser.turn_old_file_into_list_of_entries(args.old_file)

    package_list: List[PackageEntry] = CondaParser.get_package_list_from_conda()
    package_list = OldFileParser.prefill_license_data_from_old_package_list(
        old_list=old_package_list, new_list=package_list
    )
    package_list = PipParser.enhance_with_pip_licenses(input_package_list=package_list)

    generate_output(package_list=package_list)


if __name__ == "__main__":
    main()
