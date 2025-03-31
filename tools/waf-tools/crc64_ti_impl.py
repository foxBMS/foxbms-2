#!/usr/bin/env python3

"""Implementation of the CRC64 Polynomial
x\\ :sup:`64` + x\\ :sup:`4` + x\\ :sup:`3` + x + 1

See the document (available at https://www.ti.com/lit/an/spna235/spna235.pdf)

.. code-block:: text

   Texas Instruments
   Application Report
   SPNA235-August 2016
   Using the CRC Module on Hercules™-Based Microcontrollers

and the accompanying linked software examples (available at
http://www.ti.com/lit/zip/spna235).

The CRC function 'generate_crc_64_for_tms570_on_board_crc_calculation' is
implemented in Python based on the example code from 'CRC64_calc.c'
(which is part of http://www.ti.com/lit/zip/spna235).

'CRC64_calc.c' is released under the following license:

.. code-block:: c

   /*
    * CRC64_calc.c
    *
    * Function to calculate CRC64 in software.
    *
    * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
    *
    *
    *  Redistribution and use in source and binary forms, with or without
    *  modification, are permitted provided that the following conditions
    *  are met:
    *
    *    Redistributions of source code must retain the above copyright
    *    notice, this list of conditions and the following disclaimer.
    *
    *    Redistributions in binary form must reproduce the above copyright
    *    notice, this list of conditions and the following disclaimer in the
    *    documentation and/or other materials provided with the
    *    distribution.
    *
    *    Neither the name of Texas Instruments Incorporated nor the names of
    *    its contributors may be used to endorse or promote products derived
    *    from this software without specific prior written permission.
    *
    *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    *
   */

"""


def generate_crc_64_for_tms570_on_board_crc_calculation(
    data_64_bits: int, crc_64_bits: int = 0
) -> int:
    """Python-port of the TI CRC64 implementation
    x\\ :sup:`64` + x\\ :sup:`4` + x\\ :sup:`3` + x + 1
    test_data_1 = 0x1122334455667788, crc_1 = 12129596691791633497
    test_data_2 = 0x99AABBCCDDEEFF00, crc_2 = 13985017789367110321
    test_data_3 = 0x123456789ABCDEF0 repeat 1024 times, crc_3 = 11957925961466737347

    Args:
        data_64_bits: 64-bit data that works as input data for crc calculation.
        crc_64_bits: 64-bit crc signature that works as the root for crc
            calculation.

    Returns:
        generated CRC signature
    """
    # Because of big endian, the lower 32-bit data/crc will be swapped with the
    # higher 32-bit data/crc
    crc_64_bits = (crc_64_bits >> 32) | ((crc_64_bits & 0xFFFFFFFF) << 32)
    data_64_bits = (data_64_bits >> 32) | ((data_64_bits & 0xFFFFFFFF) << 32)

    mask_bits = 0xFFFFFFFFFFFFFFFF ^ ((1 << 0) | (1 << 1) | (1 << 3) | (1 << 4))

    # Do crc calculation
    next_crc = crc_64_bits
    for i in range(63, -1, -1):
        # Shift the whole crc without processing it one step left
        body_next_crc = next_crc << 1

        # Mask it with 0, 1, 3, 4
        mask_next_crc = body_next_crc & mask_bits

        # The first bit of th new crc
        crc_bit_start = (next_crc >> 63) ^ ((data_64_bits >> i) & 1)

        # Calculate the bit to be filled in 1, 3, 4
        bit_one = crc_bit_start ^ ((next_crc >> 0) & 1)
        bit_three = crc_bit_start ^ ((next_crc >> 2) & 1)
        bit_four = crc_bit_start ^ ((next_crc >> 3) & 1)

        # Process it by xor the corresponding bits
        next_crc = mask_next_crc | (
            (crc_bit_start << 0) | (bit_one << 1) | (bit_three << 3) | (bit_four << 4)
        )

    # Swap the crc_64_bits back
    crc_64_bits = (next_crc >> 32) | ((next_crc & 0xFFFFFFFF) << 32)

    return crc_64_bits
