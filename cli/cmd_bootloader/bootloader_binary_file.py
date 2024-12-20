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


"""Implementation for handling and preparing an application binary for usage
with the Bootloader.
This includes e.g., calculating a CRC table of the application binary.
Note: As the TMS570 is a big endian MCU, all operations are implemented as
such.
"""

import json
import logging
import sys
from pathlib import Path
from typing import Optional, Tuple

import numpy as np

from ..helpers.misc import FOXBMS_BIN_FILE, PROJECT_BUILD_ROOT, get_sha256_file_hash_str
from .app_constants import APP_MEMORY_MAP, APP_SIZE, SIZE_OF_VECTOR_TABLE_IN_BYTES
from .crc64_ti_impl import generate_crc_64_for_tms570_on_board_crc_calculation

CRC_TABLE_COLUMN_IDX = {
    "data_64_bits_hex": 1,
    "crc_64_bits_hex": 2,
    "data_64_bits": 3,
    "crc_64_bits": 4,
}
PATH_CRC_64_TABLE = PROJECT_BUILD_ROOT / "bootloader_crc_64_bits.csv"

NUM_OF_BYTES_IN_8_BYTES = 8


# pylint: disable-next=too-many-instance-attributes
class BootloaderBinaryFile:
    """Class to handle an application binary file and prepare the binary data
    for bootloader usage (e.g., calculate a CRC for of the application binary).
    """

    def __init__(
        self,
        path_app_binary: Path = FOXBMS_BIN_FILE,
        app_size: Optional[int] = APP_SIZE,
        path_crc_64_table: Optional[Path] = PATH_CRC_64_TABLE,
    ):
        """Initialization function

        Args:
            path_app_binary: the path of the binary file of the application
                that will be sent
            app_size: the size of the binary file of the application which
                should be the sum of the sector sizes
            path_crc_64_table: the path of the temporary file that stores the
                CRC table
        """

        # Input sanitation
        if not path_app_binary.is_file():
            sys.exit(f"Binary '{path_app_binary}' does not exist.")
        self.app_file = path_app_binary

        if not path_crc_64_table:
            path_crc_64_table = PATH_CRC_64_TABLE
        self.crc64_table_file = path_crc_64_table
        self.app_hash_file = self.crc64_table_file.with_suffix(".json")

        # Parse the binary file
        byte_array_original = self._parse_binary_file()

        # Extract the vector table from the byte array and convert it into one
        # number (uint64)
        self.data64_vector_table = self.get_data64_vector_table(
            byte_array_original[:SIZE_OF_VECTOR_TABLE_IN_BYTES]
        )

        # Use default app_size
        if not app_size:
            app_size = APP_SIZE

        # Check if the app size is valid
        if not self._check_app_size(app_size):
            sys.exit(
                "The app size is invalid.\n"
                "The app size must be the sum of sizes of the used sectors."
            )

        # Append the byte array with zeros to let it have the size of app_size
        byte_array_with_zeros = self.fill_byte_array_with_app_size(
            byte_array=byte_array_original[SIZE_OF_VECTOR_TABLE_IN_BYTES:],
            app_size=app_size,
        )

        # Calculate the CRC (uint64) for vector table
        self.crc64_vector_table = self._calculate_crc64_for_vector_table()

        # Extract the byte_array for app (without the part for vector table)
        self.byte_array = byte_array_with_zeros

        # Length of program
        self.len_of_program_in_bytes = len(self.byte_array)
        self.len_of_program_in_8_bytes = int(
            self.len_of_program_in_bytes / NUM_OF_BYTES_IN_8_BYTES
        )

        # Dictionary that specifies the column names of the generated CRC table
        self.crc_table_column_idx = CRC_TABLE_COLUMN_IDX

        # Check if there is csv files that stores the CRC table
        if self._is_crc64_table_valid():
            self.array_csv = np.genfromtxt(
                self.crc64_table_file, delimiter=",", dtype=str
            )
        else:
            self.array_csv = self._create_crc64_table()

    def _parse_binary_file(self) -> bytearray:
        """Parse the binary file into a byte array.
        Returns:
            byte array of the application binary
        """
        with open(self.app_file, "rb") as file:
            bin_array_in_bytes = bytearray(file.read())
            if (len(bin_array_in_bytes) % NUM_OF_BYTES_IN_8_BYTES) != 0:
                num_of_zero_bytes_to_fill = NUM_OF_BYTES_IN_8_BYTES - (
                    len(bin_array_in_bytes) % NUM_OF_BYTES_IN_8_BYTES
                )
                for _ in range(num_of_zero_bytes_to_fill):
                    bin_array_in_bytes.append(0)
            return bin_array_in_bytes

    def _calculate_crc64_for_vector_table(self) -> int:
        """Calculates the 64-bits CRC signature of the vector table.

        Returns:
            calculated 64-bit CRC signature
        """
        crc_64_bits = 0
        for i_vector_table_part in range(4):
            crc_64_bits = generate_crc_64_for_tms570_on_board_crc_calculation(
                data_64_bits=self.data64_vector_table[i_vector_table_part],
                crc_64_bits=crc_64_bits,
            )
        return crc_64_bits

    def _is_crc64_table_valid(self) -> bool:
        """This function checks if the csv file contains the CRC table for
        this binary file.

        Returns:
            True if the CRC table is valid, False otherwise
        """
        if not self.crc64_table_file.is_file():
            logging.info(
                "'%s' does not exist, need to run CRC table calculation.",
                self.crc64_table_file,
            )
            return False
        if not self.app_hash_file.is_file():
            logging.info(
                "'%s' does not exist, need to run CRC table calculation.",
                self.app_hash_file,
            )
            return False

        try:
            old_hash_obj = json.loads(self.app_hash_file.read_text(encoding="utf-8"))
        except json.decoder.JSONDecodeError:
            logging.info(
                "'%s' is invalid, need to run CRC table calculation.",
                self.app_hash_file,
            )
            return False

        try:
            old_hash = old_hash_obj[self.app_file.as_posix()]
        except KeyError:
            logging.info(
                "Key '%s' does not exist in '%s', need to run CRC table calculation.",
                self.app_file.as_posix(),
                self.app_hash_file,
            )
            return False

        current_hash = get_sha256_file_hash_str(self.app_file)

        if current_hash != old_hash:
            logging.info(
                "App binary '%s' changed, need to run CRC table calculation.",
                self.app_file,
            )
            return False

        logging.info("'%s' is valid, loading CRC table from it", self.crc64_table_file)
        return True

    def _create_crc64_table(self) -> np.ndarray:
        """Generate a CRC 64 table (as numpy.ndarray) for the application binary
        Returns:
            numpy array of the CRC table
        """
        logging.debug("Generating CRC table at '%s'", self.crc64_table_file)
        current_hash = get_sha256_file_hash_str(self.app_file)
        with open(self.app_hash_file, "w", encoding="utf-8") as f:
            f.write(json.dumps({self.app_file.as_posix(): current_hash}, indent=2))

        # Calculate the list_crc_64_bits
        logging.info(
            "Calculating CRC table for '%s' (takes some time...)", self.app_file
        )

        list_csv = []
        list_name = [""] + list(self.crc_table_column_idx.keys())
        list_csv.append(list_name)
        crc_64_bits = 0
        for i_data_in_8_bytes in range(self.len_of_program_in_8_bytes):
            if i_data_in_8_bytes % 1000 == 0:
                logging.info(
                    "CRC table calculation: %.0f%%",
                    int((i_data_in_8_bytes / self.len_of_program_in_8_bytes) * 100),
                )
            data_64_bits = int.from_bytes(
                self.byte_array[
                    i_data_in_8_bytes * NUM_OF_BYTES_IN_8_BYTES : (
                        i_data_in_8_bytes + 1
                    )
                    * NUM_OF_BYTES_IN_8_BYTES
                ],
                "big",
            )
            crc_64_bits = generate_crc_64_for_tms570_on_board_crc_calculation(
                data_64_bits=data_64_bits, crc_64_bits=crc_64_bits
            )
            list_csv.append(
                [
                    f"{i_data_in_8_bytes}",
                    f"{hex(data_64_bits)}",
                    f"{hex(crc_64_bits)}",
                    f"{data_64_bits}",
                    f"{crc_64_bits}",
                ]
            )
        logging.info("CRC table calculation: 100%")

        # Create a Numpy array from the list of CRC
        crc64 = np.array(list_csv)
        np.savetxt(self.crc64_table_file, crc64, delimiter=",", fmt="%s")

        return crc64

    def get_crc_and_data_by_index(self, index_of_8_bytes: int) -> Tuple[int, int]:
        """Get the CRC signature and data in 64 bits from the CRC table array
        based on the index.

        Args:
            index_of_8_bytes: index of the CRC table array

        Returns:
            TBD
        """

        # Check if the data is the same as current byte array
        if int(self.array_csv[index_of_8_bytes + 1][1], 16) != int(
            self.byte_array[
                index_of_8_bytes * NUM_OF_BYTES_IN_8_BYTES : (index_of_8_bytes + 1)
                * NUM_OF_BYTES_IN_8_BYTES
            ].hex(),
            16,
        ):
            sys.exit(
                "The data in byte_array does not match the one in array_csv, "
                "the CRC table might be outdated."
            )

        return int(
            self.array_csv[index_of_8_bytes + 1][
                self.crc_table_column_idx["crc_64_bits_hex"]
            ],
            16,
        ), int(
            self.array_csv[index_of_8_bytes + 1][
                self.crc_table_column_idx["data_64_bits_hex"]
            ],
            16,
        )

    @staticmethod
    def _check_app_size(app_size_input: int) -> bool:
        """Check if the size of app is valid.

        In the C code part of the bootloader, during data transmission,
        the CRC calculation will be executed for each sector (using the
        result of the previous CRC calculation as the initial value).
        The final CRC calculation result will be stored in the flash of the
        MCU.
        Before running the application, a new CRC calculation will be
        performed on the entire size of the application memory, and the result
        will be compared with the stored CRC signature from the data
        transmission.
        The application will only run if the calculated CRCs match.
        Due to this validation process, the size of the application needs to be
        the sum of the sector sizes.

        Args:
            app_size_input: the size of application that needs to be checked.
        Returns:
            True if the app size is valid, False otherwise.
        """
        app_size = 0
        for flash_sector in APP_MEMORY_MAP:
            app_size += flash_sector.size_in_bytes
            if app_size == app_size_input:
                return True
        return False

    @staticmethod
    def get_data64_vector_table(byte_array_vector_table: bytearray) -> list[int]:
        """Convert byte array of vector table to list of integer.

        Args:
            byte_array_vector_table: byte array of vector table

        Returns:
            list of integers
        """
        data64_vector_table = []
        for i in range(4):
            data64 = int.from_bytes(byte_array_vector_table[i * 8 : (i + 1) * 8], "big")
            data64_vector_table.append(data64)
        return data64_vector_table

    @staticmethod
    def fill_byte_array_with_app_size(
        byte_array: bytearray, app_size: int
    ) -> bytearray:
        """Fill a byte array to a certain size.

        Args:
            byte_array: byte array to be filled
            app_size: size of the app

        Returns:
            new byte array with the given size
        """
        while len(byte_array) < app_size:
            byte_array.append(0)

        if len(byte_array) > app_size:
            byte_array = byte_array[:app_size]

        return byte_array
