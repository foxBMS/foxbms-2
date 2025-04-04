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

"""Template for Python scripts"""

import argparse
import logging
import math
from pathlib import Path

SCRIPT_PATH = Path(__file__).parent.resolve()


def sym_hex(number):
    """fix hex number output for sym files"""
    out = hex(number)[2:]
    try:
        out_as_hex = "0x" + out.upper()
        if int(out_as_hex, 0) > 9:
            out = (out.upper() + "h").zfill(3)
    except ValueError:
        pass
    return out


def main():
    """This script produces a symbol file for cell voltages and temperatures"""
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
        "-c",
        dest="number_of_cells",
        action="store",
        default=54,
        help="Number of cells",
    )
    parser.add_argument(
        "-t",
        dest="number_of_cell_temperatures",
        action="store",
        default=30,
        help="Number of cell temperatures",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    logging.debug(args)

    invalid = "-m /e:f_ValidInvalidFlag"
    # cell voltages
    all_symbols = SCRIPT_PATH / "combined_symbol_file.txt"
    all_symbols.write_text(
        'FormatVersion=6.0 // Do not edit this line!\nTitle="foxBMS"\n\n\n{SENDRECEIVE}\n\n'
    )
    cell_voltages = SCRIPT_PATH / "multiplexed_cell_voltages_for_sym_file.txt"
    volts = []
    name = "CellVoltage"
    cycle_time_ms = str(math.ceil(args.number_of_cells / 4) * 100)
    for i in range(0, args.number_of_cells):
        volts.append("[f_CellVoltages]")
        if not i:
            volts.append(
                "ID=250h // (in:can_cbs_tx_cell-voltages.c:CANTX_CellVoltages, fv:tx)"
            )
        # pylint: disable=line-too-long
        # fmt: off
        volts.append("Len=8")
        volts.append(f"CycleTime={cycle_time_ms} -p")
        volts.append(f"Mux={name}s_{(i * 4):03d}_{((i * 4) + 3):03d} 0,8 {sym_hex(i)} -m")
        volts.append(f"Var={name}_{((i * 4) + 0):03d}_invalidFlag bit 11,1 {invalid}")
        volts.append(f"Var={name}_{((i * 4) + 1):03d}_invalidFlag bit 10,1 {invalid}")
        volts.append(f"Var={name}_{((i * 4) + 2):03d}_invalidFlag bit 9,1 {invalid}")
        volts.append(f"Var={name}_{((i * 4) + 3):03d}_invalidFlag bit 8,1 {invalid}")
        volts.append(f"Var={name}_{((i * 4) + 0):03d} unsigned 12,13 -m /u:mV // Voltage of cell {(i * 4) + 0}")
        volts.append(f"Var={name}_{((i * 4) + 1):03d} unsigned 25,13 -m /u:mV // Voltage of cell {(i * 4) + 1}")
        volts.append(f"Var={name}_{((i * 4) + 2):03d} unsigned 38,13 -m /u:mV // Voltage of cell {(i * 4) + 2}")
        volts.append(f"Var={name}_{((i * 4) + 3):03d} unsigned 51,13 -m /u:mV // Voltage of cell {(i * 4) + 3}")
        volts.append("")
        # fmt: on
        # pylint: enable=line-too-long
    cell_voltages.write_text("\n".join(volts))
    with open(all_symbols, "a+", encoding="utf-8") as f:
        f.write("\n".join(volts))

    # cell temperatures
    cell_temperatures = SCRIPT_PATH / "multiplexed_cell_temperatures_for_sym_file.txt"
    temps = []
    name = "CellTemperature"
    cycle_time_ms = str(math.ceil(args.number_of_cell_temperatures / 6) * 100)
    for i in range(0, args.number_of_cell_temperatures):
        temps.append("[f_CellTemperatures]")
        if not i:
            temps.append(
                "ID=260h // (in:can_cbs_tx_cell-temperatures.c:CANTX_CellTemperatures, fv:tx)"
            )
        # pylint: disable=line-too-long
        # fmt: off
        temps.append("Len=8")
        temps.append(f"CycleTime={cycle_time_ms} -p")
        temps.append(f"Mux={name}s_{(i * 6):03d}_{((i * 6) + 5):03d} 0,8 {sym_hex(i)} -m")
        temps.append(f"Var={name}_{((i * 6) + 0):03d}_invalidFlag bit 15,1 {invalid}")
        temps.append(f"Var={name}_{((i * 6) + 1):03d}_invalidFlag bit 14,1 {invalid}")
        temps.append(f"Var={name}_{((i * 6) + 2):03d}_invalidFlag bit 13,1 {invalid}")
        temps.append(f"Var={name}_{((i * 6) + 3):03d}_invalidFlag bit 12,1 {invalid}")
        temps.append(f"Var={name}_{((i * 6) + 4):03d}_invalidFlag bit 11,1 {invalid}")
        temps.append(f"Var={name}_{((i * 6) + 5):03d}_invalidFlag bit 10,1 {invalid}")
        temps.append(f"Var={name}_{((i * 6) + 0):03d} signed 16,8 -m /u:degC // Temperature of cell {(i * 6) + 0}")
        temps.append(f"Var={name}_{((i * 6) + 1):03d} signed 24,8 -m /u:degC // Temperature of cell {(i * 6) + 1}")
        temps.append(f"Var={name}_{((i * 6) + 2):03d} signed 32,8 -m /u:degC // Temperature of cell {(i * 6) + 2}")
        temps.append(f"Var={name}_{((i * 6) + 3):03d} signed 40,8 -m /u:degC // Temperature of cell {(i * 6) + 3}")
        temps.append(f"Var={name}_{((i * 6) + 4):03d} signed 48,8 -m /u:degC // Temperature of cell {(i * 6) + 4}")
        temps.append(f"Var={name}_{((i * 6) + 5):03d} signed 56,8 -m /u:degC // Temperature of cell {(i * 6) + 5}")
        temps.append("")
        # fmt: on
        # pylint: enable=line-too-long
    cell_temperatures.write_text("\n".join(temps))
    with open(all_symbols, "a+", encoding="utf-8") as f:
        f.write("\n".join(temps))


if __name__ == "__main__":
    main()
