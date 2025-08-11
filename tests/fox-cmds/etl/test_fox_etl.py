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

"""Verify the output of the 'fox etl' command."""

import sys
from json import loads
from pathlib import Path

import pandas as pd

OUTPUT_DIR = Path(__file__).parent / "out"
EXPECTED_OUTPUTS = {
    "filter-and-decode": {
        "Timestamp": 10.000,
        "0x0241_MaximumCellVoltage_mV": 2006,
        "0x0241_MinimumCellVoltage_mV": 1994,
        "0x0241_MaximumCellTemperature_degC": 26.0,
        "0x0241_MinimumCellTemperature_degC": 25.5,
    },
    "table_one_one": {
        "Date": ["2025-01-01 00:00:10.000000"],
        "0x0241_MaximumCellVoltage_mV": [2006],
        "0x0241_MinimumCellVoltage_mV": [1994],
        "0x0241_MaximumCellTemperature_degC": [26],
        "0x0241_MinimumCellTemperature_degC": [25.5],
    },
    "table_many_one": {
        "Date": ["2025-01-01 00:00:10.000000"],
        "0x0241_MaximumCellVoltage_mV": [2006],
        "0x0241_MinimumCellVoltage_mV": [1994],
        "0x0241_MaximumCellTemperature_degC": [26],
        "0x0241_MinimumCellTemperature_degC": [25.5],
        "0x0243_Voltage_V": [63.93],
        "0x0243_Current_A": [0],
        "0x0243_Power_kW": [0],
    },
    "table_many_many": {
        "f_StringMinimumMaximumValues_Mux_String0": {
            "Date": ["2025-01-01 00:00:10.000000"],
            "0x0241_MaximumCellVoltage_mV": [2006],
            "0x0241_MinimumCellVoltage_mV": [1994],
            "0x0241_MaximumCellTemperature_degC": [26],
            "0x0241_MinimumCellTemperature_degC": [25.5],
        },
        "f_StringValuesP0_Mux_String0": {
            "Date": ["2025-01-01 00:00:01.000000", "2025-01-01 00:01:01.000000"],
            "0x0243_Voltage_V": [63.93, 63.93],
            "0x0243_Current_A": [0, 0],
            "0x0243_Power_kW": [0, 0],
        },
    },
}


def main():
    """Verify the output of the 'fox etl' command."""
    output = {}
    test_specification = sys.argv[1]
    match test_specification:
        case "filter-and-decode":
            for i in OUTPUT_DIR.glob("*.json"):
                output.update(loads(i.read_text(encoding="utf-8")))
            if not output == EXPECTED_OUTPUTS[sys.argv[1]]:
                print(output, file=sys.stderr)
                print(EXPECTED_OUTPUTS[sys.argv[1]], file=sys.stderr)
                sys.exit(f"Unexpected output for testcase {sys.argv[1]}.")
        case "table_one_one":
            dfs = []
            for i in OUTPUT_DIR.glob("*.csv"):
                dfs.append(pd.read_csv(i))
            expected_df = pd.DataFrame.from_dict(EXPECTED_OUTPUTS[sys.argv[1]])
            if not expected_df.equals(dfs[0]):
                print("Expected_df: ", expected_df.to_string())
                print("Expected_df: ", expected_df.dtypes)
                print("Current_df: ", dfs[0].to_string())
                print("Current_df: ", dfs[0].dtypes)
                print(f"Differences: {expected_df.compare(dfs[0])}")
                sys.exit(f"Unexpected output for testcase {sys.argv[1]}.")
        case "table_many_one":
            dfs = []
            for i in OUTPUT_DIR.glob("*.csv"):
                dfs.append(pd.read_csv(i))
            expected_df = pd.DataFrame.from_dict(EXPECTED_OUTPUTS[sys.argv[1]])
            if not expected_df.equals(dfs[0]):
                print("Expected_df: ", expected_df.to_string())
                print("Expected_df: ", expected_df.dtypes)
                print("Current_df: ", dfs[0].to_string())
                print("Current_df: ", dfs[0].dtypes)
                print(f"Differences: {expected_df.compare(dfs[0])}")
                sys.exit(f"Unexpected output for testcase {sys.argv[1]}.")
        case "table_many_many":
            dfs = []
            for i in OUTPUT_DIR.glob("*.csv"):
                current_df = pd.read_csv(i)
                expected_df = pd.DataFrame.from_dict(
                    EXPECTED_OUTPUTS[sys.argv[1]][i.stem]
                )
                if not expected_df.equals(current_df):
                    print("Expected_df: ", expected_df.to_string())
                    print("Expected_df: ", expected_df.dtypes)
                    print("Current_df: ", current_df.to_string())
                    print("Current_df: ", current_df.dtypes)
                    print(f"Differences: {expected_df.compare(current_df)}")
                    sys.exit(f"Unexpected output for testcase {sys.argv[1]}.")
        case _:
            print("No default case defined")
            sys.exit(1)


if __name__ == "__main__":
    main()
