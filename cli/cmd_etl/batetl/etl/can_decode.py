#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Filters CAN messages"""

import logging
from pathlib import Path

from cantools import database
from cantools.database.can.signal import Signal


class CANDecode:  # pylint: disable=too-few-public-methods
    """Class to decode CAN messages

    :param dbc: Path to the .dbc file
    :param timestamp_pos: Position of the timestamp in a CAN message
    :param id_pos: Position of the CAN id in a CAN message
    :param data_pos: Position of the data in a CAN message
    """

    def __init__(
        self, dbc: str, timestamp_pos: int, id_pos: int, data_pos: int
    ) -> None:
        try:
            self._database = database.load_file(
                Path(dbc), database_format="dbc", encoding="utf-8"
            )
        except database.UnsupportedDatabaseFormatError:
            logging.error("Invalid DBC file")
            raise SystemExit(1) from None
        self._timestamp_pos = timestamp_pos
        self._id_pos = id_pos
        self._data_pos = data_pos

    def decode_msg(self, msg: str) -> tuple[int, str] | tuple[None, None]:
        """Method to the decoded passed CAN messages.

        :param msg: CAN message as string
        :return: Either a tuple with the message name and the decoded message
            or in case of an exception an empty tuple
        """
        try:
            return self._handle_decoding(msg)
        except IndexError:
            logging.info("Index error at msg: %s", msg)
            return None, None
        except ValueError:
            logging.info(
                "CAN id or data in msg %s contain non-hexadecimal "
                "values. Wrong id-pos or data-pos ?",
                msg,
            )
            return None, None
        except KeyError:
            logging.info("CAN id in msg %s is not listed in the DBC file", msg)
            return None, None
        except database.DecodeError:
            # happens if the CAN msg is not complete
            logging.info("CAN msg %s can not be decoded", msg)
            return None, None

    def _handle_decoding(self, msg: str) -> tuple[str, str]:
        """Handles the decoding of the CAN message without exception handling

        :param msg: CAN message as string
        :return: A tuple with the message name and the decoded message
        """
        msg_parts = [x for x in msg.lstrip().split() if x]
        # convert id to hex value
        msg_id = int(msg_parts[self._id_pos], 16)
        can_message = self._database.get_message_by_frame_id(msg_id)
        data = bytes.fromhex(
            " ".join(msg_parts[self._data_pos : can_message.length + self._data_pos])
        )
        decoded_data = self._database.decode_message(msg_id, data)
        timestamp_format = f'"Timestamp": {msg_parts[self._timestamp_pos]}'
        if can_message.is_multiplexed():
            decoded_signals = list(decoded_data.keys())
            # remove multiplexer from msg signals
            signal_list = [
                signal
                for signal in can_message.signals
                if signal.name in decoded_signals and not signal.is_multiplexer
            ]
            data_format = CANDecode._data_format(msg_id, signal_list, decoded_data)
            return can_message.name, "{{{},{}}}\n".format(  # pylint: disable=consider-using-f-string
                timestamp_format,
                data_format,
            )
        # list of all non multiplexed signals
        signal_list = can_message.signals
        # pass a list with can signal objects
        data_format = CANDecode._data_format(msg_id, signal_list, decoded_data)
        return can_message.name, "{{{},{}}}\n".format(timestamp_format, data_format)  # pylint: disable=consider-using-f-string

    @staticmethod
    def _data_format(
        msg_id: int, signals: list[Signal], decoded_data: dict[str, str]
    ) -> str:
        """returns the decoded CAN data as comma separated string compatible
        to json object notation

        :param msg_id: The CAN id of the message
        :param signals: List containing all signals as cantools signal objects
        :param decoded_data: Decoded data of each signal as dictionary
        :return: Decoded data as json object {timestamp,data}
        """
        return "".join(
            [
                f'"{msg_id}_{signal.name}_{signal.unit}":"{decoded_data[signal.name]}",'
                if signal.unit is None
                else f'"{msg_id}_{signal.name}_{signal.unit}":{decoded_data[signal.name]},'
                for signal in signals
            ]
        )[:-1]  # trailing comma
