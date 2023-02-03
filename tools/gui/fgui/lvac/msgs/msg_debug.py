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

"""Implementation of the 'foxBMS_Debug' message"""

import datetime
import logging

import can
from cantools.database.can import Database, Message


class DebugMessageState:  # pylint: disable=too-few-public-methods
    """State of the Debug message, i.e., how many debug messages have been
    received and what is their handling state."""

    def __init__(self):
        self.number_of_requests: int = 0
        self.set_rtc_time: int = 0
        self.get_rtc_time: int = 0
        self.get_software_version: int = 0
        self.fram_initialization: int = 0
        self.software_reset: int = 0


class DebugMessage:
    """Implements the encoding of the 'DebugMessage' and the processing wrapper
    for the GUI frame."""

    def __init__(self, dbc: Database) -> None:
        self.message: Message = dbc.get_message_by_name("foxBMS_Debug")
        self.mux_name = "foxBMS_Debug_Mux"
        self.mux_values = {
            "foxBMS_VersionInfo": 0x00,
            "foxBMS_Rtc": 0x01,
            "foxBMS_SoftwareReset": 0x02,
            "foxBMS_FramInitialization": 0x03,
            "foxBMS_TimeInfo": 0x04,
        }

    def trigger_fram_initialization(self) -> can.Message:
        "Process FRAM initialization trigger message"
        msg_data = {
            self.mux_name: self.mux_values["foxBMS_FramInitialization"],
            "InitializeFram": 1,
        }
        data = self.message.encode(msg_data, padding=True)
        logging.debug(msg_data)
        return can.Message(
            arbitration_id=self.message.frame_id, data=data, is_extended_id=False
        )

    def trigger_software_reset(self) -> can.Message:
        "Process Software reset message"
        msg_data = {
            self.mux_name: self.mux_values["foxBMS_SoftwareReset"],
            "foxBMS_TriggerSoftwareReset": 1,
        }
        data = self.message.encode(msg_data, padding=True)
        logging.debug(msg_data)
        return can.Message(
            arbitration_id=self.message.frame_id, data=data, is_extended_id=False
        )

    def get_version_info_message(self, _type: str = "all") -> can.Message:
        "Process software version information message"
        bms_software_version = 0
        mcu_lot_number = 0
        mcu_unique_die_id = 0
        mcu_wafer_information = 0
        if _type == "foxBMS_GetBmsSoftwareVersion":
            bms_software_version = 1
        elif _type == "foxBMS_GetMcuLotNumber":
            mcu_lot_number = 1
        elif _type == "foxBMS_GetMcuUniqueDieId":
            mcu_unique_die_id = 1
        elif _type == "foxBMS_GetMcuWaferInformation":
            mcu_wafer_information = 1
        elif _type == "all":
            bms_software_version = 1
            mcu_lot_number = 1
            mcu_unique_die_id = 1
            mcu_wafer_information = 1
        else:
            logging.error(f"Unsupported value {_type}")
        msg_data = {
            self.mux_name: self.mux_values["foxBMS_VersionInfo"],
            "foxBMS_GetBmsSoftwareVersion": bms_software_version,
            "foxBMS_GetMcuLotNumber": mcu_lot_number,
            "foxBMS_GetMcuUniqueDieId": mcu_unique_die_id,
            "foxBMS_GetMcuWaferInformation": mcu_wafer_information,
        }
        data = self.message.encode(msg_data, padding=True)
        logging.debug(msg_data)
        return can.Message(
            arbitration_id=self.message.frame_id, data=data, is_extended_id=False
        )

    def set_rtc_time_message(self) -> can.Message:
        "Process RTC setting message"
        now = datetime.datetime.now()
        today = datetime.datetime.today()
        msg_data = {
            self.mux_name: self.mux_values["foxBMS_Rtc"],
            "SetHundredthOfSeconds": int(now.microsecond / 10000),
            "SetSeconds": int(now.second),
            "SetMinutes": int(now.minute),
            "SetHours": int(now.hour),
            "SetWeekday": int(today.weekday()),
            "SetDay": int(today.day),
            "SetMonth": int(today.month),
            "SetYear": int(str(today.year)[-2:]),
        }
        logging.debug(msg_data)
        data = self.message.encode(msg_data, padding=True)
        return can.Message(
            arbitration_id=self.message.frame_id, data=data, is_extended_id=False
        )

    def get_rtc_time_message(self) -> can.Message:
        "Process getting the RTC message"
        msg_data = {
            self.mux_name: self.mux_values["foxBMS_TimeInfo"],
            "foxBMS_RequestRtcTime": 1,
        }
        data = self.message.encode(msg_data, padding=True)
        logging.debug(msg_data)
        return can.Message(
            arbitration_id=self.message.frame_id, data=data, is_extended_id=False
        )

    def process_debug_messages(self, the_app) -> list[can.Message]:
        """Process the specific multiplexer of the the debug message"""
        msgs = []
        if the_app.debug_message_state.set_rtc_time:
            msgs.append(self.set_rtc_time_message())
            the_app.debug_message_state.number_of_requests -= 1
            the_app.debug_message_state.set_rtc_time -= 1

        if the_app.debug_message_state.get_rtc_time:
            msgs.append(self.get_rtc_time_message())
            the_app.debug_message_state.number_of_requests -= 1
            the_app.debug_message_state.get_rtc_time -= 1

        if the_app.debug_message_state.get_software_version:
            msgs.append(self.get_version_info_message())
            the_app.debug_message_state.number_of_requests -= 1
            the_app.debug_message_state.get_software_version -= 1

        if the_app.debug_message_state.fram_initialization:
            msgs.append(self.trigger_fram_initialization())
            the_app.debug_message_state.number_of_requests -= 1
            the_app.debug_message_state.fram_initialization -= 1

        if the_app.debug_message_state.software_reset:
            msgs.append(self.trigger_software_reset())
            the_app.debug_message_state.number_of_requests -= 1
            the_app.debug_message_state.software_reset -= 1

        return msgs
