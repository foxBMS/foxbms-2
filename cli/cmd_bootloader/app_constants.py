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

"""Defines MCU and binary specific constants. These values shall be aligned
with the embedded counterpart implementation of the bootloader."""

from dataclasses import dataclass


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

NUM_OF_BYTES_PER_DATA_LOOPS = 8  #: TBD
SIZE_OF_SUB_SECTOR_IN_LOOPS = 1024  #: TBD
SIZE_OF_VECTOR_TABLE_IN_BYTES = 32  #: TBD
