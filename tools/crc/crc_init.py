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

"""Generate CRC lookup tables for usage in foxBMS"""

import argparse
import logging
import sys

LINE_LENGTH = 120


def get_hex_rep(table):
    """Generate nice printable hex representation of the lookup table"""
    max_str_len = len(hex(max(table)))
    hex_table = []
    for i in table:
        hex_rep = f"{i:#0{max_str_len}X}u"
        hex_rep = hex_rep[:2].lower() + hex_rep[2:]
        hex_table.append(hex_rep)
    return hex_table


def generate_c_table(table, crc_len):
    """Generate a CRC table as the foxBMS C style guide requires"""
    lines = [
        f"/* precomputed CRC-{crc_len} Table */",
        f"static const unsigned int crc{crc_len}Table[{len(table)}] = {{",
        "    ",
    ]
    index = len(lines) - 1
    for i in get_hex_rep(table):
        if len(lines[index] + f"{i},") < LINE_LENGTH + 1:
            lines[index] += f"{i}, "
        else:
            index += 1
            lines.append(f"    {i}, ")
    lines.append("};")
    print("\n".join(lines))


def precalculate_crc_table(polynomial, width):
    """Generate a CRC lookup table based on the polynomial"""
    mask = 1 << (width - 1)
    table = []
    for i in range(256):
        remainder = i << (width - 8)
        for _ in range(8):
            if remainder & mask:
                remainder = (remainder << 1) ^ polynomial
            else:
                remainder <<= 1
        remainder = remainder & 0xFFFF
        table.append(remainder)
    return table


def check_positive_integer(value):
    """Check that the provided value is castable to int"""
    try:
        value = int(value)
    except ValueError:
        sys.exit("Width must be an integer.")
    if value <= 0:
        sys.exit("Width must be a positive integer.")
    return value


def check_hex(value):
    """Check that the provided value is a hex representation"""
    if not value.lower().startswith("0x"):
        sys.exit("Polynomial must be provided as hex representation.")
    return value


def main():
    """This script does this and that"""
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
        "-p",
        "--polynomial",
        dest="polynomial",
        action="store",
        type=check_hex,
        default="0xC599",
        help="CRC polynomial",
    )

    parser.add_argument(
        "-w",
        "--width",
        dest="width",
        action="store",
        type=check_positive_integer,
        default=15,
        help="CRC width",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    polynomial = int(args.polynomial, 16)
    width = int(args.width)
    logging.debug(f"polynomial: {polynomial:#0x}")
    logging.debug(f"width:      {width}")
    table = precalculate_crc_table(polynomial, width)
    logging.debug("C code:")
    generate_c_table(table, width)


if __name__ == "__main__":
    main()
