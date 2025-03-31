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


"""Implementation for handling and preparing an application binary for usage
with the Bootloader."""

import json
import sys
from json import JSONDecodeError
from pathlib import Path

import numpy as np

from ..helpers.misc import (
    FOXBMS_APP_CRC_FILE,
    FOXBMS_APP_INFO_FILE,
    FOXBMS_BIN_FILE,
    get_sha256_file_hash_str,
)

CRC_TABLE_COLUMN_IDX = {
    "data_64_bits_hex": 1,
    "crc_64_bits_hex": 2,
    "data_64_bits": 3,
    "crc_64_bits": 4,
}


NUM_OF_BYTES_IN_8_BYTES = 8


# pylint: disable-next=too-many-instance-attributes
# pylint: disable-next=too-few-public-methods
class BootloaderBinaryFile:
    """Class to handle an application binary file and prepare the binary data
    for bootloader usage (e.g., calculate a CRC for of the application binary).
    """

    def __init__(
        self,
        app: Path = FOXBMS_BIN_FILE,
        crc_table: Path = FOXBMS_APP_CRC_FILE,
        program_info: Path = FOXBMS_APP_INFO_FILE,
    ):
        # Get the app relevant information from json file, PS: the app_size
        # only refers to the size of the app data part which does not contain
        # the vector table
        try:
            program = json.loads(program_info.read_text("utf-8"))
        except FileNotFoundError:
            sys.exit(
                f"{program_info} not found, please run 'waf build_app_embedded' "
                "command to build the project first/again, exit."
            )
        except (JSONDecodeError, TypeError):
            sys.exit(f"Can not load {program_info}, exit.")

        self.len_of_program_in_bytes = program["len_of_program_in_bytes"]
        self.len_of_program_in_8_bytes = program["len_of_program_in_8_bytes"]
        hash_app = program["hash_app"]
        hash_csv = program["hash_csv"]

        # Validate the bin and csv file using hash stored in json file
        try:
            hash_app_current = get_sha256_file_hash_str(app)
        except FileNotFoundError:
            sys.exit(
                f"{app} not found, please run 'waf build_app_embedded' "
                "command to build the project first/again, exit."
            )
        hash_csv_current = get_sha256_file_hash_str(crc_table)
        if (
            app != Path(program["bin_file"])
            or hash_app_current != hash_app
            or hash_csv != hash_csv_current
        ):
            sys.exit(
                f"{app} does not match {program_info}"
                " Please try to remove all these files using 'waf clean_app_embedded'"
                " and build them again using 'waf build_app_embedded'. Abort!"
            )

        # Extract the CRC array from the .csv file
        try:
            self.array_csv = np.genfromtxt(crc_table, delimiter=",", dtype=str)
        except FileNotFoundError:
            sys.exit(
                f"{crc_table} not found, please run 'waf build_app_embedded' "
                "command to build the project first/again, exit."
            )

        # Get the vector table and crc for the vector table
        self.crc64_vector_table = program["vector_table_crc"]
        self.data64_vector_table = program["vector_table"]

    def get_crc_and_data_by_index(self, index_of_8_bytes: int) -> tuple[int, int]:
        """Get the CRC signature and data in 64 bits from the CRC table array
        based on the index.

        Args:
            index_of_8_bytes: index of the CRC table array

        Returns:
            TBD
        """

        return int(
            self.array_csv[index_of_8_bytes + 1][
                CRC_TABLE_COLUMN_IDX["crc_64_bits_hex"]
            ],
            16,
        ), int(
            self.array_csv[index_of_8_bytes + 1][
                CRC_TABLE_COLUMN_IDX["data_64_bits_hex"]
            ],
            16,
        )
