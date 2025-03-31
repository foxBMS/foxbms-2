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

"""This file contains functions to extract enum from dbc file and print them."""

import sys
from enum import Enum
from pathlib import Path
from typing import cast

import cantools


def print_enum(enum_variable):
    """Print an enum"""
    # Print the enum class name
    print(
        "\nclass " + enum_variable.__name__ + "(Enum):",
    )
    for member in enum_variable:
        print(f"\t{member.name} = {member.value}")


def extract_enum_from_dbc_file(enum_name: str, dbc_file: Path) -> Enum:
    """Extract enum from the dbc file.

    Args:
        dbc_file: .dbc file where the definition of CAN messages are saved.
        enum_name: the name of the enum.

    Returns:
        enum with the name of the enum.
    """

    # Load the DBC file
    if not dbc_file.is_file():
        sys.exit(f"The input '{dbc_file}' is not a file.")

    db = cantools.database.load_file(dbc_file)
    db = cast(cantools.database.can.database.Database, db)
    # Loop through all messages to find the right one with right signal name
    for message in db.messages:
        for signal in message.signals:
            if signal.name == enum_name:
                if not signal.choices:
                    sys.exit(f"There is no enum for signal '{enum_name}'.")
                choices = {str(value): key for key, value in signal.choices.items()}
                retval = Enum(signal.name, choices)  #
                print_enum(retval)
                return retval

    # If no such message available
    sys.exit(f"Cannot find signal '{enum_name}' in the dbc file.")


if __name__ == "__main__":
    pass
    # Some examples:
    # dbc_path = ? # Enter a path of dbc file
    # YesNoAnswer = extract_enum_from_dbc_file('YesNoAnswer',dbc_path)
    # AcknowledgeFlag = extract_enum_from_dbc_file("AcknowledgeFlag",dbc_path)
    # AcknowledgeMessage = extract_enum_from_dbc_file("AcknowledgeMessage",dbc_path)
    # StatusCode = extract_enum_from_dbc_file("StatusCode",dbc_path)
    # RequestCode8Bits = extract_enum_from_dbc_file("RequestCode8Bits",dbc_path)
    # CanFsmState = extract_enum_from_dbc_file("CanFsmState",dbc_path)
    # BootFsmState = extract_enum_from_dbc_file("BootFsmState",dbc_path)
