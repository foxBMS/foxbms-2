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

"""Template for Python scripts"""

import logging
import argparse


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
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    logging.debug(args)

    with open("multiplexed_cellVoltages_for_sym_file.txt", "w") as f:
        # create .sym file messages for 54 * 4 cell voltages
        for i in range(0, 54):
            f.write("[foxBMS_CellVoltage]\n")
            if i == 0:
                f.write("ID=240h\n")
            f.write("DLC=8\n")
            f.write(
                "Mux=mux_cellVoltage_"
                + str(i * 4)
                + "_"
                + str((i * 4) + 3)
                + " 0,8 "
                + hex(i)[2:].upper()
                + "h  -m\n"
            )
            f.write("Var=cellVoltage_" + str(i * 4) + "_invalidFlag unsigned 11,1 -m\n")
            f.write(
                "Var=cellVoltage_"
                + str((i * 4) + 1)
                + "_invalidFlag unsigned 10,1 -m\n"
            )
            f.write(
                "Var=cellVoltage_" + str((i * 4) + 2) + "_invalidFlag unsigned 9,1 -m\n"
            )
            f.write(
                "Var=cellVoltage_" + str((i * 4) + 3) + "_invalidFlag unsigned 8,1 -m\n"
            )
            f.write("Var=cellVoltage_" + str(i * 4) + " unsigned 12,13 -m /u:mV\n")
            f.write(
                "Var=cell_voltage_" + str((i * 4) + 1) + " unsigned 25,13 -m /u:mV\n"
            )
            f.write(
                "Var=cell_voltage_" + str((i * 4) + 2) + " unsigned 38,13 -m /u:mV\n"
            )
            f.write(
                "Var=cell_voltage_" + str((i * 4) + 3) + " unsigned 51,13 -m /u:mV\n"
            )
            f.write("\n")

    with open("multiplexed_cellTemperatures_for_sym_file.txt", "w") as f:
        # create .sym file messages for 30 * 6 cell temperatures
        for i in range(0, 30):
            f.write("[foxBMS_CellTemperature]\n")
            if i == 0:
                f.write("ID=250h\n")
            f.write("DLC=8\n")
            f.write(
                "Mux=mux_cellTemperature_"
                + str(i * 6)
                + "_"
                + str((i * 6) + 5)
                + " 0,8 "
                + hex(i)[2:].upper()
                + "h  -m\n"
            )
            f.write(
                "Var=cellTemperature_" + str(i * 6) + "_invalidFlag unsigned 15,1 -m\n"
            )
            f.write(
                "Var=cellTemperature_"
                + str((i * 6) + 1)
                + "_invalidFlag unsigned 14,1 -m\n"
            )
            f.write(
                "Var=cellTemperature_"
                + str((i * 6) + 2)
                + "_invalidFlag unsigned 13,1 -m\n"
            )
            f.write(
                "Var=cellTemperature_"
                + str((i * 6) + 3)
                + "_invalidFlag unsigned 12,1 -m\n"
            )
            f.write(
                "Var=cellTemperature_"
                + str((i * 6) + 4)
                + "_invalidFlag unsigned 11,1 -m\n"
            )
            f.write(
                "Var=cellTemperature_"
                + str((i * 6) + 5)
                + "_invalidFlag unsigned 10,1 -m\n"
            )
            f.write("Var=cellTemperature_" + str(i * 6) + " signed 16,8 -m /u:degC\n")
            f.write(
                "Var=cellTemperature_" + str((i * 6) + 1) + " signed 24,8 -m /u:degC\n"
            )
            f.write(
                "Var=cellTemperature_" + str((i * 6) + 2) + " signed 32,8 -m /u:degC\n"
            )
            f.write(
                "Var=cellTemperature_" + str((i * 6) + 3) + " signed 40,8 -m /u:degC\n"
            )
            f.write(
                "Var=cellTemperature_" + str((i * 6) + 4) + " signed 48,8 -m /u:degC\n"
            )
            f.write(
                "Var=cellTemperature_" + str((i * 6) + 5) + " signed 56,8 -m /u:degC\n"
            )
            f.write("\n")


if __name__ == "__main__":
    main()
