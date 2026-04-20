#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Verify the output of the 'fox plot' command."""

import sys
from pathlib import Path

import numpy as np
from PIL import Image

BASE_PATH = "tests/fox-cmds/plot"
OUT = f"{BASE_PATH}/plot_example_data"


def _check(reference: np.ndarray, comparison: np.ndarray, test: str) -> None:
    if not np.array_equal(reference, comparison):
        sys.exit(f"Unexpected output for testcase {test}.")


TEST_CASES = {
    "example_from_docs": {
        "reference": f"{BASE_PATH}/example_from_docs_reference.png",
        "comparison": f"{OUT}/cell_voltage_current.png",
    },
    "font": {
        "reference": f"{BASE_PATH}/font_reference.png",
        "comparison": f"{OUT}/font.png",
    },
    "minimal": {
        "reference": f"{BASE_PATH}/minimal_reference.png",
        "comparison": f"{OUT}/minimal.png",
    },
    "datetime": {
        "reference": f"{BASE_PATH}/datetime_reference.png",
        "comparison": f"{OUT}/datetime.png",
    },
    "two_plots-png": {
        "reference": f"{BASE_PATH}/two_plots_reference.png",
        "comparison": f"{OUT}/two_plots.png",
    },
    "two_plots-jpg": {
        "reference": f"{BASE_PATH}/two_plots_reference.jpg",
        "comparison": f"{OUT}/two_plots.jpg",
    },
    "three_y": {
        "reference": f"{BASE_PATH}/three_y_reference.png",
        "comparison": f"{OUT}/three_y.png",
    },
    "package": {
        "reference": f"{Path(__file__).parents[3] / BASE_PATH}/package_reference.png",
        "comparison": f"{Path(__file__).parents[3] / OUT}/package.png",
    },
}


def main():
    """Verify the Output of the 'fox plot' command"""
    try:
        test = sys.argv[1]
    except IndexError:
        sys.exit("No test case provided.")
    test_config = TEST_CASES.get(test, "")
    if not test_config:
        sys.exit("Unknown test case provided.")

    reference = np.array(Image.open(test_config["reference"]))
    comparison = np.array(Image.open(test_config["comparison"]))
    _check(reference, comparison, test)


if __name__ == "__main__":
    main()
