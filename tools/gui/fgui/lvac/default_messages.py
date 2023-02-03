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

"""Wrapper to retrieve several messages from the DBC file.
This should help to keep most of the 'message'-configuration out of the GUI
implementation."""


import can
from cantools.database.can import Database, Message


def _get_bms_state_request_message(dbc: Database):
    """Returns the BMS state request message"""
    return dbc.get_message_by_name("foxBMS_BmsStateRequest")


def _get_state_request_signal_mode(message: Message):
    """Returns the signal of the request mode of BMS state request message"""
    return message.get_signal_by_name("foxBMS_modeRequest")


def _get_balancing_threshold_signal(message: Message):
    """Returns the balancing threshold signal"""
    return message.get_signal_by_name("foxBMS_balancingThreshold")


def _set_bms_state_request_message(
    message: Message,
    mode_request,
    activate_balancing,
    balancing_threshold,
) -> can.Message:

    data = message.encode(
        {
            "foxBMS_modeRequest": mode_request,
            "foxBMS_activateBalancing": activate_balancing,
            "foxBMS_balancingThreshold": balancing_threshold,
            "foxBMS_externallyPrecharged": False,
            "foxBMS_resetFlags": False,
            "foxBMS_chargerConnected": False,
            "foxBMS_disableInsulationMon": False,
            "foxBMS_heaterOverride_ON": False,
            "foxBMS_heaterOverride_OFF": False,
        },
        padding=True,
    )
    return can.Message(arbitration_id=message.frame_id, data=data, is_extended_id=False)


def _get_cell_voltages_msg_id(dbc: Database):
    cell_voltage_msg = dbc.get_message_by_name("foxBMS_CellVoltages")
    return cell_voltage_msg.frame_id


def _get_cell_temperatures_msg_id(dbc: Database):
    cell_temperatures_msg = dbc.get_message_by_name("foxBMS_CellTemperatures")
    return cell_temperatures_msg.frame_id


def _get_debug_response_msg_id(dbc: Database):
    debug_response_msg = dbc.get_message_by_name("foxBMS_DebugResponse")
    return debug_response_msg.frame_id


def _get_message_type(value: can.message.Message, frame_id):
    if value.arbitration_id == frame_id:
        return True
    return False


def _get_balancing_threshold_limits(balancing_threshold_signal):
    """Returns the maximum value of the balancing signal"""
    return (balancing_threshold_signal.minimum, balancing_threshold_signal.maximum)
