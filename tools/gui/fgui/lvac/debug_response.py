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

"""Decoding and displaying of the 'DebugResponse' message"""

month_to_number = {
    "January": 1,
    "February": 2,
    "March": 3,
    "April": 4,
    "May": 5,
    "June": 6,
    "July": 7,
    "August": 8,
    "September": 9,
    "October": 10,
    "November": 11,
    "December": 12,
}


def decode_debug_response(value, dbc):
    "print debug information to stdout"
    msg = dbc.decode_message(value.arbitration_id, value.data)
    mux_value = msg["foxBMS_DebugResponse_Mux"]
    if mux_value == "foxBMS_RtcTime":
        hundredth_of_seconds = str(msg["GetHundredthOfSeconds"]).rjust(3, "0")
        seconds = str(msg["GetSeconds"]).rjust(2, "0")
        minutes = str(msg["GetMinutes"]).rjust(2, "0")
        hours = str(msg["GetHours"]).rjust(2, "0")
        day = str(msg["GetDay"]).rjust(2, "0")
        month = str(month_to_number[str(msg["GetMonth"])]).rjust(2, "0")
        year = str(msg["GetYear"]).rjust(2, "0")
        nice_msg = f"BMS time: {year}-{month}-{day}"
        nice_msg += f"T{hours}:{minutes}:{seconds},{hundredth_of_seconds}"
    elif mux_value == "foxBMS_BmsSoftwareVersionInfo":
        dirty = bool(msg["foxBMS_dirtyFlag"])
        major = str(msg["foxBMS_majorVersionNumber"])
        minor = str(msg["foxBMS_minorVersionNumber"])
        patch = str(msg["foxBMS_patchVersionNumber"])
        release_distance = str(msg["foxBMS_releaseDistance"])
        release_distance_overflow = bool(msg["foxBMS_releaseDistanceOverflow"])
        under_version_control = bool(msg["foxBMS_underVersionControl"])
        nice_msg = f"BMS Software version info: version: v{major}.{minor}.{patch}\n"
        nice_msg += f"BMS Software version info: using vcs: {under_version_control}\n"
        nice_msg += f"BMS Software version info: is dirty: {dirty}\n"
        nice_msg += "BMS Software version info: release distance: "
        nice_msg += f"{release_distance}\n"
        nice_msg += "BMS Software version info: release distance overflow: "
        nice_msg += f"{release_distance_overflow}"
    elif mux_value == "foxBMS_BootInformation":
        boot_data = msg["foxBMS_MagicBootData"]
        boot_data_bin = bin(boot_data)[2:].rjust(56, "0")
        if boot_data == 0xFEFEFEFEFEFEFE:
            nice_msg = f"------> BMS Boot message (start marker {boot_data_bin})"
        elif boot_data == 0x01010101010101:
            nice_msg = f"------> BMS Boot message (end marker {boot_data_bin})"
        else:
            nice_msg = f"Unexpected startup data '{boot_data_bin}'"
    elif mux_value == "foxBMS_McuUniqueDieId":
        nice_msg = f"MCU die ID: {msg['MCU_uniqueId']}"
    elif mux_value == "foxBMS_McuLotNumber":
        nice_msg = f"MCU lot number: {msg['MCU_LotNumber']}"
    elif mux_value == "foxBMS_McuWaferInformation":
        nice_msg = f"MCU wafer information: {msg['MCU_waferNumber']} "
        nice_msg += f"(x={msg['MCU_xWaferCoordinate']}, "
        nice_msg += f"y={msg['MCU_yWaferCoordinate']}) "
    else:
        nice_msg = msg
    print(nice_msg)
