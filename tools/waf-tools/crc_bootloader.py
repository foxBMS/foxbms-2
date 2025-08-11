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
with the Bootloader.
This includes e.g., calculating a CRC table of the application binary.
Note: As the TMS570 is a big endian MCU, all operations are implemented as
such.
"""

import hashlib
import json
import sys
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from crc64_ti_impl import generate_crc_64_for_tms570_on_board_crc_calculation


@dataclass
class FlashSector:
    """Description of the memory map of a flash memory"""

    idx: int  #: ID of the sector
    size_in_bytes: int  #: size of the sector in bytes


#: Memory map of the application on the TMS570-based foxBMS BMS-Master
APP_MEMORY_MAP = [
    FlashSector(7, 0x20000),
    FlashSector(8, 0x20000),
    FlashSector(9, 0x20000),
    FlashSector(10, 0x40000),
    FlashSector(11, 0x40000),
    FlashSector(12, 0x40000),
    FlashSector(13, 0x40000),
    FlashSector(14, 0x40000),
    FlashSector(15, 0x40000),
    FlashSector(16, 0x20000),
    FlashSector(17, 0x20000),
    FlashSector(18, 0x20000),
    FlashSector(19, 0x20000),
    FlashSector(20, 0x20000),
    FlashSector(21, 0x20000),
    FlashSector(22, 0x20000),
    FlashSector(23, 0x20000),
    FlashSector(24, 0x20000),
    FlashSector(25, 0x20000),
    FlashSector(26, 0x20000),
    FlashSector(27, 0x20000),
    FlashSector(28, 0x20000),
    FlashSector(29, 0x20000),
    FlashSector(30, 0x20000),
    FlashSector(31, 0x20000),
]

SIZE_OF_VECTOR_TABLE_IN_BYTES = 32  #: TBD


CRC_TABLE_COLUMN_IDX = {
    "data_64_bits_hex": 1,
    "crc_64_bits_hex": 2,
    "data_64_bits": 3,
    "crc_64_bits": 4,
}
NUM_OF_BYTES_IN_8_BYTES = 8


def get_sha256_file_hash(
    file_path: Path, buffer_size: int = 65536, file_hash=None
) -> "hashlib._Hash":
    """Calculate the SHA256 hash of a file"""
    if not file_hash:
        file_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        while True:
            data = f.read(buffer_size)
            if not data:
                break
            file_hash.update(data)
    return file_hash


def get_sha256_file_hash_str(file_path: Path, buffer_size: int = 65536) -> str:
    """Returns the string representation of a SHA256 hash of a file"""
    return get_sha256_file_hash(
        file_path=file_path, buffer_size=buffer_size
    ).hexdigest()


# pylint: disable-next=too-many-instance-attributes
class BootloaderBinaryFile:
    """Class to handle an application binary file and prepare the binary data
    for bootloader usage (e.g., calculate a CRC for of the application binary).
    """

    def __init__(self, app_file: Path, crc64_table: Path, info_file: Path):
        """Initialization function

        Args:
            app_file: the path of the binary file of the application
                that will be sent
            path_crc_64_table: the path of the temporary file that stores the
                CRC table
        """
        self.app_file = app_file
        self.info_file = info_file
        self.crc64_table = crc64_table

        # Parse the binary file
        byte_array_original = self._parse_binary_file()

        # Extract the vector table from the byte array and convert it into one
        # number (uint64)
        self.data64_vector_table = self.get_data64_vector_table(
            byte_array_original[:SIZE_OF_VECTOR_TABLE_IN_BYTES]
        )

        # Calculate the app size
        bin_size = self.app_file.stat().st_size
        flash_memory_size = 0
        app_size_temp = 0
        for flash_sector in APP_MEMORY_MAP:
            flash_memory_size += flash_sector.size_in_bytes
            if flash_memory_size >= bin_size:
                app_size_temp = flash_memory_size
                break
        if not app_size_temp:
            sys.exit(
                f"The binary file size {hex(bin_size)} surpasses the available "
                f"memory limit of {hex(flash_memory_size)}!"
            )
        self.app_size = app_size_temp

        # Check if the app size is valid
        if not self._check_app_size(self.app_size):
            sys.exit(
                "The app size is invalid.\n"
                "The app size must be the sum of sizes of the used sectors."
            )
        # Append the byte array with zeros to let it have the size of app_size
        byte_array_with_zeros = self.fill_byte_array_with_app_size(
            byte_array=byte_array_original[SIZE_OF_VECTOR_TABLE_IN_BYTES:],
            app_size=self.app_size,
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
        crc64 = self._create_crc64_table()

        # Save CRC and data into a csv file
        np.savetxt(self.crc64_table, crc64, delimiter=",", fmt="%s")

        # Calculate the hash for csv and json bin file
        hash_app = get_sha256_file_hash_str(self.app_file)
        hash_csv = get_sha256_file_hash_str(self.crc64_table)

        # Dump all relevant information in to a json file
        info_file.write_text(
            json.dumps(
                {
                    "len_of_program_in_bytes": self.len_of_program_in_bytes,
                    "len_of_program_in_8_bytes": self.len_of_program_in_8_bytes,
                    "app_size": self.app_size,
                    "bin_file": self.app_file.absolute().as_posix(),
                    "vector_table_crc": self.crc64_vector_table,
                    "vector_table": self.data64_vector_table,
                    "hash_app": hash_app,
                    "hash_csv": hash_csv,
                }
            )
        )

    def _parse_binary_file(self) -> bytearray:
        """Parse the binary file into a byte array.
        Returns:
            byte array of the application binary
        """
        with open(self.app_file, "rb") as file:
            bin_array_in_bytes = bytearray(file.read())
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

    def _create_crc64_table(self) -> np.ndarray:
        """Generate a CRC 64 table (as numpy.ndarray) for the application binary
        Returns:
            numpy array of the CRC table
        """

        list_csv = []
        list_name = [""] + list(self.crc_table_column_idx.keys())
        list_csv.append(list_name)
        crc_64_bits = 0
        for i_data_in_8_bytes in range(self.len_of_program_in_8_bytes):
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
        # Create a Numpy array from the list of CRC
        crc64 = np.array(list_csv)
        return crc64

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

        return byte_array
